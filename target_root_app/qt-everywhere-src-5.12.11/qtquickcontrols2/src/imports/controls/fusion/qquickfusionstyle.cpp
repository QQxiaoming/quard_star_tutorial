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

#include "qquickfusionstyle_p.h"

#include <QtGui/qcolor.h>
#include <QtGui/qpalette.h>
#include <QtGui/qpa/qplatformtheme.h>
#include <QtGui/private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

QQuickFusionStyle::QQuickFusionStyle(QObject *parent)
    : QObject(parent)
{
}

QColor QQuickFusionStyle::lightShade()
{
    return QColor(255, 255, 255, 90);
}

QColor QQuickFusionStyle::darkShade()
{
    return QColor(0, 0, 0, 60);
}

QColor QQuickFusionStyle::topShadow()
{
    return QColor(0, 0, 0, 18);
}

QColor QQuickFusionStyle::innerContrastLine()
{
    return QColor(255, 255, 255, 30);
}

QColor QQuickFusionStyle::highlight(const QPalette &palette)
{
    return palette.color(QPalette::Highlight);
}

QColor QQuickFusionStyle::highlightedText(const QPalette &palette)
{
    return palette.color(QPalette::HighlightedText);
}

QColor QQuickFusionStyle::outline(const QPalette &palette)
{
    if (palette.window().style() == Qt::TexturePattern)
        return QColor(0, 0, 0, 160);
    return palette.background().color().darker(140);
}

QColor QQuickFusionStyle::highlightedOutline(const QPalette &palette)
{
    QColor highlightedOutline = highlight(palette).darker(125);
    if (highlightedOutline.value() > 160)
        highlightedOutline.setHsl(highlightedOutline.hue(), highlightedOutline.saturation(), 160);
    return highlightedOutline;
}

QColor QQuickFusionStyle::tabFrameColor(const QPalette &palette)
{
    if (palette.window().style() == Qt::TexturePattern)
        return QColor(255, 255, 255, 8);
    return buttonColor(palette).lighter(104);
}

QColor QQuickFusionStyle::buttonColor(const QPalette &palette, bool highlighted, bool down, bool hovered)
{
    QColor buttonColor = palette.button().color();
    int val = qGray(buttonColor.rgb());
    buttonColor = buttonColor.lighter(100 + qMax(1, (180 - val)/6));
    buttonColor.setHsv(buttonColor.hue(), buttonColor.saturation() * 0.75, buttonColor.value());
    if (highlighted)
        buttonColor = mergedColors(buttonColor, highlightedOutline(palette).lighter(130), 90);
    if (!hovered)
        buttonColor = buttonColor.darker(104);
    if (down)
        buttonColor = buttonColor.darker(110);
    return buttonColor;
}

QColor QQuickFusionStyle::buttonOutline(const QPalette &palette, bool highlighted, bool enabled)
{
    QColor darkOutline = enabled && highlighted ? highlightedOutline(palette) : outline(palette);
    return !enabled ? darkOutline.lighter(115) : darkOutline;
}

QColor QQuickFusionStyle::gradientStart(const QColor &baseColor)
{
    return baseColor.lighter(124);
}

QColor QQuickFusionStyle::gradientStop(const QColor &baseColor)
{
    return baseColor.lighter(102);
}

QColor QQuickFusionStyle::mergedColors(const QColor &colorA, const QColor &colorB, int factor)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

QColor QQuickFusionStyle::grooveColor(const QPalette &palette)
{
    QColor color = buttonColor(palette);
    color.setHsv(color.hue(),
                 qMin(255, color.saturation()),
                 qMin<int>(255, color.value() * 0.9));
    return color;
}

QT_END_NAMESPACE
