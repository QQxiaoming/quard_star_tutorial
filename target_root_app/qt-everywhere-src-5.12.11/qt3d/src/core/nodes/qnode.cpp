/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnode.h"
#include "qnode_p.h"

#include <Qt3DCore/QComponent>
#include <Qt3DCore/qaspectengine.h>
#include <Qt3DCore/qdynamicpropertyupdatedchange.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qnodedestroyedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <QtCore/QChildEvent>
#include <QtCore/QEvent>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>

#include <Qt3DCore/private/corelogging_p.h>
#include <Qt3DCore/private/qdestructionidandtypecollector_p.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <Qt3DCore/private/qnodevisitor_p.h>
#include <Qt3DCore/private/qpostman_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <QtCore/private/qmetaobject_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QNodePrivate::QNodePrivate()
    : QObjectPrivate()
    , m_changeArbiter(nullptr)
    , m_typeInfo(nullptr)
    , m_scene(nullptr)
    , m_id(QNodeId::createId())
    , m_blockNotifications(false)
    , m_hasBackendNode(false)
    , m_enabled(true)
    , m_notifiedParent(false)
    , m_defaultPropertyTrackMode(QNode::TrackFinalValues)
    , m_propertyChangesSetup(false)
    , m_signals(this)
{
}

QNodePrivate::~QNodePrivate()
{
}

void QNodePrivate::init(QNode *parent)
{
    if (!parent)
        return;

    // If we have a QNode parent that has a scene (and hence change arbiter),
    // copy these to this QNode. If valid, then also notify the backend
    // in a deferred way when the object is fully constructed. This is delayed
    // until the object is fully constructed as it involves calling a virtual
    // function of QNode.
    m_parentId = parent->id();
    const auto parentPrivate = get(parent);
    m_scene = parentPrivate->m_scene;
    Q_Q(QNode);
    if (m_scene) {
        // schedule the backend notification and scene registering -> set observers through scene
        m_scene->postConstructorInit()->addNode(q);
    }
}

/*!
 * \internal
 *
 * Sends QNodeCreatedChange events to the aspects.
 */
void QNodePrivate::notifyCreationChange()
{
    Q_Q(QNode);
    // Do nothing if we already have already sent a node creation change
    // and not a subsequent node destroyed change.
    if (m_hasBackendNode || !m_scene)
        return;
    QNodeCreatedChangeGenerator generator(q);
    const auto creationChanges = generator.creationChanges();
    for (const auto &change : creationChanges)
        notifyObservers(change);
}

/*!
 * \internal
 *
 * Notify the backend that the parent lost this node as a child and
 * that this node is being destroyed. We only send the node removed
 * change for the parent's children property iff we have an id for
 * a parent node. This is set/unset in the _q_addChild()/_q_removeChild()
 * functions (and initialized in init() if there is a parent at
 * construction time).
 *
 * Likewise, we only send the node destroyed change, iff we have
 * previously sent a node created change. This is tracked via the
 * m_hasBackendNode member.
 */
void QNodePrivate::notifyDestructionChangesAndRemoveFromScene()
{
    Q_Q(QNode);

    // We notify the backend that the parent lost us as a child
    if (m_changeArbiter != nullptr && !m_parentId.isNull()) {
        const auto change = QPropertyNodeRemovedChangePtr::create(m_parentId, q);
        change->setPropertyName("children");
        notifyObservers(change);
    }

    // Tell the backend we are about to be destroyed
    if (m_hasBackendNode) {
        const QDestructionIdAndTypeCollector collector(q);
        const auto destroyedChange = QNodeDestroyedChangePtr::create(q, collector.subtreeIdsAndTypes());
        notifyObservers(destroyedChange);
    }

    // We unset the scene from the node as its backend node was/is about to be destroyed
    QNodeVisitor visitor;
    visitor.traverse(q, this, &QNodePrivate::unsetSceneHelper);
}

/*!
 * \internal
 *
 * Sends a QNodeCreatedChange event to the aspects and then also notifies the
 * parent backend node of its new child. This is called in a deferred manner
 * by NodePostConstructorInit::processNodes to notify the backend of newly created
 * nodes with a parent that is already part of the scene.
 *
 * Also notify the scene of this node, so it may set it's change arbiter.
 */
