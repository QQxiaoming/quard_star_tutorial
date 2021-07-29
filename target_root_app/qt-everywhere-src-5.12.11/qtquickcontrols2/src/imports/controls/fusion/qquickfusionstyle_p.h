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

#ifndef QQUICKFUSIONSTYLE_P_H
#define QQUICKFUSIONSTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QQuickFusionStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor lightShade READ lightShade CONSTANT)
    Q_PROPERTY(QColor darkShade READ darkShade CONSTANT)
    Q_PROPERTY(QColor topShadow READ topShadow CONSTANT)
    Q_PROPERTY(QColor innerContrastLine READ innerContrastLine CONSTANT)

public:
    explicit QQuickFusionStyle(QObject *parent = nullptr);

    static QColor lightShade();
    static QColor darkShade();
    static QColor topShadow();
    static QColor innerContrastLine();

    Q_INVOKABLE static QColor highlight(const QPalette &palette);
    Q_INVOKABLE static QColor highlightedText(const QPalette &palette);
    Q_INVOKABLE static QColor outline(const QPalette &palette);
    Q_INVOKABLE static QColor highlightedOutline(const QPalette &palette);
    Q_INVOKABLE static QColor tabFrameColor(const QPalette &palette);
    Q_INVOKABLE static QColor buttonColor(const QPalette &palette, bool highlighted = false, bool down = false, bool hovered = false);
    Q_INVOKABLE static QColor buttonOutline(const QPalette &palette, bool highlighted = false, bool enabled = true);
    Q_INVOKABLE static QColor gradientStart(const QColor &baseColor);
    Q_INVOKABLE static QColor gradientStop(const QColor &baseColor);
    Q_INVOKABLE static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50);
    Q_INVOKABLE static QColor grooveColor(const QPalette &palette);
};

QT_END_NAMESPACE

#endif // QQUICKFUSIONSTYLE_P_H
