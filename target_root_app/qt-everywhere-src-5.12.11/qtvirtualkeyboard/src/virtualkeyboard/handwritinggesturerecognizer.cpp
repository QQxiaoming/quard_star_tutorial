/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
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

#include <QtVirtualKeyboard/private/handwritinggesturerecognizer_p.h>

#include <QtCore/qmath.h>
#include <QVector2D>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

HandwritingGestureRecognizer::HandwritingGestureRecognizer(QObject *parent) :
    GestureRecognizer(parent),
    m_dpi(96)
{
}

void HandwritingGestureRecognizer::setDpi(int value)
{
    m_dpi = value >= 0 ? value : 96;
}

int HandwritingGestureRecognizer::dpi() const
{
    return m_dpi;
}

QVariantMap HandwritingGestureRecognizer::recognize(const QList<QVirtualKeyboardTrace *> traceList)
{
    if (traceList.count() > 0 && traceList.count() < 3) {

        // Swipe gesture detection
        // =======================
        //
        // The following algorithm is based on the assumption that a
        // vector composed of two arbitrary selected, but consecutive
        // measuring points, and a vector composed of the first and last
        // of the measuring points, are approximately in the same angle.
        //
        // If the measuring points are located very close to each other,
        // the angle can fluctuate a lot. This has been taken into account
        // by setting a minimum Euclidean distance between the measuring
        // points.
        //

        // Minimum euclidean distance of a segment (in millimeters)
        static const int MINIMUM_EUCLIDEAN_DISTANCE = 8;

        // Maximum theta variance (in degrees)
        static const qreal THETA_THRESHOLD = 25.0;

        // Maximum width variance in multitouch swipe (+- in percent)
        static const int MAXIMUM_WIDTH_VARIANCE = 20;

        const qreal minimumEuclideanDistance = MINIMUM_EUCLIDEAN_DISTANCE / 25.4 * m_dpi;
        static const qreal thetaThreshold = qDegreesToRadians(THETA_THRESHOLD);

        QList<QVector2D> swipeVectors;

        int traceIndex;
        const int traceCount = traceList.size();
        for (traceIndex = 0; traceIndex < traceCount; ++traceIndex) {

            const QVirtualKeyboardTrace *trace = traceList.at(traceIndex);
            const QVariantList &points = trace->points();
            QVector2D swipeVector;
            const int pointCount = points.count();
            int pointIndex = 0;
            if (pointCount >= 2) {

                QPointF startPosition = points.first().toPointF();
                swipeVector = QVector2D(points.last().toPointF() - startPosition);
                const qreal swipeLength = swipeVector.length();

                if (swipeLength >= minimumEuclideanDistance) {

                    QPointF previousPosition = startPosition;
                    qreal euclideanDistance = 0;
                    for (pointIndex = 1; pointIndex < pointCount; ++pointIndex) {

                        QPointF currentPosition(points.at(pointIndex).toPointF());

                        euclideanDistance += QVector2D(currentPosition - previousPosition).length();
                        if (euclideanDistance >= minimumEuclideanDistance) {

                            // Set the angle (theta) between the sample vector and the swipe vector
                            const QVector2D sampleVector(currentPosition - startPosition);
                            const qreal theta = qAcos(QVector2D::dotProduct(swipeVector, sampleVector) / (swipeLength * sampleVector.length()));

                            // Rejected when theta above threshold
                            if (theta >= thetaThreshold) {
                                swipeVector = QVector2D();
                                break;
                            }

                            startPosition = currentPosition;
                            euclideanDistance = 0;
                        }

                        previousPosition = currentPosition;
                    }

                    if (pointIndex < pointCount) {
                        swipeVector = QVector2D();
                        break;
                    }

                    // Check to see if angle and length matches to existing touch points
                    if (!swipeVectors.isEmpty()) {
                        bool matchesToExisting = true;
                        const qreal minimumSwipeLength = (swipeLength * (100.0 - MAXIMUM_WIDTH_VARIANCE) / 100.0);
                        const qreal maximumSwipeLength = (swipeLength * (100.0 + MAXIMUM_WIDTH_VARIANCE) / 100.0);
                        for (const QVector2D &otherSwipeVector : qAsConst(swipeVectors)) {
                            const qreal otherSwipeLength = otherSwipeVector.length();
                            const qreal theta = qAcos(QVector2D::dotProduct(swipeVector, otherSwipeVector) / (swipeLength * otherSwipeLength));

                            if (theta >= thetaThreshold) {
                                matchesToExisting = false;
                                break;
                            }

                            if (otherSwipeLength < minimumSwipeLength || otherSwipeLength > maximumSwipeLength) {
                                matchesToExisting = false;
                                break;
                            }
                        }

                        if (!matchesToExisting) {
                            swipeVector = QVector2D();
                            break;
                        }
                    }
                } else {
                    swipeVector = QVector2D();
                }
            }

            if (swipeVector.isNull())
                break;

            swipeVectors.append(swipeVector);
        }

        if (swipeVectors.size() == traceCount) {

            QVariantMap swipeGesture;

            // Get swipe angle from the first vector:
            //    0 degrees == right
            //   90 degrees == down
            //  180 degrees == left
            //  270 degrees == up
            QList<QVector2D>::ConstIterator swipeVector = swipeVectors.constBegin();
            qreal swipeLength = swipeVector->length();
            qreal swipeAngle = qAcos(swipeVector->x() / swipeLength);
            if (swipeVector->y() < 0)
                swipeAngle = 2 * M_PI - swipeAngle;

            // Calculate an average length of the vector
            ++swipeVector;
            for (const auto cend = swipeVectors.cend(); swipeVector != cend; ++swipeVector)
                swipeLength += swipeVector->length();
            swipeLength /= traceCount;

            swipeGesture[QLatin1String("type")] = QLatin1String("swipe");
            swipeGesture[QLatin1String("angle")] = swipeAngle;
            swipeGesture[QLatin1String("angle_degrees")] = qRadiansToDegrees(swipeAngle);
            swipeGesture[QLatin1String("length")] = swipeLength;
            swipeGesture[QLatin1String("length_mm")] = swipeLength / m_dpi * 25.4;
            swipeGesture[QLatin1String("touch_count")] = traceCount;

            return swipeGesture;
        }
    }

    return QVariantMap();
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
