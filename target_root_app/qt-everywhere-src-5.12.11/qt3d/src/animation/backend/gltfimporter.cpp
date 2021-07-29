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

#include "gltfimporter_p.h"
#include <Qt3DAnimation/private/animationlogging_p.h>
#include <Qt3DAnimation/private/fcurve_p.h>
#include <Qt3DAnimation/private/keyframe_p.h>

#include <QtGui/qopengl.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qversionnumber.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

namespace {

QString gltfTargetPropertyToChannelName(const QString &propertyName)
{
    if (propertyName == QLatin1String("rotation"))
        return QLatin1String("Rotation");
    else if (propertyName == QLatin1String("translation"))
        return QLatin1String("Location");
    else if (propertyName == QLatin1String("scale"))
        return QLatin1String("Scale");

    qWarning() << "Unknown target property name";
    return QString();
}

QKeyFrame::InterpolationType gltfToQKeyFrameInterpolation(GLTFImporter::Sampler::InterpolationMode mode)
{
    switch (mode) {
    case GLTFImporter::Sampler::Linear:
        return QKeyFrame::LinearInterpolation;
    case GLTFImporter::Sampler::Step:
        return QKeyFrame::ConstantInterpolation;
    case GLTFImporter::Sampler::CubicSpline:
        return QKeyFrame::BezierInterpolation;
    case GLTFImporter::Sampler::CatmullRomSpline:
        // TODO: Implement this interpolation type
        qWarning() << "Unhandled interpolation type";
        return QKeyFrame::LinearInterpolation;
    }

    return QKeyFrame::LinearInterpolation;
}

void jsonArrayToSqt(const QJsonArray &jsonArray, Qt3DCore::Sqt &sqt)
{
    Q_ASSERT(jsonArray.size() == 16);
    QMatrix4x4 m;
    float *data = m.data();
    int i = 0;
    for (const auto &element : jsonArray)
        *(data + i++) = static_cast<float>(element.toDouble());

    decomposeQMatrix4x4(m, sqt);
}

void jsonArrayToVector3D(const QJsonArray &jsonArray, QVector3D &v)
{
    Q_ASSERT(jsonArray.size() == 3);
    v.setX(static_cast<float>(jsonArray.at(0).toDouble()));
    v.setY(static_cast<float>(jsonArray.at(1).toDouble()));
    v.setZ(static_cast<float>(jsonArray.at(2).toDouble()));
}

void jsonArrayToQuaternion(const QJsonArray &jsonArray, QQuaternion &q)
{
    Q_ASSERT(jsonArray.size() == 4);
    q.setX(static_cast<float>(jsonArray.at(0).toDouble()));
    q.setY(static_cast<float>(jsonArray.at(1).toDouble()));
    q.setZ(static_cast<float>(jsonArray.at(2).toDouble()));
    q.setScalar(static_cast<float>(jsonArray.at(3).toDouble()));
}

}

#define KEY_ACCESSORS               QLatin1String("accessors")
#define KEY_ANIMATIONS              QLatin1String("animations")
#define KEY_ASSET                   QLatin1String("asset")
#define KEY_BUFFER                  QLatin1String("buffer")
#define KEY_BUFFERS                 QLatin1String("buffers")
#define KEY_BUFFER_VIEW             QLatin1String("bufferView")
#define KEY_BUFFER_VIEWS            QLatin1String("bufferViews")
#define KEY_BYTE_LENGTH             QLatin1String("byteLength")
#define KEY_BYTE_OFFSET             QLatin1String("byteOffset")
#define KEY_BYTE_STRIDE             QLatin1String("byteStride")
#define KEY_CAMERA                  QLatin1String("camera")
#define KEY_CHANNELS                QLatin1String("channels")
#define KEY_CHILDREN                QLatin1String("children")
#define KEY_COMPONENT_TYPE          QLatin1String("componentType")
#define KEY_COUNT                   QLatin1String("count")
#define KEY_JOINTS                  QLatin1String("joints")
#define KEY_INPUT                   QLatin1String("input")
#define KEY_INTERPOLATION           QLatin1String("interpolation")
#define KEY_INVERSE_BIND_MATRICES   QLatin1String("inverseBindMatrices")
#define KEY_MATRIX                  QLatin1String("matrix")
#define KEY_MESH                    QLatin1String("mesh")
#define KEY_NAME                    QLatin1String("name")
#define KEY_NODE                    QLatin1String("node")
#define KEY_NODES                   QLatin1String("nodes")
#define KEY_OUTPUT                  QLatin1String("output")
#define KEY_PATH                    QLatin1String("path")
#define KEY_ROTATION                QLatin1String("rotation")
#define KEY_SAMPLER                 QLatin1String("sampler")
#define KEY_SAMPLERS                QLatin1String("samplers")
#define KEY_SCALE                   QLatin1String("scale")
#define KEY_SKIN                    QLatin1String("skin")
#define KEY_SKINS                   QLatin1String("skins")
#define KEY_TARGET                  QLatin1String("target")
#define KEY_TRANSLATION             QLatin1String("translation")
#define KEY_TYPE                    QLatin1String("type")
#define KEY_URI                     QLatin1String("uri")
#define KEY_VERSION                 QLatin1String("version")

