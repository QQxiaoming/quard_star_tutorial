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

#include "animationutils_p.h"
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/clipblendnode_p.h>
#include <Qt3DAnimation/private/clipblendnodevisitor_p.h>
#include <Qt3DAnimation/private/clipblendvalue_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qpropertyupdatedchangebase_p.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qcolor.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvarlengtharray.h>
#include <Qt3DAnimation/private/animationlogging_p.h>

#include <numeric>

QT_BEGIN_NAMESPACE

namespace {
const auto slerpThreshold = 0.01f;
}

namespace Qt3DAnimation {
namespace Animation {

inline QVector<float> valueToVector(const QVector3D &value)
{
    return { value.x(), value.y(), value.z() };
}

inline QVector<float> valueToVector(const QQuaternion &value)
{
    return { value.scalar(), value.x(), value.y(), value.z() };
}

ClipEvaluationData evaluationDataForClip(AnimationClip *clip,
                                         const AnimatorEvaluationData &animatorData)
{
    // global time values expected in seconds
    ClipEvaluationData result;
    result.currentLoop = animatorData.currentLoop;
    result.localTime = localTimeFromElapsedTime(animatorData.currentTime, animatorData.elapsedTime,
                                                animatorData.playbackRate, clip->duration(),
                                                animatorData.loopCount, result.currentLoop);
    result.isFinalFrame = isFinalFrame(result.localTime, clip->duration(),
                                       result.currentLoop, animatorData.loopCount);
    const bool hasNormalizedTime = isValidNormalizedTime(animatorData.normalizedLocalTime);
    result.normalizedLocalTime = hasNormalizedTime ? animatorData.normalizedLocalTime
                                                   : result.localTime / clip->duration();
    return result;
}

double localTimeFromElapsedTime(double t_current_local,
                                double t_elapsed_global,
                                double playbackRate,
                                double duration,
                                int loopCount,
                                int &currentLoop)
{
    // Calculate the new local time.
    // playhead + rate * dt
    // where playhead is completed loops * duration + current loop local time
    double t_local = currentLoop * duration + t_current_local + playbackRate * t_elapsed_global;
    double loopNumber = 0;
    if (loopCount == 1) {
        t_local = qBound(0.0, t_local, duration);
    } else if (loopCount < 0) {
        // Loops forever
        (void) std::modf(t_local / duration, &loopNumber);
        t_local = std::fmod(t_local, duration);
    } else {
        // N loops
        t_local = qBound(0.0, t_local, double(loopCount) * duration);
        (void) std::modf(t_local / duration, &loopNumber);
        t_local = std::fmod(t_local, duration);

        // Ensure we clamp to end of final loop
        if (int(loopNumber) == loopCount) {
            loopNumber = loopCount - 1;
            t_local = duration;
        }
    }

    qCDebug(Jobs) << "current loop =" << loopNumber
                  << "t =" << t_local
                  << "duration =" << duration;

    currentLoop = int(loopNumber);

    return t_local;
}

double phaseFromElapsedTime(double t_current_local,
                            double t_elapsed_global,
                            double playbackRate,
                            double duration,
                            int loopCount,
                            int &currentLoop)
{
    const double t_local = localTimeFromElapsedTime(t_current_local, t_elapsed_global, playbackRate,
                                                    duration, loopCount, currentLoop);
    return t_local / duration;
}

/*!
    \internal

    Calculates the indices required to map from the component ordering within the
    provided \a channel, into the standard channel orderings expected by Qt types.

    For example, given a channel representing a rotation with the components ordered
    as X, Y, Z, Y, this function will return the indices [3, 0, 1, 2] which can then
    later be used as part of the format vector in the formatClipResults() function to
    remap the channels into the standard W, X, Y, Z order required by QQuaternion.
*/
ComponentIndices channelComponentsToIndices(const Channel &channel,
                                            int dataType,
                                            int expectedComponentCount,
                                            int offset)
{
#if defined Q_COMPILER_UNIFORM_INIT
    static const QVector<char> standardSuffixes = { 'X', 'Y', 'Z', 'W' };
    static const QVector<char> quaternionSuffixes = { 'W', 'X', 'Y', 'Z' };
    static const QVector<char> colorSuffixesRGB = { 'R', 'G', 'B' };
    static const QVector<char> colorSuffixesRGBA = { 'R', 'G', 'B', 'A' };
#else
    static const QVector<char> standardSuffixes = (QVector<char>() << 'X' << 'Y' << 'Z' << 'W');
    static const QVector<char> quaternionSuffixes = (QVector<char>() << 'W' << 'X' << 'Y' << 'Z');
    static const QVector<char> colorSuffixesRGB = (QVector<char>() << 'R' << 'G' << 'B');
    static const QVector<char> colorSuffixesRGBA = (QVector<char>() << 'R' << 'G' << 'B' << 'A');
#endif

    switch (dataType) {
    case QVariant::Quaternion:
        return channelComponentsToIndicesHelper(channel, expectedComponentCount,
                                                offset, quaternionSuffixes);
    case QVariant::Color:
        if (expectedComponentCount == 3)
            return channelComponentsToIndicesHelper(channel, expectedComponentCount,
                                                    offset, colorSuffixesRGB);
        Q_ASSERT(expectedComponentCount == 4);
        return channelComponentsToIndicesHelper(channel, expectedComponentCount,
                                                offset, colorSuffixesRGBA);
    default:
        return channelComponentsToIndicesHelper(channel, expectedComponentCount,
                                                offset, standardSuffixes);
    }
}

ComponentIndices channelComponentsToIndicesHelper(const Channel &channel,
                                                  int expectedComponentCount,
                                                  int offset,
                                                  const QVector<char> &suffixes)
{
    const int actualComponentCount = channel.channelComponents.size();
    if (actualComponentCount != expectedComponentCount) {
        qWarning() << "Data type expects" << expectedComponentCount
                   << "but found" << actualComponentCount << "components in the animation clip";
    }

    ComponentIndices indices(expectedComponentCount);

    // Generate the set of channel suffixes
    QVector<char> channelSuffixes;
    channelSuffixes.reserve(expectedComponentCount);
    for (int i = 0; i < expectedComponentCount; ++i) {
        const QString &componentName = channel.channelComponents[i].name;

        // An unset component name indicates that the no mapping is necessary
        // and the index can be used as-is.
        if (componentName.isEmpty()) {
            indices[i] = i + offset;
            continue;
        }

        char channelSuffix = componentName.at(componentName.length() - 1).toLatin1();
        channelSuffixes.push_back(channelSuffix);
    }

    // We can short-circuit if the channels were all unnamed (in order)
    if (channelSuffixes.isEmpty())
        return indices;

    // Find index of standard index in channel indexes
    for (int i = 0; i < expectedComponentCount; ++i) {
        int index = channelSuffixes.indexOf(suffixes[i]);
        if (index != -1)
            indices[i] = index + offset;
        else
            indices[i] = -1;
    }

    return indices;
}

ClipResults evaluateClipAtLocalTime(AnimationClip *clip, float localTime)
{
    QVector<float> channelResults;
    Q_ASSERT(clip);

    // Ensure we have enough storage to hold the evaluations
    channelResults.resize(clip->channelCount());

    // Iterate over channels and evaluate the fcurves
    const QVector<Channel> &channels = clip->channels();
    int i = 0;
    for (const Channel &channel : channels) {
        if (channel.name.contains(QStringLiteral("Rotation")) &&
                        channel.channelComponents.size() == 4) {

            // Try to SLERP
            const int nbKeyframes = channel.channelComponents[0].fcurve.keyframeCount();
            const bool canSlerp = std::find_if(std::begin(channel.channelComponents)+1,
                                               std::end(channel.channelComponents),
                                               [nbKeyframes](const ChannelComponent &v) {
                return v.fcurve.keyframeCount() != nbKeyframes;
            }) == std::end(channel.channelComponents);

            if (!canSlerp) {
                // Interpolate per component
                for (const auto &channelComponent : qAsConst(channel.channelComponents)) {
                    const int lowerKeyframeBound = channelComponent.fcurve.lowerKeyframeBound(localTime);
                    channelResults[i++] = channelComponent.fcurve.evaluateAtTime(localTime, lowerKeyframeBound);
                }
            } else {
                // There's only one keyframe. We cant compute omega. Interpolate per component
                if (channel.channelComponents[0].fcurve.keyframeCount() == 1) {
                    for (const auto &channelComponent : qAsConst(channel.channelComponents))
                        channelResults[i++] = channelComponent.fcurve.keyframe(0).value;
                } else {
                    auto quaternionFromChannel = [channel](const int keyframe) {
                        const float w = channel.channelComponents[0].fcurve.keyframe(keyframe).value;
                        const float x = channel.channelComponents[1].fcurve.keyframe(keyframe).value;
                        const float y = channel.channelComponents[2].fcurve.keyframe(keyframe).value;
                        const float z = channel.channelComponents[3].fcurve.keyframe(keyframe).value;
                        QQuaternion quat{w,x,y,z};
                        quat.normalize();
                        return quat;
                    };

                    const int lowerKeyframeBound = std::max(0, channel.channelComponents[0].fcurve.lowerKeyframeBound(localTime));
                    const auto lowerQuat = quaternionFromChannel(lowerKeyframeBound);
                    const auto higherQuat = quaternionFromChannel(lowerKeyframeBound + 1);
                    auto cosHalfTheta = QQuaternion::dotProduct(lowerQuat, higherQuat);
                    // If the two keyframe quaternions are equal, just return the first one as the interpolated value.
                    if (std::abs(cosHalfTheta) >= 1.0f) {
                        channelResults[i++] = lowerQuat.scalar();
                        channelResults[i++] = lowerQuat.x();
                        channelResults[i++] = lowerQuat.y();
                        channelResults[i++] = lowerQuat.z();
                    } else {
                        const auto sinHalfTheta = std::sqrt(1.0f - std::pow(cosHalfTheta,2.0f));
                        if (std::abs(sinHalfTheta) < ::slerpThreshold) {
                            auto initial_i = i;
                            for (const auto &channelComponent : qAsConst(channel.channelComponents))
                                channelResults[i++] = channelComponent.fcurve.evaluateAtTime(localTime, lowerKeyframeBound);

                            // Normalize the resulting quaternion
                            QQuaternion quat{channelResults[initial_i], channelResults[initial_i+1], channelResults[initial_i+2], channelResults[initial_i+3]};
                            quat.normalize();
                            channelResults[initial_i+0] = quat.scalar();
                            channelResults[initial_i+1] = quat.x();
                            channelResults[initial_i+2] = quat.y();
                            channelResults[initial_i+3] = quat.z();
                        } else {
                            const auto reverseQ1 = cosHalfTheta < 0 ? -1.0f : 1.0f;
                            cosHalfTheta *= reverseQ1;
                            const auto halfTheta = std::acos(cosHalfTheta);
                            for (const auto &channelComponent : qAsConst(channel.channelComponents))
                                channelResults[i++] = channelComponent.fcurve.evaluateAtTimeAsSlerp(localTime,
                                                                                                    lowerKeyframeBound,
                                                                                                    halfTheta,
                                                                                                    sinHalfTheta,
                                                                                                    reverseQ1);
                        }
                    }
                }
            }
        } else {
            // If the channel is not a Rotation, apply linear interpolation per channel component
            // TODO How do we handle other interpolations. For exammple, color interpolation
            // in a linear perceptual way or other non linear spaces?
            for (const auto &channelComponent : qAsConst(channel.channelComponents)) {
                const int lowerKeyframeBound = channelComponent.fcurve.lowerKeyframeBound(localTime);
                channelResults[i++] = channelComponent.fcurve.evaluateAtTime(localTime, lowerKeyframeBound);
            }
        }
    }
    return channelResults;
}

ClipResults evaluateClipAtPhase(AnimationClip *clip, float phase)
{
    // Calculate the clip local time from the phase and clip duration
    const double localTime = phase * clip->duration();
    return evaluateClipAtLocalTime(clip, localTime);
}

template<typename Container>
Container mapChannelResultsToContainer(const MappingData &mappingData,
                                       const QVector<float> &channelResults)
{
    Container r;
    r.reserve(channelResults.size());

    const ComponentIndices channelIndices = mappingData.channelIndices;
    for (const int channelIndex : channelIndices)
        r.push_back(channelResults.at(channelIndex));

    return r;
}

QVariant buildPropertyValue(const MappingData &mappingData, const QVector<float> &channelResults)
{
    const int vectorOfFloatType = qMetaTypeId<QVector<float>>();

    if (mappingData.type == vectorOfFloatType)
        return QVariant::fromValue(channelResults);

    switch (mappingData.type) {
    case QMetaType::Float:
    case QVariant::Double: {
        return QVariant::fromValue(channelResults[mappingData.channelIndices[0]]);
    }

    case QVariant::Vector2D: {
        const QVector2D vector(channelResults[mappingData.channelIndices[0]],
                channelResults[mappingData.channelIndices[1]]);
        return QVariant::fromValue(vector);
    }

    case QVariant::Vector3D: {
        const QVector3D vector(channelResults[mappingData.channelIndices[0]],
                channelResults[mappingData.channelIndices[1]],
                channelResults[mappingData.channelIndices[2]]);
        return QVariant::fromValue(vector);
    }

    case QVariant::Vector4D: {
        const QVector4D vector(channelResults[mappingData.channelIndices[0]],
                channelResults[mappingData.channelIndices[1]],
                channelResults[mappingData.channelIndices[2]],
                channelResults[mappingData.channelIndices[3]]);
        return QVariant::fromValue(vector);
    }

    case QVariant::Quaternion: {
        QQuaternion q(channelResults[mappingData.channelIndices[0]],
                channelResults[mappingData.channelIndices[1]],
                channelResults[mappingData.channelIndices[2]],
                channelResults[mappingData.channelIndices[3]]);
        q.normalize();
        return QVariant::fromValue(q);
    }

    case QVariant::Color: {
        // A color can either be a vec3 or a vec4
        const QColor color =
                QColor::fromRgbF(channelResults[mappingData.channelIndices[0]],
                channelResults[mappingData.channelIndices[1]],
                channelResults[mappingData.channelIndices[2]],
                mappingData.channelIndices.size() > 3 ? channelResults[mappingData.channelIndices[3]] : 1.0f);
        return QVariant::fromValue(color);
    }

    case QVariant::List: {
        const QVariantList results = mapChannelResultsToContainer<QVariantList>(
                    mappingData, channelResults);
        return QVariant::fromValue(results);
    }
    default:
        qWarning() << "Unhandled animation type" << mappingData.type;
        break;
    }

    return QVariant();
}

QVector<Qt3DCore::QSceneChangePtr> preparePropertyChanges(Qt3DCore::QNodeId animatorId,
                                                          const QVector<MappingData> &mappingDataVec,
                                                          const QVector<float> &channelResults,
                                                          bool finalFrame,
                                                          float normalizedLocalTime)
{
    QVector<Qt3DCore::QSceneChangePtr> changes;
    QVarLengthArray<Skeleton *, 4> dirtySkeletons;

    // Iterate over the mappings
    for (const MappingData &mappingData : mappingDataVec) {
        if (!mappingData.propertyName)
            continue;

        // Build the new value from the channel/fcurve evaluation results
        const QVariant v = buildPropertyValue(mappingData, channelResults);
        if (!v.isValid())
            continue;

        // TODO: Avoid wrapping joint transform components up in a variant, just
        // to immediately unwrap them again. Refactor buildPropertyValue() to call
        // helper functions that we can call directly here for joints.
        if (mappingData.skeleton && mappingData.jointIndex != -1) {
            // Remember that this skeleton is dirty. We will ask each dirty skeleton
            // to send its set of local poses to observers below.
            if (!dirtySkeletons.contains(mappingData.skeleton))
                dirtySkeletons.push_back(mappingData.skeleton);

            switch (mappingData.jointTransformComponent) {
            case Scale:
                mappingData.skeleton->setJointScale(mappingData.jointIndex, v.value<QVector3D>());
                break;

            case Rotation:
                mappingData.skeleton->setJointRotation(mappingData.jointIndex, v.value<QQuaternion>());
                break;

            case Translation:
                mappingData.skeleton->setJointTranslation(mappingData.jointIndex, v.value<QVector3D>());
                break;

            default:
                Q_UNREACHABLE();
                break;
            }
        } else {
            // Construct a property update change, set target, property and delivery options
            auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(mappingData.targetId);
            e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            e->setPropertyName(mappingData.propertyName);
            // Handle intermediate updates vs final flag properly
            Qt3DCore::QPropertyUpdatedChangeBasePrivate::get(e.data())->m_isIntermediate = !finalFrame;
            // Assign new value and send
            e->setValue(v);
            changes.push_back(e);
        }
    }

    for (const auto skeleton : dirtySkeletons)
        skeleton->sendLocalPoses();

    if (isValidNormalizedTime(normalizedLocalTime)) {
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(animatorId);
        e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
        e->setPropertyName("normalizedTime");
        e->setValue(normalizedLocalTime);
        Qt3DCore::QPropertyUpdatedChangeBasePrivate::get(e.data())->m_isIntermediate = !finalFrame;
        changes.push_back(e);
    }

    // If it's the final frame, notify the frontend that we've stopped
    if (finalFrame) {
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(animatorId);
        e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
        e->setPropertyName("running");
        e->setValue(false);
        changes.push_back(e);
    }

    return changes;
}

QVector<AnimationCallbackAndValue> prepareCallbacks(const QVector<MappingData> &mappingDataVec,
                                                    const QVector<float> &channelResults)
{
    QVector<AnimationCallbackAndValue> callbacks;
    for (const MappingData &mappingData : mappingDataVec) {
        if (!mappingData.callback)
            continue;
        const QVariant v = buildPropertyValue(mappingData, channelResults);
        if (v.isValid()) {
            AnimationCallbackAndValue callback;
            callback.callback = mappingData.callback;
            callback.flags = mappingData.callbackFlags;
            callback.value = v;
            callbacks.append(callback);
        }
    }
    return callbacks;
}

// TODO: Optimize this even more by combining the work done here with the functions:
// buildRequiredChannelsAndTypes() and assignChannelComponentIndices(). We are
// currently repeating the iteration over mappings and extracting/generating
// channel names, types and joint indices.
QVector<MappingData> buildPropertyMappings(const QVector<ChannelMapping*> &channelMappings,
                                           const QVector<ChannelNameAndType> &channelNamesAndTypes,
                                           const QVector<ComponentIndices> &channelComponentIndices,
                                           const QVector<QBitArray> &sourceClipMask)
{
    // Accumulate the required number of mappings
    int maxMappingDatas = 0;
    for (const auto mapping : channelMappings) {
        switch (mapping->mappingType()) {
        case ChannelMapping::ChannelMappingType:
        case ChannelMapping::CallbackMappingType:
            ++maxMappingDatas;
            break;

        case ChannelMapping::SkeletonMappingType: {
            Skeleton *skeleton = mapping->skeleton();
            maxMappingDatas += 3 * skeleton->jointCount(); // S, R, T
            break;
        }
        }
    }
    QVector<MappingData> mappingDataVec;
    mappingDataVec.reserve(maxMappingDatas);

    // Iterate over the mappings
    for (const auto mapping : channelMappings) {
        switch (mapping->mappingType()) {
        case ChannelMapping::ChannelMappingType:
        case ChannelMapping::CallbackMappingType: {
            // Populate the data we need, easy stuff first
            MappingData mappingData;
            mappingData.targetId = mapping->targetId();
            mappingData.propertyName = mapping->propertyName();
            mappingData.type = mapping->type();
            mappingData.callback = mapping->callback();
            mappingData.callbackFlags = mapping->callbackFlags();

            if (mappingData.type == static_cast<int>(QVariant::Invalid)) {
                qWarning() << "Unknown type for node id =" << mappingData.targetId
                           << "and property =" << mapping->propertyName()
                           << "and callback =" << mapping->callback();
                continue;
            }

            // Try to find matching channel name and type
            const ChannelNameAndType nameAndType = { mapping->channelName(),
                                                     mapping->type(),
                                                     mapping->componentCount(),
                                                     mapping->peerId()
                                                   };
            const int index = channelNamesAndTypes.indexOf(nameAndType);
            if (index != -1) {
                // Do we have any animation data for this channel? If not, don't bother
                // adding a mapping for it.
                const bool hasChannelIndices = sourceClipMask[index].count(true) != 0;
                if (!hasChannelIndices)
                    continue;

                // We got one!
                mappingData.channelIndices = channelComponentIndices[index];
                mappingDataVec.push_back(mappingData);
            }
            break;
        }

        case ChannelMapping::SkeletonMappingType: {
            const QVector<ChannelNameAndType> jointProperties
                    = { { QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), Translation },
                        { QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), Rotation },
                        { QLatin1String("Scale"), static_cast<int>(QVariant::Vector3D), Scale } };
            const QHash<QString, const char *> channelNameToPropertyName
                    = { { QLatin1String("Location"), "translation" },
                        { QLatin1String("Rotation"), "rotation" },
                        { QLatin1String("Scale"), "scale" } };
            Skeleton *skeleton = mapping->skeleton();
            const int jointCount = skeleton->jointCount();
            for (int jointIndex = 0; jointIndex < jointCount; ++jointIndex) {
                // Populate the data we need, easy stuff first
                MappingData mappingData;
                mappingData.targetId = mapping->skeletonId();
                mappingData.skeleton = mapping->skeleton();

                const int propertyCount = jointProperties.size();
                for (int propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex) {
                    // Get the name, type and index
                    ChannelNameAndType nameAndType = jointProperties[propertyIndex];
                    nameAndType.jointIndex = jointIndex;
                    nameAndType.mappingId = mapping->peerId();

                    // Try to find matching channel name and type
                    const int index = channelNamesAndTypes.indexOf(nameAndType);
                    if (index == -1)
                        continue;

                    // Do we have any animation data for this channel? If not, don't bother
                    // adding a mapping for it.
                    const bool hasChannelIndices = sourceClipMask[index].count(true) != 0;
                    if (!hasChannelIndices)
                        continue;

                    if (index != -1) {
                        // We got one!
                        mappingData.propertyName = channelNameToPropertyName[nameAndType.name];
                        mappingData.type = nameAndType.type;
                        mappingData.channelIndices = channelComponentIndices[index];
                        mappingData.jointIndex = jointIndex;

                        // Convert property name for joint transform components to
                        // an enumerated type so we can avoid the string comparisons
                        // when sending the change events after evaluation.
                        // TODO: Replace this logic as we now do it in buildRequiredChannelsAndTypes()
                        if (qstrcmp(mappingData.propertyName, "scale") == 0)
                            mappingData.jointTransformComponent = Scale;
                        else if (qstrcmp(mappingData.propertyName, "rotation") == 0)
                            mappingData.jointTransformComponent = Rotation;
                        else if (qstrcmp(mappingData.propertyName, "translation") == 0)
                            mappingData.jointTransformComponent = Translation;

                        mappingDataVec.push_back(mappingData);
                    }
                }
            }
            break;
        }
        }
    }