void QNodePrivate::_q_postConstructorInit()
{
    Q_Q(QNode);

    // If we've already done the work then bail out. This can happen if the
    // user creates a QNode subclass with an explicit parent, then immediately
    // sets the new QNode as a property on another node. In this case, the
    // property setter will call this function directly, but as we can't
    // un-schedule a deferred invocation, this function will be called again
    // the next time the event loop spins. So, catch this case and abort.
    if (m_hasBackendNode)
        return;

    // Check that the parent hasn't been unset since this call was enqueued
    auto parentNode = q->parentNode();
    if (!parentNode)
        return;

    // Set the scene on this node and all children it references so that all
    // children have a scene set since notifyCreationChanges will set
    // m_hasBackendNode to true for all children, which would prevent them from
    // ever having their scene set
    if (m_scene) {
        QNodeVisitor visitor;
        visitor.traverse(q, parentNode->d_func(), &QNodePrivate::setSceneHelper);
    }

    // Let the backend know we have been added to the scene
    notifyCreationChange();

    // Let the backend parent know that they have a new child
    Q_ASSERT(parentNode);
    QNodePrivate::get(parentNode)->_q_addChild(q);
}

/*!
 * \internal
 *
 * Called by _q_setParentHelper() or _q_postConstructorInit()
 * on the main thread.
 */
void QNodePrivate::_q_addChild(QNode *childNode)
{
    Q_ASSERT(childNode);
    Q_ASSERT_X(childNode->parent() == q_func(), Q_FUNC_INFO,  "not a child of this node");

    // Have we already notified the parent about its new child? If so, bail out
    // early so that we do not send more than one new child event to the backend
    QNodePrivate *childD = QNodePrivate::get(childNode);
    if (childD->m_notifiedParent == true)
        return;

    // Store our id as the parentId in the child so that even if the child gets
    // removed from the scene as part of the destruction of the parent, when the
    // parent's children are deleted in the QObject dtor, we still have access to
    // the parentId. If we didn't store this, we wouldn't have access at that time
    // because the parent would then only be a QObject, the QNode part would have
    // been destroyed already.
    childD->m_parentId = m_id;

    if (!m_scene)
        return;

    // We need to send a QPropertyNodeAddedChange to the backend
    // to notify the backend that we have a new child
    if (m_changeArbiter != nullptr) {
        // Flag that we have notified the parent. We do this immediately before
        // creating the change because that recurses back into this function and
        // we need to catch that to avoid sending more than one new child event
        // to the backend.
        childD->m_notifiedParent = true;
        const auto change = QPropertyNodeAddedChangePtr::create(m_id, childNode);
        change->setPropertyName("children");
        notifyObservers(change);
    }

    // Update the scene
    // TODO: Fold this into the QNodeCreatedChangeGenerator so we don't have to
    // traverse the sub tree three times!
    QNodeVisitor visitor;
    visitor.traverse(childNode, this, &QNodePrivate::addEntityComponentToScene);
}

/*!
 * \internal
 *
 * Called by _q_setParentHelper on the main thread.
 */
void QNodePrivate::_q_removeChild(QNode *childNode)
{
    Q_ASSERT(childNode);
    Q_ASSERT_X(childNode->parent() == q_func(), Q_FUNC_INFO, "not a child of this node");

    QNodePrivate::get(childNode)->m_parentId = QNodeId();

    // We notify the backend that we lost a child
    if (m_changeArbiter != nullptr) {
        const auto change = QPropertyNodeRemovedChangePtr::create(m_id, childNode);
        change->setPropertyName("children");
        notifyObservers(change);
    }
}

/*!
 * \internal
 *
 * Reparents the public QNode to \a parent. If the new parent is nullptr then this
 * QNode is no longer part of the scene and so we notify the backend of its removal
 * from its parent's list of children, and then send a QNodeDestroyedChange to the
 * aspects so that the corresponding backend node is destroyed.
 *
 * If \a parent is not null, then we must tell its new parent about this QNode now
 * being a child of it on the backend. If this QNode did not have a parent upon
 * entry to this function, then we must first send a QNodeCreatedChange to the backend
 * prior to sending the QPropertyNodeAddedChange to its parent.
 *
 * Note: This function should never be called from the ctor directly as the type may
 * not be fully created yet and creating creation changes involves calling a virtual
 * function on QNode. The function _q_notifyCreationAndChildChanges() is used
 * for sending initial notification when a parent is passed to the QNode ctor.
 * That function does a subset of this function with the assumption that the new object
 * had no parent before (must be true as it is newly constructed).
 */