GLTFImporter::BufferData::BufferData()
    : byteLength(0)
    , data()
{
}

GLTFImporter::BufferData::BufferData(const QJsonObject &json)
    : byteLength(json.value(KEY_BYTE_LENGTH).toInt())
    , path(json.value(KEY_URI).toString())
    , data()
{
}

GLTFImporter::BufferView::BufferView()
    : byteOffset(0)
    , byteLength(0)
    , bufferIndex(-1)
    , target(0)
{
}

GLTFImporter::BufferView::BufferView(const QJsonObject &json)
    : byteOffset(json.value(KEY_BYTE_OFFSET).toInt())
    , byteLength(json.value(KEY_BYTE_LENGTH).toInt())
    , bufferIndex(json.value(KEY_BUFFER).toInt())
    , target(0)
{
    const auto targetValue = json.value(KEY_TARGET);
    if (!targetValue.isUndefined())
        target = targetValue.toInt();
}

GLTFImporter::AccessorData::AccessorData()
    : type(Qt3DRender::QAttribute::Float)
    , dataSize(0)
    , count(0)
    , byteOffset(0)
    , byteStride(0)
{
}

GLTFImporter::AccessorData::AccessorData(const QJsonObject &json)
    : bufferViewIndex(json.value(KEY_BUFFER_VIEW).toInt(-1))
    , type(accessorTypeFromJSON(json.value(KEY_COMPONENT_TYPE).toInt()))
    , dataSize(accessorDataSizeFromJson(json.value(KEY_TYPE).toString()))
    , count(json.value(KEY_COUNT).toInt())
    , byteOffset(0)
    , byteStride(0)
{
    const auto byteOffsetValue = json.value(KEY_BYTE_OFFSET);
    if (!byteOffsetValue.isUndefined())
        byteOffset = byteOffsetValue.toInt();
    const auto byteStrideValue = json.value(KEY_BYTE_STRIDE);
    if (!byteStrideValue.isUndefined())
        byteStride = byteStrideValue.toInt();
}

GLTFImporter::Skin::Skin()
    : inverseBindAccessorIndex(-1)
    , jointNodeIndices()
{
}

GLTFImporter::Skin::Skin(const QJsonObject &json)
    : name(json.value(KEY_NAME).toString())
    , inverseBindAccessorIndex(json.value(KEY_INVERSE_BIND_MATRICES).toInt())
{
    QJsonArray jointNodes = json.value(KEY_JOINTS).toArray();
    jointNodeIndices.reserve(jointNodes.size());
    for (const auto jointNodeValue : jointNodes)
        jointNodeIndices.push_back(jointNodeValue.toInt());
}

GLTFImporter::Channel::Channel()
    : samplerIndex(-1)
    , targetNodeIndex(-1)
    , targetProperty()
{
}

GLTFImporter::Channel::Channel(const QJsonObject &json)
    : samplerIndex(json.value(KEY_SAMPLER).toInt())
    , targetNodeIndex(-1)
    , targetProperty()
{
    const auto targetJson = json.value(KEY_TARGET).toObject();
    targetNodeIndex = targetJson.value(KEY_NODE).toInt();
    targetProperty = targetJson.value(KEY_PATH).toString();
}

GLTFImporter::Sampler::Sampler()
    : inputAccessorIndex(-1)
    , outputAccessorIndex(-1)
    , interpolationMode(Linear)
{
}