    return mappingDataVec;
}

QVector<ChannelNameAndType> buildRequiredChannelsAndTypes(Handler *handler,
                                                          const ChannelMapper *mapper)
{
    ChannelMappingManager *mappingManager = handler->channelMappingManager();
    const QVector<Qt3DCore::QNodeId> mappingIds = mapper->mappingIds();

    // Reserve enough storage assuming each mapping is for a different channel.
    // May be overkill but avoids potential for multiple allocations
    QVector<ChannelNameAndType> namesAndTypes;
    namesAndTypes.reserve(mappingIds.size());

    // Iterate through the mappings and add ones not already used by an earlier mapping.
    // We could add them all then sort and remove duplicates. However, our approach has the
    // advantage of keeping the blend tree format more consistent with the mapping
    // orderings which will have better cache locality when generating events.
    for (const Qt3DCore::QNodeId mappingId : mappingIds) {
        // Get the mapping object
        ChannelMapping *mapping = mappingManager->lookupResource(mappingId);
        Q_ASSERT(mapping);

        switch (mapping->mappingType()) {
        case ChannelMapping::ChannelMappingType:
        case ChannelMapping::CallbackMappingType: {
            // Get the name and type
            const ChannelNameAndType nameAndType{ mapping->channelName(),
                                                  mapping->type(),
                                                  mapping->componentCount(),
                                                  mappingId };

            // Add if not already contained
            if (!namesAndTypes.contains(nameAndType))
                namesAndTypes.push_back(nameAndType);

            break;
        }

        case ChannelMapping::SkeletonMappingType: {
            // Add an entry for each scale/rotation/translation property of each joint index
            // of the target skeleton.
            const QVector<ChannelNameAndType> jointProperties
                    = { { QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), Translation },
                        { QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), Rotation },
                        { QLatin1String("Scale"), static_cast<int>(QVariant::Vector3D), Scale } };
            Skeleton *skeleton = handler->skeletonManager()->lookupResource(mapping->skeletonId());
            const int jointCount = skeleton->jointCount();
            for (int jointIndex = 0; jointIndex < jointCount; ++jointIndex) {
                const int propertyCount = jointProperties.size();
                for (int propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex) {
                    // Get the name, type and index
                    ChannelNameAndType nameAndType = jointProperties[propertyIndex];
                    nameAndType.jointName = skeleton->jointName(jointIndex);
                    nameAndType.jointIndex = jointIndex;
                    nameAndType.mappingId = mappingId;

                    // Add if not already contained
                    if (!namesAndTypes.contains(nameAndType))
                        namesAndTypes.push_back(nameAndType);
                }
            }

            break;
        }
        }
    }

    return namesAndTypes;
}

