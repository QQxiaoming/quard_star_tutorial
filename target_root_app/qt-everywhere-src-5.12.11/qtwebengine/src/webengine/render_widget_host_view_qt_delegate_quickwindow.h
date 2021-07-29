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

#ifndef RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICKWINDOW_H
#define RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICKWINDOW_H

#include "render_widget_host_view_qt_delegate.h"

#include "render_widget_host_view_qt_delegate_quick.h"

#include <QQuickWindow>
#include <QScopedPointer>

namespace QtWebEngineCore {

class RenderWidgetHostViewQtDelegateQuickWindow : public QQuickWindow , public RenderWidgetHostViewQtDelegate {

public:
    RenderWidgetHostViewQtDelegateQuickWindow(RenderWidgetHostViewQtDelegate *realDelegate);
    ~RenderWidgetHostViewQtDelegateQuickWindow();

    void initAsPopup(const QRect&) override;
    QRectF screenRect() const override;
    QRectF contentsRect() const override;
    void setKeyboardFocus() override {}
    bool hasKeyboardFocus() override { return false; }
    void lockMouse() override {}
    void unlockMouse() override {}
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
    void inputMethodStateChanged(bool, bool) override {}
    void setInputMethodHints(Qt::InputMethodHints) override { }
    void setClearColor(const QColor &) override { }
    bool copySurface(const QRect &, const QSize &, QImage &) override { return false; }

private:
    QScopedPointer<RenderWidgetHostViewQtDelegate> m_realDelegate;
};

} // namespace QtWebEngineCore

#endif // RENDER_WIDGET_HOST_VIEW_QT_DELEGATE_QUICKWINDOW_H