GLTFImporter::Sampler::Sampler(const QJsonObject &json)
    : inputAccessorIndex(json.value(KEY_INPUT).toInt())
    , outputAccessorIndex(json.value(KEY_OUTPUT).toInt())
    , interpolationMode(Linear)
{
    const auto interpolation = json.value(KEY_INTERPOLATION).toString();
    if (interpolation == QLatin1String("LINEAR"))
        interpolationMode = Linear;
    else if (interpolation == QLatin1String("STEP"))
        interpolationMode = Step;
    else if (interpolation == QLatin1String("CATMULLROMSPLINE"))
        interpolationMode = CatmullRomSpline;
    else if (interpolation == QLatin1String("CUBICSPLINE"))
        interpolationMode = CubicSpline;
}

QString GLTFImporter::Sampler::interpolationModeString() const
{
    switch (interpolationMode) {
    case Linear: return QLatin1String("LINEAR");
    case Step: return QLatin1String("STEP");
    case CatmullRomSpline: return QLatin1String("CATMULLROMSPLINE");
    case CubicSpline: return QLatin1String("CUBICSPLINE");
    }

    return QLatin1String("Unknown");
}

GLTFImporter::Animation::Animation()
    : name()
    , channels()
    , samplers()
{
}

GLTFImporter::Animation::Animation(const QJsonObject &json)
    : name(json.value(KEY_NAME).toString())
{
    QJsonArray channelsArray = json.value(KEY_CHANNELS).toArray();
    channels.reserve(channelsArray.size());
    for (const auto channelValue : channelsArray) {
        Channel channel(channelValue.toObject());
        channels.push_back(channel);
    }

    QJsonArray samplersArray = json.value(KEY_SAMPLERS).toArray();
    samplers.reserve(samplersArray.size());
    for (const auto samplerValue : samplersArray) {
        Sampler sampler(samplerValue.toObject());
        samplers.push_back(sampler);
    }
}

GLTFImporter::Node::Node()
    : localTransform()
    , childNodeIndices()
    , name()
    , parentNodeIndex(-1)
    , cameraIndex(-1)
    , meshIndex(-1)
    , skinIndex(-1)
{
}

GLTFImporter::Node::Node(const QJsonObject &json)
    : localTransform()
    , childNodeIndices()
    , name(json.value(KEY_NAME).toString())
    , parentNodeIndex(-1)
    , cameraIndex(-1)
    , meshIndex(-1)
    , skinIndex(-1)
{
    // Child nodes - we setup the parent links in a later pass
    QJsonArray childNodes = json.value(KEY_CHILDREN).toArray();
    childNodeIndices.reserve(childNodes.size());
    for (const auto childNodeValue : childNodes)
        childNodeIndices.push_back(childNodeValue.toInt());

    // Local transform - matrix or scale, rotation, translation
    const auto matrixValue = json.value(KEY_MATRIX);
    if (!matrixValue.isUndefined()) {
        jsonArrayToSqt(matrixValue.toArray(), localTransform);
    } else {
        const auto scaleValue = json.value(KEY_SCALE);
        const auto rotationValue = json.value(KEY_ROTATION);
        const auto translationValue = json.value(KEY_TRANSLATION);

        QVector3D s(1.0f, 1.0f, 1.0f);
        if (!scaleValue.isUndefined())
            jsonArrayToVector3D(scaleValue.toArray(), localTransform.scale);

        QQuaternion r;
        if (!rotationValue.isUndefined())
            jsonArrayToQuaternion(json.value(KEY_ROTATION).toArray(), localTransform.rotation);

        QVector3D t;
        if (!translationValue.isUndefined())
            jsonArrayToVector3D(json.value(KEY_TRANSLATION).toArray(), localTransform.translation);
    }

    // Referenced objects
    const auto cameraValue = json.value(KEY_CAMERA);
    if (!cameraValue.isUndefined())
        cameraIndex = cameraValue.toInt();

    const auto meshValue = json.value(KEY_MESH);
    if (!meshValue.isUndefined())
        meshIndex = meshValue.toInt();

    const auto skinValue = json.value(KEY_SKIN);
    if (!skinValue.isUndefined())
        skinIndex = skinValue.toInt();
}