QVector<ComponentIndices> assignChannelComponentIndices(const QVector<ChannelNameAndType> &namesAndTypes)
{
    QVector<ComponentIndices> channelComponentIndices;
    channelComponentIndices.reserve(namesAndTypes.size());

    int baseIndex = 0;
    for (const auto &entry : namesAndTypes) {
        // Populate indices in order
        const int componentCount = entry.componentCount;
        ComponentIndices indices(componentCount);
        std::iota(indices.begin(), indices.end(), baseIndex);

        // Append to the results
        channelComponentIndices.push_back(indices);

        // Increment baseIndex
        baseIndex += componentCount;
    }

    return channelComponentIndices;
}

QVector<Qt3DCore::QNodeId> gatherValueNodesToEvaluate(Handler *handler,
                                                      Qt3DCore::QNodeId blendTreeRootId)
{
    Q_ASSERT(handler);
    Q_ASSERT(blendTreeRootId.isNull() == false);

    // We need the ClipBlendNodeManager to be able to lookup nodes from their Ids
    ClipBlendNodeManager *nodeManager = handler->clipBlendNodeManager();

    // Visit the tree in a pre-order manner and collect the dependencies
    QVector<Qt3DCore::QNodeId> clipIds;
    ClipBlendNodeVisitor visitor(nodeManager,
                                 ClipBlendNodeVisitor::PreOrder,
                                 ClipBlendNodeVisitor::VisitOnlyDependencies);

    auto func = [&clipIds, nodeManager] (ClipBlendNode *blendNode) {
        // Check if this is a value node itself
        if (blendNode->blendType() == ClipBlendNode::ValueType)
            clipIds.append(blendNode->peerId());

        const auto dependencyIds = blendNode->currentDependencyIds();
        for (const auto dependencyId : dependencyIds) {
            // Look up the blend node and if it's a value type (clip),
            // add it to the set of value node ids that need to be evaluated
            ClipBlendNode *node = nodeManager->lookupNode(dependencyId);
            if (node && node->blendType() == ClipBlendNode::ValueType)
                clipIds.append(dependencyId);
        }
    };
    visitor.traverse(blendTreeRootId, func);

    // Sort and remove duplicates
    std::sort(clipIds.begin(), clipIds.end());
    auto last = std::unique(clipIds.begin(), clipIds.end());
    clipIds.erase(last, clipIds.end());
    return clipIds;
}

