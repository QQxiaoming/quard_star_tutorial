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

#include "drilldownchart.h"
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

DrilldownChart::DrilldownChart(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QChart(QChart::ChartTypeCartesian, parent, wFlags),
      m_currentSeries(0),
      m_axisX(new QBarCategoryAxis()),
      m_axisY(new QValueAxis())
{
      addAxis(m_axisY, Qt::AlignLeft);
      addAxis(m_axisX, Qt::AlignBottom);
}

void DrilldownChart::changeSeries(DrilldownBarSeries *series)
{
    if (m_currentSeries)
        removeSeries(m_currentSeries);

    m_currentSeries = series;

    // Reset axis
    m_axisX->setCategories(m_currentSeries->categories());
    addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);
    m_axisY->setRange(0,m_currentSeries->maxValue());
    setTitle(series->name());
}

void DrilldownChart::handleClicked(int index, QBarSet *barset)
{
    Q_UNUSED(barset)
    DrilldownBarSeries *series = static_cast<DrilldownBarSeries *>(sender());
    changeSeries(series->drilldownSeries(index));
}

#include "moc_drilldownchart.cpp"
