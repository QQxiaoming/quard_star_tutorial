/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#include "qquickpalette_p.h"

QT_BEGIN_NAMESPACE

QColor QQuickPalette::alternateBase() const
{
    return v.color(QPalette::AlternateBase);
}

void QQuickPalette::setAlternateBase(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::AlternateBase, color);
}

void QQuickPalette::resetAlternateBase()
{
    v.resolve(v.resolve() & ~(1 << QPalette::AlternateBase));
}

QColor QQuickPalette::base() const
{
    return v.color(QPalette::Base);
}

void QQuickPalette::setBase(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Base, color);
}

void QQuickPalette::resetBase()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Base));
}

QColor QQuickPalette::brightText() const
{
    return v.color(QPalette::BrightText);
}

void QQuickPalette::setBrightText(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::BrightText, color);
}

void QQuickPalette::resetBrightText()
{
    v.resolve(v.resolve() & ~(1 << QPalette::BrightText));
}

QColor QQuickPalette::button() const
{
    return v.color(QPalette::Button);
}

void QQuickPalette::setButton(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Button, color);
}

void QQuickPalette::resetButton()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Button));
}

QColor QQuickPalette::buttonText() const
{
    return v.color(QPalette::ButtonText);
}

void QQuickPalette::setButtonText(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::ButtonText, color);
}

void QQuickPalette::resetButtonText()
{
    v.resolve(v.resolve() & ~(1 << QPalette::ButtonText));
}

QColor QQuickPalette::dark() const
{
    return v.color(QPalette::Dark);
}

void QQuickPalette::setDark(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Dark, color);
}

void QQuickPalette::resetDark()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Dark));
}

QColor QQuickPalette::highlight() const
{
    return v.color(QPalette::Highlight);
}

void QQuickPalette::setHighlight(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Highlight, color);
}

void QQuickPalette::resetHighlight()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Highlight));
}

QColor QQuickPalette::highlightedText() const
{
    return v.color(QPalette::HighlightedText);
}

void QQuickPalette::setHighlightedText(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::HighlightedText, color);
}

void QQuickPalette::resetHighlightedText()
{
    v.resolve(v.resolve() & ~(1 << QPalette::HighlightedText));
}

QColor QQuickPalette::light() const
{
    return v.color(QPalette::Light);
}

void QQuickPalette::setLight(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Light, color);
}

void QQuickPalette::resetLight()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Light));
}

QColor QQuickPalette::link() const
{
    return v.color(QPalette::Link);
}

void QQuickPalette::setLink(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Link, color);
}

void QQuickPalette::resetLink()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Link));
}

QColor QQuickPalette::linkVisited() const
{
    return v.color(QPalette::LinkVisited);
}

void QQuickPalette::setLinkVisited(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::LinkVisited, color);
}

void QQuickPalette::resetLinkVisited()
{
    v.resolve(v.resolve() & ~(1 << QPalette::LinkVisited));
}

QColor QQuickPalette::mid() const
{
    return v.color(QPalette::Mid);
}

void QQuickPalette::setMid(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Mid, color);
}

void QQuickPalette::resetMid()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Mid));
}

QColor QQuickPalette::midlight() const
{
    return v.color(QPalette::Midlight);
}

void QQuickPalette::setMidlight(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Midlight, color);
}

void QQuickPalette::resetMidlight()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Midlight));
}

QColor QQuickPalette::shadow() const
{
    return v.color(QPalette::Shadow);
}

void QQuickPalette::setShadow(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Shadow, color);
}

void QQuickPalette::resetShadow()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Shadow));
}

QColor QQuickPalette::text() const
{
    return v.color(QPalette::Text);
}

void QQuickPalette::setText(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Text, color);
}

void QQuickPalette::resetText()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Text));
}

QColor QQuickPalette::toolTipBase() const
{
    return v.color(QPalette::ToolTipBase);
}

void QQuickPalette::setToolTipBase(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::ToolTipBase, color);
}

void QQuickPalette::resetToolTipBase()
{
    v.resolve(v.resolve() & ~(1 << QPalette::ToolTipBase));
}

QColor QQuickPalette::toolTipText() const
{
    return v.color(QPalette::ToolTipText);
}

void QQuickPalette::setToolTipText(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::ToolTipText, color);
}

void QQuickPalette::resetToolTipText()
{
    v.resolve(v.resolve() & ~(1 << QPalette::ToolTipText));
}

QColor QQuickPalette::window() const
{
    return v.color(QPalette::Window);
}

void QQuickPalette::setWindow(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::Window, color);
}

void QQuickPalette::resetWindow()
{
    v.resolve(v.resolve() & ~(1 << QPalette::Window));
}

QColor QQuickPalette::windowText() const
{
    return v.color(QPalette::WindowText);
}

void QQuickPalette::setWindowText(const QColor &color)
{
    v.setColor(QPalette::All, QPalette::WindowText, color);
}

void QQuickPalette::resetWindowText()
{
    v.resolve(v.resolve() & ~(1 << QPalette::WindowText));
}

QT_END_NAMESPACE