ClipFormat generateClipFormatIndices(const QVector<ChannelNameAndType> &targetChannels,
                                     const QVector<ComponentIndices> &targetIndices,
                                     const AnimationClip *clip)
{
    Q_ASSERT(targetChannels.size() == targetIndices.size());

    // Reserve enough storage for all the format indices
    const int channelCount = targetChannels.size();
    ClipFormat f;
    f.namesAndTypes.resize(channelCount);
    f.formattedComponentIndices.resize(channelCount);
    f.sourceClipMask.resize(channelCount);
    int indexCount = 0;
    for (const auto &targetIndexVec : qAsConst(targetIndices))
        indexCount += targetIndexVec.size();
    ComponentIndices &sourceIndices = f.sourceClipIndices;
    sourceIndices.resize(indexCount);

    // Iterate through the target channels
    auto formatIt = sourceIndices.begin();
    for (int i = 0; i < channelCount; ++i) {
        // Find the index of the channel from the clip
        const ChannelNameAndType &targetChannel = targetChannels[i];
        const int clipChannelIndex = clip->channelIndex(targetChannel.name,
                                                        targetChannel.jointIndex);
        const int componentCount = targetIndices[i].size();

        if (clipChannelIndex != -1) {
            // Found a matching channel in the clip. Populate the corresponding
            // entries in the format vector with the *source indices*
            // needed to build the formatted results.
            const int baseIndex = clip->channelComponentBaseIndex(clipChannelIndex);
            const auto channelIndices = channelComponentsToIndices(clip->channels()[clipChannelIndex],
                                                                   targetChannel.type,
                                                                   targetChannel.componentCount,
                                                                   baseIndex);
            std::copy(channelIndices.begin(), channelIndices.end(), formatIt);

            f.sourceClipMask[i].resize(componentCount);
            for (int j = 0; j < componentCount; ++j)
                f.sourceClipMask[i].setBit(j, channelIndices[j] != -1);
        } else {
            // No such channel in this clip. We'll use default values when
            // mapping from the clip to the formatted clip results.
            std::fill(formatIt, formatIt + componentCount, -1);
            f.sourceClipMask[i].fill(false, componentCount);
        }

        f.formattedComponentIndices[i] = targetIndices[i];
        f.namesAndTypes[i] = targetChannels[i];
        formatIt += componentCount;
    }

    return f;
}