void QNodePrivate::_q_setParentHelper(QNode *parent)
{
    Q_Q(QNode);
    QNode *oldParentNode = q->parentNode();

    // If we had a parent, we let him know that we are about to change
    // parent
    if (oldParentNode && m_hasBackendNode) {
        QNodePrivate::get(oldParentNode)->_q_removeChild(q);

        // If we have an old parent but the new parent is null or if the new
        // parent hasn't yet been added to the backend the backend node needs
        // to be destroyed
        // e.g:
        // QEntity *child = new QEntity(some_parent);
        // After some time, in a later event loop
        // QEntity *newSubTreeRoot = new QEntity(someGlobalExisitingRoot)
        // child->setParent(newSubTreeRoot)
        if (!parent || !QNodePrivate::get(parent)->m_hasBackendNode)
            notifyDestructionChangesAndRemoveFromScene();
    }

    // Flag that we need to notify any new parent
    m_notifiedParent = false;

    // Basically QObject::setParent but for QObjectPrivate
    QObjectPrivate::setParent_helper(parent);

    if (parent) {
        // If we had no parent but are about to set one,
        // we need to send a QNodeCreatedChange
        QNodePrivate *newParentPrivate = QNodePrivate::get(parent);

        // Set the scene helper / arbiter
        if (newParentPrivate->m_scene) {
            QNodeVisitor visitor;
            visitor.traverse(q, parent->d_func(), &QNodePrivate::setSceneHelper);
        }

        // We want to make sure that subTreeRoot is always created before
        // child.
        // Given a case such as below
        // QEntity *subTreeRoot = new QEntity(someGlobalExisitingRoot)
        // QEntity *child = new QEntity();
        // child->setParent(subTreeRoot)
        // We need to take into account that subTreeRoot needs to be
        // created in the backend before the child.
        // Therefore we only call notifyCreationChanges if the parent
        // hasn't been created yet as we know that when the parent will be
        // fully created, it will also send the changes for all of its
        // children

        if (newParentPrivate->m_hasBackendNode)
            notifyCreationChange();

        // If we have a valid new parent, we let him know that we are its child
        QNodePrivate::get(parent)->_q_addChild(q);
    }
}

void QNodePrivate::registerNotifiedProperties()
{
    Q_Q(QNode);
    if (m_propertyChangesSetup)
        return;

    const int offset = QNode::staticMetaObject.propertyOffset();
    const int count = q->metaObject()->propertyCount();

    for (int index = offset; index < count; index++)
        m_signals.connectToPropertyChange(q, index);

    m_propertyChangesSetup = true;
}

void QNodePrivate::unregisterNotifiedProperties()
{
    Q_Q(QNode);
    if (!m_propertyChangesSetup)
        return;

    const int offset = QNode::staticMetaObject.propertyOffset();
    const int count = q->metaObject()->propertyCount();

    for (int index = offset; index < count; index++)
        m_signals.disconnectFromPropertyChange(q, index);

    m_propertyChangesSetup = false;
}

void QNodePrivate::propertyChanged(int propertyIndex)
{
    // Bail out early if we can to avoid the cost below
    if (m_blockNotifications)
        return;

    const auto toBackendValue = [](const QVariant &data) -> QVariant
    {
        if (data.canConvert<QNode*>()) {
            QNode *node = data.value<QNode*>();

            // Ensure the node and all ancestors have issued their node creation changes.
            // We can end up here if a newly created node with a parent is immediately set
            // as a property on another node. In this case the deferred call to
            // _q_postConstructorInit() will not have happened yet as the event
            // loop will still be blocked. We need to do this for all ancestors,
            // since the subtree of this node otherwise can end up on the backend
            // with a reference to a non-existent parent.
            if (node)
                QNodePrivate::get(node)->_q_ensureBackendNodeCreated();

            const QNodeId id = node ? node->id() : QNodeId();
            return QVariant::fromValue(id);
        }

        return data;
    };

    Q_Q(QNode);

    const QMetaProperty property = q->metaObject()->property(propertyIndex);

    const QVariant data = property.read(q);

    if (data.type() == QVariant::List) {
        QSequentialIterable iterable = data.value<QSequentialIterable>();
        QVariantList variants;
        variants.reserve(iterable.size());
        for (const auto &v : iterable)
            variants.append(toBackendValue(v));
        notifyPropertyChange(property.name(), variants);
    } else {
        notifyPropertyChange(property.name(), toBackendValue(data));
    }
}

/*!
    \internal
    Recursively sets and adds the nodes in the subtree of base node \a root to the scene.
    Also takes care of connecting Components and Entities together in the scene.
 */
