/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmapobjectview_p.h"
#include "qmapobjectview_p_p.h"
#include <QtQml/private/qqmldelegatemodel_p.h>
#include <QtLocation/private/qgeomap_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapObjectView
    \instantiates QMapObjectView
    \inqmlmodule Qt.labs.location
    \ingroup qml-QtLocation5-maps
    \inherits QGeoMapObject

    \brief The MapObjectView is used to populate Map with map objects from a model.

    The MapObjectView is used to populate Map with map objects, either from a model or via
    \l addMapObject or \l removeMapObject.

    The MapObjectView type only makes sense when contained in a Map, meaning that it will not work when added inside
    other QML elements.
    This can also be intended as an object layer on top of a Map.
*/

/*

    QMapObjectViewPrivate

*/

static const QQmlIncubator::IncubationMode incubationMode = QQmlIncubator::Asynchronous;

QMapObjectViewPrivate::QMapObjectViewPrivate(QGeoMapObject *q)
    : QGeoMapObjectPrivate(q)
{
}

QMapObjectViewPrivate::~QMapObjectViewPrivate()
{

}

QGeoMapObject::Type QMapObjectViewPrivate::type() const
{
    return QGeoMapObject::ViewType;
}


/*

    QMapObjectViewPrivateDefault

*/


QMapObjectViewPrivateDefault::QMapObjectViewPrivateDefault(const QMapObjectViewPrivate &other) : QMapObjectViewPrivate(other.q)
{
}

QMapObjectViewPrivateDefault::~QMapObjectViewPrivateDefault()
{

}

QMapObjectViewPrivateDefault::QMapObjectViewPrivateDefault(QGeoMapObject *q) : QMapObjectViewPrivate(q)
{

}

QGeoMapObjectPrivate *QMapObjectViewPrivateDefault::clone()
{
    return new QMapObjectViewPrivateDefault(*this);
}

/*

    QMapObjectView

*/


QMapObjectView::QMapObjectView(QObject *parent)
    : QGeoMapObject(QExplicitlySharedDataPointer<QGeoMapObjectPrivate>(new QMapObjectViewPrivateDefault(this)), parent)
{

}

QMapObjectView::~QMapObjectView()
{
    flushDelegateModel();
    flushUserAddedMapObjects();
}

QList<QGeoMapObject *> QMapObjectView::geoMapObjectChildren() const
{
    auto kids = QGeoMapObject::geoMapObjectChildren();
    auto size = m_instantiatedMapObjects.count();
    for (int i = 0; i < size; ++i) {
        auto obj = qobject_cast<QGeoMapObject*>(m_instantiatedMapObjects[i]);
        if (obj)
            kids << obj;
    }
    for (int i = 0; i < m_userAddedMapObjects.size(); ++i) {
        auto obj = m_userAddedMapObjects.at(i);
        if (obj)
            kids << obj;
    }
    return kids;
}

void QMapObjectView::classBegin()
{
    QQmlContext *ctx = qmlContext(this);
    m_delegateModel = new QQmlDelegateModel(ctx, this);
    m_delegateModel->classBegin();

    QQmlInstanceModel *model = m_delegateModel;
    connect(model, &QQmlInstanceModel::modelUpdated, this, &QMapObjectView::modelUpdated);
    connect(model, &QQmlInstanceModel::createdItem, this, &QMapObjectView::createdItem);
//    connect(model, &QQmlInstanceModel::destroyingItem, this, &QMapObjectView::destroyingItem);
//    connect(model, &QQmlInstanceModel::initItem, this, &QMapObjectView::initItem);
}

void QMapObjectView::componentComplete()
{
    QGeoMapObject::componentComplete();
    if (m_delegate)
        m_delegateModel->setDelegate(m_delegate);
    if (m_model.isValid())
        m_delegateModel->setModel(m_model);
    m_delegateModel->componentComplete();
}

/*!
    \qmlproperty Variant Qt.labs.location::MapObjectView::model

    This property holds the model that provides data used for creating the map items defined by the
    delegate. Only QAbstractItemModel based models are supported.
*/
QVariant QMapObjectView::model() const
{
    return m_model;
}

/*!
    \qmlproperty Component Qt.labs.location::MapObjectView::delegate

    This property holds the delegate which defines how each item in the
    model should be displayed. The Component must contain exactly one
    QGeoMapObject -derived object as the root object.
*/
QQmlComponent *QMapObjectView::delegate() const
{
    return m_delegate;
}

void QMapObjectView::setModel(QVariant model)
{
    if (m_model == model)
        return;
    m_model = model;

    if (d_ptr->m_componentCompleted)
        m_delegateModel->setModel(model);

    emit modelChanged(model);
}

void QMapObjectView::setDelegate(QQmlComponent *delegate)
{
    if (m_delegate == delegate)
        return;
    m_delegate = delegate;

    if (d_ptr->m_componentCompleted)
        m_delegateModel->setDelegate(delegate);

    emit delegateChanged(delegate);
}

/*!
    \qmlmethod void Qt.labs.location::MapObjectView::addMapObject(MapObject object)

    Adds the given \a object to the MapObjectView (for example MapIconObject, MapRouteObject), and,
    indirectly, to the underlying map. If the object already is on the MapObjectView, it will not be added again.

    \sa removeMapObject
*/
void QMapObjectView::addMapObject(QGeoMapObject *object)
{
    if (m_userAddedMapObjects.indexOf(object) < 0)
        m_userAddedMapObjects.append(object);
    if (map() && object->map() != map())
        object->setMap(map());
}