ClipResults formatClipResults(const ClipResults &rawClipResults,
                              const ComponentIndices &format)
{
    // Resize the output to match the number of indices
    const int elementCount = format.size();
    ClipResults formattedClipResults(elementCount);

    // Perform a gather operation to format the data

    // TODO: For large numbers of components do this in parallel with
    // for e.g. a parallel_for() like construct
    // TODO: We could potentially avoid having holes in these intermediate
    // vectors by adjusting the component indices stored in the MappingData
    // and format vectors. Needs careful investigation!
    for (int i = 0; i < elementCount; ++i) {
        if (format[i] == -1)
            continue;
        formattedClipResults[i] = rawClipResults[format[i]];
    }

    return formattedClipResults;
}

ClipResults evaluateBlendTree(Handler *handler,
                              BlendedClipAnimator *animator,
                              Qt3DCore::QNodeId blendTreeRootId)
{
    Q_ASSERT(handler);
    Q_ASSERT(blendTreeRootId.isNull() == false);
    const Qt3DCore::QNodeId animatorId = animator->peerId();

    // We need the ClipBlendNodeManager to be able to lookup nodes from their Ids
    ClipBlendNodeManager *nodeManager = handler->clipBlendNodeManager();

    // Visit the tree in a post-order manner and for each interior node call
    // blending function. We only need to visit the nodes that affect the blend
    // tree at this time.
    ClipBlendNodeVisitor visitor(nodeManager,
                                 ClipBlendNodeVisitor::PostOrder,
                                 ClipBlendNodeVisitor::VisitOnlyDependencies);

    // TODO: When jobs can spawn other jobs we could evaluate subtrees of
    // the blend tree in parallel. Since it's just a dependency tree, it maps
    // simply onto the dependencies between jobs.
    auto func = [animatorId] (ClipBlendNode *blendNode) {
        // Look up the blend node and if it's an interior node, perform
        // the blend operation
        if (blendNode->blendType() != ClipBlendNode::ValueType)
            blendNode->blend(animatorId);
    };
    visitor.traverse(blendTreeRootId, func);

    // The clip results stored in the root node for this animator
    // now represent the result of the blend tree evaluation
    ClipBlendNode *blendTreeRootNode = nodeManager->lookupNode(blendTreeRootId);
    Q_ASSERT(blendTreeRootNode);
    return blendTreeRootNode->clipResults(animatorId);
}

