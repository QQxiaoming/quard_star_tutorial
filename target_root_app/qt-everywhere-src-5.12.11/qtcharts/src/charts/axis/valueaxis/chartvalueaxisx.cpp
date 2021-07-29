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

#include <private/chartvalueaxisx_p.h>
#include <QtCharts/QAbstractAxis>
#include <private/chartpresenter_p.h>
#include <QtCharts/QValueAxis>
#include <private/abstractchartlayout_p.h>
#include <QtWidgets/QGraphicsLayout>
#include <QtCore/QtMath>
#include <QtCore/QDebug>


QT_CHARTS_BEGIN_NAMESPACE

ChartValueAxisX::ChartValueAxisX(QValueAxis *axis, QGraphicsItem *item )
    : HorizontalAxis(axis, item),
      m_axis(axis)
{
    QObject::connect(m_axis, SIGNAL(tickCountChanged(int)), this, SLOT(handleTickCountChanged(int)));
    QObject::connect(m_axis, SIGNAL(minorTickCountChanged(int)),
                     this, SLOT(handleMinorTickCountChanged(int)));
    QObject::connect(m_axis, SIGNAL(labelFormatChanged(QString)), this, SLOT(handleLabelFormatChanged(QString)));
    QObject::connect(m_axis, SIGNAL(tickIntervalChanged(qreal)), this, SLOT(handleTickIntervalChanged(qreal)));
    QObject::connect(m_axis, SIGNAL(tickAnchorChanged(qreal)), this, SLOT(handleTickAnchorChanged(qreal)));
    QObject::connect(m_axis, SIGNAL(tickTypeChanged(QValueAxis::TickType)), this,
                     SLOT(handleTickTypeChanged(QValueAxis::TickType)));
}

ChartValueAxisX::~ChartValueAxisX()
{
}

QVector<qreal> ChartValueAxisX::calculateLayout() const
{
    if (m_axis->tickType() == QValueAxis::TicksFixed) {
        int tickCount = m_axis->tickCount();

        Q_ASSERT(tickCount >= 2);

        QVector<qreal> points;
        points.resize(tickCount);

        const QRectF &gridRect = gridGeometry();
        const qreal deltaX = gridRect.width() / (qreal(tickCount) - 1.0);
        for (int i = 0; i < tickCount; ++i)
            points[i] = qreal(i) * deltaX + gridRect.left();
        return points;
    } else { // QValueAxis::TicksDynamic
        const qreal interval = m_axis->tickInterval();
        qreal value = m_axis->tickAnchor();
        const qreal maxValue = max();
        const qreal minValue = min();

        // Find the first major tick right after the min of range
        if (value > minValue)
            value = value - int((value - minValue) / interval) * interval;
        else
            value = value + qCeil((minValue - value) / interval) * interval;

        const QRectF &gridRect = gridGeometry();
        const qreal deltaX = gridRect.width() / (maxValue - minValue);

        QVector<qreal> points;
        const qreal leftPos = gridRect.left();
        while (value <= maxValue || qFuzzyCompare(value, maxValue)) {
            points << (value - minValue) * deltaX + leftPos;
            value += interval;
        }

        return points;
    }
}

void ChartValueAxisX::updateGeometry()
{
    const QVector<qreal>& layout = ChartAxisElement::layout();
    const QVector<qreal>& dynamicMinorTicklayout = ChartAxisElement::dynamicMinorTicklayout();
    if (layout.isEmpty() && dynamicMinorTicklayout.isEmpty())
        return;
    setLabels(createValueLabels(min(), max(), layout.size(), m_axis->tickInterval(),
                                m_axis->tickAnchor(), m_axis->tickType(), m_axis->labelFormat()));
    HorizontalAxis::updateGeometry();
}

void ChartValueAxisX::handleTickCountChanged(int tick)
{
    Q_UNUSED(tick);
    QGraphicsLayoutItem::updateGeometry();
    if (presenter()) presenter()->layout()->invalidate();
}

void ChartValueAxisX::handleMinorTickCountChanged(int tick)
{
    Q_UNUSED(tick);
    QGraphicsLayoutItem::updateGeometry();
    if (presenter())
        presenter()->layout()->invalidate();
}

void ChartValueAxisX::handleLabelFormatChanged(const QString &format)
{
    Q_UNUSED(format);
    QGraphicsLayoutItem::updateGeometry();
    if (presenter()) presenter()->layout()->invalidate();
}

void ChartValueAxisX::handleTickIntervalChanged(qreal interval)
{
    Q_UNUSED(interval)
    QGraphicsLayoutItem::updateGeometry();
    if (presenter()) presenter()->layout()->invalidate();
}

void ChartValueAxisX::handleTickAnchorChanged(qreal anchor)
{
    Q_UNUSED(anchor)
    QGraphicsLayoutItem::updateGeometry();
    if (presenter()) presenter()->layout()->invalidate();
}

void ChartValueAxisX::handleTickTypeChanged(QValueAxis::TickType type)
{
    Q_UNUSED(type)
    QGraphicsLayoutItem::updateGeometry();
    if (presenter()) presenter()->layout()->invalidate();
}

QSizeF ChartValueAxisX::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(constraint)

    QSizeF sh;

    QSizeF base = HorizontalAxis::sizeHint(which, constraint);
    QStringList ticksList = createValueLabels(min(), max(), m_axis->tickCount(),
                                              m_axis->tickInterval(), m_axis->tickAnchor(),
                                              m_axis->tickType(), m_axis->labelFormat());
    // Width of horizontal axis sizeHint indicates the maximum distance labels can extend past
    // first and last ticks. Base width is irrelevant.
    qreal width = 0;
    qreal height = 0;

    switch (which) {
        case Qt::MinimumSize: {
            QRectF boundingRect = ChartPresenter::textBoundingRect(axis()->labelsFont(),
                                                                   QStringLiteral("..."),
                                                                   axis()->labelsAngle());
            width = boundingRect.width() / 2.0;
            height = boundingRect.height() + labelPadding() + base.height() + 1.0;
            sh = QSizeF(width, height);
            break;
        }
        case Qt::PreferredSize: {
            qreal labelHeight = 0.0;
            qreal firstWidth = -1.0;
            foreach (const QString& s, ticksList) {
                QRectF rect = ChartPresenter::textBoundingRect(axis()->labelsFont(), s, axis()->labelsAngle());
                labelHeight = qMax(rect.height(), labelHeight);
                width = rect.width();
                if (firstWidth < 0.0)
                    firstWidth = width;
            }
            height = labelHeight + labelPadding() + base.height() + 1.0;
            width = qMax(width, firstWidth) / 2.0;
            sh = QSizeF(width, height);
            break;
        }
        default:
            break;
    }
    return sh;
}

QT_CHARTS_END_NAMESPACE

#include "moc_chartvalueaxisx_p.cpp"
