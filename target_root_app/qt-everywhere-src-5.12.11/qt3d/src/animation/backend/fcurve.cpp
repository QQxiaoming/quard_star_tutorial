/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "fcurve_p.h"
#include <private/bezierevaluator_p.h>

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/QLatin1String>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

FCurve::FCurve()
    : m_rangeFinder(m_localTimes)
{
}

float FCurve::evaluateAtTime(float localTime) const
{
    return evaluateAtTime(localTime, lowerKeyframeBound(localTime));
}


float FCurve::evaluateAtTime(float localTime, int lowerBound) const
{
    // TODO: Implement extrapolation beyond first/last keyframes
    if (localTime < m_localTimes.first()) {
        return m_keyframes.first().value;
    } else if (localTime > m_localTimes.last()) {
        return m_keyframes.last().value;
    } else {
        // Find keyframes that sandwich the requested localTime
        if (lowerBound < 0) // only one keyframe
            return m_keyframes.first().value;

        const float t0 = m_localTimes[lowerBound];
        const float t1 = m_localTimes[lowerBound + 1];
        const Keyframe &keyframe0(m_keyframes[lowerBound]);
        const Keyframe &keyframe1(m_keyframes[lowerBound + 1]);

        switch (keyframe0.interpolation) {
        case QKeyFrame::ConstantInterpolation:
            return keyframe0.value;
        case QKeyFrame::LinearInterpolation:
            if (localTime >= t0 && localTime <= t1 && t1 > t0) {
                float t = (localTime - t0) / (t1 - t0);
                return (1 - t) * keyframe0.value + t * keyframe1.value;
            }
            break;
        case QKeyFrame::BezierInterpolation:
        {
            BezierEvaluator evaluator(t0, keyframe0, t1, keyframe1);
            return evaluator.valueForTime(localTime);
        }
        default:
            qWarning("Unknown interpolation type %d", keyframe0.interpolation);
            break;
        }
    }

    return m_keyframes.first().value;
}

float FCurve::evaluateAtTimeAsSlerp(float localTime, int lowerBound, float halfTheta, float sinHalfTheta, float reverseQ1) const
{
    // TODO: Implement extrapolation beyond first/last keyframes
    if (localTime < m_localTimes.first())
        return m_keyframes.first().value;

    if (localTime > m_localTimes.last())
        return m_keyframes.last().value;
    // Find keyframes that sandwich the requested localTime
    if (lowerBound < 0) // only one keyframe
        return m_keyframes.first().value;

    const float t0 = m_localTimes[lowerBound];
    const float t1 = m_localTimes[lowerBound + 1];
    const Keyframe &keyframe0(m_keyframes[lowerBound]);
    const Keyframe &keyframe1(m_keyframes[lowerBound + 1]);

    switch (keyframe0.interpolation) {
    case QKeyFrame::ConstantInterpolation:
        return keyframe0.value;
    case QKeyFrame::LinearInterpolation:
        if (localTime >= t0 && localTime <= t1 && t1 > t0) {
            const auto t = (localTime - t0) / (t1 - t0);

            const auto A = std::sin((1.0f-t) * halfTheta) / sinHalfTheta;
            const auto B = std::sin(t * halfTheta) / sinHalfTheta;
            return A * keyframe0.value + reverseQ1 * B * keyframe1.value;
        }
        break;
    case QKeyFrame::BezierInterpolation:
        // TODO implement a proper slerp bezier interpolation
        BezierEvaluator evaluator(t0, keyframe0, t1, keyframe1);
        return evaluator.valueForTime(localTime);
    }

    return m_keyframes.first().value;
}

int FCurve::lowerKeyframeBound(float localTime) const
{
    if (localTime < m_localTimes.first())
        return 0;
    if (localTime > m_localTimes.last())
        return 0;
    return m_rangeFinder.findLowerBound(localTime);
}

float FCurve::startTime() const
{
    if (!m_localTimes.isEmpty())
        return m_localTimes.first();
    return 0.0f;
}

