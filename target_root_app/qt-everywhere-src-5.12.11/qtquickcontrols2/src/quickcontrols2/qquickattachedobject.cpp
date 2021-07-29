/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
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

#include "qquickattachedobject_p.h"

#include <QtCore/qpointer.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickitemchangelistener_p.h>
#include <QtQuickTemplates2/private/qquickpopup_p.h>

QT_BEGIN_NAMESPACE

static QQuickAttachedObject *attachedObject(const QMetaObject *type, QObject *object, bool create = false)
{
    if (!object)
        return nullptr;
    int idx = -1;
    return qobject_cast<QQuickAttachedObject *>(qmlAttachedPropertiesObject(&idx, object, type, create));
}

static QQuickAttachedObject *findAttachedParent(const QMetaObject *type, QObject *object)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (item) {
        // lookup parent items and popups
        QQuickItem *parent = item->parentItem();
        while (parent) {
            QQuickAttachedObject *attached = attachedObject(type, parent);
            if (attached)
                return attached;

            QQuickPopup *popup = qobject_cast<QQuickPopup *>(parent->parent());
            if (popup)
                return attachedObject(type, popup);

            parent = parent->parentItem();
        }

        // fallback to item's window
        QQuickAttachedObject *attached = attachedObject(type, item->window());
        if (attached)
            return attached;
    } else {
        // lookup popup's window
        QQuickPopup *popup = qobject_cast<QQuickPopup *>(object);
        if (popup)
            return attachedObject(type, popup->popupItem()->window());
    }

    // lookup parent window
    QQuickWindow *window = qobject_cast<QQuickWindow *>(object);
    if (window) {
        QQuickWindow *parentWindow = qobject_cast<QQuickWindow *>(window->parent());
        if (parentWindow) {
            QQuickAttachedObject *attached = attachedObject(type, window);
            if (attached)
                return attached;
        }
    }

    // fallback to engine (global)
    if (object) {
        QQmlEngine *engine = qmlEngine(object);
        if (engine) {
            QByteArray name = QByteArray("_q_") + type->className();
            QQuickAttachedObject *attached = engine->property(name).value<QQuickAttachedObject *>();
            if (!attached) {
                attached = attachedObject(type, engine, true);
                engine->setProperty(name, QVariant::fromValue(attached));
            }
            return attached;
        }
    }

    return nullptr;
}

static QList<QQuickAttachedObject *> findAttachedChildren(const QMetaObject *type, QObject *object)
{
    QList<QQuickAttachedObject *> children;

    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        QQuickWindow *window = qobject_cast<QQuickWindow *>(object);
        if (window) {
            item = window->contentItem();

            const auto windowChildren = window->children();
            for (QObject *child : windowChildren) {
                QQuickWindow *childWindow = qobject_cast<QQuickWindow *>(child);
                if (childWindow) {
                    QQuickAttachedObject *attached = attachedObject(type, childWindow);
                    if (attached)
                        children += attached;
                }
            }
        }
    }

    if (item) {
        const auto childItems = item->childItems();
        for (QQuickItem *child : childItems) {
            QQuickAttachedObject *attached = attachedObject(type, child);
            if (attached)
                children += attached;
            else
                children += findAttachedChildren(type, child);
        }
    }

    return children;
}

static QQuickItem *findAttachedItem(QObject *parent)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(parent);
    if (!item) {
        QQuickPopup *popup = qobject_cast<QQuickPopup *>(parent);
        if (popup)
            item = popup->popupItem();
    }
    return item;
}

class QQuickAttachedObjectPrivate : public QObjectPrivate, public QQuickItemChangeListener
{
    Q_DECLARE_PUBLIC(QQuickAttachedObject)

public:
    static QQuickAttachedObjectPrivate *get(QQuickAttachedObject *attachedObject)
    {
        return attachedObject->d_func();
    }

    void attachTo(QObject *object);
    void detachFrom(QObject *object);

