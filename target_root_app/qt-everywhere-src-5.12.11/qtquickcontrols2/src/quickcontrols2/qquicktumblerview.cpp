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

#include "qquicktumblerview_p.h"

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquicklistview_p.h>
#include <QtQuick/private/qquickpathview_p.h>

#include <QtQuickTemplates2/private/qquicktumbler_p.h>
#include <QtQuickTemplates2/private/qquicktumbler_p_p.h>

QT_BEGIN_NAMESPACE

QQuickTumblerView::QQuickTumblerView(QQuickItem *parent) :
    QQuickItem(parent)
{
    // We don't call createView() here because we don't know what the wrap flag is set to
    // yet, and we don't want to create a view that might never get used.
}

QVariant QQuickTumblerView::model() const
{
    return m_model;
}

void QQuickTumblerView::setModel(const QVariant &model)
{
    if (model == m_model)
        return;

    m_model = model;

    if (m_pathView) {
        m_pathView->setModel(m_model);
    } else if (m_listView) {
        // QQuickItemView::setModel() resets the current index,
        // but if we're still creating the Tumbler, it should be maintained.
        const int oldCurrentIndex = m_listView->currentIndex();
        m_listView->setModel(m_model);
        if (!isComponentComplete())
            m_listView->setCurrentIndex(oldCurrentIndex);
    }

    emit modelChanged();
}

QQmlComponent *QQuickTumblerView::delegate() const
{
    return m_delegate;
}

void QQuickTumblerView::setDelegate(QQmlComponent *delegate)
{
    if (delegate == m_delegate)
        return;

    m_delegate = delegate;

    if (m_pathView)
        m_pathView->setDelegate(m_delegate);
    else if (m_listView)
        m_listView->setDelegate(m_delegate);

    emit delegateChanged();
}

QQuickPath *QQuickTumblerView::path() const
{
    return m_path;
}

void QQuickTumblerView::setPath(QQuickPath *path)
{
    if (path == m_path)
        return;

    m_path = path;
    emit pathChanged();
}

void QQuickTumblerView::createView()
{
    Q_ASSERT(m_tumbler);

    // We create a view regardless of whether or not we know
    // the count yet, because we rely on the view to tell us the count.
    if (m_tumbler->wrap()) {
        if (m_listView) {
            // It's necessary to call deleteLater() rather than delete,
            // as this code is most likely being run in rensponse to a signal
            // emission somewhere in the list view's internals, so we need to
            // wait until that has finished.
            m_listView->deleteLater();
            QQml_setParent_noEvent(m_listView, nullptr);
            // The auto tests pass with unparenting the list view alone, but
            // just to be sure, we unset some other things as well.
            m_listView->setParentItem(nullptr);
            m_listView->setVisible(false);
            m_listView->setModel(QVariant());
            m_listView = nullptr;
        }

        if (!m_pathView) {
            m_pathView = new QQuickPathView;
            QQmlEngine::setContextForObject(m_pathView, qmlContext(this));
            QQml_setParent_noEvent(m_pathView, this);
            m_pathView->setParentItem(this);
            m_pathView->setPath(m_path);
            m_pathView->setDelegate(m_delegate);
            m_pathView->setPreferredHighlightBegin(0.5);
            m_pathView->setPreferredHighlightEnd(0.5);
            m_pathView->setHighlightMoveDuration(1000);
            m_pathView->setClip(true);

            // Give the view a size.
            updateView();
            // Set the model.
            updateModel();
        }
    } else {
        if (m_pathView) {
            m_pathView->deleteLater();
            QQml_setParent_noEvent(m_pathView, nullptr);
            m_pathView->setParentItem(nullptr);
            m_pathView->setVisible(false);
            m_pathView->setModel(QVariant());
            m_pathView = nullptr;
        }

        if (!m_listView) {
            m_listView = new QQuickListView;
            QQmlEngine::setContextForObject(m_listView, qmlContext(this));
            QQml_setParent_noEvent(m_listView, this);
            m_listView->setParentItem(this);
            m_listView->setSnapMode(QQuickListView::SnapToItem);
            m_listView->setHighlightRangeMode(QQuickListView::StrictlyEnforceRange);
            m_listView->setClip(true);

            // Give the view a size.
            updateView();
            // Set the model.
            updateModel();

            // Set these after the model is set so that the currentItem animation
            // happens instantly on startup/after switching models. If we set them too early,
            // the view animates any potential currentIndex change over one second,
            // which we don't want when the contentItem has just been created.
            m_listView->setDelegate(m_delegate);
            m_listView->setHighlightMoveDuration(1000);
        }
    }
}

