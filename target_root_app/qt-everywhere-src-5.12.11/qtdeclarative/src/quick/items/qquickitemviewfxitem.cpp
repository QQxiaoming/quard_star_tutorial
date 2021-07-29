/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#include "qquickitemviewfxitem_p_p.h"
#include "qquickitem_p.h"
#include "qquickitemview_p_p.h"

QT_BEGIN_NAMESPACE

QQuickItemViewFxItem::QQuickItemViewFxItem(QQuickItem *item, bool ownItem, QQuickItemChangeListener* changeListener)
    : item(item)
    , ownItem(ownItem)
    , changeListener(changeListener)
    , transitionableItem(nullptr)
    , releaseAfterTransition(false)
    , trackGeom(false)
{
}

QQuickItemViewFxItem::~QQuickItemViewFxItem()
{
    delete transitionableItem;
    transitionableItem = nullptr;

    if (ownItem && item) {
        trackGeometry(false);
        item->setParentItem(0);
        item->deleteLater();
    }
}

qreal QQuickItemViewFxItem::itemX() const
{
    return transitionableItem ? transitionableItem->itemX() : (item ? item->x() : 0);
}

qreal QQuickItemViewFxItem::itemY() const
{
    return transitionableItem ? transitionableItem->itemY() : (item ? item->y() : 0);
}

void QQuickItemViewFxItem::moveTo(const QPointF &pos, bool immediate)
{
    if (transitionableItem)
        transitionableItem->moveTo(pos, immediate);
    else if (item)
        item->setPosition(pos);
}

void QQuickItemViewFxItem::setVisible(bool visible)
{
    if (!visible && transitionableItem && transitionableItem->transitionScheduledOrRunning())
        return;
    if (item)
        QQuickItemPrivate::get(item)->setCulled(!visible);
}

void QQuickItemViewFxItem::trackGeometry(bool track)
{
    if (track) {
        if (!trackGeom) {
            if (item) {
                QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(item);
                itemPrivate->addItemChangeListener(changeListener, QQuickItemPrivate::Geometry);
            }
            trackGeom = true;
        }
    } else {
        if (trackGeom) {
            if (item) {
                QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(item);
                itemPrivate->removeItemChangeListener(changeListener, QQuickItemPrivate::Geometry);
            }
            trackGeom = false;
        }
    }
}

QRectF QQuickItemViewFxItem::geometry() const
{
    return QRectF(item->position(), item->size());
}

void QQuickItemViewFxItem::setGeometry(const QRectF &geometry)
{
    item->setPosition(geometry.topLeft());
    item->setSize(geometry.size());
}

QQuickItemViewTransitioner::TransitionType QQuickItemViewFxItem::scheduledTransitionType() const
{
    return transitionableItem ? transitionableItem->nextTransitionType : QQuickItemViewTransitioner::NoTransition;
}

bool QQuickItemViewFxItem::transitionScheduledOrRunning() const
{
    return transitionableItem ? transitionableItem->transitionScheduledOrRunning() : false;
}

bool QQuickItemViewFxItem::transitionRunning() const
{
    return transitionableItem ? transitionableItem->transitionRunning() : false;
}

bool QQuickItemViewFxItem::isPendingRemoval() const
{
    return transitionableItem ? transitionableItem->isPendingRemoval() : false;
}

void QQuickItemViewFxItem::transitionNextReposition(QQuickItemViewTransitioner *transitioner, QQuickItemViewTransitioner::TransitionType type, bool asTarget)
{
    if (!transitioner)
        return;
    if (!transitionableItem)
        transitionableItem = new QQuickItemViewTransitionableItem(item);
    transitioner->transitionNextReposition(transitionableItem, type, asTarget);
}

bool QQuickItemViewFxItem::prepareTransition(QQuickItemViewTransitioner *transitioner, const QRectF &viewBounds)
{
    return transitionableItem ? transitionableItem->prepareTransition(transitioner, index, viewBounds) : false;
}

void QQuickItemViewFxItem::startTransition(QQuickItemViewTransitioner *transitioner)
{
    if (transitionableItem)
        transitionableItem->startTransition(transitioner, index);
}

QT_END_NAMESPACE