void QNodePrivate::setSceneHelper(QNode *root)
{
    // Sets the scene
    root->d_func()->setScene(m_scene);
    // addObservable sets the QChangeArbiter
    m_scene->addObservable(root);

    // We also need to handle QEntity <-> QComponent relationships
    if (QComponent *c = qobject_cast<QComponent *>(root)) {
        const QVector<QEntity *> entities = c->entities();
        for (QEntity *entity : entities) {
            if (!m_scene->hasEntityForComponent(c->id(), entity->id())) {
                if (!c->isShareable() && !m_scene->entitiesForComponent(c->id()).isEmpty())
                    qWarning() << "Trying to assign a non shareable component to more than one Entity";
                m_scene->addEntityForComponent(c->id(), entity->id());
            }
        }
    }
}

/*!
    \internal

    Recursively unsets and remove nodes in the subtree of base node \a root from
    the scene. Also takes care of removing Components and Entities connections.
 */
void QNodePrivate::unsetSceneHelper(QNode *node)
{
    QNodePrivate *nodePrivate = QNodePrivate::get(node);

    // We also need to handle QEntity <-> QComponent relationships removal
    if (QComponent *c = qobject_cast<QComponent *>(node)) {
        const QVector<QEntity *> entities = c->entities();
        for (QEntity *entity : entities) {
            if (nodePrivate->m_scene)
                nodePrivate->m_scene->removeEntityForComponent(c->id(), entity->id());
        }
    }

    if (nodePrivate->m_scene != nullptr)
        nodePrivate->m_scene->removeObservable(node);
    nodePrivate->setScene(nullptr);
}

/*!
    \internal
 */
void QNodePrivate::addEntityComponentToScene(QNode *root)
{
    if (QEntity *e = qobject_cast<QEntity *>(root)) {
        const auto components = e->components();
        for (QComponent *c : components) {
            if (!m_scene->hasEntityForComponent(c->id(), e->id()))
                m_scene->addEntityForComponent(c->id(), e->id());
        }
    }
}

/*!
    \internal
 */
// Called in the main thread by QScene -> following QEvent::childAdded / addChild
void QNodePrivate::setArbiter(QLockableObserverInterface *arbiter)
{
    if (m_changeArbiter && m_changeArbiter != arbiter)
        unregisterNotifiedProperties();
    m_changeArbiter = static_cast<QAbstractArbiter *>(arbiter);
    if (m_changeArbiter)
        registerNotifiedProperties();
}

/*!
 * \internal
 * Makes sure this node has a backend by traversing the tree up to the most distant ancestor
 * without a backend node and initializing that node. This is done to make sure the parent nodes
 * are always created before the child nodes, since child nodes reference parent nodes at creation
 * time.
 */
void QNodePrivate::_q_ensureBackendNodeCreated()
{
    if (m_hasBackendNode)
        return;

    Q_Q(QNode);

    QNode *nextNode = q;
    QNode *topNodeWithoutBackend = nullptr;
    while (nextNode != nullptr && !QNodePrivate::get(nextNode)->m_hasBackendNode) {
        topNodeWithoutBackend = nextNode;
        nextNode = nextNode->parentNode();
    }
    QNodePrivate::get(topNodeWithoutBackend)->_q_postConstructorInit();
}

/*!
    \class Qt3DCore::QNode
    \inherits QObject

    \inmodule Qt3DCore
    \since 5.5

    \brief QNode is the base class of all Qt3D node classes used to build a
    Qt3D scene.

    The owernship of QNode is determined by the QObject parent/child
    relationship between nodes. By itself, a QNode has no visual appearance
    and no particular meaning, it is there as a way of building a node based tree
    structure.

    The parent of a QNode instance can only be another QNode instance.

    Each QNode instance has a unique id that allows it to be recognizable
    from other instances.

    When properties are defined on a QNode subclass, their NOTIFY signal
    will automatically generate notifications that the Qt3D backend aspects will
    receive.

    \sa QEntity, QComponent
*/

/*!
 * Sends the \a change QSceneChangePtr to any QBackendNodes in the registered
 * aspects that correspond to this QNode.
 *
 * For the common case of a QObject property change, QNode handles this for you
 * automatically by sending a QPropertyUpdatedChange event to the backend nodes.
 * You only need to call this function if you wish to send a specific type of
 * change in place of the automatic handling.
 */
void QNode::notifyObservers(const QSceneChangePtr &change)
{
    Q_D(QNode);
    d->notifyObservers(change);
}

