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

#include "qquickcolor_p.h"

QT_BEGIN_NAMESPACE

QQuickColor::QQuickColor(QObject *parent) :
    QObject(parent)
{
}

QColor QQuickColor::transparent(const QColor &color, qreal opacity) const
{
    return QColor(color.red(), color.green(), color.blue(),
                  int(qreal(255) * qBound(qreal(0), opacity, qreal(1))));
}

QColor QQuickColor::blend(const QColor &a, const QColor &b, qreal factor) const
{
    if (factor <= 0.0)
        return a;
    if (factor >= 1.0)
        return b;

    QColor color;
    color.setRedF(a.redF() * (1.0 - factor) + b.redF() * factor);
    color.setGreenF(a.greenF() * (1.0 - factor) + b.greenF() * factor);
    color.setBlueF(a.blueF() * (1.0 - factor) + b.blueF() * factor);
    return color;
}

QT_END_NAMESPACE