/*!
    \qmlmethod void Qt.labs.location::MapObjectView::removeMapObject(MapObject object)

    Removes the given \a object from the MapObjectView (for example MapIconObject, MapRouteObject), and,
    indirectly, from the underlying map.

    \sa addMapObject
*/
void QMapObjectView::removeMapObject(QGeoMapObject *object)
{
    int idx = m_userAddedMapObjects.indexOf(object);
    if ( idx >= 0) {
        object->setMap(nullptr);
        m_userAddedMapObjects.remove(idx);
    }
}

void QMapObjectView::destroyingItem(QObject * /*object*/)
{

}

void QMapObjectView::initItem(int /*index*/, QObject * /*object*/)
{

}

void QMapObjectView::modelUpdated(const QQmlChangeSet &changeSet, bool reset)
{
    // move changes are expressed as one remove + one insert, with the same moveId.
    // For simplicity, they will be treated as remove + insert.
    // Changes will be also ignored, as they represent only data changes, not layout changes
    if (reset) { // Assuming this means "remove everything already instantiated"
        flushDelegateModel();
    } else {
        // Remove map objects from the back to the front to retain the mapping to what is received from the changesets
        const QVector<QQmlChangeSet::Change> &removes = changeSet.removes();
        std::map<int, int> mapRemoves;
        for (int i = 0; i < removes.size(); i++)
            mapRemoves.insert(std::pair<int, int>(removes.at(i).start(), i));

        for (auto rit = mapRemoves.rbegin(); rit != mapRemoves.rend(); ++rit) {
            const QQmlChangeSet::Change &c = removes.at(rit->second);
            for (int idx = c.end() - 1; idx >= c.start(); --idx)
                    removeMapObjectFromMap(idx);
        }
    }

    QBoolBlocker createBlocker(m_creatingObject, true);
    for (const QQmlChangeSet::Change &c: changeSet.inserts()) {
        for (int idx = c.start(); idx < c.end(); idx++) {
            m_instantiatedMapObjects.insert(idx, nullptr);
            QGeoMapObject *mo = qobject_cast<QGeoMapObject *>(m_delegateModel->object(idx, incubationMode));
            if (mo) // if not, a createdItem signal will be emitted later, else it has been emitted already while createBlocker is in effect.
                addMapObjectToMap(mo, idx);
        }
    }
}

void QMapObjectView::addMapObjectToMap(QGeoMapObject *object, int index)
{
    if (!object)
        return;

    m_instantiatedMapObjects[index] = object;
    if (map())
        object->setMap(map());
    else
        m_pendingMapObjects << object;

    // ToDo:
    // Figure out the proper way to replace "mo->setVisible(visible());". Options:
    // - simply leave it to the user to set up a property binding
    // - set up a property binding automatically
    // - add a viewVisibility member to QGeoMapObject that gets combined at all times,
    //   and a connection for it.
}

void QMapObjectView::removeMapObjectFromMap(int index)
{
    if (index >= 0 && index < m_instantiatedMapObjects.size()) {
        QGeoMapObject *mo = m_instantiatedMapObjects.takeAt(index);
        if (!mo)
            return;

        mo->setMap(nullptr);
        QQmlInstanceModel::ReleaseFlags releaseStatus = m_delegateModel->release(mo);
#ifdef QT_DEBUG
    if (releaseStatus == QQmlInstanceModel::Referenced)
        qWarning() << "object "<<mo<<" still referenced";
#else
    Q_UNUSED(releaseStatus)
#endif
    }
}

// See QObject *QQmlDelegateModel::object(int index, QQmlIncubator::IncubationMode incubationMode) doc
// for explanation on when createdItem is emitted.
void QMapObjectView::createdItem(int index, QObject * /*object*/)
{
    if (m_creatingObject) {
        // see QDeclarativeGeoMapItemView::createdItem
        return;
    }

    // If here, according to the documentation above, object() should be called again for index,
    // or else, it will be destroyed exiting this scope
    QGeoMapObject *mo = nullptr;
    mo = qobject_cast<QGeoMapObject *>(m_delegateModel->object(index, incubationMode));
    if (mo)
        addMapObjectToMap(mo, index);
    else
        qWarning() << "QQmlDelegateModel::object called in createdItem for " << index << " produced a null object";
}


void QMapObjectView::flushDelegateModel()
{
    // Backward as removeItemFromMap modifies m_instantiatedItems
    for (int i = m_instantiatedMapObjects.size() -1; i >= 0 ; i--)
        removeMapObjectFromMap(i);
}

void QMapObjectView::flushUserAddedMapObjects()
{
    for (int i = 0; i < m_userAddedMapObjects.size(); ++i) {
        auto obj = m_userAddedMapObjects.at(i);
        if (obj)
            obj->setMap(nullptr); // obj parent might not be this. If so, it would not be destroyed by destroying this view.
    }
}

void QMapObjectView::setMap(QGeoMap *map)
{
    QMapObjectViewPrivate *d = static_cast<QMapObjectViewPrivate *>(d_ptr.data());
    if (d->m_map == map)
        return;

    QGeoMapObject::setMap(map); // This is where the specialized pimpl gets created and injected

    for (int i = 0; i < m_userAddedMapObjects.size(); ++i) {
        auto obj = m_userAddedMapObjects.at(i);
        if (obj && obj->map() != map)
            obj->setMap(map);
    }

    if (!map) {
        // Map was set, now it has ben re-set to NULL
        flushDelegateModel();
        flushUserAddedMapObjects();
        d_ptr = new QMapObjectViewPrivateDefault(*d);
    } else if (d->m_componentCompleted) {
        // Map was null, now it's set AND delegateModel is already complete.
        // some delegates may have been incubated but not added to the map.
        for (int i = 0; i < m_pendingMapObjects.size(); ++i) {
            auto obj = m_pendingMapObjects.at(i);
            if (obj && obj->map() != map)
                obj->setMap(map);
        }
        m_pendingMapObjects.clear();
    }
}

QT_END_NAMESPACE