QVector<float> defaultValueForChannel(Handler *handler,
                                      const ChannelNameAndType &channelDescription)
{
    QVector<float> result;

    // Does the channel repesent a joint in a skeleton or is it a general channel?
    ChannelMappingManager *mappingManager = handler->channelMappingManager();
    const ChannelMapping *mapping = mappingManager->lookupResource(channelDescription.mappingId);
    switch (mapping->mappingType()) {
    case ChannelMapping::SkeletonMappingType: {
        // Default channel values for a joint in a skeleton, should be taken
        // from the default pose of the joint itself. I.e. if a joint is not
        // explicitly animated, then it should retain it's initial rest pose.
        Skeleton *skeleton = mapping->skeleton();
        const int jointIndex = channelDescription.jointIndex;
        switch (channelDescription.jointTransformComponent) {
        case Translation:
            result = valueToVector(skeleton->jointTranslation(jointIndex));
            break;

        case Rotation:
            result = valueToVector(skeleton->jointRotation(jointIndex));
            break;

        case Scale:
            result = valueToVector(skeleton->jointScale(jointIndex));
            break;

        case NoTransformComponent:
            Q_UNREACHABLE();
            break;
        }
        break;
    }

    case ChannelMapping::ChannelMappingType:
    case ChannelMapping::CallbackMappingType: {
        // Do our best to provide a sensible default value.
        if (channelDescription.type == QMetaType::QQuaternion) {
            result = valueToVector(QQuaternion()); // (1, 0, 0, 0)
            break;
        }

        if (channelDescription.name.toLower() == QLatin1String("scale")) {
            result = valueToVector(QVector3D(1.0f, 1.0f, 1.0f));
            break;
        }

        // Everything else gets all zeros
        const int componentCount = mapping->componentCount();
        result = QVector<float>(componentCount, 0.0f);
        break;
    }

    }

    return result;
}

void applyComponentDefaultValues(const QVector<ComponentValue> &componentDefaults,
                                 ClipResults &formattedClipResults)
{
    for (const auto &componentDefault : componentDefaults)
        formattedClipResults[componentDefault.componentIndex] = componentDefault.value;
}

} // Animation
} // Qt3DAnimation

QT_END_NAMESPACE