Qt3DRender::QAttribute::VertexBaseType GLTFImporter::accessorTypeFromJSON(int componentType)
{
    if (componentType == GL_BYTE)
        return Qt3DRender::QAttribute::Byte;
    else if (componentType == GL_UNSIGNED_BYTE)
        return Qt3DRender::QAttribute::UnsignedByte;
    else if (componentType == GL_SHORT)
        return Qt3DRender::QAttribute::Short;
    else if (componentType == GL_UNSIGNED_SHORT)
        return Qt3DRender::QAttribute::UnsignedShort;
    else if (componentType == GL_UNSIGNED_INT)
        return Qt3DRender::QAttribute::UnsignedInt;
    else if (componentType == GL_FLOAT)
        return Qt3DRender::QAttribute::Float;

    // There shouldn't be an invalid case here
    qWarning("unsupported accessor type %d", componentType);
    return Qt3DRender::QAttribute::Float;
}

uint GLTFImporter::accessorTypeSize(Qt3DRender::QAttribute::VertexBaseType componentType)
{
    switch (componentType) {
    case Qt3DRender::QAttribute::Byte:
    case Qt3DRender::QAttribute::UnsignedByte:
         return 1;

    case Qt3DRender::QAttribute::Short:
    case Qt3DRender::QAttribute::UnsignedShort:
        return 2;

    case Qt3DRender::QAttribute::Int:
    case Qt3DRender::QAttribute::Float:
        return 4;

    default:
        qWarning("Unhandled accessor data type %d", componentType);
        return 0;
    }
}

uint GLTFImporter::accessorDataSizeFromJson(const QString &type)
{
    QString typeName = type.toUpper();
    if (typeName == QLatin1String("SCALAR"))
        return 1;
    if (typeName == QLatin1String("VEC2"))
        return 2;
    if (typeName == QLatin1String("VEC3"))
        return 3;
    if (typeName == QLatin1String("VEC4"))
        return 4;
    if (typeName == QLatin1String("MAT2"))
        return 4;
    if (typeName == QLatin1String("MAT3"))
        return 9;
    if (typeName == QLatin1String("MAT4"))
        return 16;

    return 0;
}

GLTFImporter::GLTFImporter()
{
}

bool GLTFImporter::load(QIODevice *ioDev)
{
    QByteArray jsonData = ioDev->readAll();
    QJsonDocument sceneDocument = QJsonDocument::fromBinaryData(jsonData);
    if (sceneDocument.isNull())
        sceneDocument = QJsonDocument::fromJson(jsonData);

    if (Q_UNLIKELY(!setJSON(sceneDocument))) {
        qWarning("not a JSON document");
        return false;
    }

    auto file = qobject_cast<QFile*>(ioDev);
    if (file) {
        QFileInfo finfo(file->fileName());
        setBasePath(finfo.dir().absolutePath());
    }

    return parse();
}

QHash<int, int> GLTFImporter::createNodeIndexToJointIndexMap(const Skin &skin) const
{
    const int jointCount = skin.jointNodeIndices.size();
    QHash<int, int> nodeIndexToJointIndexMap;
    nodeIndexToJointIndexMap.reserve(jointCount);
    for (int i = 0; i < jointCount; ++i)
        nodeIndexToJointIndexMap.insert(skin.jointNodeIndices[i], i);
    return nodeIndexToJointIndexMap;
}

