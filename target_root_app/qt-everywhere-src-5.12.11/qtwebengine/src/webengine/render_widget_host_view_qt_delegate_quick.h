/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICK_H

#include "render_widget_host_view_qt_delegate.h"

#include <QQuickItem>

QT_BEGIN_NAMESPACE
class QQuickWebEngineView;
class QQuickWebEngineViewPrivate;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateQuick : public QQuickItem, public RenderWidgetHostViewQtDelegate
{
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateQuick(RenderWidgetHostViewQtDelegateClient *client, bool isPopup);
    ~RenderWidgetHostViewQtDelegateQuick();

    void initAsPopup(const QRect&) override;
    QRectF screenRect() const override;
    QRectF contentsRect() const override;
    void setKeyboardFocus() override;
    bool hasKeyboardFocus() override;
    void lockMouse() override;
    void unlockMouse() override;
    void show() override;
    void hide() override;
    bool isVisible() const override;
    QWindow* window() const override;
    QSGTexture *createTextureFromImage(const QImage &) override;
    QSGLayer *createLayer() override;
    QSGInternalImageNode *createInternalImageNode() override;
    QSGImageNode *createImageNode() override;
    QSGRectangleNode *createRectangleNode() override;
    void update() override;
    void updateCursor(const QCursor &) override;
    void resize(int width, int height) override;
    void move(const QPoint&) override { }
    void inputMethodStateChanged(bool editorVisible, bool isPasswordInput) override;
    void setInputMethodHints(Qt::InputMethodHints) override { }
    // The QtQuick view doesn't have a backbuffer of its own and doesn't need this
    void setClearColor(const QColor &) override { }
    bool copySurface(const QRect &rect, const QSize &size, QImage &image) override;

protected:
    bool event(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void touchEvent(QTouchEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

private slots:
    void onWindowPosChanged();
    void onHide();

private:
    friend QQuickWebEngineViewPrivate;

    RenderWidgetHostViewQtDelegateClient *m_client;
    QList<QMetaObject::Connection> m_windowConnections;
    bool m_isPopup;
    QPointF m_lastGlobalPos;
    QQuickWebEngineView *m_view = nullptr;
};

} // namespace QtWebEngineCore

#endif