    void itemWindowChanged(QQuickWindow *window);
    void itemParentChanged(QQuickItem *item, QQuickItem *parent) override;

    QList<QQuickAttachedObject *> attachedChildren;
    QPointer<QQuickAttachedObject> attachedParent;
};

void QQuickAttachedObjectPrivate::attachTo(QObject *object)
{
    QQuickItem *item = findAttachedItem(object);
    if (item) {
        connect(item, &QQuickItem::windowChanged, this, &QQuickAttachedObjectPrivate::itemWindowChanged);
        QQuickItemPrivate::get(item)->addItemChangeListener(this, QQuickItemPrivate::Parent);
    }
}

void QQuickAttachedObjectPrivate::detachFrom(QObject *object)
{
    QQuickItem *item = findAttachedItem(object);
    if (item) {
        disconnect(item, &QQuickItem::windowChanged, this, &QQuickAttachedObjectPrivate::itemWindowChanged);
        QQuickItemPrivate::get(item)->removeItemChangeListener(this, QQuickItemPrivate::Parent);
    }
}

void QQuickAttachedObjectPrivate::itemWindowChanged(QQuickWindow *window)
{
    Q_Q(QQuickAttachedObject);
    QQuickAttachedObject *attachedParent = nullptr;
    QQuickItem *item = qobject_cast<QQuickItem *>(q->sender());
    if (item)
        attachedParent = findAttachedParent(q->metaObject(), item);
    if (!attachedParent)
        attachedParent = attachedObject(q->metaObject(), window);
    q->setAttachedParent(attachedParent);
}

void QQuickAttachedObjectPrivate::itemParentChanged(QQuickItem *item, QQuickItem *parent)
{
    Q_Q(QQuickAttachedObject);
    Q_UNUSED(parent);
    q->setAttachedParent(findAttachedParent(q->metaObject(), item));
}

QQuickAttachedObject::QQuickAttachedObject(QObject *parent)
    : QObject(*(new QQuickAttachedObjectPrivate), parent)
{
    Q_D(QQuickAttachedObject);
    d->attachTo(parent);
}

QQuickAttachedObject::~QQuickAttachedObject()
{
    Q_D(QQuickAttachedObject);
    d->detachFrom(parent());
    setAttachedParent(nullptr);
}

QList<QQuickAttachedObject *> QQuickAttachedObject::attachedChildren() const
{
    Q_D(const QQuickAttachedObject);
    return d->attachedChildren;
}

QQuickAttachedObject *QQuickAttachedObject::attachedParent() const
{
    Q_D(const QQuickAttachedObject);
    return d->attachedParent;
}

void QQuickAttachedObject::setAttachedParent(QQuickAttachedObject *parent)
{
    Q_D(QQuickAttachedObject);
    if (d->attachedParent == parent)
        return;

    QQuickAttachedObject *oldParent = d->attachedParent;
    if (d->attachedParent)
        QQuickAttachedObjectPrivate::get(d->attachedParent)->attachedChildren.removeOne(this);
    d->attachedParent = parent;
    if (parent)
        QQuickAttachedObjectPrivate::get(parent)->attachedChildren.append(this);
    attachedParentChange(parent, oldParent);
}

void QQuickAttachedObject::init()
{
    QQuickAttachedObject *attachedParent = findAttachedParent(metaObject(), parent());
    if (attachedParent)
        setAttachedParent(attachedParent);

    const QList<QQuickAttachedObject *> attachedChildren = findAttachedChildren(metaObject(), parent());
    for (QQuickAttachedObject *child : attachedChildren)
        child->setAttachedParent(this);
}

void QQuickAttachedObject::attachedParentChange(QQuickAttachedObject *newParent, QQuickAttachedObject *oldParent)
{
    Q_UNUSED(newParent);
    Q_UNUSED(oldParent);
}

QT_END_NAMESPACE
