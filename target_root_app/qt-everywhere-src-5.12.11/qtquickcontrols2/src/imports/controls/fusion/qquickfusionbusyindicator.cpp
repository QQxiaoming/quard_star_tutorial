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

#include "qquickfusionbusyindicator_p.h"

#include <QtGui/qpainter.h>

QT_BEGIN_NAMESPACE

QQuickFusionBusyIndicator::QQuickFusionBusyIndicator(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

QColor QQuickFusionBusyIndicator::color() const
{
    return m_color;
}

void QQuickFusionBusyIndicator::setColor(const QColor &color)
{
    if (color == m_color)
        return;

    m_color = color;
    update();
}

bool QQuickFusionBusyIndicator::isRunning() const
{
    return isVisible();
}

void QQuickFusionBusyIndicator::setRunning(bool running)
{
    if (running) {
        setVisible(true);
        update();
    }
}

void QQuickFusionBusyIndicator::paint(QPainter *painter)
{
    const qreal w = width();
    const qreal h = height();
    if (w <= 0 || h <= 0 || !isRunning())
        return;

    const qreal sz = qMin(w, h);
    const qreal dx = (w - sz) / 2;
    const qreal dy = (h - sz) / 2;
    const int hpw = qRound(qMax(qreal(1), sz / 14)) & -1;
    const int pw = 2 * hpw;
    const QRectF bounds(dx + hpw, dy + hpw, sz - pw - 1, sz - pw - 1);

    QConicalGradient gradient;
    gradient.setCenter(QPointF(dx + sz / 2, dy + sz / 2));
    gradient.setColorAt(0, m_color);
    gradient.setColorAt(0.1, m_color);
    gradient.setColorAt(1, Qt::transparent);

    painter->translate(0.5, 0.5);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(gradient, pw, Qt::SolidLine));
    painter->drawArc(bounds, 0, 360 * 16);
    painter->setPen(QPen(m_color, pw, Qt::SolidLine, Qt::RoundCap));
    painter->drawArc(bounds, 0, 20 * 16);
}

void QQuickFusionBusyIndicator::itemChange(ItemChange change, const ItemChangeData &data)
{
    QQuickPaintedItem::itemChange(change, data);

    if (change == ItemOpacityHasChanged && qFuzzyIsNull(data.realValue))
        setVisible(false);
}

QT_END_NAMESPACE