GLTFImporter::AnimationNameAndChannels GLTFImporter::createAnimationData(int animationIndex, const QString &animationName) const
{
    AnimationNameAndChannels nameAndChannels;
    if (m_animations.isEmpty()) {
        qCWarning(Jobs) << "File does not contain any animation data";
        return nameAndChannels;
    }

    if (m_animations.size() == 1) {
        animationIndex = 0;
    } else if (animationIndex < 0 && !animationName.isEmpty()) {
        for (int i = 0; i < m_animations.size(); ++i) {
            if (m_animations[i].name == animationName) {
                animationIndex = i;
                break;
            }
        }
    }

    if (animationIndex < 0 || animationIndex >= m_animations.size()) {
        qCWarning(Jobs) << "Invalid animation index. Skipping.";
        return nameAndChannels;
    }
    const Animation &animation = m_animations[animationIndex];
    nameAndChannels.name = animation.name;

    // Create node index to joint index lookup tables for each skin
    QVector<QHash<int, int>> nodeIndexToJointIndexMaps;
    nodeIndexToJointIndexMaps.reserve(m_skins.size());
    for (const auto &skin : m_skins)
        nodeIndexToJointIndexMaps.push_back(createNodeIndexToJointIndexMap(skin));

    int channelIndex = 0;
    for (const auto &channel : animation.channels) {
        Qt3DAnimation::Animation::Channel outputChannel;
        outputChannel.name = gltfTargetPropertyToChannelName(channel.targetProperty);

        // Find the node index to joint index map that contains the target node and
        // look up the joint index from it. If no such map is found, the target joint
        // is not part of a skeleton and so we can just set the jointIndex to -1.
        int jointIndex = -1;
        for (const auto &map : nodeIndexToJointIndexMaps) {
            const auto result = map.find(channel.targetNodeIndex);
            if (result != map.cend()) {
                jointIndex = result.value();
                break;
            }
        }
        outputChannel.jointIndex = jointIndex;

        const auto &sampler = animation.samplers[channel.samplerIndex];
        const auto interpolationType = gltfToQKeyFrameInterpolation(sampler.interpolationMode);

        if (sampler.inputAccessorIndex == -1 || sampler.outputAccessorIndex == -1) {
            qWarning() << "Skipping channel due to invalid accessor indices in the sampler" << endl;
            continue;
        }

        const auto &inputAccessor = m_accessors[sampler.inputAccessorIndex];
        const auto &outputAccessor = m_accessors[sampler.outputAccessorIndex];

        if (inputAccessor.type != Qt3DRender::QAttribute::Float) {
            qWarning() << "Input accessor has wrong data type. Skipping channel.";
            continue;
        }

        if (outputAccessor.type != Qt3DRender::QAttribute::Float) {
            qWarning() << "Output accessor has wrong data type. Skipping channel.";
            continue;
        }

        if (inputAccessor.count != outputAccessor.count) {
            qWarning() << "Warning!!! Input accessor has" << inputAccessor.count
                       << "entries and output accessor has" << outputAccessor.count
                       << "entries. They should match. Please check your data.";
            continue;
        }

        // TODO: Allow Qt 3D animation data to share timestamps between multiple
        // channel components. I.e. allow key frame values of composite types.
        // Doesn't give as much freedom but more efficient at runtime.

        // Get the key frame times first as these are common to all components of the
        // key frame values.
        const int keyFrameCount = inputAccessor.count;
        QVector<float> keyframeTimes(keyFrameCount);
        for (int i = 0; i < keyFrameCount; ++i) {
            const auto rawTimestamp = accessorData(sampler.inputAccessorIndex, i);
            keyframeTimes[i] = *reinterpret_cast<const float*>(rawTimestamp.data);
        }

        // Create a ChannelComponent for each component of the output sampler and
        // populate it with data.
        switch (outputAccessor.dataSize) {
        // TODO: Handle other types as needed
        case 3: {
            // vec3
            const int componentCount = 3;

            // Construct the channel component names and add component to the channel
            const QStringList suffixes
                    = (QStringList() << QLatin1String("X") << QLatin1String("Y") << QLatin1String("Z"));
            outputChannel.channelComponents.resize(componentCount);
            for (int componentIndex = 0; componentIndex < componentCount; ++componentIndex) {
                outputChannel.channelComponents[componentIndex].name
                        = QString(QLatin1String("%1 %2")).arg(outputChannel.name,
                                                              suffixes[componentIndex]);
            }

            // Populate the fcurves in the channel components
            for (int i = 0; i < keyFrameCount; ++i) {
                const auto rawKeyframeValue = accessorData(sampler.outputAccessorIndex, i);
                QVector3D v;
                memcpy(&v, rawKeyframeValue.data, rawKeyframeValue.byteLength);

                for (int componentIndex = 0; componentIndex < componentCount; ++componentIndex) {
                    Keyframe keyFrame;
                    keyFrame.interpolation = interpolationType;
                    keyFrame.value = v[componentIndex];
                    outputChannel.channelComponents[componentIndex].fcurve.appendKeyframe(keyframeTimes[i], keyFrame);
                }
            }

            break;
        } // case 3

        case 4: {
            // vec4 or quaternion
            const int componentCount = 4;

            // Construct the channel component names and add component to the channel
            const QStringList rotationSuffixes = (QStringList()
                << QLatin1String("X") << QLatin1String("Y") << QLatin1String("Z") << QLatin1String("W"));
            const QStringList standardSuffixes = (QStringList()
                << QLatin1String("X") << QLatin1String("Y") << QLatin1String("Z"));
            const QStringList suffixes = (channel.targetProperty == QLatin1String("rotation"))
                   ? rotationSuffixes : standardSuffixes;
            outputChannel.channelComponents.resize(componentCount);
            for (int componentIndex = 0; componentIndex < componentCount; ++componentIndex) {
                outputChannel.channelComponents[componentIndex].name
                        = QString(QLatin1String("%1 %2")).arg(outputChannel.name,
                                                              suffixes[componentIndex]);
            }

            // Populate the fcurves in the channel components
            for (int i = 0; i < keyFrameCount; ++i) {
                const auto rawKeyframeValue = accessorData(sampler.outputAccessorIndex, i);
                QVector4D v;
                memcpy(&v, rawKeyframeValue.data, rawKeyframeValue.byteLength);

                for (int componentIndex = 0; componentIndex < componentCount; ++componentIndex) {
                    Keyframe keyFrame;
                    keyFrame.interpolation = interpolationType;
                    keyFrame.value = v[componentIndex];
                    outputChannel.channelComponents[componentIndex].fcurve.appendKeyframe(keyframeTimes[i], keyFrame);
                }
            }

            break;
        } // case 4
        }

        nameAndChannels.channels.push_back(outputChannel);
        ++channelIndex;
    }

    return nameAndChannels;
}

