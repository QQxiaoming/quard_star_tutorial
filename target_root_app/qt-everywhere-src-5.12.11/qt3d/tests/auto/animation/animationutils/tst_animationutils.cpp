/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QTest>
#include <Qt3DAnimation/private/animationclip_p.h>
#include <Qt3DAnimation/private/animationutils_p.h>
#include <Qt3DAnimation/private/blendedclipanimator_p.h>
#include <Qt3DAnimation/private/clock_p.h>
#include <Qt3DAnimation/private/channelmapper_p.h>
#include <Qt3DAnimation/private/channelmapping_p.h>
#include <Qt3DAnimation/private/clipblendvalue_p.h>
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/additiveclipblend_p.h>
#include <Qt3DAnimation/private/lerpclipblend_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qcolor.h>
#include <QtCore/qbitarray.h>

#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

using namespace Qt3DAnimation::Animation;

Q_DECLARE_METATYPE(Qt3DAnimation::Animation::Handler*)
Q_DECLARE_METATYPE(QVector<ChannelMapping *>)
Q_DECLARE_METATYPE(Clock *)
Q_DECLARE_METATYPE(ChannelMapper *)
Q_DECLARE_METATYPE(AnimationClip *)
Q_DECLARE_METATYPE(QVector<MappingData>)
Q_DECLARE_METATYPE(QVector<Qt3DCore::QPropertyUpdatedChangePtr>)
Q_DECLARE_METATYPE(Channel)
Q_DECLARE_METATYPE(AnimatorEvaluationData)
Q_DECLARE_METATYPE(ClipEvaluationData)
Q_DECLARE_METATYPE(ClipAnimator *)
Q_DECLARE_METATYPE(BlendedClipAnimator *)
Q_DECLARE_METATYPE(QVector<ChannelNameAndType>)
Q_DECLARE_METATYPE(QVector<AnimationCallbackAndValue>)
Q_DECLARE_METATYPE(ClipFormat)
Q_DECLARE_METATYPE(ChannelNameAndType)

namespace {

class MeanBlendNode : public ClipBlendNode
{
public:
    MeanBlendNode()
        : ClipBlendNode(ClipBlendNode::LerpBlendType)
    {}

    void setValueNodeIds(Qt3DCore::QNodeId value1Id,
                         Qt3DCore::QNodeId value2Id)
    {
        m_value1Id = value1Id;
        m_value2Id = value2Id;
    }

    inline QVector<Qt3DCore::QNodeId> allDependencyIds() const override
    {
        return currentDependencyIds();
    }

    QVector<Qt3DCore::QNodeId> currentDependencyIds() const final
    {
        return QVector<Qt3DCore::QNodeId>() << m_value1Id << m_value2Id;
    }

    using ClipBlendNode::setClipResults;

    double duration() const final { return 0.0f; }

protected:
    ClipResults doBlend(const QVector<ClipResults> &blendData) const final
    {
        Q_ASSERT(blendData.size() == 2);
        const int elementCount = blendData.first().size();
        ClipResults blendResults(elementCount);

        for (int i = 0; i < elementCount; ++i)
            blendResults[i] = 0.5f * (blendData[0][i] + blendData[1][i]);

        return blendResults;
    }

private:
    Qt3DCore::QNodeId m_value1Id;
    Qt3DCore::QNodeId m_value2Id;
};

bool fuzzyCompare(float x1, float x2)
{
    if (qFuzzyIsNull(x1) && qFuzzyIsNull(x2)) {
        return true;
    } else if ((qFuzzyIsNull(x1) && !qFuzzyIsNull(x2)) ||
               (!qFuzzyIsNull(x1) && qFuzzyIsNull(x2))) {
        return false;
    } else {
        return qFuzzyCompare(x1, x2);
    }
}

class DummyCallback : public Qt3DAnimation::QAnimationCallback
{
public:
    void valueChanged(const QVariant &) override { }
};

} // anonymous


class tst_AnimationUtils : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

public:
    ChannelMapping *createChannelMapping(Handler *handler,
                                         const QString &channelName,
                                         const Qt3DCore::QNodeId targetId,
                                         const char *propertyName,
                                         int type,
                                         int componentCount)
    {
        auto channelMappingId = Qt3DCore::QNodeId::createId();
        ChannelMapping *channelMapping = handler->channelMappingManager()->getOrCreateResource(channelMappingId);
        setPeerId(channelMapping, channelMappingId);
        channelMapping->setHandler(handler);
        channelMapping->setTargetId(targetId);
        channelMapping->setPropertyName(propertyName);
        channelMapping->setChannelName(channelName);
        channelMapping->setType(type);
        channelMapping->setMappingType(ChannelMapping::ChannelMappingType);
        channelMapping->setComponentCount(componentCount);
        return channelMapping;
    }

    ChannelMapping *createChannelMapping(Handler *handler,
                                         const Qt3DCore::QNodeId skeletonId)
    {
        auto channelMappingId = Qt3DCore::QNodeId::createId();
        ChannelMapping *channelMapping = handler->channelMappingManager()->getOrCreateResource(channelMappingId);
        setPeerId(channelMapping, channelMappingId);
        channelMapping->setHandler(handler);
        channelMapping->setSkeletonId(skeletonId);
        channelMapping->setMappingType(ChannelMapping::SkeletonMappingType);
        return channelMapping;
    }

    ChannelMapper *createChannelMapper(Handler *handler,
                                       const QVector<Qt3DCore::QNodeId> &mappingIds)
    {
        auto channelMapperId = Qt3DCore::QNodeId::createId();
        ChannelMapper *channelMapper = handler->channelMapperManager()->getOrCreateResource(channelMapperId);
        setPeerId(channelMapper, channelMapperId);
        channelMapper->setMappingIds(mappingIds);
        return channelMapper;
    }

    AnimationClip *createAnimationClipLoader(Handler *handler,
                                                   const QUrl &source)
    {
        auto clipId = Qt3DCore::QNodeId::createId();
        AnimationClip *clip = handler->animationClipLoaderManager()->getOrCreateResource(clipId);
        setPeerId(clip, clipId);
        clip->setDataType(AnimationClip::File);
        clip->setSource(source);
        clip->loadAnimation();
        return clip;
    }

    ClipAnimator *createClipAnimator(Handler *handler,
                                     qint64 globalStartTimeNS,
                                     int loops)
    {
        auto animatorId = Qt3DCore::QNodeId::createId();
        ClipAnimator *animator = handler->clipAnimatorManager()->getOrCreateResource(animatorId);
        setPeerId(animator, animatorId);
        animator->setHandler(handler);
        animator->setStartTime(globalStartTimeNS);
        animator->setLoops(loops);
        return animator;
    }

    BlendedClipAnimator *createBlendedClipAnimator(Handler *handler,
                                                   qint64 globalStartTimeNS,
                                                   int loops)
    {
        auto animatorId = Qt3DCore::QNodeId::createId();
        BlendedClipAnimator *animator = handler->blendedClipAnimatorManager()->getOrCreateResource(animatorId);
        setPeerId(animator, animatorId);
        animator->setStartTime(globalStartTimeNS);
        animator->setLoops(loops);
        return animator;
    }

    LerpClipBlend *createLerpClipBlend(Handler *handler)
    {
        auto lerpId = Qt3DCore::QNodeId::createId();
        LerpClipBlend *lerp = new LerpClipBlend();
        setPeerId(lerp, lerpId);
        lerp->setClipBlendNodeManager(handler->clipBlendNodeManager());
        lerp->setHandler(handler);
        handler->clipBlendNodeManager()->appendNode(lerpId, lerp);
        return lerp;
    }

    AdditiveClipBlend *createAdditiveClipBlend(Handler *handler)
    {
        auto additiveId = Qt3DCore::QNodeId::createId();
        AdditiveClipBlend *additive = new AdditiveClipBlend();
        setPeerId(additive, additiveId);
        additive->setClipBlendNodeManager(handler->clipBlendNodeManager());
        additive->setHandler(handler);
        handler->clipBlendNodeManager()->appendNode(additiveId, additive);
        return additive;
    }

    ClipBlendValue *createClipBlendValue(Handler *handler)
    {
        auto valueId = Qt3DCore::QNodeId::createId();
        ClipBlendValue *value = new ClipBlendValue();
        setPeerId(value, valueId);
        value->setClipBlendNodeManager(handler->clipBlendNodeManager());
        value->setHandler(handler);
        handler->clipBlendNodeManager()->appendNode(valueId, value);
        return value;
    }

    MeanBlendNode *createMeanBlendNode(Handler *handler)
    {
        auto id = Qt3DCore::QNodeId::createId();
        MeanBlendNode *node = new MeanBlendNode();
        setPeerId(node, id);
        node->setClipBlendNodeManager(handler->clipBlendNodeManager());
        node->setHandler(handler);
        handler->clipBlendNodeManager()->appendNode(id, node);
        return node;
    }

    Skeleton *createSkeleton(Handler *handler, int jointCount)
    {
        auto skeletonId = Qt3DCore::QNodeId::createId();
        Skeleton *skeleton = handler->skeletonManager()->getOrCreateResource(skeletonId);
        setPeerId(skeleton, skeletonId);
        skeleton->setJointCount(jointCount);
        return skeleton;
    }

