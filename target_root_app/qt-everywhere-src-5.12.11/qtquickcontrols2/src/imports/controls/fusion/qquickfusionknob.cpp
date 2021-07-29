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

#include "qquickfusionknob_p.h"

#include <QtCore/qmath.h>
#include <QtGui/qpainter.h>

QT_BEGIN_NAMESPACE

QQuickFusionKnob::QQuickFusionKnob(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

QPalette QQuickFusionKnob::palette() const
{
    return m_palette;
}

void QQuickFusionKnob::setPalette(const QPalette &palette)
{
    if (palette == m_palette)
        return;

    m_palette = palette;
    update();
}

// extracted from QStyleHelper::drawDial()
void QQuickFusionKnob::paint(QPainter *painter)
{
    const qreal w = width();
    const qreal h = height();
    if (w <= 0 || h <= 0)
        return;

    QColor color = m_palette.color(QPalette::Button);
    color.setHsv(color.hue(),
                 qMin(140, color .saturation()),
                 qMax(180, color.value()));
    color = color.lighter(104);
    color.setAlphaF(qreal(0.8));

    const qreal sz = qMin(w, h);
    QRectF rect(0, 0, sz, sz);
    rect.moveCenter(QPointF(w / 2.0, h / 2.0));
    const QPointF center = rect.center();

    QRadialGradient gradient(center.x() + rect.width() / 2,
                             center.y() + rect.width(),
                             rect.width() * 2,
                             center.x(), center.y());
    gradient.setColorAt(1, color.darker(140));
    gradient.setColorAt(qreal(0.4), color.darker(120));
    gradient.setColorAt(0, color.darker(110));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(gradient);
    painter->setPen(QColor(255, 255, 255, 150));
    painter->drawEllipse(rect);
    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawEllipse(rect.adjusted(1, 1, -1, -1));
}

QT_END_NAMESPACE
