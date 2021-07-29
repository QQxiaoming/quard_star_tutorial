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

#include "qquickmaterialtheme_p.h"
#include "qquickmaterialstyle_p.h"

#include <QtGui/qpa/qplatformdialoghelper.h>
#include <QtGui/qfont.h>
#include <QtGui/qfontinfo.h>
#include <QtQuickTemplates2/private/qquicktheme_p.h>

QT_BEGIN_NAMESPACE

void QQuickMaterialTheme::initialize(QQuickTheme *theme)
{
    QFont systemFont;
    QFont buttonFont;
    QFont toolTipFont;
    QFont itemViewFont;
    QFont listViewFont;
    QFont menuItemFont;
    QFont editorFont;

    QFont font;
    font.setFamily(QLatin1String("Roboto"));
    QString family = QFontInfo(font).family();

    if (family != QLatin1String("Roboto")) {
        font.setFamily(QLatin1String("Noto"));
        family = QFontInfo(font).family();
    }

    if (family == QLatin1String("Roboto") || family == QLatin1String("Noto")) {
        systemFont.setFamily(family);
        buttonFont.setFamily(family);
        toolTipFont.setFamily(family);
        itemViewFont.setFamily(family);
        listViewFont.setFamily(family);
        menuItemFont.setFamily(family);
        editorFont.setFamily(family);
    }

    const bool dense = QQuickMaterialStyle::variant() == QQuickMaterialStyle::Dense;
    systemFont.setPixelSize(dense ? 13 : 14);
    theme->setFont(QQuickTheme::System, systemFont);

    // https://material.io/guidelines/components/buttons.html#buttons-style
    buttonFont.setPixelSize(dense ? 13 : 14);
    buttonFont.setCapitalization(QFont::AllUppercase);
    buttonFont.setWeight(QFont::Medium);
    theme->setFont(QQuickTheme::Button, buttonFont);
    theme->setFont(QQuickTheme::TabBar, buttonFont);
    theme->setFont(QQuickTheme::ToolBar, buttonFont);

    // https://material.io/guidelines/components/tooltips.html
    toolTipFont.setPixelSize(dense ? 10 : 14);
    toolTipFont.setWeight(QFont::Medium);
    theme->setFont(QQuickTheme::ToolTip, toolTipFont);

    itemViewFont.setPixelSize(dense ? 13 : 14);
    itemViewFont.setWeight(QFont::Medium);
    theme->setFont(QQuickTheme::ItemView, itemViewFont);

    // https://material.io/guidelines/components/lists.html#lists-specs
    listViewFont.setPixelSize(dense ? 13 : 16);
    theme->setFont(QQuickTheme::ListView, listViewFont);

    menuItemFont.setPixelSize(dense ? 13 : 16);
    theme->setFont(QQuickTheme::Menu, menuItemFont);
    theme->setFont(QQuickTheme::MenuBar, menuItemFont);
    theme->setFont(QQuickTheme::ComboBox, menuItemFont);

    editorFont.setPixelSize(dense ? 13 : 16);
    theme->setFont(QQuickTheme::TextArea, editorFont);
    theme->setFont(QQuickTheme::TextField, editorFont);
    theme->setFont(QQuickTheme::SpinBox, editorFont);
}

QT_END_NAMESPACE