/*!
    Called when one or more backend aspects sends a notification \a change to the
    current Qt3DCore::QNode instance.

    \note This method should be reimplemented in your subclasses to properly
    handle the \a change.
*/
void QNode::sceneChangeEvent(const QSceneChangePtr &change)
{
    Q_UNUSED(change);
    if (change->type() == Qt3DCore::PropertyUpdated) {
        // TODO: Do this more efficiently. We could pass the metaobject and property
        //       index to the animation aspect via the QChannelMapping. This would
        //       allow us to avoid the propertyIndex lookup here by sending them in
        //       a new subclass of QPropertyUpdateChange.
        // Try to find property and call setter
        auto e = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
        const QMetaObject *mo = metaObject();
        const int propertyIndex = mo->indexOfProperty(e->propertyName());
        QMetaProperty mp = mo->property(propertyIndex);
        bool wasBlocked = blockNotifications(true);
        mp.write(this, e->value());
        blockNotifications(wasBlocked);
    } else {
        // Nothing is handling this change, warn the user.
        qWarning() << Q_FUNC_INFO << "sceneChangeEvent should have been subclassed";
    }
}

/*!
    \internal
 */
void QNodePrivate::setScene(QScene *scene)
{
    if (m_scene != scene) {
        if (m_scene != nullptr)
            m_scene->removePropertyTrackDataForNode(m_id);
        m_scene = scene;
        // set PropertyTrackData in the scene
        updatePropertyTrackMode();
    }
}

/*!
    \internal
 */
QScene *QNodePrivate::scene() const
{
    return m_scene;
}

/*!
    \internal
 */
void QNodePrivate::notifyPropertyChange(const char *name, const QVariant &value)
{
    // Bail out early if we can to avoid operator new
    if (m_blockNotifications)
        return;

    auto e = QPropertyUpdatedChangePtr::create(m_id);
    e->setPropertyName(name);
    e->setValue(value);
    notifyObservers(e);
}

void QNodePrivate::notifyDynamicPropertyChange(const QByteArray &name, const QVariant &value)
{
    // Bail out early if we can to avoid operator new
    if (m_blockNotifications)
        return;

    auto e = QDynamicPropertyUpdatedChangePtr::create(m_id);
    e->setPropertyName(name);
    e->setValue(value);
    notifyObservers(e);
}

/*!
    \internal
 */
// Called by the main thread
void QNodePrivate::notifyObservers(const QSceneChangePtr &change)
{
    Q_ASSERT(change);

    // Don't send notifications if we are blocking
    if (m_blockNotifications && change->type() == PropertyUpdated)
        return;

    if (m_changeArbiter != nullptr) {
        QAbstractPostman *postman = m_changeArbiter->postman();
        if (postman != nullptr)
            postman->notifyBackend(change);
    }
}

// Inserts this tree into the main Scene tree.
// Needed when SceneLoaders provide a cloned tree from the backend
// and need to insert it in the main scene tree
// QNode *root;
// QNode *subtree;
// QNodePrivate::get(root)->insertTree(subtree);

/*!
    \internal
 */
void QNodePrivate::insertTree(QNode *treeRoot, int depth)
{
    if (m_scene != nullptr) {
        treeRoot->d_func()->setScene(m_scene);
        m_scene->addObservable(treeRoot);
    }

    for (QObject *c : treeRoot->children()) {
        QNode *n = nullptr;
        if ((n = qobject_cast<QNode *>(c)) != nullptr)
            insertTree(n, depth + 1);
    }

    if (depth == 0)
        treeRoot->setParent(q_func());
}

void QNodePrivate::updatePropertyTrackMode()
{
    if (m_scene != nullptr) {
        QScene::NodePropertyTrackData trackData;
        trackData.defaultTrackMode = m_defaultPropertyTrackMode;
        trackData.trackedPropertiesOverrides = m_trackedPropertiesOverrides;
        m_scene->setPropertyTrackDataForNode(m_id, trackData);
    }
}

/*!
    \internal
 */
QNodePrivate *QNodePrivate::get(QNode *q)
{
    return q->d_func();
}

/*!
    \internal
 */
void QNodePrivate::nodePtrDeleter(QNode *q)
{
    QObject *p = q->parent();
    if (p == nullptr)
        p = q;
    p->deleteLater();
}

/*!
    \fn Qt3DCore::QNodeId Qt3DCore::qIdForNode(Qt3DCore::QNode *node)
    \relates Qt3DCore::QNode
    \return node id for \a node.
*/

/*!
    \fn template<typename T> Qt3DCore::QNodeIdVector Qt3DCore::qIdsForNodes(const T &nodes)
    \relates Qt3DCore::QNode
    \return vector of node ids for \a nodes.
*/

