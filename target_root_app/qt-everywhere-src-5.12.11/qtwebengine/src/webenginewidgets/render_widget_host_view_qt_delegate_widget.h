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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WIDGET_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_WIDGET_H

#include "render_widget_host_view_qt_delegate.h"
#include "web_contents_adapter_client.h"

#include <QQuickItem>
#include <QQuickWidget>

QT_BEGIN_NAMESPACE
class QWebEnginePage;
class QWebEnginePagePrivate;
QT_END_NAMESPACE

namespace QtWebEngineCore {

// Useful information keyboard and mouse QEvent propagation.
// A RenderWidgetHostViewQtDelegateWidget instance initialized as a popup will receive
// no keyboard focus (so all keyboard QEvents will be sent to the parent RWHVQD instance),
// but will still receive mouse input (all mouse QEvent moves and clicks will be given to the popup
// RWHVQD instance, and the mouse interaction area covers the surface of the whole parent
// QWebEngineView, and not only the smaller surface that an HTML select popup would occupy).
class RenderWidgetHostViewQtDelegateWidget : public QQuickWidget, public RenderWidgetHostViewQtDelegate {
    Q_OBJECT
public:
    RenderWidgetHostViewQtDelegateWidget(RenderWidgetHostViewQtDelegateClient *client, QWidget *parent = 0);
    ~RenderWidgetHostViewQtDelegateWidget();

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
    void move(const QPoint &screenPos) override;
    void inputMethodStateChanged(bool editorVisible, bool passwordInput) override;
    void setInputMethodHints(Qt::InputMethodHints) override;
    void setClearColor(const QColor &color) override;
    bool copySurface(const QRect &, const QSize &, QImage &) override;

protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *resizeEvent) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void closeEvent(QCloseEvent *event) override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

private slots:
    void onWindowPosChanged();
    void connectRemoveParentBeforeParentDelete();
    void removeParentBeforeParentDelete();

private:
    friend QWebEnginePagePrivate;

    RenderWidgetHostViewQtDelegateClient *m_client;
    QScopedPointer<QQuickItem> m_rootItem;
    bool m_isPopup;
    QColor m_clearColor;
    QPoint m_lastGlobalPos;
    QList<QMetaObject::Connection> m_windowConnections;
    QWebEnginePage *m_page = nullptr;
    QMetaObject::Connection m_parentDestroyedConnection;
};

} // namespace QtWebEngineCore

#endif
