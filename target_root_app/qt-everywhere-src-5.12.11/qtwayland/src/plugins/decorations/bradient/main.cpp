/****************************************************************************
**
** Copyright (C) 2016 Robin Burchell <robin.burchell@viroteck.net>
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QLinearGradient>

#include <qpa/qwindowsysteminterface.h>

#include <QtWaylandClient/private/qwaylanddecorationplugin_p.h>
#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

#define BUTTON_SPACING 5
#define BUTTON_WIDTH 18
#define BUTTONS_RIGHT_MARGIN 8

enum Button
{
    None,
    Close,
    Maximize,
    Minimize
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandBradientDecoration : public QWaylandAbstractDecoration
{
public:
    QWaylandBradientDecoration();
protected:
    QMargins margins() const override;
    void paint(QPaintDevice *device) override;
    bool handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,Qt::MouseButtons b,Qt::KeyboardModifiers mods) override;
    bool handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::TouchPointState state, Qt::KeyboardModifiers mods) override;
private:
    void processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    void processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b,Qt::KeyboardModifiers mods);
    bool clickButton(Qt::MouseButtons b, Button btn);

    QRectF closeButtonRect() const;
    QRectF maximizeButtonRect() const;
    QRectF minimizeButtonRect() const;

    QColor m_foregroundColor;
    QColor m_foregroundInactiveColor;
    QColor m_backgroundColor;
    QStaticText m_windowTitle;
    Button m_clicking = None;
};



QWaylandBradientDecoration::QWaylandBradientDecoration()
{
    QPalette palette;
    m_foregroundColor = palette.color(QPalette::Active, QPalette::WindowText);
    m_backgroundColor = palette.color(QPalette::Active, QPalette::Window);
    m_foregroundInactiveColor = palette.color(QPalette::Disabled, QPalette::WindowText);

    QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
    option.setWrapMode(QTextOption::NoWrap);
    m_windowTitle.setTextOption(option);
}

QRectF QWaylandBradientDecoration::closeButtonRect() const
{
    return QRectF(window()->frameGeometry().width() - BUTTON_WIDTH - BUTTON_SPACING * 0 - BUTTONS_RIGHT_MARGIN,
                  (margins().top() - BUTTON_WIDTH) / 2, BUTTON_WIDTH, BUTTON_WIDTH);
}

QRectF QWaylandBradientDecoration::maximizeButtonRect() const
{
    return QRectF(window()->frameGeometry().width() - BUTTON_WIDTH * 2 - BUTTON_SPACING * 1 - BUTTONS_RIGHT_MARGIN,
                  (margins().top() - BUTTON_WIDTH) / 2, BUTTON_WIDTH, BUTTON_WIDTH);
}

QRectF QWaylandBradientDecoration::minimizeButtonRect() const
{
    return QRectF(window()->frameGeometry().width() - BUTTON_WIDTH * 3 - BUTTON_SPACING * 2 - BUTTONS_RIGHT_MARGIN,
                  (margins().top() - BUTTON_WIDTH) / 2, BUTTON_WIDTH, BUTTON_WIDTH);
}

QMargins QWaylandBradientDecoration::margins() const
{
    return QMargins(3, 30, 3, 3);
}

void QWaylandBradientDecoration::paint(QPaintDevice *device)
{
    bool active = window()->handle()->isActive();
    QRect surfaceRect(QPoint(), window()->frameGeometry().size());
    QRect clips[] =
    {
        QRect(0, 0, surfaceRect.width(), margins().top()),
        QRect(0, surfaceRect.height() - margins().bottom(), surfaceRect.width(), margins().bottom()),
        QRect(0, margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom()),
        QRect(surfaceRect.width() - margins().right(), margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom())
    };

    QRect top = clips[0];

    QPainter p(device);
    p.setRenderHint(QPainter::Antialiasing);

    // Title bar
    QPainterPath roundedRect;
    roundedRect.addRoundedRect(surfaceRect, 3, 3);
    for (int i = 0; i < 4; ++i) {
        p.save();
        p.setClipRect(clips[i]);
        p.fillPath(roundedRect, m_backgroundColor);
        p.restore();
    }

    // Window icon
    QIcon icon = waylandWindow()->windowIcon();
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(QSize(128, 128));
        QPixmap scaled = pixmap.scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QRectF iconRect(0, 0, 22, 22);
        p.drawPixmap(iconRect.adjusted(margins().left() + BUTTON_SPACING, 4,
                                       margins().left() + BUTTON_SPACING, 4),
                     scaled, iconRect);
    }

    // Window title
    QString windowTitleText = window()->title();
    if (!windowTitleText.isEmpty()) {
        if (m_windowTitle.text() != windowTitleText) {
            m_windowTitle.setText(windowTitleText);
            m_windowTitle.prepare();
        }

        QRect titleBar = top;
        titleBar.setLeft(margins().left() + BUTTON_SPACING +
            (icon.isNull() ? 0 : 22 + BUTTON_SPACING));
        titleBar.setRight(minimizeButtonRect().left() - BUTTON_SPACING);

        p.save();
        p.setClipRect(titleBar);
        p.setPen(active ? m_foregroundColor : m_foregroundInactiveColor);
        QSizeF size = m_windowTitle.size();
        int dx = (top.width() - size.width()) /2;
        int dy = (top.height()- size.height()) /2;
        QFont font = p.font();
        font.setPixelSize(14);
        p.setFont(font);
        QPoint windowTitlePoint(top.topLeft().x() + dx,
                 top.topLeft().y() + dy);
        p.drawStaticText(windowTitlePoint, m_windowTitle);
        p.restore();
    }

    QRectF rect;

    // Default pen
    QPen pen(active ? m_foregroundColor : m_foregroundInactiveColor);
    p.setPen(pen);

    // Close button
    p.save();
    rect = closeButtonRect();
    qreal crossSize = rect.height() / 2.3;
    QPointF crossCenter(rect.center());
    QRectF crossRect(crossCenter.x() - crossSize / 2, crossCenter.y() - crossSize / 2, crossSize, crossSize);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawLine(crossRect.topLeft(), crossRect.bottomRight());
    p.drawLine(crossRect.bottomLeft(), crossRect.topRight());
    p.restore();

    // Maximize button
    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    rect = maximizeButtonRect().adjusted(4, 5, -4, -5);
    if ((window()->windowStates() & Qt::WindowMaximized)) {
        qreal inset = 2;
        QRectF rect1 = rect.adjusted(inset, 0, 0, -inset);
        QRectF rect2 = rect.adjusted(0, inset, -inset, 0);
        p.drawRect(rect1);
        p.setBrush(m_backgroundColor); // need to cover up some lines from the other rect
        p.drawRect(rect2);
    } else {
        p.drawRect(rect);
        p.drawLine(rect.left(), rect.top() + 1, rect.right(), rect.top() + 1);
    }
    p.restore();

    // Minimize button
    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    rect = minimizeButtonRect().adjusted(5, 5, -5, -5);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawLine(rect.bottomLeft(), rect.bottomRight());
    p.restore();
}

bool QWaylandBradientDecoration::clickButton(Qt::MouseButtons b, Button btn)
{
    if (isLeftClicked(b)) {
        m_clicking = btn;
        return false;
    } else if (isLeftReleased(b)) {
        if (m_clicking == btn) {
            m_clicking = None;
            return true;
        } else {
            m_clicking = None;
        }
    }
    return false;
}

bool QWaylandBradientDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::MouseButtons b, Qt::KeyboardModifiers mods)

{
    Q_UNUSED(global);

    // Figure out what area mouse is in
    if (local.y() <= margins().top()) {
        processMouseTop(inputDevice,local,b,mods);
    } else if (local.y() > window()->height() + margins().top()) {
        processMouseBottom(inputDevice,local,b,mods);
    } else if (local.x() <= margins().left()) {
        processMouseLeft(inputDevice,local,b,mods);
    } else if (local.x() > window()->width() + margins().left()) {
        processMouseRight(inputDevice,local,b,mods);
    } else {
#if QT_CONFIG(cursor)
        waylandWindow()->restoreMouseCursor(inputDevice);
#endif
        setMouseButtons(b);
        return false;
    }

    setMouseButtons(b);
    return true;
}

bool QWaylandBradientDecoration::handleTouch(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::TouchPointState state, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(inputDevice);
    Q_UNUSED(global);
    Q_UNUSED(mods);
    bool handled = state == Qt::TouchPointPressed;
    if (handled) {
        if (closeButtonRect().contains(local))
            QWindowSystemInterface::handleCloseEvent(window());
        else if (maximizeButtonRect().contains(local))
            window()->setWindowStates(window()->windowStates() ^ Qt::WindowMaximized);
        else if (minimizeButtonRect().contains(local))
            window()->setWindowState(Qt::WindowMinimized);
        else if (local.y() <= margins().top())
            waylandWindow()->shellSurface()->move(inputDevice);
        else
            handled = false;
    }

    return handled;
}

void QWaylandBradientDecoration::processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods);
    if (local.y() <= margins().bottom()) {
        if (local.x() <= margins().left()) {
            //top left bit
#if QT_CONFIG(cursor)
            waylandWindow()->setMouseCursor(inputDevice, Qt::SizeFDiagCursor);
#endif
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_LEFT,b);
        } else if (local.x() > window()->width() + margins().left()) {
            //top right bit
#if QT_CONFIG(cursor)
            waylandWindow()->setMouseCursor(inputDevice, Qt::SizeBDiagCursor);
#endif
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_RIGHT,b);
        } else {
            //top resize bit
#if QT_CONFIG(cursor)
            waylandWindow()->setMouseCursor(inputDevice, Qt::SplitVCursor);
#endif
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP,b);
        }
    } else if (local.x() <= margins().left()) {
        processMouseLeft(inputDevice, local, b, mods);
    } else if (local.x() > window()->width() + margins().left()) {
        processMouseRight(inputDevice, local, b, mods);
    } else if (closeButtonRect().contains(local)) {
        if (clickButton(b, Close))
            QWindowSystemInterface::handleCloseEvent(window());
    } else if (maximizeButtonRect().contains(local)) {
        if (clickButton(b, Maximize))
            window()->setWindowStates(window()->windowStates() ^ Qt::WindowMaximized);
    } else if (minimizeButtonRect().contains(local)) {
        if (clickButton(b, Minimize))
            window()->setWindowState(Qt::WindowMinimized);
    } else {
#if QT_CONFIG(cursor)
        waylandWindow()->restoreMouseCursor(inputDevice);
#endif
        startMove(inputDevice,b);
    }
}

void QWaylandBradientDecoration::processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods);
    if (local.x() <= margins().left()) {
        //bottom left bit
#if QT_CONFIG(cursor)
        waylandWindow()->setMouseCursor(inputDevice, Qt::SizeBDiagCursor);
#endif
        startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT,b);
    } else if (local.x() > window()->width() + margins().left()) {
        //bottom right bit
#if QT_CONFIG(cursor)
        waylandWindow()->setMouseCursor(inputDevice, Qt::SizeFDiagCursor);
#endif
        startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT,b);
    } else {
        //bottom bit
#if QT_CONFIG(cursor)
        waylandWindow()->setMouseCursor(inputDevice, Qt::SplitVCursor);
#endif
        startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_BOTTOM,b);
    }
}

void QWaylandBradientDecoration::processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
#if QT_CONFIG(cursor)
    waylandWindow()->setMouseCursor(inputDevice, Qt::SplitHCursor);
#endif
    startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_LEFT,b);
}

void QWaylandBradientDecoration::processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
#if QT_CONFIG(cursor)
    waylandWindow()->setMouseCursor(inputDevice, Qt::SplitHCursor);
#endif
    startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_RIGHT,b);
}

class QWaylandBradientDecorationPlugin : public QWaylandDecorationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandDecorationFactoryInterface_iid FILE "bradient.json")
public:
    QWaylandAbstractDecoration *create(const QString&, const QStringList&) override;
};

QWaylandAbstractDecoration *QWaylandBradientDecorationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new QWaylandBradientDecoration();
}

}

QT_END_NAMESPACE

#include "main.moc"