/*!
    \fn void Qt3DCore::QNodeCommand::setReplyToCommandId(CommandId id)

    Sets the command \a id to which the message is a reply.

*/
/*!
    \fn  Qt3DCore::QNode::PropertyTrackingMode Qt3DCore::QNode::defaultPropertyTrackingMode() const

    Returns the default property tracking mode which determines whether a
    QNode should be listening for property updates.

*/
/*!
    \fn Qt3DCore::QNode::clearPropertyTracking(const QString &propertyName)

    Clears the tracking property called \a propertyName.
*/
/*!
    \fn Qt3DCore::QNode::PropertyTrackingMode Qt3DCore::QNode::propertyTracking(const QString &propertyName) const

    Returns the tracking mode of \a propertyName.
*/

/*!
    \fn Qt3DCore::QNode::setPropertyTracking(const QString &propertyName, Qt3DCore::QNode::PropertyTrackingMode trackMode)

    Sets the property tracking for \a propertyName and \a trackMode.
*/

/*!
     Creates a new QNode instance with parent \a parent.

     \note The backend aspects will be notified that a QNode instance is
     part of the scene only if it has a parent; unless this is the root node of
     the Qt3D scene.

     \sa setParent()
*/
QNode::QNode(QNode *parent)
    : QNode(*new QNodePrivate, parent) {}

/*! \internal */
QNode::QNode(QNodePrivate &dd, QNode *parent)
    : QObject(dd, parent)
{
    Q_D(QNode);
    d->init(parent);
}

/*!
    \fn Qt3DCore::QNode::nodeDestroyed()
    Emitted when the node is destroyed.
*/

/*! \internal */
QNode::~QNode()
{
    Q_D(QNode);
    // Disconnect each connection that was stored
    for (const auto &nodeConnectionPair : qAsConst(d->m_destructionConnections))
        QObject::disconnect(nodeConnectionPair.second);
    d->m_destructionConnections.clear();
    Q_EMIT nodeDestroyed();

    // Notify the backend that the parent lost this node as a child and
    // that this node is being destroyed.
    d->notifyDestructionChangesAndRemoveFromScene();
}

/*!
    Returns the id that uniquely identifies the QNode instance.
*/
QNodeId QNode::id() const
{
    Q_D(const QNode);
    return d->m_id;
}

/*!
    \property Qt3DCore::QNode::parent

    Holds the immediate QNode parent, or null if the node has no parent.

    Setting the parent will notify the backend aspects about current QNode
    instance's parent change.

    \note if \a parent happens to be null, this will actually notify that the
    current QNode instance was removed from the scene.
*/
QNode *QNode::parentNode() const
{
    return qobject_cast<QNode*>(parent());
}

/*!
    Returns \c true if aspect notifications are blocked; otherwise returns \c false.
    By default, notifications are \e not blocked.

    \sa blockNotifications()
*/
bool QNode::notificationsBlocked() const
{
    Q_D(const QNode);
    return d->m_blockNotifications;
}

/*!
    If \a block is \c true, property change notifications sent by this object
    to aspects are blocked. If \a block is \c false, no such blocking will occur.

    The return value is the previous value of notificationsBlocked().

    Note that the other notification types will be sent even if the
    notifications for this object have been blocked.

    \sa notificationsBlocked()
*/
bool QNode::blockNotifications(bool block)
{
    Q_D(QNode);
    bool previous = d->m_blockNotifications;
    d->m_blockNotifications = block;
    return previous;
}

// Note: should never be called from the ctor directly as the type may not be fully
// created yet
void QNode::setParent(QNode *parent)
{
    Q_D(QNode);

    // If we already have a parent don't do anything. Be careful to ensure
    // that QNode knows about the parent, not just QObject (by checking the ids)
    if (parentNode() == parent &&
            ((parent != nullptr && d->m_parentId == parentNode()->id()) || parent == nullptr))
        return;

    // remove ourself from postConstructorInit queue. The call to _q_setParentHelper
    // will take care of creating the backend node if necessary depending on new parent.
    if (d->m_scene)
        d->m_scene->postConstructorInit()->removeNode(this);

    d->_q_setParentHelper(parent);

    // Block notifications as we want to let the _q_setParentHelper
    // manually handle them
    const bool blocked = blockNotifications(true);
    emit parentChanged(parent);
    blockNotifications(blocked);
}

/*!
    \typedef Qt3DCore::QNodePtr
    \relates Qt3DCore::QNode

    A shared pointer for QNode.
*/
/*!
    \typedef Qt3DCore::QNodeVector
    \relates Qt3DCore::QNode

    List of QNode pointers.
*/

/*!
 * Returns a list filled with the QNode children of the current
 * QNode instance.
 */