private Q_SLOTS:
    void checkBuildPropertyMappings_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<QVector<ChannelMapping *>>("channelMappings");
        QTest::addColumn<QVector<ChannelNameAndType>>("channelNamesAndTypes");
        QTest::addColumn<QVector<ComponentIndices>>("channelComponentIndices");
        QTest::addColumn<QVector<QBitArray>>("sourceClipMask");
        QTest::addColumn<QVector<MappingData>>("expectedResults");

        // Single ChannelMapping
        {
            Handler *handler = new Handler();

            auto channelMapping = createChannelMapping(handler,
                                                       QLatin1String("Location"),
                                                       Qt3DCore::QNodeId::createId(),
                                                       "translation",
                                                       static_cast<int>(QVariant::Vector3D),
                                                       3);

            QVector<ChannelMapping *> channelMappings = { channelMapping };

            // Create a few channels in the format description
            ChannelNameAndType rotation = { QLatin1String("Rotation"),
                                            static_cast<int>(QVariant::Quaternion),
                                            4,
                                            channelMapping->peerId() };
            ChannelNameAndType location = { QLatin1String("Location"),
                                            static_cast<int>(QVariant::Vector3D),
                                            3,
                                            channelMapping->peerId() };
            ChannelNameAndType baseColor = { QLatin1String("BaseColor"),
                                             static_cast<int>(QVariant::Vector3D),
                                             3,
                                             channelMapping->peerId() };
            ChannelNameAndType metalness = { QLatin1String("Metalness"),
                                             static_cast<int>(QVariant::Double),
                                             1,
                                             channelMapping->peerId() };
            ChannelNameAndType roughness = { QLatin1String("Roughness"),
                                             static_cast<int>(QVariant::Double),
                                             1,
                                             channelMapping->peerId() };
            ChannelNameAndType morphTargetWeightsList = { QLatin1String("MorphTargetWeightsList"),
                                                      static_cast<int>(QVariant::List),
                                                      5,
                                                      channelMapping->peerId() };
            ChannelNameAndType morphTargetWeightsVec = { QLatin1String("MorphTargetWeightsVec"),
                                                      qMetaTypeId<QVector<float>>(),
                                                      6,
                                                      channelMapping->peerId() };
            ChannelNameAndType rgbColor = { QLatin1String("rgbColor"),
                                            static_cast<int>(QVariant::Color),
                                            3,
                                            channelMapping->peerId() };

            ChannelNameAndType rgbaColor = { QLatin1String("rgbaColor"),
                                            static_cast<int>(QVariant::Color),
                                            4,
                                            channelMapping->peerId() };

            QVector<ChannelNameAndType> channelNamesAndTypes
                    = { rotation, location, baseColor, metalness, roughness,
                        morphTargetWeightsList, morphTargetWeightsVec, rgbColor, rgbaColor };

            // And the matching indices
            ComponentIndices rotationIndices = { 0, 1, 2, 3 };
            ComponentIndices locationIndices = { 4, 5, 6 };
            ComponentIndices baseColorIndices = { 7, 8, 9 };
            ComponentIndices metalnessIndices = { 10 };
            ComponentIndices roughnessIndices = { 11 };
            ComponentIndices morphTargetListIndices = { 12, 13, 14, 15, 16 };
            ComponentIndices morphTargetVecIndices = { 17, 18, 19, 20, 21, 22 };
            ComponentIndices rgbColorIndices = { 23, 24, 25 };
            ComponentIndices rgbaColorIndices = { 26, 27, 28, 29 };
            QVector<ComponentIndices> channelComponentIndices
                    = { rotationIndices, locationIndices, baseColorIndices,
                        metalnessIndices, roughnessIndices, morphTargetListIndices,
                        morphTargetVecIndices, rgbColorIndices, rgbaColorIndices };

            QVector<QBitArray> sourceClipMask = { QBitArray(4, true),
                                                  QBitArray(3, true),
                                                  QBitArray(3, true),
                                                  QBitArray(1, true),
                                                  QBitArray(1, true),
                                                  QBitArray(5, true),
                                                  QBitArray(6, true),
                                                  QBitArray(3, true),
                                                  QBitArray(4, true),
                                                };

            MappingData expectedMapping;
            expectedMapping.targetId = channelMapping->targetId();
            expectedMapping.propertyName = channelMapping->propertyName();
            expectedMapping.type = channelMapping->type();
            expectedMapping.channelIndices = locationIndices;
            QVector<MappingData> expectedResults = { expectedMapping };

            QTest::newRow("single mapping")
                    << handler
                    << channelMappings
                    << channelNamesAndTypes
                    << channelComponentIndices
                    << sourceClipMask
                    << expectedResults;
        }

        // Multiple ChannelMappings
        {
            Handler *handler = new Handler();

            auto locationMapping = createChannelMapping(handler,
                                                        QLatin1String("Location"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "translation",
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3);

            auto metalnessMapping = createChannelMapping(handler,
                                                         QLatin1String("Metalness"),
                                                         Qt3DCore::QNodeId::createId(),
                                                         "metalness",
                                                         static_cast<int>(QVariant::Double),
                                                         1);

            auto baseColorMapping = createChannelMapping(handler,
                                                         QLatin1String("BaseColor"),
                                                         Qt3DCore::QNodeId::createId(),
                                                         "baseColor",
                                                         static_cast<int>(QVariant::Vector3D),
                                                         3);

            auto roughnessMapping = createChannelMapping(handler,
                                                         QLatin1String("Roughness"),
                                                         Qt3DCore::QNodeId::createId(),
                                                         "roughness",
                                                         static_cast<int>(QVariant::Double),
                                                         1);

            auto rotationMapping = createChannelMapping(handler,
                                                        QLatin1String("Rotation"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "rotation",
                                                        static_cast<int>(QVariant::Quaternion),
                                                        4);

            auto morphTargetMapping = createChannelMapping(handler,
                                                           QLatin1String("MorphTargetWeights"),
                                                           Qt3DCore::QNodeId::createId(),
                                                           "weights",
                                                           static_cast<int>(QVariant::List),
                                                           5);


            QVector<ChannelMapping *> channelMappings
                    = { locationMapping, metalnessMapping,
                        baseColorMapping, roughnessMapping,
                        rotationMapping, morphTargetMapping };

            // Create a few channels in the format description
            ChannelNameAndType rotation = { QLatin1String("Rotation"),
                                            static_cast<int>(QVariant::Quaternion),
                                            4,
                                            rotationMapping->peerId() };
            ChannelNameAndType location = { QLatin1String("Location"),
                                            static_cast<int>(QVariant::Vector3D),
                                            3,
                                            locationMapping->peerId() };
            ChannelNameAndType baseColor = { QLatin1String("BaseColor"),
                                             static_cast<int>(QVariant::Vector3D),
                                             3,
                                             baseColorMapping->peerId() };
            ChannelNameAndType metalness = { QLatin1String("Metalness"),
                                             static_cast<int>(QVariant::Double),
                                             1,
                                             metalnessMapping->peerId() };
            ChannelNameAndType roughness = { QLatin1String("Roughness"),
                                             static_cast<int>(QVariant::Double),
                                             1,
                                             roughnessMapping->peerId() };
            ChannelNameAndType morphTarget = { QLatin1String("MorphTargetWeights"),
                                               static_cast<int>(QVariant::List),
                                               5,
                                               morphTargetMapping->peerId() };
            QVector<ChannelNameAndType> channelNamesAndTypes
                    = { rotation, location, baseColor, metalness, roughness,
                        morphTarget };

            // And the matching indices
            ComponentIndices rotationIndices = { 0, 1, 2, 3 };
            ComponentIndices locationIndices = { 4, 5, 6 };
            ComponentIndices baseColorIndices = { 7, 8, 9 };
            ComponentIndices metalnessIndices = { 10 };
            ComponentIndices roughnessIndices = { 11 };
            ComponentIndices morphTargetIndices = { 12, 13, 14, 15, 16 };
            QVector<ComponentIndices> channelComponentIndices
                    = { rotationIndices, locationIndices, baseColorIndices,
                        metalnessIndices, roughnessIndices, morphTargetIndices };

            QVector<QBitArray> sourceClipMask = { QBitArray(4, true),
                                                  QBitArray(3, true),
                                                  QBitArray(3, true),
                                                  QBitArray(1, true),
                                                  QBitArray(1, true),
                                                  QBitArray(5, true) };

            MappingData expectedLocationMapping;
            expectedLocationMapping.targetId = locationMapping->targetId();
            expectedLocationMapping.propertyName = locationMapping->propertyName();
            expectedLocationMapping.type = locationMapping->type();
            expectedLocationMapping.channelIndices = locationIndices;

            MappingData expectedMetalnessMapping;
            expectedMetalnessMapping.targetId = metalnessMapping->targetId();
            expectedMetalnessMapping.propertyName = metalnessMapping->propertyName();
            expectedMetalnessMapping.type = metalnessMapping->type();
            expectedMetalnessMapping.channelIndices = metalnessIndices;

            MappingData expectedBaseColorMapping;
            expectedBaseColorMapping.targetId = baseColorMapping->targetId();
            expectedBaseColorMapping.propertyName = baseColorMapping->propertyName();
            expectedBaseColorMapping.type = baseColorMapping->type();
            expectedBaseColorMapping.channelIndices = baseColorIndices;

            MappingData expectedRoughnessMapping;
            expectedRoughnessMapping.targetId = roughnessMapping->targetId();
            expectedRoughnessMapping.propertyName = roughnessMapping->propertyName();
            expectedRoughnessMapping.type = roughnessMapping->type();
            expectedRoughnessMapping.channelIndices = roughnessIndices;

            MappingData expectedRotationMapping;
            expectedRotationMapping.targetId = rotationMapping->targetId();
            expectedRotationMapping.propertyName = rotationMapping->propertyName();
            expectedRotationMapping.type = rotationMapping->type();
            expectedRotationMapping.channelIndices = rotationIndices;

            MappingData expectedMorphTargetMapping;
            expectedMorphTargetMapping.targetId = morphTargetMapping->targetId();
            expectedMorphTargetMapping.propertyName = morphTargetMapping->propertyName();
            expectedMorphTargetMapping.type = morphTargetMapping->type();
            expectedMorphTargetMapping.channelIndices = morphTargetIndices;

            QVector<MappingData> expectedResults
                    = { expectedLocationMapping,
                        expectedMetalnessMapping,
                        expectedBaseColorMapping,
                        expectedRoughnessMapping,
                        expectedRotationMapping,
                        expectedMorphTargetMapping };

            QTest::newRow("multiple mappings")
                    << handler
                    << channelMappings
                    << channelNamesAndTypes
                    << channelComponentIndices
                    << sourceClipMask
                    << expectedResults;
        }

        // Single skeleton mapping
        {
            Handler *handler = new Handler();
            const int jointCount = 4;
            auto skeleton = createSkeleton(handler, jointCount);
            auto channelMapping = createChannelMapping(handler, skeleton->peerId());

            QVector<ChannelMapping *> channelMappings = { channelMapping };

            // Create a few channels in the format description
            QVector<ChannelNameAndType> channelNamesAndTypes;
            for (int i = 0; i < jointCount; ++i) {
                ChannelNameAndType locationDescription = { QLatin1String("Location"),
                                                           static_cast<int>(QVariant::Vector3D),
                                                           3,
                                                           channelMapping->peerId() };
                locationDescription.jointIndex = i;
                locationDescription.jointTransformComponent = Translation;
                channelNamesAndTypes.push_back(locationDescription);

                ChannelNameAndType rotationDescription = { QLatin1String("Rotation"),
                                                           static_cast<int>(QVariant::Quaternion),
                                                           4,
                                                           channelMapping->peerId() };
                rotationDescription.jointIndex = i;
                rotationDescription.jointTransformComponent = Rotation;
                channelNamesAndTypes.push_back(rotationDescription);

                ChannelNameAndType scaleDescription = { QLatin1String("Scale"),
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3,
                                                        channelMapping->peerId() };
                scaleDescription.jointIndex = i;
                scaleDescription.jointTransformComponent = Scale;
                channelNamesAndTypes.push_back(scaleDescription);
            }

            // And the matching indices
            QVector<ComponentIndices> channelComponentIndices;
            channelComponentIndices.push_back({ 0, 1, 2 });
            channelComponentIndices.push_back({ 3, 4, 5, 6 });
            channelComponentIndices.push_back({ 7, 8, 9 });

            channelComponentIndices.push_back({ 10, 11, 12 });
            channelComponentIndices.push_back({ 13, 14, 15, 16 });
            channelComponentIndices.push_back({ 17, 18, 19 });

            channelComponentIndices.push_back({ 20, 21, 22 });
            channelComponentIndices.push_back({ 23, 24, 25, 26 });
            channelComponentIndices.push_back({ 27, 28, 29 });

            channelComponentIndices.push_back({ 30, 31, 32 });
            channelComponentIndices.push_back({ 33, 34, 35, 36 });
            channelComponentIndices.push_back({ 37, 38, 39 });

            QVector<QBitArray> sourceClipMask = { QBitArray(3, true),
                                                  QBitArray(4, true),
                                                  QBitArray(3, true),
                                                  QBitArray(3, true),
                                                  QBitArray(4, true),
                                                  QBitArray(3, true),
                                                  QBitArray(3, true),
                                                  QBitArray(4, true),
                                                  QBitArray(3, true),
                                                  QBitArray(3, true),
                                                  QBitArray(4, true),
                                                  QBitArray(3, true) };

            QVector<MappingData> expectedResults;
            int componentIndicesIndex = 0;
            for (int i = 0; i < jointCount; ++i) {
                MappingData locationMapping;
                locationMapping.targetId = channelMapping->skeletonId();
                locationMapping.propertyName = "translation";
                locationMapping.type = static_cast<int>(QVariant::Vector3D);
                locationMapping.channelIndices = channelComponentIndices[componentIndicesIndex++];
                locationMapping.jointIndex = i;

                MappingData rotationMapping;
                rotationMapping.targetId = channelMapping->skeletonId();
                rotationMapping.propertyName = "rotation";
                rotationMapping.type = static_cast<int>(QVariant::Quaternion);
                rotationMapping.channelIndices = channelComponentIndices[componentIndicesIndex++];
                rotationMapping.jointIndex = i;

                MappingData scaleMapping;
                scaleMapping.targetId = channelMapping->skeletonId();
                scaleMapping.propertyName = "scale";
                scaleMapping.type = static_cast<int>(QVariant::Vector3D);
                scaleMapping.channelIndices = channelComponentIndices[componentIndicesIndex++];
                scaleMapping.jointIndex = i;

                expectedResults << locationMapping << rotationMapping << scaleMapping;
            }

            QTest::newRow("single skeleton mapping")
                    << handler
                    << channelMappings
                    << channelNamesAndTypes
                    << channelComponentIndices
                    << sourceClipMask
                    << expectedResults;
        }
    }

    void checkBuildPropertyMappings()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(QVector<ChannelMapping *>, channelMappings);
        QFETCH(QVector<ChannelNameAndType>, channelNamesAndTypes);
        QFETCH(QVector<ComponentIndices>, channelComponentIndices);
        QFETCH(QVector<QBitArray>, sourceClipMask);
        QFETCH(QVector<MappingData>, expectedResults);

        // WHEN
        const QVector<MappingData> actualResults = buildPropertyMappings(channelMappings,
                                                                         channelNamesAndTypes,
                                                                         channelComponentIndices,
                                                                         sourceClipMask);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            const auto actualMapping = actualResults[i];
            const auto expectedMapping = expectedResults[i];

            QCOMPARE(actualMapping.targetId, expectedMapping.targetId);
            QCOMPARE(actualMapping.jointIndex, expectedMapping.jointIndex);
            QCOMPARE(actualMapping.propertyName, expectedMapping.propertyName);
            QCOMPARE(actualMapping.type, expectedMapping.type);
            QCOMPARE(actualMapping.channelIndices.size(), expectedMapping.channelIndices.size());
            for (int j = 0; j < actualMapping.channelIndices.size(); ++j) {
                QCOMPARE(actualMapping.channelIndices[j], expectedMapping.channelIndices[j]);
            }
        }

        // Cleanup
        delete handler;
    }

    void checkLocalTimeFromElapsedTime_data()
    {
        QTest::addColumn<double>("elapsedTime");
        QTest::addColumn<double>("currentTime");
        QTest::addColumn<double>("playbackRate");
        QTest::addColumn<double>("duration");
        QTest::addColumn<int>("loopCount");
        QTest::addColumn<int>("currentLoop");
        QTest::addColumn<double>("expectedLocalTime");
        QTest::addColumn<int>("expectedCurrentLoop");

        double elapsedTime;
        double currentTime;
        double playbackRate;
        double duration;
        int loopCount;
        int currentLoop;
        double expectedLocalTime;
        int expectedCurrentLoop;

        elapsedTime = 0.0;
        currentTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        currentLoop = 0;
        expectedLocalTime = 0.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_current = 0, t_elapsed = 0, loop_current = 0")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;

        elapsedTime = 0.5;
        currentTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        currentLoop = 0;
        expectedLocalTime = 0.5;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_current = 0, t_elapsed = 0.5, loop_current = 0")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;

        elapsedTime = 1.5;
        currentTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        currentLoop = 0;
        expectedLocalTime = 1.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_current = 0, t_elapsed = 1.5, loop_current = 0")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;

        elapsedTime = 0.5;
        currentTime = 0.6;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        currentLoop = 0;
        expectedLocalTime = 1.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_current = 0.5, t_elapsed = 0.6, loop_current = 0")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;

        elapsedTime = 0.5;
        currentTime = 0.6;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 2;
        currentLoop = 0;
        expectedLocalTime = 0.1;
        expectedCurrentLoop = 1;
        QTest::newRow("simple, t_current = 0.5, t_elapsed = 0.6, loop_current = 0, loop_count = 2")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;

        elapsedTime = 0.5;
        currentTime = 0.6;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 2;
        currentLoop = 1;
        expectedLocalTime = 1.0;
        expectedCurrentLoop = 1; // We clamp at end of final loop
        QTest::newRow("simple, t_current = 0.5, t_elapsed = 0.6, loop_current = 1, loop_count = 2")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;

        elapsedTime = 0.5;
        currentTime = 0.6;
        playbackRate = 0.1;
        duration = 1.0;
        loopCount = 2;
        currentLoop = 1;
        expectedLocalTime = 0.65;
        expectedCurrentLoop = 1;
        QTest::newRow("simple, t_current = 0.5, t_elapsed = 0.6, loop_current = 1, loop_count = 2")
                << elapsedTime << currentTime << playbackRate << duration << loopCount << currentLoop
                << expectedLocalTime << expectedCurrentLoop;
    }

    void checkLocalTimeFromElapsedTime()
    {
        // GIVEN
        QFETCH(double, elapsedTime);
        QFETCH(double, currentTime);
        QFETCH(double, playbackRate);
        QFETCH(double, duration);
        QFETCH(int, loopCount);
        QFETCH(int, currentLoop);
        QFETCH(double, expectedLocalTime);
        QFETCH(int, expectedCurrentLoop);

        // WHEN
        int actualCurrentLoop = currentLoop;
        double actualLocalTime = localTimeFromElapsedTime(currentTime,
                                                          elapsedTime,
                                                          playbackRate,
                                                          duration,
                                                          loopCount,
                                                          actualCurrentLoop);

        // THEN
        QCOMPARE(actualCurrentLoop, expectedCurrentLoop);
        QCOMPARE(actualLocalTime, expectedLocalTime);
    }

    void checkPreparePropertyChanges_data()
    {
        QTest::addColumn<Qt3DCore::QNodeId>("animatorId");
        QTest::addColumn<QVector<MappingData>>("mappingData");
        QTest::addColumn<QVector<float>>("channelResults");
        QTest::addColumn<float>("normalizedTime");
        QTest::addColumn<bool>("finalFrame");
        QTest::addColumn<QVector<Qt3DCore::QPropertyUpdatedChangePtr>>("expectedChanges");

        Qt3DCore::QNodeId animatorId;
        QVector<MappingData> mappingData;
        QVector<float> channelResults;
        bool finalFrame;
        float normalizedTime;
        QVector<Qt3DCore::QPropertyUpdatedChangePtr> expectedChanges;

        // Single property, vec3
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "translation";
            mapping.type = static_cast<int>(QVariant::Vector3D);
            mapping.channelIndices = QVector<int>() << 0 << 1 << 2;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 1.0f << 2.0f << 3.0f;
            finalFrame = false;
            normalizedTime = 1.1f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            change->setValue(QVariant::fromValue(QVector3D(1.0f, 2.0f, 3.0f)));
            expectedChanges.push_back(change);

            QTest::newRow("vec3 translation, final = false")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            normalizedTime = 1.0f;
            auto normalizedTimeChange = Qt3DCore::QPropertyUpdatedChangePtr::create(animatorId);
            normalizedTimeChange->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            normalizedTimeChange->setPropertyName("normalizedTime");
            normalizedTimeChange->setValue(normalizedTime);
            expectedChanges.push_back(normalizedTimeChange);

            finalFrame = true;
            auto animatorChange = Qt3DCore::QPropertyUpdatedChangePtr::create(animatorId);
            animatorChange->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            animatorChange->setPropertyName("running");
            animatorChange->setValue(false);
            expectedChanges.push_back(animatorChange);

            QTest::newRow("vec3 translation, final = true, normalizedTime = 1.0f")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Multiple properties, all vec3
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData translationMapping;
            translationMapping.targetId = Qt3DCore::QNodeId::createId();
            translationMapping.propertyName = "translation";
            translationMapping.type = static_cast<int>(QVariant::Vector3D);
            translationMapping.channelIndices = QVector<int>() << 0 << 1 << 2;
            mappingData.push_back(translationMapping);

            MappingData scaleMapping;
            scaleMapping.targetId = Qt3DCore::QNodeId::createId();
            scaleMapping.propertyName = "scale";
            scaleMapping.type = static_cast<int>(QVariant::Vector3D);
            scaleMapping.channelIndices = QVector<int>() << 3 << 4 << 5;
            mappingData.push_back(scaleMapping);

            channelResults = QVector<float>() << 1.0f << 2.0f << 3.0f
                                              << 4.0f << 5.0f << 6.0f;
            finalFrame = false;
            normalizedTime = -0.1f; // Invalid

            auto translationChange = Qt3DCore::QPropertyUpdatedChangePtr::create(translationMapping.targetId);
            translationChange->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            translationChange->setPropertyName(translationMapping.propertyName);
            translationChange->setValue(QVariant::fromValue(QVector3D(1.0f, 2.0f, 3.0f)));
            expectedChanges.push_back(translationChange);

            auto scaleChange = Qt3DCore::QPropertyUpdatedChangePtr::create(scaleMapping.targetId);
            scaleChange->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            scaleChange->setPropertyName(scaleMapping.propertyName);
            scaleChange->setValue(QVariant::fromValue(QVector3D(4.0f, 5.0f, 6.0f)));
            expectedChanges.push_back(scaleChange);

            QTest::newRow("vec3 translation, vec3 scale, final = false")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            normalizedTime = 0.5f;
            auto normalizedTimeChange = Qt3DCore::QPropertyUpdatedChangePtr::create(animatorId);
            normalizedTimeChange->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            normalizedTimeChange->setPropertyName("normalizedTime");
            normalizedTimeChange->setValue(normalizedTime);
            expectedChanges.push_back(normalizedTimeChange);

            finalFrame = true;
            auto animatorChange = Qt3DCore::QPropertyUpdatedChangePtr::create(animatorId);
            animatorChange->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            animatorChange->setPropertyName("running");
            animatorChange->setValue(false);
            expectedChanges.push_back(animatorChange);

            QTest::newRow("vec3 translation, vec3 scale, final = true")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Single property, double
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "mass";
            mapping.type = static_cast<int>(QVariant::Double);
            mapping.channelIndices = QVector<int>() << 0;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 3.5f;
            finalFrame = false;
            normalizedTime = -1.0f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            change->setValue(QVariant::fromValue(3.5f));
            expectedChanges.push_back(change);

            QTest::newRow("double mass")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Single property, vec2
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "pos";
            mapping.type = static_cast<int>(QVariant::Vector2D);
            mapping.channelIndices = QVector<int>() << 0 << 1;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 2.0f << 1.0f;
            finalFrame = false;
            normalizedTime = 1.1f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            change->setValue(QVariant::fromValue(QVector2D(2.0f, 1.0f)));
            expectedChanges.push_back(change);

            QTest::newRow("vec2 pos")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Single property, vec4
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "foo";
            mapping.type = static_cast<int>(QVariant::Vector4D);
            mapping.channelIndices = QVector<int>() << 0 << 1 << 2 << 3;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 4.0f << 3.0f << 2.0f << 1.0f;
            finalFrame = false;
            normalizedTime = 1.1f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            change->setValue(QVariant::fromValue(QVector4D(4.0f, 3.0f, 2.0f, 1.0f)));
            expectedChanges.push_back(change);

            QTest::newRow("vec4 foo")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Single property, quaternion
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "rotation";
            mapping.type = static_cast<int>(QVariant::Quaternion);
            mapping.channelIndices = QVector<int>() << 0 << 1 << 2 << 3;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 1.0f << 0.0f << 0.0f << 1.0f;
            finalFrame = false;
            normalizedTime = -0.1f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            change->setValue(QVariant::fromValue(QQuaternion(1.0f, 0.0f, 0.0f, 1.0f).normalized()));
            expectedChanges.push_back(change);

            QTest::newRow("quaternion rotation")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Single property, QColor
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "color";
            mapping.type = static_cast<int>(QVariant::Color);
            mapping.channelIndices = QVector<int>() << 0 << 1 << 2;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 0.5f << 0.4f << 0.3f;
            finalFrame = false;
            normalizedTime = 1.1f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            change->setValue(QVariant::fromValue(QColor::fromRgbF(0.5f, 0.4f, 0.3f)));
            expectedChanges.push_back(change);

            QTest::newRow("QColor color")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

        // Single property, QVariantList
        {
            animatorId = Qt3DCore::QNodeId::createId();
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "weights";
            mapping.type = static_cast<int>(QVariant::List);
            mapping.channelIndices = QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5 << 6;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 0.5f << 0.4f << 0.3f << 0.0f << 1.0f << 0.6f << 0.9f;
            finalFrame = false;
            normalizedTime = 1.1f; // Invalid

            auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(mapping.targetId);
            change->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
            change->setPropertyName(mapping.propertyName);
            QVariantList expectedValue = QVariantList() << 0.5f << 0.4f << 0.3f << 0.0f << 1.0f << 0.6f << 0.9f;
            change->setValue(QVariant::fromValue(expectedValue));
            expectedChanges.push_back(change);

            QTest::newRow("QVariantList weights")
                    << animatorId << mappingData << channelResults << normalizedTime
                    << finalFrame << expectedChanges;

            mappingData.clear();
            channelResults.clear();
            expectedChanges.clear();
        }

    }

    void checkPreparePropertyChanges()
    {
        // GIVEN
        QFETCH(Qt3DCore::QNodeId, animatorId);
        QFETCH(QVector<MappingData>, mappingData);
        QFETCH(QVector<float>, channelResults);
        QFETCH(float, normalizedTime);
        QFETCH(bool, finalFrame);
        QFETCH(QVector<Qt3DCore::QPropertyUpdatedChangePtr>, expectedChanges);

        // WHEN
        QVector<Qt3DCore::QSceneChangePtr> actualChanges
                = preparePropertyChanges(animatorId, mappingData, channelResults, finalFrame, normalizedTime);

        // THEN
        QCOMPARE(actualChanges.size(), expectedChanges.size());
        for (int i = 0; i < actualChanges.size(); ++i) {
            auto expectedChange = expectedChanges[i];
            auto actualChange
                    = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(actualChanges[i]);

            QCOMPARE(actualChange->subjectId(), expectedChange->subjectId());
            QCOMPARE(actualChange->deliveryFlags(), expectedChange->deliveryFlags());
            QCOMPARE(actualChange->propertyName(), expectedChange->propertyName());
            QCOMPARE(actualChange->value(), expectedChange->value());
        }
    }

    void checkPrepareCallbacks_data()
    {
        QTest::addColumn<QVector<MappingData>>("mappingData");
        QTest::addColumn<QVector<float>>("channelResults");
        QTest::addColumn<QVector<AnimationCallbackAndValue> >("expectedValues");

        QVector<MappingData> mappingData;
        QVector<float> channelResults;
        QVector<AnimationCallbackAndValue> expectedValues;

        // vec3
        {
            DummyCallback callback; // safe since the object is never used, just the address
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "translation";
            mapping.type = static_cast<int>(QVariant::Vector3D);
            mapping.channelIndices = QVector<int>() << 0 << 1 << 2;
            mapping.callback = &callback;
            mapping.callbackFlags = 0;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 1.0f << 2.0f << 3.0f;

            AnimationCallbackAndValue cbv;
            cbv.callback = mapping.callback;
            cbv.flags = mapping.callbackFlags;
            cbv.value = QVariant::fromValue<QVector3D>(QVector3D(1.0f, 2.0f, 3.0f));
            expectedValues.push_back(cbv);

            QTest::newRow("vec3 translation, no flags") << mappingData << channelResults << expectedValues;

            mappingData.clear();
            channelResults.clear();
            expectedValues.clear();
        }

        // double
        {
            DummyCallback callback;
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "something";
            mapping.type = static_cast<int>(QVariant::Double);
            mapping.channelIndices = QVector<int>() << 0;
            mapping.callback = &callback;
            mapping.callbackFlags = 0;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 1.0f;

            AnimationCallbackAndValue cbv;
            cbv.callback = mapping.callback;
            cbv.flags = mapping.callbackFlags;
            cbv.value = QVariant(double(1.0));
            expectedValues.push_back(cbv);

            QTest::newRow("double, no flags") << mappingData << channelResults << expectedValues;

            mappingData.clear();
            channelResults.clear();
            expectedValues.clear();
        }

        // float, set a flag
        {
            DummyCallback callback;
            MappingData mapping;
            mapping.targetId = Qt3DCore::QNodeId::createId();
            mapping.propertyName = "opacity";
            mapping.type = static_cast<int>(QMetaType::Float);
            mapping.channelIndices = QVector<int>() << 0;
            mapping.callback = &callback;
            mapping.callbackFlags = Qt3DAnimation::QAnimationCallback::OnThreadPool;
            mappingData.push_back(mapping);
            channelResults = QVector<float>() << 0.5f;

            AnimationCallbackAndValue cbv;
            cbv.callback = mapping.callback;
            cbv.flags = mapping.callbackFlags;
            cbv.value = QVariant(float(0.5f));
            expectedValues.push_back(cbv);

            QTest::newRow("float, OnThreadPool") << mappingData << channelResults << expectedValues;

            mappingData.clear();
            channelResults.clear();
            expectedValues.clear();
        }
    }

    void checkPrepareCallbacks()
    {
        // GIVEN
        QFETCH(QVector<MappingData>, mappingData);
        QFETCH(QVector<float>, channelResults);
        QFETCH(QVector<AnimationCallbackAndValue>, expectedValues);

        // WHEN
        QVector<AnimationCallbackAndValue> callbacks = prepareCallbacks(mappingData, channelResults);

        // THEN
        QCOMPARE(callbacks.size(), expectedValues.size());
        for (int i = 0; i < callbacks.size(); ++i) {
            auto expected = expectedValues[i];
            auto actual = callbacks[i];

            QCOMPARE(actual.callback, expected.callback);
            QCOMPARE(actual.flags, expected.flags);
            QCOMPARE(actual.value, expected.value);
        }
    }

    void checkEvaluateClipAtLocalTime_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<AnimationClip *>("clip");
        QTest::addColumn<float>("localTime");
        QTest::addColumn<ClipResults>("expectedResults");

        Handler *handler;
        AnimationClip *clip;
        float localTime;
        ClipResults expectedResults;

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            localTime = 0.0f;
            expectedResults = QVector<float>() << 0.0f << 0.0f << 0.0f;

            QTest::newRow("clip1.json, t = 0.0")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            localTime = clip->duration();
            expectedResults = QVector<float>() << 5.0f << 0.0f << 0.0f;

            QTest::newRow("clip1.json, t = duration")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            localTime = clip->duration() / 2.0f;
            expectedResults = QVector<float>() << 2.5f << 0.0f << 0.0f;

            QTest::newRow("clip1.json, t = duration/2")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip2.json"));
            localTime = 0.0f;
            expectedResults = QVector<float>()
                    << 0.0f << 0.0f << 0.0f             // Translation
                    << 1.0f << 0.0f << 0.0f << 0.0f;    // Rotation

            QTest::newRow("clip2.json, t = 0.0")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }
        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip2.json"));
            localTime = clip->duration();
            expectedResults = QVector<float>()
                    << 5.0f << 0.0f << 0.0f             // Translation
                    << 0.0f << 0.0f << -1.0f << 0.0f;   // Rotation

            QTest::newRow("clip2.json, t = duration")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }
        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip2.json"));
            localTime = clip->duration() / 2.0f;
            expectedResults = QVector<float>()
                    << 2.5f << 0.0f << 0.0f             // Translation
                    << 0.5f << 0.0f << -0.5f << 0.0f;   // Rotation

            QTest::newRow("clip2.json, t = duration/2")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }
        {
            // a clip with linear interpolation
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip4.json"));
            localTime = clip->duration();
            expectedResults = QVector<float>() << 5.0 << -2.0f << 6.0f;

            QTest::newRow("clip4.json, linear, t = duration")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }
        {
            // a clip with linear interpolation
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip4.json"));
            localTime = clip->duration() / 2.0f;
            expectedResults = QVector<float>() << 2.5f << -1.0f << 3.0f;

            QTest::newRow("clip4.json, linear, t = duration/2")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }
        {
            // a clip with slerp interpolation
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip6.json"));
            localTime = clip->duration() / 2.0f;
            expectedResults = QVector<float>() << 0.923822f << 0.382626f << 0.0f << 0.0f;

            QTest::newRow("clip6.json, slerp, t = duration/2")
                    << handler << clip << localTime << expectedResults;
            expectedResults.clear();
        }
    }

    void checkEvaluateClipAtLocalTime()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(AnimationClip *, clip);
        QFETCH(float, localTime);
        QFETCH(ClipResults, expectedResults);

        // WHEN
        ClipResults actualResults = evaluateClipAtLocalTime(clip, localTime);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            auto actual = actualResults[i];
            auto expected = expectedResults[i];

            QVERIFY(fuzzyCompare(actual, expected) == true);
        }

        // Cleanup
        delete handler;
    }

    void checkEvaluateClipAtPhase_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<AnimationClip *>("clip");
        QTest::addColumn<float>("phase");
        QTest::addColumn<ClipResults>("expectedResults");

        Handler *handler;
        AnimationClip *clip;
        float phase;
        ClipResults expectedResults;

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            phase = 0.0f;
            expectedResults = QVector<float>() << 0.0f << 0.0f << 0.0f;

            QTest::newRow("clip1.json, phi = 0.0")
                    << handler << clip << phase << expectedResults;
            expectedResults.clear();
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            phase = 1.0f;
            expectedResults = QVector<float>() << 5.0f << 0.0f << 0.0f;

            QTest::newRow("clip1.json, phi = 1.0")
                    << handler << clip << phase << expectedResults;
            expectedResults.clear();
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            phase = 0.5f;
            expectedResults = QVector<float>() << 2.5f << 0.0f << 0.0f;

            QTest::newRow("clip1.json, phi = 0.5")
                    << handler << clip << phase << expectedResults;
            expectedResults.clear();
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip2.json"));
            phase = 0.0f;
            expectedResults = QVector<float>()
                    << 0.0f << 0.0f << 0.0f             // Translation
                    << 1.0f << 0.0f << 0.0f << 0.0f;    // Rotation

            QTest::newRow("clip2.json, phi = 0.0")
                    << handler << clip << phase << expectedResults;
            expectedResults.clear();
        }
        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip2.json"));
            phase = 1.0f;
            expectedResults = QVector<float>()
                    << 5.0f << 0.0f << 0.0f             // Translation
                    << 0.0f << 0.0f << -1.0f << 0.0f;   // Rotation

            QTest::newRow("clip2.json, t = 1.0")
                    << handler << clip << phase << expectedResults;
            expectedResults.clear();
        }
        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip2.json"));
            phase = 0.5f;
            expectedResults = QVector<float>()
                    << 2.5f << 0.0f << 0.0f             // Translation
                    << 0.5f << 0.0f << -0.5f << 0.0f;   // Rotation

            QTest::newRow("clip2.json, phi = 0.5")
                    << handler << clip << phase << expectedResults;
            expectedResults.clear();
        }
    }

    void checkEvaluateClipAtPhase()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(AnimationClip *, clip);
        QFETCH(float, phase);
        QFETCH(ClipResults, expectedResults);

        // WHEN
        ClipResults actualResults = evaluateClipAtPhase(clip, phase);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            auto actual = actualResults[i];
            auto expected = expectedResults[i];

            QVERIFY(fuzzyCompare(actual, expected) == true);
        }

        // Cleanup
        delete handler;
    }

    void checkChannelComponentsToIndicesHelper_data()
    {
        QTest::addColumn<Channel>("channel");
        QTest::addColumn<int>("dataType");
        QTest::addColumn<int>("expectedChannelComponentCount");
        QTest::addColumn<int>("offset");
        QTest::addColumn<QVector<char>>("suffixes");
        QTest::addColumn<QVector<int>>("expectedResults");

        Channel channel;
        int dataType;
        int expectedChannelComponentCount;
        int offset;
        QVector<char> suffixes;
        QVector<int> expectedResults;

        // already sorted vec3, no component names, with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("Location");
            channel.channelComponents.resize(3);
            // leave 'name' empty

            dataType = static_cast<int>(QVariant::Vector3D);
            expectedChannelComponentCount = 3;
            offset = 0;
            // suffixes expected to be ignored
            expectedResults = (QVector<int>() << 0 << 1 << 2);

            QTest::newRow("vec3 location, pre-sorted, no component names, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 4;
            expectedResults = (QVector<int>() << 4 << 5 << 6);
            QTest::newRow("vec3 location, pre-sorted, no component names, offset = 4")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;
            expectedResults.clear();
        }

        // vec3 with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("Location");
            channel.channelComponents.resize(3);
            channel.channelComponents[0].name = QLatin1String("Location X");
            channel.channelComponents[1].name = QLatin1String("Location Y");
            channel.channelComponents[2].name = QLatin1String("Location Z");

            dataType = static_cast<int>(QVariant::Vector3D);
            expectedChannelComponentCount = 3;
            offset = 0;
            suffixes = (QVector<char>() << 'X' << 'Y' << 'Z' << 'W');
            expectedResults = (QVector<int>() << 0 << 1 << 2);

            QTest::newRow("vec3 location, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 4;
            expectedResults = (QVector<int>() << 4 << 5 << 6);
            QTest::newRow("vec3 location, offset = 4")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // vec2 with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("pos");
            channel.channelComponents.resize(2);
            channel.channelComponents[0].name = QLatin1String("pos X");
            channel.channelComponents[1].name = QLatin1String("pos Y");

            dataType = static_cast<int>(QVariant::Vector2D);
            expectedChannelComponentCount = 2;
            offset = 0;
            suffixes = (QVector<char>() << 'X' << 'Y' << 'Z' << 'W');
            expectedResults = (QVector<int>() << 0 << 1);

            QTest::newRow("vec2 pos, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 2;
            expectedResults = (QVector<int>() << 2 << 3);
            QTest::newRow("vec2 pos, offset = 2")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // vec4 with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("foo");
            channel.channelComponents.resize(4);
            channel.channelComponents[0].name = QLatin1String("foo X");
            channel.channelComponents[1].name = QLatin1String("foo Y");
            channel.channelComponents[2].name = QLatin1String("foo Z");
            channel.channelComponents[3].name = QLatin1String("foo W");

            dataType = static_cast<int>(QVariant::Vector4D);
            expectedChannelComponentCount = 4;
            offset = 0;
            suffixes = (QVector<char>() << 'X' << 'Y' << 'Z' << 'W');
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3);

            QTest::newRow("vec4 foo, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13);
            QTest::newRow("vec4 foo, offset = 10")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // double with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("foo");
            channel.channelComponents.resize(1);
            channel.channelComponents[0].name = QLatin1String("Mass X");

            dataType = static_cast<int>(QVariant::Double);
            expectedChannelComponentCount = 1;
            offset = 0;
            suffixes = (QVector<char>() << 'X' << 'Y' << 'Z' << 'W');
            expectedResults = (QVector<int>() << 0);

            QTest::newRow("double Mass, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 5;
            expectedResults = (QVector<int>() << 5);
            QTest::newRow("double Mass, offset = 5")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // quaternion with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("Rotation");
            channel.channelComponents.resize(4);
            channel.channelComponents[0].name = QLatin1String("Rotation W");
            channel.channelComponents[1].name = QLatin1String("Rotation X");
            channel.channelComponents[2].name = QLatin1String("Rotation Y");
            channel.channelComponents[3].name = QLatin1String("Rotation Z");

            dataType = static_cast<int>(QVariant::Quaternion);
            expectedChannelComponentCount = 4;
            offset = 0;
            suffixes = (QVector<char>() << 'W' << 'X' << 'Y' << 'Z');
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3);

            QTest::newRow("quaternion Rotation, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13);
            QTest::newRow("quaternion Rotation, offset = 10")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // quaternion with and without offset, randomized
        {
            channel = Channel();
            channel.name = QLatin1String("Rotation");
            channel.channelComponents.resize(4);
            channel.channelComponents[0].name = QLatin1String("Rotation X");
            channel.channelComponents[1].name = QLatin1String("Rotation W");
            channel.channelComponents[2].name = QLatin1String("Rotation Z");
            channel.channelComponents[3].name = QLatin1String("Rotation Y");

            dataType = static_cast<int>(QVariant::Quaternion);
            expectedChannelComponentCount = 4;
            offset = 0;
            suffixes = (QVector<char>() << 'W' << 'X' << 'Y' << 'Z');
            expectedResults = (QVector<int>() << 1 << 0 << 3 << 2);

            QTest::newRow("quaternion Rotation, offset = 0, randomized")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 11 << 10 << 13 << 12);
            QTest::newRow("quaternion Rotation, offset = 10, randomized")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // color with and without offset 3 components
        {
            channel = Channel();
            channel.name = QLatin1String("Color");
            channel.channelComponents.resize(3);
            channel.channelComponents[0].name = QLatin1String("Color R");
            channel.channelComponents[1].name = QLatin1String("Color G");
            channel.channelComponents[2].name = QLatin1String("Color B");

            dataType = static_cast<int>(QVariant::Color);
            expectedChannelComponentCount = 3;
            offset = 0;
            suffixes = (QVector<char>() << 'R' << 'G' << 'B');
            expectedResults = (QVector<int>() << 0 << 1 << 2);

            QTest::newRow("QColor RGB Color, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12);
            QTest::newRow("QColor RGB Color, offset = 10")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // color with and without offset 4 components
        {
            channel = Channel();
            channel.name = QLatin1String("Color");
            channel.channelComponents.resize(4);
            channel.channelComponents[0].name = QLatin1String("Color R");
            channel.channelComponents[1].name = QLatin1String("Color G");
            channel.channelComponents[2].name = QLatin1String("Color B");
            channel.channelComponents[3].name = QLatin1String("Color A");

            dataType = static_cast<int>(QVariant::Color);
            expectedChannelComponentCount = 4;
            offset = 0;
            suffixes = (QVector<char>() << 'R' << 'G' << 'B' << 'A');
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3);

            QTest::newRow("QColor RGBA Color, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13);
            QTest::newRow("QColor RGBA Color, offset = 10")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;

            suffixes.clear();
            expectedResults.clear();
        }

        // weights as list with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("MorphWeights");
            channel.channelComponents.resize(6);
            // leave channel component names empty

            dataType = static_cast<int>(QVariant::List);
            expectedChannelComponentCount = 6;
            offset = 0;
            // suffixes expected to be ignored
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);

            QTest::newRow("MorphWeights List count = 6, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;
            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13 << 14 << 15);
            QTest::newRow("MorphWeights List count = 6, offset = 10")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;
            suffixes.clear();
            expectedResults.clear();
        }

        // weights as vec of float with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("MorphWeights");
            channel.channelComponents.resize(6);
            // leave channel component names empty

            dataType = qMetaTypeId<QVector<float>>();
            expectedChannelComponentCount = 6;
            offset = 0;
            // suffixes expected to be ignored
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3 << 4 << 5);

            QTest::newRow("MorphWeights Vec count = 6, offset = 0")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;
            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13 << 14 << 15);
            QTest::newRow("MorphWeights Vec count = 6, offset = 10")
                    << channel << dataType << expectedChannelComponentCount
                    << offset << suffixes << expectedResults;
            suffixes.clear();
            expectedResults.clear();
        }
    }

    void checkChannelComponentsToIndicesHelper()
    {
        // GIVEN
        QFETCH(Channel, channel);
        QFETCH(int, offset);
        QFETCH(int, expectedChannelComponentCount);
        QFETCH(QVector<char>, suffixes);
        QFETCH(QVector<int>, expectedResults);

        // WHEN
        QVector<int> actualResults
                = channelComponentsToIndicesHelper(channel, expectedChannelComponentCount,
                                                   offset, suffixes);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            QCOMPARE(actualResults[i], expectedResults[i]);
        }
    }

    void checkChannelComponentsToIndices_data()
    {
        QTest::addColumn<Channel>("channel");
        QTest::addColumn<int>("dataType");
        QTest::addColumn<int>("componentCount");
        QTest::addColumn<int>("offset");
        QTest::addColumn<QVector<int>>("expectedResults");

        Channel channel;
        int dataType;
        int componentCount;
        int offset;
        QVector<int> expectedResults;

        // Quaternion
        {
            channel = Channel();
            channel.name = QLatin1String("Rotation");
            channel.channelComponents.resize(4);
            channel.channelComponents[0].name = QLatin1String("Rotation W");
            channel.channelComponents[1].name = QLatin1String("Rotation X");
            channel.channelComponents[2].name = QLatin1String("Rotation Y");
            channel.channelComponents[3].name = QLatin1String("Rotation Z");

            dataType = static_cast<int>(QVariant::Quaternion);
            componentCount = 4;
            offset = 0;
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3);

            QTest::newRow("quaternion Rotation, offset = 0")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13);
            QTest::newRow("quaternion Rotation, offset = 10")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();
        }

        // vec3 with and without offset
        {
            channel = Channel();
            channel.name = QLatin1String("Location");
            channel.channelComponents.resize(3);
            channel.channelComponents[0].name = QLatin1String("Location X");
            channel.channelComponents[1].name = QLatin1String("Location Y");
            channel.channelComponents[2].name = QLatin1String("Location Z");

            dataType = static_cast<int>(QVariant::Vector3D);
            componentCount = 3;
            offset = 0;
            expectedResults = (QVector<int>() << 0 << 1 << 2);

            QTest::newRow("vec3 location, offset = 0")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();

            offset = 4;
            expectedResults = (QVector<int>() << 4 << 5 << 6);
            QTest::newRow("vec3 location, offset = 4")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();
        }

        // QColor
        {
            channel = Channel();
            channel.name = QLatin1String("Color");
            channel.channelComponents.resize(3);
            channel.channelComponents[0].name = QLatin1String("Color R");
            channel.channelComponents[1].name = QLatin1String("Color G");
            channel.channelComponents[2].name = QLatin1String("Color B");

            dataType = static_cast<int>(QVariant::Color);
            componentCount = 3;
            offset = 0;
            expectedResults = (QVector<int>() << 0 << 1 << 2);

            QTest::newRow("QColor RGB Color, offset = 0")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12);
            QTest::newRow("QColor RGB Color, offset = 10")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();
        }

        {
            channel = Channel();
            channel.name = QLatin1String("Color");
            channel.channelComponents.resize(4);
            channel.channelComponents[0].name = QLatin1String("Color R");
            channel.channelComponents[1].name = QLatin1String("Color G");
            channel.channelComponents[2].name = QLatin1String("Color B");
            channel.channelComponents[3].name = QLatin1String("Color A");

            dataType = static_cast<int>(QVariant::Color);
            componentCount = 4;
            offset = 0;
            expectedResults = (QVector<int>() << 0 << 1 << 2 << 3);

            QTest::newRow("QColor RGBA Color, offset = 0")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();

            offset = 10;
            expectedResults = (QVector<int>() << 10 << 11 << 12 << 13);
            QTest::newRow("QColor RGBA Color, offset = 10")
                    << channel << dataType << componentCount << offset << expectedResults;

            expectedResults.clear();
        }
    }

    void checkChannelComponentsToIndices()
    {
        QFETCH(Channel, channel);
        QFETCH(int, dataType);
        QFETCH(int, componentCount);
        QFETCH(int, offset);
        QFETCH(QVector<int>, expectedResults);

        // WHEN
        QVector<int> actualResults
                = channelComponentsToIndices(channel, dataType, componentCount, offset);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            QCOMPARE(actualResults[i], expectedResults[i]);
        }
    }

    void checkEvaluationDataForClip_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<AnimationClip *>("clip");
        QTest::addColumn<AnimatorEvaluationData>("animatorData");
        QTest::addColumn<ClipEvaluationData>("expectedClipData");

        Handler *handler;
        AnimationClip *clip;
        AnimatorEvaluationData animatorData;
        ClipEvaluationData clipData;
        auto* clock = new Clock;

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            auto animator = createClipAnimator(handler, globalStartTimeNS, loops);
            animator->setCurrentLoop(0);
            clipData.currentLoop = animator->currentLoop();
            const qint64 elapsedTimeNS = 0;
            animatorData = evaluationDataForAnimator(animator, clock, elapsedTimeNS); // Tested elsewhere

            clipData.localTime = localTimeFromElapsedTime(animatorData.currentTime,
                                                         animatorData.elapsedTime,
                                                         animatorData.playbackRate,
                                                         clip->duration(),
                                                         animatorData.loopCount,
                                                         clipData.currentLoop); // Tested elsewhere
            clipData.isFinalFrame = false;

            QTest::newRow("clip1.json, globalTime = 0")
                    << handler << clip << animatorData << clipData;
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            auto animator = createClipAnimator(handler, globalStartTimeNS, loops);
            animator->setCurrentLoop(0);
            clipData.currentLoop = animator->currentLoop();
            const qint64 elapsedTimeNS = toNsecs(clip->duration()+1); // +1 to ensure beyond end
            animatorData = evaluationDataForAnimator(animator, nullptr, elapsedTimeNS); // Tested elsewhere

            clipData.localTime = localTimeFromElapsedTime(animatorData.currentTime,
                                                         animatorData.elapsedTime,
                                                         animatorData.playbackRate,
                                                         clip->duration(),
                                                         animatorData.loopCount,
                                                         clipData.currentLoop); // Tested elsewhere
            clipData.isFinalFrame = true;

            QTest::newRow("clip1.json, elapsedTime = duration + 1")
                    << handler << clip << animatorData << clipData;
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            const qint64 globalStartTimeNS = 0;
            const int loops = 0; // Infinite loops
            auto animator = createClipAnimator(handler, globalStartTimeNS, loops);
            animator->setCurrentLoop(0);
            clipData.currentLoop = animator->currentLoop();
            const qint64 elapsedTimeNS = toNsecs(2.0 * clip->duration());
            animatorData = evaluationDataForAnimator(animator, clock, elapsedTimeNS); // Tested elsewhere

            clipData.localTime = localTimeFromElapsedTime(animatorData.currentTime,
                                                         animatorData.elapsedTime,
                                                         animatorData.playbackRate,
                                                         clip->duration(),
                                                         animatorData.loopCount,
                                                         clipData.currentLoop); // Tested elsewhere
            clipData.isFinalFrame = false;

            QTest::newRow("clip1.json, elapsedTime = 2 * duration, loops = infinite")
                    << handler << clip << animatorData << clipData;
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            const qint64 globalStartTimeNS = 0;
            const int loops = 2;
            auto animator = createClipAnimator(handler, globalStartTimeNS, loops);
            animator->setCurrentLoop(0);
            clipData.currentLoop = animator->currentLoop();
            const qint64 elapsedTimeNS = toNsecs(2.0 * clip->duration() + 1.0); // +1 to ensure beyond end of clip
            animatorData = evaluationDataForAnimator(animator, nullptr, elapsedTimeNS); // Tested elsewhere

            clipData.localTime = localTimeFromElapsedTime(animatorData.currentTime,
                                                         animatorData.elapsedTime,
                                                         animatorData.playbackRate,
                                                         clip->duration(),
                                                         animatorData.loopCount,
                                                         clipData.currentLoop); // Tested elsewhere
            clipData.isFinalFrame = true;

            QTest::newRow("clip1.json, elapsedTime = 2 * duration + 1, loops = 2")
                    << handler << clip << animatorData << clipData;
        }

        {
            handler = new Handler();
            clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));
            const qint64 globalStartTimeNS = 0;
            const int loops = 2;
            auto animator = createClipAnimator(handler, globalStartTimeNS, loops);
            animator->setCurrentLoop(1);
            clipData.currentLoop = animator->currentLoop();
            const qint64 elapsedTimeNS = toNsecs(clip->duration() + 1.0); // +1 to ensure beyond end of clip
            animatorData = evaluationDataForAnimator(animator, nullptr, elapsedTimeNS); // Tested elsewhere

            clipData.localTime = localTimeFromElapsedTime(animatorData.currentTime,
                                                         animatorData.elapsedTime,
                                                         animatorData.playbackRate,
                                                         clip->duration(),
                                                         animatorData.loopCount,
                                                         clipData.currentLoop); // Tested elsewhere
            clipData.isFinalFrame = true;

            QTest::newRow("clip1.json, elapsedTime = duration + 1, loops = 2, current_loop = 1")
                    << handler << clip << animatorData << clipData;
        }
    }

    void checkEvaluationDataForClip()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(AnimationClip *, clip);
        QFETCH(AnimatorEvaluationData, animatorData);
        QFETCH(ClipEvaluationData, expectedClipData);

        // WHEN
        ClipEvaluationData actualClipData = evaluationDataForClip(clip, animatorData);

        // THEN
        QCOMPARE(actualClipData.currentLoop, expectedClipData.currentLoop);
        QVERIFY(fuzzyCompare(actualClipData.localTime, expectedClipData.localTime) == true);
        QCOMPARE(actualClipData.isFinalFrame, expectedClipData.isFinalFrame);

        // Cleanup
        delete handler;
    }

    void checkEvaluationDataForAnimator_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<ClipAnimator *>("animator");
        QTest::addColumn<qint64>("elapsedTime");
        QTest::addColumn<AnimatorEvaluationData>("expectedAnimatorData");

        Handler *handler;
        ClipAnimator *animator;
        qint64 elapsedTimeNS;
        AnimatorEvaluationData expectedAnimatorData;

        {
            handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            elapsedTimeNS = 0;

            expectedAnimatorData.loopCount = loops;
            expectedAnimatorData.playbackRate = 1.0; // hard-wired for now
            expectedAnimatorData.elapsedTime = 0.0;

            QTest::newRow("globalStartTime = 0, elapsedTime = 0, loops = 1")
                    << handler << animator << elapsedTimeNS << expectedAnimatorData;
        }

        {
            handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loops = 5;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            elapsedTimeNS = 0;

            expectedAnimatorData.loopCount = loops;
            expectedAnimatorData.playbackRate = 1.0; // hard-wired for now
            expectedAnimatorData.elapsedTime = 0.0;

            QTest::newRow("globalStartTime = 0, elapsedTime = 0, loops = 5")
                    << handler << animator << elapsedTimeNS << expectedAnimatorData;
        }

        {
            handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            elapsedTimeNS = 5000000000;

            expectedAnimatorData.loopCount = loops;
            expectedAnimatorData.playbackRate = 1.0; // hard-wired for now
            expectedAnimatorData.elapsedTime = 5.0;

            QTest::newRow("globalStartTime = 0, elapsedTime = 5, loops = 1")
                    << handler << animator << elapsedTimeNS << expectedAnimatorData;
        }

        {
            handler = new Handler();
            const qint64 globalStartTimeNS = 3000000000;
            const int loops = 1;
            animator = createClipAnimator(handler, globalStartTimeNS, loops);
            elapsedTimeNS = 2000000000;

            expectedAnimatorData.loopCount = loops;
            expectedAnimatorData.playbackRate = 1.0; // hard-wired for now
            expectedAnimatorData.elapsedTime = 2.0;

            QTest::newRow("globalStartTime = 3, elapsedTime = 2, loops = 1")
                    << handler << animator << elapsedTimeNS << expectedAnimatorData;
        }
    }

    void checkEvaluationDataForAnimator()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(ClipAnimator *, animator);
        QFETCH(qint64, elapsedTime);
        QFETCH(AnimatorEvaluationData, expectedAnimatorData);

        // WHEN
        AnimatorEvaluationData actualAnimatorData = evaluationDataForAnimator(animator, nullptr, elapsedTime);

        // THEN
        QCOMPARE(actualAnimatorData.loopCount, expectedAnimatorData.loopCount);
        QVERIFY(fuzzyCompare(actualAnimatorData.playbackRate, expectedAnimatorData.playbackRate) == true);
        QVERIFY(fuzzyCompare(actualAnimatorData.elapsedTime, expectedAnimatorData.elapsedTime) == true);

        // Cleanup
        delete handler;
    }

    void checkGatherValueNodesToEvaluate_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<Qt3DCore::QNodeId>("blendTreeRootId");
        QTest::addColumn<QVector<Qt3DCore::QNodeId>>("expectedIds");

        {
            Handler *handler = new Handler;

            const auto lerp = createLerpClipBlend(handler);
            const auto value1 = createClipBlendValue(handler);
            const auto clip1Id = Qt3DCore::QNodeId::createId();
            value1->setClipId(clip1Id);
            lerp->setStartClipId(value1->peerId());

            const auto value2 = createClipBlendValue(handler);
            const auto clip2Id = Qt3DCore::QNodeId::createId();
            value2->setClipId(clip2Id);
            lerp->setEndClipId(value2->peerId());

            QVector<Qt3DCore::QNodeId> expectedIds = { value1->peerId(), value2->peerId() };

            QTest::newRow("simple lerp") << handler << lerp->peerId() << expectedIds;
        }

        {
            Handler *handler = new Handler;

            const auto value1 = createClipBlendValue(handler);
            const auto clip1Id = Qt3DCore::QNodeId::createId();
            value1->setClipId(clip1Id);

            QVector<Qt3DCore::QNodeId> expectedIds = { value1->peerId() };

            QTest::newRow("value only") << handler << value1->peerId() << expectedIds;
        }
    }

    void checkGatherValueNodesToEvaluate()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(Qt3DCore::QNodeId, blendTreeRootId);
        QFETCH(QVector<Qt3DCore::QNodeId>, expectedIds);

        // WHEN
        QVector<Qt3DCore::QNodeId> actualIds = gatherValueNodesToEvaluate(handler, blendTreeRootId);

        // THEN
        QCOMPARE(actualIds.size(), expectedIds.size());
        for (int i = 0; i < actualIds.size(); ++i)
            QCOMPARE(actualIds[i], expectedIds[i]);

        // Cleanup
        delete handler;
    }

    void checkEvaluateBlendTree_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<BlendedClipAnimator *>("animator");
        QTest::addColumn<Qt3DCore::QNodeId>("blendNodeId");
        QTest::addColumn<ClipResults>("expectedResults");

        {
            /*
                ValueNode1----
                             |
                             MeanBlendNode
                             |
                ValueNode2----
            */

            auto handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loopCount = 1;
            auto animator = createBlendedClipAnimator(handler, globalStartTimeNS, loopCount);

            // Set up the blend node and dependencies (evaluated clip results of the
            // dependent nodes in the animator indexed by their ids).
            MeanBlendNode *blendNode = createMeanBlendNode(handler);

            // First clip to use in the mean
            auto valueNode1 = createClipBlendValue(handler);
            ClipResults valueNode1Results = { 0.0f, 0.0f, 0.0f };
            valueNode1->setClipResults(animator->peerId(), valueNode1Results);

            // Second clip to use in the mean
            auto valueNode2 = createClipBlendValue(handler);
            ClipResults valueNode2Results = { 1.0f, 1.0f, 1.0f };
            valueNode2->setClipResults(animator->peerId(), valueNode2Results);

            blendNode->setValueNodeIds(valueNode1->peerId(), valueNode2->peerId());

            ClipResults expectedResults = { 0.5f, 0.5f, 0.5f };

            QTest::newRow("mean node, 1 channel")
                    << handler << animator << blendNode->peerId() << expectedResults;
        }

        {
            /*
                ValueNode1----
                             |
                             MeanBlendNode
                             |
                ValueNode2----
            */

            auto handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loopCount = 1;
            auto animator = createBlendedClipAnimator(handler, globalStartTimeNS, loopCount);

            // Set up the blend node and dependencies (evaluated clip results of the
            // dependent nodes in the animator indexed by their ids).
            MeanBlendNode *blendNode = createMeanBlendNode(handler);

            // First clip to use in the mean
            auto valueNode1 = createClipBlendValue(handler);
            ClipResults valueNode1Results = { 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f };
            valueNode1->setClipResults(animator->peerId(), valueNode1Results);

            // Second clip to use in the mean
            auto valueNode2 = createClipBlendValue(handler);
            ClipResults valueNode2Results = { 1.0f, 1.0f, 1.0f, 2.0f, 4.0f, 6.0f };
            valueNode2->setClipResults(animator->peerId(), valueNode2Results);

            blendNode->setValueNodeIds(valueNode1->peerId(), valueNode2->peerId());

            ClipResults expectedResults = { 0.5f, 0.5f, 0.5f, 1.5f, 3.0f, 4.5f };

            QTest::newRow("mean node, 2 channels")
                    << handler << animator << blendNode->peerId() << expectedResults;
        }

        {
            /*
                ValueNode1----
                             |
                             MeanBlendNode1------
                             |                  |
                ValueNode2----                  |
                                                MeanBlendNode3
                ValueNode3----                  |
                             |                  |
                             MeanBlendNode2------
                             |
                ValueNode4----
            */

            auto handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loopCount = 1;
            auto animator = createBlendedClipAnimator(handler, globalStartTimeNS, loopCount);

            // Set up the blend node and dependencies (evaluated clip results of the
            // dependent nodes in the animator indexed by their ids).

            // MeanBlendNode1
            MeanBlendNode *meanNode1 = createMeanBlendNode(handler);

            // First clip to use in mean1
            auto valueNode1 = createClipBlendValue(handler);
            ClipResults valueNode1Results = { 0.0f, 0.0f, 0.0f };
            valueNode1->setClipResults(animator->peerId(), valueNode1Results);

            // Second clip to use in mean1
            auto valueNode2 = createClipBlendValue(handler);
            ClipResults valueNode2Results = { 2.0f, 2.0f, 2.0f };
            valueNode2->setClipResults(animator->peerId(), valueNode2Results);

            meanNode1->setValueNodeIds(valueNode1->peerId(), valueNode2->peerId());


            // MeanBlendNode2
            MeanBlendNode *meanNode2 = createMeanBlendNode(handler);

            // First clip to use in mean1
            auto valueNode3 = createClipBlendValue(handler);
            ClipResults valueNode3Results = { 10.0f, 10.0f, 10.0f };
            valueNode3->setClipResults(animator->peerId(), valueNode3Results);

            // Second clip to use in mean1
            auto valueNode4 = createClipBlendValue(handler);
            ClipResults valueNode4Results = { 20.0f, 20.0f, 20.0f };
            valueNode4->setClipResults(animator->peerId(), valueNode4Results);

            meanNode2->setValueNodeIds(valueNode3->peerId(), valueNode4->peerId());


            // MeanBlendNode3
            MeanBlendNode *meanNode3 = createMeanBlendNode(handler);
            meanNode3->setValueNodeIds(meanNode1->peerId(), meanNode2->peerId());

            // Mean1 = 1
            // Mean2 = 15
            // Mean3 = (1 + 15 ) / 2 = 8
            ClipResults expectedResults = { 8.0f, 8.0f, 8.0f };

            QTest::newRow("3 mean nodes, 1 channel")
                    << handler << animator << meanNode3->peerId() << expectedResults;
        }

        {
            /*
                ValueNode1----
                             |
                             MeanBlendNode1------
                             |                  |
                ValueNode2----                  |
                                                MeanBlendNode3---
                ValueNode3----                  |               |
                             |                  |               |
                             MeanBlendNode2------               AdditiveBlendNode1
                             |                                  |
                ValueNode4----                                  |
                                                ValueNode5-------
            */

            auto handler = new Handler();
            const qint64 globalStartTimeNS = 0;
            const int loopCount = 1;
            auto animator = createBlendedClipAnimator(handler, globalStartTimeNS, loopCount);

            // Set up the blend node and dependencies (evaluated clip results of the
            // dependent nodes in the animator indexed by their ids).

            // MeanBlendNode1
            MeanBlendNode *meanNode1 = createMeanBlendNode(handler);

            // First clip to use in mean1
            auto valueNode1 = createClipBlendValue(handler);
            ClipResults valueNode1Results = { 0.0f, 0.0f, 0.0f };
            valueNode1->setClipResults(animator->peerId(), valueNode1Results);

            // Second clip to use in mean1
            auto valueNode2 = createClipBlendValue(handler);
            ClipResults valueNode2Results = { 2.0f, 2.0f, 2.0f };
            valueNode2->setClipResults(animator->peerId(), valueNode2Results);

            meanNode1->setValueNodeIds(valueNode1->peerId(), valueNode2->peerId());


            // MeanBlendNode2
            MeanBlendNode *meanNode2 = createMeanBlendNode(handler);

            // First clip to use in mean2
            auto valueNode3 = createClipBlendValue(handler);
            ClipResults valueNode3Results = { 10.0f, 10.0f, 10.0f };
            valueNode3->setClipResults(animator->peerId(), valueNode3Results);

            // Second clip to use in mean2
            auto valueNode4 = createClipBlendValue(handler);
            ClipResults valueNode4Results = { 20.0f, 20.0f, 20.0f };
            valueNode4->setClipResults(animator->peerId(), valueNode4Results);

            meanNode2->setValueNodeIds(valueNode3->peerId(), valueNode4->peerId());


            // MeanBlendNode3
            MeanBlendNode *meanNode3 = createMeanBlendNode(handler);
            meanNode3->setValueNodeIds(meanNode1->peerId(), meanNode2->peerId());


            // AdditiveBlendNode1
            AdditiveClipBlend *additiveBlendNode1 = createAdditiveClipBlend(handler);
            auto valueNode5 = createClipBlendValue(handler);
            ClipResults valueNode5Results = { 1.0f, 2.0f, 3.0f };
            valueNode5->setClipResults(animator->peerId(), valueNode5Results);

            additiveBlendNode1->setBaseClipId(meanNode3->peerId());
            additiveBlendNode1->setAdditiveClipId(valueNode5->peerId());
            additiveBlendNode1->setAdditiveFactor(0.5);

            // Mean1 = 1
            // Mean2 = 15
            // Mean3 = (1 + 15 ) / 2 = 8
            // Additive1 = 8 + 0.5 * (1, 2, 3) = (8.5, 9, 9.5)
            ClipResults expectedResults = { 8.5f, 9.0f, 9.5f };

            QTest::newRow("3 mean nodes + additive, 1 channel")
                    << handler << animator << additiveBlendNode1->peerId() << expectedResults;
        }
    }

    void checkEvaluateBlendTree()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(BlendedClipAnimator *, animator);
        QFETCH(Qt3DCore::QNodeId, blendNodeId);
        QFETCH(ClipResults, expectedResults);

        // WHEN
        const ClipResults actualResults = evaluateBlendTree(handler, animator, blendNodeId);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i)
            QCOMPARE(actualResults[i], expectedResults[i]);

        // Cleanup
        delete handler;
    }

    void checkFormatClipResults_data()
    {
        QTest::addColumn<ClipResults>("rawClipResults");
        QTest::addColumn<ComponentIndices>("format");
        QTest::addColumn<ClipResults>("expectedResults");

        {
            ClipResults rawClipResults = { 1.0f, 2.0f, 3.0f };
            ComponentIndices format = { 0, 1, 2 };
            ClipResults expectedResults = { 1.0f, 2.0f, 3.0f };

            QTest::newRow("identity")
                    << rawClipResults << format << expectedResults;
        }

        {
            ClipResults rawClipResults = { 1.0f, 2.0f };
            ComponentIndices format = { 1, 0 };
            ClipResults expectedResults = { 2.0f, 1.0f };

            QTest::newRow("swap")
                    << rawClipResults << format << expectedResults;
        }

        {
            ClipResults rawClipResults = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
            ComponentIndices format = { 0, 2, 1, 3, 4 };
            ClipResults expectedResults = { 1.0f, 3.0f, 2.0f, 4.0f, 5.0f };

            QTest::newRow("swap subset")
                    << rawClipResults << format << expectedResults;
        }

        {
            ClipResults rawClipResults = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
            ComponentIndices format = { 4, 3, 2, 1, 0 };
            ClipResults expectedResults = { 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };

            QTest::newRow("reverse")
                    << rawClipResults << format << expectedResults;
        }

        {
            ClipResults rawClipResults = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
            ComponentIndices format = { 0, 1, -1, 3, 4 };
            ClipResults expectedResults = { 1.0f, 2.0f, 0.0f, 4.0f, 5.0f };

            QTest::newRow("include missing")
                    << rawClipResults << format << expectedResults;
        }
    }

    void checkFormatClipResults()
    {
        // GIVEN
        QFETCH(ClipResults, rawClipResults);
        QFETCH(ComponentIndices, format);
        QFETCH(ClipResults, expectedResults);

        // WHEN
        const ClipResults actualResults = formatClipResults(rawClipResults, format);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i)
            QCOMPARE(actualResults[i], expectedResults[i]);
    }

    void checkBuildRequiredChannelsAndTypes_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<ChannelMapper *>("mapper");
        QTest::addColumn<QVector<ChannelNameAndType>>("expectedResults");

        {
            auto handler = new Handler();
            auto channelMapping = createChannelMapping(handler,
                                                       QLatin1String("Location"),
                                                       Qt3DCore::QNodeId::createId(),
                                                       "translation",
                                                       static_cast<int>(QVariant::Vector3D),
                                                       3);
            QVector<ChannelMapping *> channelMappings;
            channelMappings.push_back(channelMapping);

            auto channelMapper = createChannelMapper(handler,
                                                     QVector<Qt3DCore::QNodeId>() << channelMapping->peerId());

            QVector<ChannelNameAndType> expectedResults;
            expectedResults.push_back({ QLatin1String("Location"),
                                        static_cast<int>(QVariant::Vector3D),
                                        3,
                                        channelMapping->peerId() });

            QTest::addRow("Location, vec3") << handler << channelMapper << expectedResults;
        }

        {
            auto handler = new Handler();
            auto channelMapping1 = createChannelMapping(handler,
                                                        QLatin1String("Location"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "translation",
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3);
            auto channelMapping2 = createChannelMapping(handler,
                                                        QLatin1String("Rotation"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "rotation",
                                                        static_cast<int>(QVariant::Quaternion),
                                                        4);
            QVector<ChannelMapping *> channelMappings;
            channelMappings.push_back(channelMapping1);
            channelMappings.push_back(channelMapping2);

            QVector<Qt3DCore::QNodeId> channelMappingIds
                    = (QVector<Qt3DCore::QNodeId>()
                       << channelMapping1->peerId()
                       << channelMapping2->peerId());
            auto channelMapper = createChannelMapper(handler, channelMappingIds);

            QVector<ChannelNameAndType> expectedResults;
            expectedResults.push_back({ QLatin1String("Location"),
                                        static_cast<int>(QVariant::Vector3D),
                                        3,
                                        channelMapping1->peerId() });
            expectedResults.push_back({ QLatin1String("Rotation"),
                                        static_cast<int>(QVariant::Quaternion),
                                        4,
                                        channelMapping2->peerId() });

            QTest::addRow("Multiple unique channels") << handler << channelMapper << expectedResults;
        }

        {
            auto handler = new Handler();
            auto channelMapping1 = createChannelMapping(handler,
                                                        QLatin1String("Location"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "translation",
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3);
            auto channelMapping2 = createChannelMapping(handler,
                                                        QLatin1String("Rotation"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "rotation",
                                                        static_cast<int>(QVariant::Quaternion),
                                                        4);
            auto channelMapping3 = createChannelMapping(handler,
                                                        QLatin1String("Location"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "translation",
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3);
            auto channelMapping4 = createChannelMapping(handler,
                                                        QLatin1String("Location"),
                                                        Qt3DCore::QNodeId::createId(),
                                                        "translation",
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3);

            QVector<ChannelMapping *> channelMappings;
            channelMappings.push_back(channelMapping1);
            channelMappings.push_back(channelMapping2);
            channelMappings.push_back(channelMapping3);
            channelMappings.push_back(channelMapping4);

            QVector<Qt3DCore::QNodeId> channelMappingIds
                    = (QVector<Qt3DCore::QNodeId>()
                       << channelMapping1->peerId()
                       << channelMapping2->peerId()
                       << channelMapping3->peerId()
                       << channelMapping4->peerId());
            auto channelMapper = createChannelMapper(handler, channelMappingIds);

            QVector<ChannelNameAndType> expectedResults;
            expectedResults.push_back({ QLatin1String("Location"),
                                        static_cast<int>(QVariant::Vector3D),
                                        3,
                                        channelMapping1->peerId() });
            expectedResults.push_back({ QLatin1String("Rotation"),
                                        static_cast<int>(QVariant::Quaternion),
                                        4,
                                        channelMapping2->peerId() });
            expectedResults.push_back({ QLatin1String("Location"),
                                        static_cast<int>(QVariant::Vector3D),
                                        3,
                                        channelMapping3->peerId() });
            expectedResults.push_back({ QLatin1String("Location"),
                                        static_cast<int>(QVariant::Vector3D),
                                        3,
                                        channelMapping4->peerId() });

            QTest::addRow("Multiple channels with repeats") << handler << channelMapper << expectedResults;
        }

        {
            auto handler = new Handler();
            const int jointCount = 10;
            auto skeleton = createSkeleton(handler, jointCount);
            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            QVector<ChannelMapping *> channelMappings;
            channelMappings.push_back(channelMapping);

            auto channelMapper = createChannelMapper(handler,
                                                     QVector<Qt3DCore::QNodeId>() << channelMapping->peerId());

            QVector<ChannelNameAndType> expectedResults;
            for (int i = 0; i < jointCount; ++i) {
                ChannelNameAndType locationDescription = { QLatin1String("Location"),
                                                           static_cast<int>(QVariant::Vector3D),
                                                           3,
                                                           channelMapping->peerId() };
                locationDescription.jointIndex = i;
                locationDescription.jointTransformComponent = Translation;
                expectedResults.push_back(locationDescription);

                ChannelNameAndType rotationDescription = { QLatin1String("Rotation"),
                                                           static_cast<int>(QVariant::Quaternion),
                                                           4,
                                                           channelMapping->peerId() };
                rotationDescription.jointIndex = i;
                rotationDescription.jointTransformComponent = Rotation;
                expectedResults.push_back(rotationDescription);

                ChannelNameAndType scaleDescription = { QLatin1String("Scale"),
                                                        static_cast<int>(QVariant::Vector3D),
                                                        3,
                                                        channelMapping->peerId() };
                scaleDescription.jointIndex = i;
                scaleDescription.jointTransformComponent = Scale;
                expectedResults.push_back(scaleDescription);
            }

            QTest::addRow("Skeleton, 10 joints") << handler << channelMapper << expectedResults;
        }
    }

    void checkBuildRequiredChannelsAndTypes()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(ChannelMapper *, mapper);
        QFETCH(QVector<ChannelNameAndType>, expectedResults);

        // WHEN
        const QVector<ChannelNameAndType> actualResults = buildRequiredChannelsAndTypes(handler, mapper);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i)
            QCOMPARE(actualResults[i], expectedResults[i]);

        // Cleanup
        delete handler;
    }

    void checkAssignChannelComponentIndices_data()
    {
        QTest::addColumn<QVector<ChannelNameAndType>>("allChannels");
        QTest::addColumn<QVector<ComponentIndices>>("expectedResults");

        {
            QVector<ChannelNameAndType> allChannels;
            allChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });

            QVector<ComponentIndices> expectedResults;
            expectedResults.push_back({ 0, 1, 2 });

            QTest::newRow("vec3 location") << allChannels << expectedResults;
        }

        {
            QVector<ChannelNameAndType> allChannels;
            allChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });
            allChannels.push_back({ QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 });

            QVector<ComponentIndices> expectedResults;
            expectedResults.push_back({ 0, 1, 2 });
            expectedResults.push_back({ 3, 4, 5, 6 });

            QTest::newRow("vec3 location, quaterion rotation") << allChannels << expectedResults;
        }

        {
            QVector<ChannelNameAndType> allChannels;
            allChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });
            allChannels.push_back({ QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 });
            allChannels.push_back({ QLatin1String("BaseColor"), static_cast<int>(QVariant::Vector3D), 3 });
            allChannels.push_back({ QLatin1String("Metalness"), static_cast<int>(QVariant::Double), 1 });
            allChannels.push_back({ QLatin1String("Roughness"), static_cast<int>(QVariant::Double), 1 });
            allChannels.push_back({ QLatin1String("MorphWeights"), static_cast<int>(QVariant::List), 6 });

            QVector<ComponentIndices> expectedResults;
            expectedResults.push_back({ 0, 1, 2 });
            expectedResults.push_back({ 3, 4, 5, 6 });
            expectedResults.push_back({ 7, 8, 9 });
            expectedResults.push_back({ 10 });
            expectedResults.push_back({ 11 });
            expectedResults.push_back({ 12, 13, 14, 15, 16, 17 });

            QTest::newRow("vec3 location, quaterion rotation, pbr metal-rough morphweights") << allChannels << expectedResults;
        }

        {
            QVector<ChannelNameAndType> allChannels;
            const int jointCount = 4;
            for (int i = 0; i < jointCount; ++i) {
                ChannelNameAndType locationDescription = { QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 };
                locationDescription.jointIndex = i;
                allChannels.push_back(locationDescription);

                ChannelNameAndType rotationDescription = { QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 };
                rotationDescription.jointIndex = i;
                allChannels.push_back(rotationDescription);

                ChannelNameAndType scaleDescription = { QLatin1String("Scale"), static_cast<int>(QVariant::Vector3D), 3 };
                scaleDescription.jointIndex = i;
                allChannels.push_back(scaleDescription);
            }

            QVector<ComponentIndices> expectedResults;
            expectedResults.push_back({ 0, 1, 2 });
            expectedResults.push_back({ 3, 4, 5, 6 });
            expectedResults.push_back({ 7, 8, 9 });

            expectedResults.push_back({ 10, 11, 12 });
            expectedResults.push_back({ 13, 14, 15, 16 });
            expectedResults.push_back({ 17, 18, 19 });

            expectedResults.push_back({ 20, 21, 22 });
            expectedResults.push_back({ 23, 24, 25, 26 });
            expectedResults.push_back({ 27, 28, 29 });

            expectedResults.push_back({ 30, 31, 32 });
            expectedResults.push_back({ 33, 34, 35, 36 });
            expectedResults.push_back({ 37, 38, 39 });

            QTest::newRow("skeleton, 4 joints") << allChannels << expectedResults;
        }
    }

    void checkAssignChannelComponentIndices()
    {
        // GIVEN
        QFETCH(QVector<ChannelNameAndType>, allChannels);
        QFETCH(QVector<ComponentIndices>, expectedResults);

        // WHEN
        const QVector<ComponentIndices> actualResults = assignChannelComponentIndices(allChannels);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            const ComponentIndices &actualResult = actualResults[i];
            const ComponentIndices &expectedResult = expectedResults[i];

            for (int j = 0; j < actualResult.size(); ++j)
                QCOMPARE(actualResult[j], expectedResult[j]);
        }
    }

    void checkGenerateClipFormatIndices_data()
    {
        QTest::addColumn<QVector<ChannelNameAndType>>("targetChannels");
        QTest::addColumn<QVector<ComponentIndices>>("targetIndices");
        QTest::addColumn<AnimationClip *>("clip");
        QTest::addColumn<ClipFormat>("expectedResults");

        {
            QVector<ChannelNameAndType> targetChannels;
            targetChannels.push_back({ QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 });
            targetChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Base Color"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Metalness"), static_cast<int>(QVariant::Double), 1 });
            targetChannels.push_back({ QLatin1String("Roughness"), static_cast<int>(QVariant::Double), 1 });

            QVector<ComponentIndices> targetIndices;
            targetIndices.push_back({ 0, 1, 2, 3 });
            targetIndices.push_back({ 4, 5, 6 });
            targetIndices.push_back({ 7, 8, 9 });
            targetIndices.push_back({ 10 });
            targetIndices.push_back({ 11 });

            auto *clip = new AnimationClip();
            clip->setDataType(AnimationClip::File);
            clip->setSource(QUrl("qrc:/clip3.json"));
            clip->loadAnimation();

            ClipFormat expectedResults;
            expectedResults.sourceClipIndices = { 0, 1, 3, 2,    // Rotation (y/z swapped in clip3.json)
                                                  4, 6, 5,       // Location (y/z swapped in clip3.json)
                                                  7, 8, 9,       // Base Color
                                                  10,            // Metalness
                                                  11 };          // Roughness
            expectedResults.sourceClipMask = { QBitArray(4, true),
                                               QBitArray(3, true),
                                               QBitArray(3, true),
                                               QBitArray(1, true),
                                               QBitArray(1, true) };
            expectedResults.namesAndTypes = targetChannels;
            expectedResults.formattedComponentIndices = targetIndices;

            QTest::newRow("rotation, location, pbr metal-rough")
                    << targetChannels << targetIndices << clip << expectedResults;
        }

        {
            QVector<ChannelNameAndType> targetChannels;
            targetChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 });
            targetChannels.push_back({ QLatin1String("Base Color"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Metalness"), static_cast<int>(QVariant::Double), 1 });
            targetChannels.push_back({ QLatin1String("Roughness"), static_cast<int>(QVariant::Double), 1 });

            QVector<ComponentIndices> targetIndices;
            targetIndices.push_back({ 0, 1, 2 });
            targetIndices.push_back({ 3, 4, 5, 6 });
            targetIndices.push_back({ 7, 8, 9 });
            targetIndices.push_back({ 10 });
            targetIndices.push_back({ 11 });

            auto *clip = new AnimationClip();
            clip->setDataType(AnimationClip::File);
            clip->setSource(QUrl("qrc:/clip3.json"));
            clip->loadAnimation();

            ClipFormat expectedResults;
            expectedResults.sourceClipIndices = { 4, 6, 5,       // Location (y/z swapped in clip3.json)
                                                  0, 1, 3, 2,    // Rotation (y/z swapped in clip3.json)
                                                  7, 8, 9,       // Base Color
                                                  10,            // Metalness
                                                  11 };          // Roughness
            expectedResults.sourceClipMask = { QBitArray(3, true),
                                               QBitArray(4, true),
                                               QBitArray(3, true),
                                               QBitArray(1, true),
                                               QBitArray(1, true) };
            expectedResults.namesAndTypes = targetChannels;
            expectedResults.formattedComponentIndices = targetIndices;

            QTest::newRow("location, rotation, pbr metal-rough")
                    << targetChannels << targetIndices << clip << expectedResults;
        }

        {
            QVector<ChannelNameAndType> targetChannels;
            targetChannels.push_back({ QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 });
            targetChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Albedo"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Metalness"), static_cast<int>(QVariant::Double), 1 });
            targetChannels.push_back({ QLatin1String("Roughness"), static_cast<int>(QVariant::Double), 1 });

            QVector<ComponentIndices> targetIndices;
            targetIndices.push_back({ 0, 1, 2, 3 });
            targetIndices.push_back({ 4, 5, 6 });
            targetIndices.push_back({ 7, 8, 9 });
            targetIndices.push_back({ 10 });
            targetIndices.push_back({ 11 });

            auto *clip = new AnimationClip();
            clip->setDataType(AnimationClip::File);
            clip->setSource(QUrl("qrc:/clip3.json"));
            clip->loadAnimation();

            ClipFormat expectedResults;
            expectedResults.sourceClipIndices = { 0, 1, 3, 2,    // Rotation (y/z swapped in clip3.json)
                                                  4, 6, 5,       // Location (y/z swapped in clip3.json)
                                                  -1, -1, -1,    // Albedo (missing from clip)
                                                  10,            // Metalness
                                                  11 };          // Roughness
            expectedResults.sourceClipMask = { QBitArray(4, true),
                                               QBitArray(3, true),
                                               QBitArray(3, false),
                                               QBitArray(1, true),
                                               QBitArray(1, true) };
            expectedResults.namesAndTypes = targetChannels;
            expectedResults.formattedComponentIndices = targetIndices;

            QTest::newRow("rotation, location, albedo (missing), metal-rough")
                    << targetChannels << targetIndices << clip << expectedResults;
        }

        {
            QVector<ChannelNameAndType> targetChannels;
            targetChannels.push_back({ QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 });
            targetChannels.push_back({ QLatin1String("Albedo"), static_cast<int>(QVariant::Vector3D), 3 });
            targetChannels.push_back({ QLatin1String("Metalness"), static_cast<int>(QVariant::Double), 1 });
            targetChannels.push_back({ QLatin1String("Roughness"), static_cast<int>(QVariant::Double), 1 });

            QVector<ComponentIndices> targetIndices;
            targetIndices.push_back({ 0, 1, 2 });
            targetIndices.push_back({ 3, 4, 5, 6 });
            targetIndices.push_back({ 7, 8, 9 });
            targetIndices.push_back({ 10 });
            targetIndices.push_back({ 11 });

            auto *clip = new AnimationClip();
            clip->setDataType(AnimationClip::File);
            clip->setSource(QUrl("qrc:/clip3.json"));
            clip->loadAnimation();

            ClipFormat expectedResults;
            expectedResults.sourceClipIndices = { 4, 6, 5,       // Location (y/z swapped in clip3.json)
                                                  0, 1, 3, 2,    // Rotation (y/z swapped in clip3.json)
                                                  -1, -1, -1,    // Albedo (missing from clip)
                                                  10,            // Metalness
                                                  11 };          // Roughness
            expectedResults.sourceClipMask = { QBitArray(3, true),
                                               QBitArray(4, true),
                                               QBitArray(3, false),
                                               QBitArray(1, true),
                                               QBitArray(1, true) };
            expectedResults.namesAndTypes = targetChannels;
            expectedResults.formattedComponentIndices = targetIndices;

            QTest::newRow("location, rotation, albedo (missing), metal-rough")
                    << targetChannels << targetIndices << clip << expectedResults;
        }

        {
            QVector<ChannelNameAndType> targetChannels;
            const int jointCount = 4;
            for (int i = 0; i < jointCount; ++i) {
                ChannelNameAndType locationDescription = { QLatin1String("Location"), static_cast<int>(QVariant::Vector3D), 3 };
                locationDescription.jointIndex = i;
                targetChannels.push_back(locationDescription);

                ChannelNameAndType rotationDescription = { QLatin1String("Rotation"), static_cast<int>(QVariant::Quaternion), 4 };
                rotationDescription.jointIndex = i;
                targetChannels.push_back(rotationDescription);

                ChannelNameAndType scaleDescription = { QLatin1String("Scale"), static_cast<int>(QVariant::Vector3D), 3 };
                scaleDescription.jointIndex = i;
                targetChannels.push_back(scaleDescription);
            }

            QVector<ComponentIndices> targetIndices;
            targetIndices.push_back({ 0, 1, 2 });
            targetIndices.push_back({ 3, 4, 5, 6 });
            targetIndices.push_back({ 7, 8, 9 });

            targetIndices.push_back({ 10, 11, 12 });
            targetIndices.push_back({ 13, 14, 15, 16 });
            targetIndices.push_back({ 17, 18, 19 });

            targetIndices.push_back({ 20, 21, 22 });
            targetIndices.push_back({ 23, 24, 25, 26 });
            targetIndices.push_back({ 27, 28, 29 });

            targetIndices.push_back({ 30, 31, 32 });
            targetIndices.push_back({ 33, 34, 35, 36 });
            targetIndices.push_back({ 37, 38, 39 });

            auto *clip = new AnimationClip();
            clip->setDataType(AnimationClip::File);
            clip->setSource(QUrl("qrc:/clip5.json"));
            clip->loadAnimation();

            ClipFormat expectedResults;
            expectedResults.sourceClipIndices = { 4, 6, 5,           // Location, joint 0 (y/z swapped in clip5.json)
                                                  0, 1, 3, 2,        // Rotation, joint 0 (y/z swapped in clip5.json)
                                                  7, 8, 9,           // Scale,    joint 0
                                                  14, 16, 15,        // Location, joint 1 (y/z swapped in clip5.json)
                                                  10, 11, 13, 12,    // Rotation, joint 1 (y/z swapped in clip5.json)
                                                  17, 18, 19,        // Scale,    joint 1
                                                  24, 26, 25,        // Location, joint 2 (y/z swapped in clip5.json)
                                                  20, 21, 23, 22,    // Rotation, joint 2 (y/z swapped in clip5.json)
                                                  27, 28, 29,        // Scale,    joint 2
                                                  34, 36, 35,        // Location, joint 3 (y/z swapped in clip5.json)
                                                  30, 31, 33, 32,    // Rotation, joint 3 (y/z swapped in clip5.json)
                                                  37, 38, 39 };      // Scale,    joint 3
            expectedResults.sourceClipMask = { QBitArray(3, true),
                                               QBitArray(4, true),
                                               QBitArray(3, true),
                                               QBitArray(3, true),
                                               QBitArray(4, true),
                                               QBitArray(3, true),
                                               QBitArray(3, true),
                                               QBitArray(4, true),
                                               QBitArray(3, true),
                                               QBitArray(3, true),
                                               QBitArray(4, true),
                                               QBitArray(3, true) };
            expectedResults.namesAndTypes = targetChannels;
            expectedResults.formattedComponentIndices = targetIndices;

            QTest::newRow("skeleton (SQT), 4 joints")
                    << targetChannels << targetIndices << clip << expectedResults;
        }
    }

    void checkGenerateClipFormatIndices()
    {
        // GIVEN
        QFETCH(QVector<ChannelNameAndType>, targetChannels);
        QFETCH(QVector<ComponentIndices>, targetIndices);
        QFETCH(AnimationClip *, clip);
        QFETCH(ClipFormat, expectedResults);

        // WHEN
        const ClipFormat actualResults = generateClipFormatIndices(targetChannels,
                                                                   targetIndices,
                                                                   clip);

        // THEN
        QCOMPARE(actualResults.sourceClipIndices.size(), expectedResults.sourceClipIndices.size());
        for (int i = 0; i < actualResults.sourceClipIndices.size(); ++i)
            QCOMPARE(actualResults.sourceClipIndices[i], expectedResults.sourceClipIndices[i]);

        QCOMPARE(actualResults.sourceClipMask.size(), expectedResults.sourceClipMask.size());
        for (int i = 0; i < actualResults.sourceClipMask.size(); ++i)
            QCOMPARE(actualResults.sourceClipMask[i], expectedResults.sourceClipMask[i]);

        QCOMPARE(actualResults.formattedComponentIndices.size(), expectedResults.formattedComponentIndices.size());
        for (int i = 0; i < actualResults.formattedComponentIndices.size(); ++i)
            QCOMPARE(actualResults.formattedComponentIndices[i], expectedResults.formattedComponentIndices[i]);

        QCOMPARE(actualResults.namesAndTypes.size(), expectedResults.namesAndTypes.size());
        for (int i = 0; i < actualResults.namesAndTypes.size(); ++i)
            QCOMPARE(actualResults.namesAndTypes[i], expectedResults.namesAndTypes[i]);

        // Cleanup
        delete clip;
    }

    void checkDefaultValueForChannel_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<ChannelNameAndType>("channelDescription");
        QTest::addColumn<QVector<float>>("expectedResults");

        {
            auto handler = new Handler();
            auto channelMapping = createChannelMapping(handler,
                                                       QLatin1String("Location"),
                                                       Qt3DCore::QNodeId::createId(),
                                                       "translation",
                                                       static_cast<int>(QVariant::Vector3D),
                                                       3);
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.name = QLatin1String("translation");
            const QVector<float> expectedResults = { 0.0f, 0.0f, 0.0f };
            QTest::newRow("translation") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto channelMapping = createChannelMapping(handler,
                                                       QLatin1String("Rotation"),
                                                       Qt3DCore::QNodeId::createId(),
                                                       "rotation",
                                                       static_cast<int>(QVariant::Quaternion),
                                                       4);
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Quaternion);
            channelDescription.name = QLatin1String("rotation");
            const QVector<float> expectedResults = { 1.0f, 0.0f, 0.0f, 0.0f };
            QTest::newRow("rotation") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto channelMapping = createChannelMapping(handler,
                                                       QLatin1String("Scale"),
                                                       Qt3DCore::QNodeId::createId(),
                                                       "scale",
                                                       static_cast<int>(QVariant::Vector3D),
                                                       3);
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.name = QLatin1String("scale");
            const QVector<float> expectedResults = { 1.0f, 1.0f, 1.0f };
            QTest::newRow("scale") << handler << channelDescription << expectedResults;
        }

        // Test skeleton cases
        {
            auto handler = new Handler();
            auto skeleton = createSkeleton(handler, 2);
            skeleton->setJointScale(0, QVector3D(2.0f, 3.0f, 4.0f));

            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.jointIndex = 0;
            channelDescription.jointTransformComponent = Scale;
            const QVector<float> expectedResults = { 2.0f, 3.0f, 4.0f };
            QTest::newRow("joint 0 scale") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto skeleton = createSkeleton(handler, 2);
            skeleton->setJointRotation(0, QQuaternion(1.0f, 0.0f, 0.0f, 0.0f));

            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.jointIndex = 0;
            channelDescription.jointTransformComponent = Rotation;
            const QVector<float> expectedResults = { 1.0f, 0.0f, 0.0f, 0.0f };
            QTest::newRow("joint 0 rotation") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto skeleton = createSkeleton(handler, 2);
            skeleton->setJointTranslation(0, QVector3D(2.0f, 3.0f, 4.0f));

            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.jointIndex = 0;
            channelDescription.jointTransformComponent = Translation;
            const QVector<float> expectedResults = { 2.0f, 3.0f, 4.0f };
            QTest::newRow("joint 0 translation") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto skeleton = createSkeleton(handler, 2);
            skeleton->setJointScale(1, QVector3D(20.0f, 30.0f, 40.0f));

            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.jointIndex = 1;
            channelDescription.jointTransformComponent = Scale;
            const QVector<float> expectedResults = { 20.0f, 30.0f, 40.0f };
            QTest::newRow("joint 1 scale") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto skeleton = createSkeleton(handler, 2);
            skeleton->setJointRotation(1, QQuaternion(1.0f, 0.0f, 0.0f, 0.0f));

            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.jointIndex = 1;
            channelDescription.jointTransformComponent = Rotation;
            const QVector<float> expectedResults = { 1.0f, 0.0f, 0.0f, 0.0f };
            QTest::newRow("joint 1 rotation") << handler << channelDescription << expectedResults;
        }

        {
            auto handler = new Handler();
            auto skeleton = createSkeleton(handler, 2);
            skeleton->setJointTranslation(1, QVector3D(4.0f, 5.0f, 6.0f));

            auto channelMapping = createChannelMapping(handler, skeleton->peerId());
            ChannelNameAndType channelDescription;
            channelDescription.mappingId = channelMapping->peerId();
            channelDescription.type = static_cast<int>(QVariant::Vector3D);
            channelDescription.jointIndex = 1;
            channelDescription.jointTransformComponent = Translation;
            const QVector<float> expectedResults = { 4.0f, 5.0f, 6.0f };
            QTest::newRow("joint 1 translation") << handler << channelDescription << expectedResults;
        }
    }

    void checkDefaultValueForChannel()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(ChannelNameAndType, channelDescription);
        QFETCH(QVector<float>, expectedResults);

        // WHEN
        auto actualResults = defaultValueForChannel(handler, channelDescription);

        // THEN
        QCOMPARE(actualResults.size(), expectedResults.size());
        for (int i = 0; i < actualResults.size(); ++i) {
            QCOMPARE(actualResults[i], expectedResults[i]);
        }
    }
};

QTEST_MAIN(tst_AnimationUtils)

#include "tst_animationutils.moc"