GLTFImporter::RawData GLTFImporter::accessorData(int accessorIndex, int index) const
{
    const AccessorData &accessor = m_accessors[accessorIndex];
    const BufferView &bufferView = m_bufferViews[accessor.bufferViewIndex];
    const BufferData &bufferData = m_bufferDatas[bufferView.bufferIndex];
    const QByteArray &ba = bufferData.data;
    const char *rawData = ba.constData() + bufferView.byteOffset + accessor.byteOffset;

    const uint typeSize = accessorTypeSize(accessor.type);
    const int stride = (accessor.byteStride == 0)
            ? accessor.dataSize * typeSize
            : accessor.byteStride;

    const char* data = rawData + index * stride;
    if (data - rawData > ba.size()) {
        qWarning("Attempting to access data beyond end of buffer");
        return RawData{ nullptr, 0 };
    }

    const quint64 byteLength = accessor.dataSize * typeSize;
    RawData rd{ data, byteLength };

    return rd;
}

void GLTFImporter::setBasePath(const QString &path)
{
    m_basePath = path;
}

bool GLTFImporter::setJSON(const QJsonDocument &json)
{
    if (!json.isObject())
        return false;
    m_json = json;
    cleanup();
    return true;
}

bool GLTFImporter::parse()
{
    // Find the glTF version
    const QJsonObject asset = m_json.object().value(KEY_ASSET).toObject();
    const QString versionString = asset.value(KEY_VERSION).toString();
    const auto version = QVersionNumber::fromString(versionString);
    switch (version.majorVersion()) {
    case 2:
        return parseGLTF2();

    default:
        qWarning() << "Unsupported version of glTF" << versionString;
        return false;
    }
}

bool GLTFImporter::parseGLTF2()
{
    bool success = true;
    const QJsonArray buffers = m_json.object().value(KEY_BUFFERS).toArray();
    for (const auto &bufferValue : buffers)
        success &= processJSONBuffer(bufferValue.toObject());

    const QJsonArray bufferViews = m_json.object().value(KEY_BUFFER_VIEWS).toArray();
    for (const auto &bufferViewValue : bufferViews)
        success &= processJSONBufferView(bufferViewValue.toObject());

    const QJsonArray accessors = m_json.object().value(KEY_ACCESSORS).toArray();
    for (const auto &accessorValue : accessors)
        success &= processJSONAccessor(accessorValue.toObject());

    const QJsonArray skins = m_json.object().value(KEY_SKINS).toArray();
    for (const auto &skinValue : skins)
        success &= processJSONSkin(skinValue.toObject());

    const QJsonArray animations = m_json.object().value(KEY_ANIMATIONS).toArray();
    for (const auto &animationValue : animations)
        success &= processJSONAnimation(animationValue.toObject());

    const QJsonArray nodes = m_json.object().value(KEY_NODES).toArray();
    for (const auto &nodeValue : nodes)
        success &= processJSONNode(nodeValue.toObject());
    setupNodeParentLinks();

    // TODO: Make a complete GLTF 2 parser by extending to other top level elements:
    // scenes, animations, meshes etc.

    return success;
}