QNodeVector QNode::childNodes() const
{
    QNodeVector nodeChildrenList;
    const QObjectList &objectChildrenList = QObject::children();
    nodeChildrenList.reserve(objectChildrenList.size());

    for (QObject *c : objectChildrenList) {
        if (QNode *n = qobject_cast<QNode *>(c))
            nodeChildrenList.push_back(n);
    }

    return nodeChildrenList;
}
void QNode::setEnabled(bool isEnabled)
{
    Q_D(QNode);

    if (d->m_enabled == isEnabled)
        return;

    d->m_enabled = isEnabled;
    emit enabledChanged(isEnabled);
}

void QNode::setDefaultPropertyTrackingMode(QNode::PropertyTrackingMode mode)
{
    Q_D(QNode);
    if (d->m_defaultPropertyTrackMode == mode)
        return;

    d->m_defaultPropertyTrackMode = mode;
    // The backend doesn't care about such notification
    const bool blocked = blockNotifications(true);
    emit defaultPropertyTrackingModeChanged(mode);
    blockNotifications(blocked);
    d->updatePropertyTrackMode();
}

/*!
    \property Qt3DCore::QNode::enabled

    Holds the QNode enabled flag.
    By default a QNode is always enabled.

    \note the interpretation of what enabled means is aspect-dependent. Even if
    enabled is set to \c false, some aspects may still consider the node in
    some manner. This is documented on a class by class basis.
*/
bool QNode::isEnabled() const
{
    Q_D(const QNode);
    return d->m_enabled;
}

/*!
    \property Qt3DCore::QNode::defaultPropertyTrackingMode

    Holds the default property tracking mode which determines whether a QNode should
    be listening for property updates. This only applies to properties which
    haven't been overridden by a call to setPropertyTracking.

    By default it is set to QNode::TrackFinalValues
*/
QNode::PropertyTrackingMode QNode::defaultPropertyTrackingMode() const
{
    Q_D(const QNode);
    return d->m_defaultPropertyTrackMode;
}

void QNode::setPropertyTracking(const QString &propertyName, QNode::PropertyTrackingMode trackMode)
{
    Q_D(QNode);
    d->m_trackedPropertiesOverrides.insert(propertyName, trackMode);
    d->updatePropertyTrackMode();
}

QNode::PropertyTrackingMode QNode::propertyTracking(const QString &propertyName) const
{
    Q_D(const QNode);
    return d->m_trackedPropertiesOverrides.value(propertyName, d->m_defaultPropertyTrackMode);
}

void QNode::clearPropertyTracking(const QString &propertyName)
{
    Q_D(QNode);
    d->m_trackedPropertiesOverrides.remove(propertyName);
    d->updatePropertyTrackMode();
}

void QNode::clearPropertyTrackings()
{
    Q_D(QNode);
    d->m_trackedPropertiesOverrides.clear();
    d->updatePropertyTrackMode();
}

QNodeCreatedChangeBasePtr QNode::createNodeCreationChange() const
{
    // Uncomment this when implementing new frontend and backend types.
    // Any classes that don't override this function will be noticeable here.
    // Note that some classes actually don't need to override as they have
    // no additional data to send. In those cases this default implementation
    // is perfectly fine.
    // const QMetaObject *mo = metaObject();
    // qDebug() << Q_FUNC_INFO << mo->className();
    return QNodeCreatedChangeBasePtr::create(this);
}

/*!
   \fn Qt3DCore::QNodeCommand::CommandId Qt3DCore::QNodeCommand::inReplyTo() const

   Returns the id of the original QNodeCommand message that
   was sent to the backend.

*/
/*!
    \fn void Qt3DCore::QNodeCommand::setData(const QVariant &data)

    Sets the data (\a data) in the backend node to perform
    the operations requested.
*/
/*!
    \fn void Qt3DCore::QNodeCommand::setName(const QString &name)


    Sets the data (\a name) in the backend node to perform
    the operations requested.
*/

/*!
    \enum Qt3DCore::QNode::PropertyTrackingMode

    Indicates how a QNode listens for property updates.

    \value TrackFinalValues
           Tracks final values
    \value DontTrackValues
           Does not track values
    \value TrackAllValues
           Tracks all values
*/
/*!
    \fn Qt3DCore::QNode::clearPropertyTrackings()

    Erases all values that have been saved by the property tracking.
*/
/*!
 * \brief Sends a command message to the backend node
 *
 * Creates a QNodeCommand message and dispatches it to the backend node. The
 * command is given and a \a name and some \a data which can be used in the
 * backend node to perform various operations.
 * This returns a CommandId which can be used to identify the initial command
 * when receiving a message in reply. If the command message is to be sent in
 * reply to another command, \a replyTo contains the id of that command.
 *
 * \sa QNodeCommand, QNode::sendReply
 */
