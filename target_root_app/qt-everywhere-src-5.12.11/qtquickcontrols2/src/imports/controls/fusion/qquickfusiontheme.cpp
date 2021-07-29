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

#include "qquickfusiontheme_p.h"

#include <QtQuickTemplates2/private/qquicktheme_p.h>
#include <QtQuickControls2/private/qquickstyle_p.h>

QT_BEGIN_NAMESPACE

void QQuickFusionTheme::initialize(QQuickTheme *theme)
{
    const bool isDarkSystemTheme = QQuickStylePrivate::isDarkSystemTheme();
    QPalette systemPalette;
    systemPalette.setColor(QPalette::Active, QPalette::ButtonText,
        isDarkSystemTheme ? QColor::fromRgb(0xe7e7e7) : QColor::fromRgb(0x252525));
    systemPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
        isDarkSystemTheme ? QColor::fromRgb(0x777777) : QColor::fromRgb(0xb6b6b6));
    theme->setPalette(QQuickTheme::System, systemPalette);
}

QT_END_NAMESPACE