float FCurve::endTime() const
{
    if (!m_localTimes.isEmpty())
        return m_localTimes.last();
    return 0.0f;
}

void FCurve::appendKeyframe(float localTime, const Keyframe &keyframe)
{
    m_localTimes.append(localTime);
    m_keyframes.append(keyframe);
}

void FCurve::read(const QJsonObject &json)
{
    clearKeyframes();

    const QJsonArray keyframeArray = json[QLatin1String("keyFrames")].toArray();
    const int keyframeCount = keyframeArray.size();

    for (int i = 0; i < keyframeCount; ++i) {
        const QJsonObject keyframeData = keyframeArray.at(i).toObject();

        // Extract the keyframe local time and value
        const QJsonArray keyframeCoords = keyframeData[QLatin1String("coords")].toArray();
        float localTime = keyframeCoords.at(0).toDouble();

        Keyframe keyframe;
        keyframe.value = keyframeCoords.at(1).toDouble();

        if (keyframeData.contains(QLatin1String("leftHandle"))) {
            keyframe.interpolation = QKeyFrame::BezierInterpolation;

            const QJsonArray leftHandle = keyframeData[QLatin1String("leftHandle")].toArray();
            keyframe.leftControlPoint[0] = leftHandle.at(0).toDouble();
            keyframe.leftControlPoint[1] = leftHandle.at(1).toDouble();

            const QJsonArray rightHandle = keyframeData[QLatin1String("rightHandle")].toArray();
            keyframe.rightControlPoint[0] = rightHandle.at(0).toDouble();
            keyframe.rightControlPoint[1] = rightHandle.at(1).toDouble();
        } else {
            keyframe.interpolation = QKeyFrame::LinearInterpolation;
        }

        appendKeyframe(localTime, keyframe);
    }

    // TODO: Ensure beziers have no loops or cusps by scaling the control points
    // back so they do not interset.
}

void FCurve::setFromQChannelComponent(const QChannelComponent &qcc)
{
    clearKeyframes();

    for (const auto &frontendKeyFrame : qcc) {
        // Extract the keyframe local time and value
        const float localTime = frontendKeyFrame.coordinates()[0];

        Keyframe keyFrame;
        keyFrame.interpolation = frontendKeyFrame.interpolationType();
        keyFrame.value = frontendKeyFrame.coordinates()[1];
        keyFrame.leftControlPoint = frontendKeyFrame.leftControlPoint();
        keyFrame.rightControlPoint = frontendKeyFrame.rightControlPoint();
        appendKeyframe(localTime, keyFrame);
    }

    // TODO: Ensure beziers have no loops or cusps by scaling the control points
    // back so they do not interset.
}

void ChannelComponent::read(const QJsonObject &json)
{
    name = json[QLatin1String("channelComponentName")].toString();
    fcurve.read(json);
}

void ChannelComponent::setFromQChannelComponent(const QChannelComponent &qcc)
{
    name = qcc.name();
    fcurve.setFromQChannelComponent(qcc);
}

void Channel::read(const QJsonObject &json)
{
    name = json[QLatin1String("channelName")].toString();
    const auto jointIndexValue = json[QLatin1String("jointIndex")];
    if (!jointIndexValue.isUndefined())
        jointIndex = jointIndexValue.toInt();
    const QJsonArray channelComponentsArray = json[QLatin1String("channelComponents")].toArray();
    const int channelCount = channelComponentsArray.size();
    channelComponents.resize(channelCount);

    for (int i = 0; i < channelCount; ++i) {
        const QJsonObject channel = channelComponentsArray.at(i).toObject();
        channelComponents[i].read(channel);
    }
}

void Channel::setFromQChannel(const QChannel &qch)
{
    name = qch.name();
    jointIndex = qch.jointIndex();
    channelComponents.resize(qch.channelComponentCount());
    int i = 0;
    for (const auto &frontendChannelComponent : qch)
        channelComponents[i++].setFromQChannelComponent(frontendChannelComponent);
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