QNodeCommand::CommandId QNode::sendCommand(const QString &name,
                                           const QVariant &data,
                                           QNodeCommand::CommandId replyTo)
{
    Q_D(QNode);

    // Bail out early, if we can, to avoid operator new
    if (d->m_blockNotifications)
        return QNodeCommand::CommandId(0);

    auto e = QNodeCommandPtr::create(d->m_id);
    e->setName(name);
    e->setData(data);
    e->setReplyToCommandId(replyTo);
    d->notifyObservers(e);
    return e->commandId();
}

/*!
 * \brief Send a \a command back to the backend node.
 *
 * Assumes the command is to be to sent back in reply to itself to the backend node.
 *
 * \sa QNodeCommand, QNode::sendCommand
 */
void QNode::sendReply(const QNodeCommandPtr &command)
{
    Q_D(QNode);
    command->setDeliveryFlags(QSceneChange::BackendNodes);
    d->notifyObservers(command);
}


namespace {

/*! \internal */
inline const QMetaObjectPrivate *priv(const uint* data)
{
    return reinterpret_cast<const QMetaObjectPrivate*>(data);
}

/*! \internal */
inline bool isDynamicMetaObject(const QMetaObject *mo)
{
    return (priv(mo->d.data)->flags & DynamicMetaObject);
}

} // anonymous

/*!
 * \internal
 *
 * Find the most derived metaobject that doesn't have a dynamic
 * metaobject farther up the chain.
 * TODO: Add support to QMetaObject to explicitly say if it's a dynamic
 * or static metaobject so we don't need this logic
 */
const QMetaObject *QNodePrivate::findStaticMetaObject(const QMetaObject *metaObject)
{
    const QMetaObject *lastStaticMetaobject = nullptr;
    auto mo = metaObject;
    while (mo) {
        const bool dynamicMetaObject = isDynamicMetaObject(mo);
        if (dynamicMetaObject)
            lastStaticMetaobject = nullptr;

        if (!dynamicMetaObject && !lastStaticMetaobject)
            lastStaticMetaobject = mo;

        mo = mo->superClass();
    }
    Q_ASSERT(lastStaticMetaobject);
    return lastStaticMetaobject;
}

/*!
 * \internal
 *
 * NodePostConstructorInit handles calling QNode::_q_postConstructorInit for
 * all nodes. By keeping track of nodes that need initialization we can
 * create them all together ensuring they get sent to the backend in a single
 * batch.
 */
NodePostConstructorInit::NodePostConstructorInit(QObject *parent)
    : QObject(parent)
    , m_requestedProcessing(false)
{
}

NodePostConstructorInit::~NodePostConstructorInit() {}

/*!
 * \internal
 *
 * Add a node to the list of nodes needing a call to _q_postConstructorInit
 * We only add the node if it does not have an ancestor already in the queue
 * because initializing the ancestor will initialize all it's children.
 * This ensures that all backend nodes are created from the top-down, with
 * all parents created before their children
 *
 */
void NodePostConstructorInit::addNode(QNode *node)
{
    Q_ASSERT(node);
    QNode *nextNode = node;
    while (nextNode != nullptr && !m_nodesToConstruct.contains(QNodePrivate::get(nextNode)))
        nextNode = nextNode->parentNode();

    if (!nextNode) {
        m_nodesToConstruct.append(QNodePrivate::get(node));
        if (!m_requestedProcessing){
            QMetaObject::invokeMethod(this, "processNodes", Qt::QueuedConnection);
            m_requestedProcessing = true;
        }
    }
}

/*!
 * \internal
 *
 * Remove a node from the queue. This will ensure none of its
 * children get initialized
 */
void NodePostConstructorInit::removeNode(QNode *node)
{
    Q_ASSERT(node);
    m_nodesToConstruct.removeAll(QNodePrivate::get(node));
}

/*!
 * \internal
 *
 * call _q_postConstructorInit for all nodes in the queue
 * and clear the queue
 */
void NodePostConstructorInit::processNodes()
{
    m_requestedProcessing = false;
    while (!m_nodesToConstruct.empty()) {
        auto node = m_nodesToConstruct.takeFirst();
        node->_q_postConstructorInit();
    }
}

} // namespace Qt3DCore

QT_END_NAMESPACE

#include "moc_qnode.cpp"