void GLTFImporter::cleanup()
{
    m_accessors.clear();
    m_bufferViews.clear();
    m_bufferDatas.clear();
}

bool GLTFImporter::processJSONBuffer(const QJsonObject &json)
{
    // Store buffer details and load data into memory
    BufferData buffer(json);
    buffer.data = resolveLocalData(buffer.path);
    if (buffer.data.isEmpty())
        return false;

    m_bufferDatas.push_back(buffer);
    return true;
}

bool GLTFImporter::processJSONBufferView(const QJsonObject &json)
{
    BufferView bufferView(json);

    // Perform sanity checks
    const auto bufferIndex = bufferView.bufferIndex;
    if (Q_UNLIKELY(bufferIndex) >= m_bufferDatas.size()) {
        qWarning("Unknown buffer %d when processing buffer view", bufferIndex);
        return false;
    }

    const auto &bufferData = m_bufferDatas[bufferIndex];
    if (bufferView.byteOffset > bufferData.byteLength) {
        qWarning("Bufferview has offset greater than buffer %d length", bufferIndex);
        return false;
    }

    if (Q_UNLIKELY(bufferView.byteOffset + bufferView.byteLength > bufferData.byteLength)) {
        qWarning("BufferView extends beyond end of buffer %d", bufferIndex);
        return false;
    }

    m_bufferViews.push_back(bufferView);
    return true;
}

bool GLTFImporter::processJSONAccessor(const QJsonObject &json)
{
    AccessorData accessor(json);

    // TODO: Perform sanity checks

    m_accessors.push_back(accessor);
    return true;
}

bool GLTFImporter::processJSONSkin(const QJsonObject &json)
{
    Skin skin(json);

    // TODO: Perform sanity checks

    m_skins.push_back(skin);
    return true;
}

bool GLTFImporter::processJSONAnimation(const QJsonObject &json)
{
    const Animation animation(json);

    for (const auto &channel : animation.channels) {
        if (channel.samplerIndex == -1)
            qWarning() << "Invalid sampler index in animation"
                       << animation.name << "for channel targeting node"
                       << channel.targetNodeIndex << " and property"
                       << channel.targetProperty;
    }

    for (const auto &sampler : animation.samplers) {
        if (sampler.inputAccessorIndex == -1) {
            qWarning() << "Sampler for animaton" << animation.name
                       << "references has an invalid input accessor index";
        }

        if (sampler.outputAccessorIndex == -1) {
            qWarning() << "Sampler for animaton" << animation.name
                       << "references has an invalid output accessor index";
        }
    }

    m_animations.push_back(animation);
    return true;
}

bool GLTFImporter::processJSONNode(const QJsonObject &json)
{
    Node node(json);

    // TODO: Perform sanity checks

    m_nodes.push_back(node);
    return true;
}

void GLTFImporter::setupNodeParentLinks()
{
    const int nodeCount = m_nodes.size();
    for (int i = 0; i < nodeCount; ++i) {
        const Node &node = m_nodes[i];
        const QVector<int> &childNodeIndices = node.childNodeIndices;
        for (const auto childNodeIndex : childNodeIndices) {
            Q_ASSERT(childNodeIndex < m_nodes.size());
            Node &childNode = m_nodes[childNodeIndex];
            Q_ASSERT(childNode.parentNodeIndex == -1);
            childNode.parentNodeIndex = i;
        }
    }
}

QByteArray GLTFImporter::resolveLocalData(const QString &path) const
{
    QDir d(m_basePath);
    Q_ASSERT(d.exists());

    QString absPath = d.absoluteFilePath(path);
    QFile f(absPath);
    f.open(QIODevice::ReadOnly);
    return f.readAll();
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