// Called whenever the size or visibleItemCount changes.
void QQuickTumblerView::updateView()
{
    QQuickItem *theView = view();
    if (!theView)
        return;

    theView->setSize(QSizeF(width(), height()));

    // Can be called in geometryChanged when it might not have a parent item yet.
    if (!m_tumbler)
        return;

    // Set view-specific properties that have a dependency on the size, etc.
    if (m_pathView) {
        m_pathView->setPathItemCount(m_tumbler->visibleItemCount() + 1);
        m_pathView->setDragMargin(width() / 2);
    } else {
        m_listView->setPreferredHighlightBegin(height() / 2 - (height() / m_tumbler->visibleItemCount() / 2));
        m_listView->setPreferredHighlightEnd(height() / 2 + (height() / m_tumbler->visibleItemCount() / 2));
    }
}

void QQuickTumblerView::updateModel()
{
    if (m_pathView && !m_pathView->model().isValid() && m_model.isValid()) {
        // QQuickPathView::setPathItemCount() resets the offset animation,
        // so we just skip the animation while constructing the view.
        const int oldHighlightMoveDuration = m_pathView->highlightMoveDuration();
        m_pathView->setHighlightMoveDuration(0);

        // Setting model can change the count, which can affect the wrap, which can cause
        // the current view to be deleted before setModel() is finished, which causes a crash.
        // Since QQuickTumbler can't know about QQuickTumblerView, we use its private API to
        // inform it that it should delay setting wrap.
        QQuickTumblerPrivate *tumblerPrivate = QQuickTumblerPrivate::get(m_tumbler);
        tumblerPrivate->beginSetModel();
        m_pathView->setModel(m_model);
        tumblerPrivate->endSetModel();

        // The count-depends-on-wrap behavior could cause wrap to change after
        // the call above, so we must check that we're still using a PathView.
        if (m_pathView)
            m_pathView->setHighlightMoveDuration(oldHighlightMoveDuration);
    } else if (m_listView && !m_listView->model().isValid() && m_model.isValid()) {
        const int currentIndex = m_tumbler->currentIndex();
        QQuickTumblerPrivate *tumblerPrivate = QQuickTumblerPrivate::get(m_tumbler);

        // setModel() causes QQuickTumblerPrivate::_q_onViewCountChanged() to
        // be called, which calls QQuickTumbler::setCurrentIndex(),
        // which results in QQuickItemViewPrivate::createHighlightItem() being
        // called. When the highlight item is created,
        // QQuickTumblerPrivate::itemChildAdded() is notified and
        // QQuickTumblerPrivate::_q_updateItemHeights() is called, which causes
        // a geometry change in the item and createHighlight() is called again.
        // However, since the highlight item hadn't been assigned yet in the
        // previous call frame, the "if (highlight) { delete highlight; }"
        // check doesn't succeed, so the item is never deleted.
        //
        // To avoid this, we tell QQuickTumblerPrivate to ignore signals while
        // setting the model, and manually call _q_onViewCountChanged() to
        // ensure the correct sequence of calls happens (_q_onViewCountChanged()
        // has to be within the ignoreSignals scope, because it also generates
        // recursion otherwise).
        tumblerPrivate->ignoreSignals = true;
        m_listView->setModel(m_model);
        m_listView->setCurrentIndex(currentIndex);

        tumblerPrivate->_q_onViewCountChanged();
        tumblerPrivate->ignoreSignals = false;
    }
}

void QQuickTumblerView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    updateView();
}

void QQuickTumblerView::componentComplete()
{
    QQuickItem::componentComplete();
    updateView();
}

void QQuickTumblerView::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    QQuickItem::itemChange(change, data);

    if (change == QQuickItem::ItemParentHasChanged && data.item) {
        if (m_tumbler)
            m_tumbler->disconnect(this);

        m_tumbler = qobject_cast<QQuickTumbler*>(parentItem());

        if (m_tumbler) {
            // We assume that the parentChanged() signal of the tumbler will be emitted before its wrap property is set...
            connect(m_tumbler, &QQuickTumbler::wrapChanged, this, &QQuickTumblerView::createView);
            connect(m_tumbler, &QQuickTumbler::visibleItemCountChanged, this, &QQuickTumblerView::updateView);
        }
    }
}

QQuickItem *QQuickTumblerView::view()
{
    if (!m_tumbler)
        return nullptr;

    if (m_tumbler->wrap())
        return m_pathView;

    return m_listView;
}

QT_END_NAMESPACE
