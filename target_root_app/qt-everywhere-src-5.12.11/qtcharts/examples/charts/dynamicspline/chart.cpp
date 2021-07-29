/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    m_axisX(new QValueAxis()),
    m_axisY(new QValueAxis()),
    m_step(0),
    m_x(5),
    m_y(1)
{
    QObject::connect(&m_timer, &QTimer::timeout, this, &Chart::handleTimeout);
    m_timer.setInterval(1000);

    m_series = new QSplineSeries(this);
    QPen green(Qt::red);
    green.setWidth(3);
    m_series->setPen(green);
    m_series->append(m_x, m_y);

    addSeries(m_series);

    addAxis(m_axisX,Qt::AlignBottom);
    addAxis(m_axisY,Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
    m_axisX->setTickCount(5);
    m_axisX->setRange(0, 10);
    m_axisY->setRange(-5, 10);

    m_timer.start();
}

Chart::~Chart()
{

}

void Chart::handleTimeout()
{
    qreal x = plotArea().width() / m_axisX->tickCount();
    qreal y = (m_axisX->max() - m_axisX->min()) / m_axisX->tickCount();
    m_x += y;
    m_y = QRandomGenerator::global()->bounded(5) - 2.5;
    m_series->append(m_x, m_y);
    scroll(x, 0);
    if (m_x == 100)
        m_timer.stop();
}
