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

#include "gltfskeletonloader_p.h"

#include <QtGui/qopengl.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qversionnumber.h>

#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DCore/private/qmath3d_p.h>

QT_BEGIN_NAMESPACE

namespace {

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

namespace Qt3DRender {
namespace Render {

#define KEY_ACCESSORS               QLatin1String("accessors")
#define KEY_ASSET                   QLatin1String("asset")
#define KEY_BUFFER                  QLatin1String("buffer")
#define KEY_BUFFERS                 QLatin1String("buffers")
#define KEY_BUFFER_VIEW             QLatin1String("bufferView")
#define KEY_BUFFER_VIEWS            QLatin1String("bufferViews")
#define KEY_BYTE_LENGTH             QLatin1String("byteLength")
#define KEY_BYTE_OFFSET             QLatin1String("byteOffset")
#define KEY_BYTE_STRIDE             QLatin1String("byteStride")
#define KEY_CAMERA                  QLatin1String("camera")
#define KEY_CHILDREN                QLatin1String("children")
#define KEY_COMPONENT_TYPE          QLatin1String("componentType")
#define KEY_COUNT                   QLatin1String("count")
#define KEY_JOINTS                  QLatin1String("joints")
#define KEY_INVERSE_BIND_MATRICES   QLatin1String("inverseBindMatrices")
#define KEY_MATRIX                  QLatin1String("matrix")
#define KEY_MESH                    QLatin1String("mesh")
#define KEY_NAME                    QLatin1String("name")
#define KEY_NODES                   QLatin1String("nodes")
#define KEY_ROTATION                QLatin1String("rotation")
#define KEY_SCALE                   QLatin1String("scale")
#define KEY_SKIN                    QLatin1String("skin")
#define KEY_SKINS                   QLatin1String("skins")
#define KEY_TARGET                  QLatin1String("target")
#define KEY_TRANSLATION             QLatin1String("translation")
#define KEY_TYPE                    QLatin1String("type")
#define KEY_URI                     QLatin1String("uri")
#define KEY_VERSION                 QLatin1String("version")

GLTFSkeletonLoader::BufferData::BufferData()
    : byteLength(0)
    , data()
{
}

GLTFSkeletonLoader::BufferData::BufferData(const QJsonObject &json)
    : byteLength(json.value(KEY_BYTE_LENGTH).toInt())
    , path(json.value(KEY_URI).toString())
    , data()
{
}

GLTFSkeletonLoader::BufferView::BufferView()
    : bufferIndex(-1)
    , byteOffset(0)
    , byteLength(0)
    , target(0)
{
}

GLTFSkeletonLoader::BufferView::BufferView(const QJsonObject &json)
    : bufferIndex(json.value(KEY_BUFFER).toInt())
    , byteOffset(json.value(KEY_BYTE_OFFSET).toInt())
    , byteLength(json.value(KEY_BYTE_LENGTH).toInt())
    , target(0)
{
    const auto targetValue = json.value(KEY_TARGET);
    if (!targetValue.isUndefined())
        target = targetValue.toInt();
}

GLTFSkeletonLoader::AccessorData::AccessorData()
    : type(QAttribute::Float)
    , dataSize(0)
    , count(0)
    , byteOffset(0)
    , byteStride(0)
{
}

GLTFSkeletonLoader::AccessorData::AccessorData(const QJsonObject &json)
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

GLTFSkeletonLoader::Skin::Skin()
    : inverseBindAccessorIndex(-1)
    , jointNodeIndices()
{
}

GLTFSkeletonLoader::Skin::Skin(const QJsonObject &json)
    : name(json.value(KEY_NAME).toString())
    , inverseBindAccessorIndex(json.value(KEY_INVERSE_BIND_MATRICES).toInt())
{
    QJsonArray jointNodes = json.value(KEY_JOINTS).toArray();
    jointNodeIndices.reserve(jointNodes.size());
    for (const auto jointNodeValue : jointNodes)
        jointNodeIndices.push_back(jointNodeValue.toInt());
}

GLTFSkeletonLoader::Node::Node()
    : localTransform()
    , childNodeIndices()
    , name()
    , parentNodeIndex(-1)
    , cameraIndex(-1)
    , meshIndex(-1)
    , skinIndex(-1)
{
}

GLTFSkeletonLoader::Node::Node(const QJsonObject &json)
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

        if (!scaleValue.isUndefined())
            jsonArrayToVector3D(scaleValue.toArray(), localTransform.scale);

        if (!rotationValue.isUndefined())
            jsonArrayToQuaternion(json.value(KEY_ROTATION).toArray(), localTransform.rotation);

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

QAttribute::VertexBaseType GLTFSkeletonLoader::accessorTypeFromJSON(int componentType)
{
    if (componentType == GL_BYTE)
        return QAttribute::Byte;
    else if (componentType == GL_UNSIGNED_BYTE)
        return QAttribute::UnsignedByte;
    else if (componentType == GL_SHORT)
        return QAttribute::Short;
    else if (componentType == GL_UNSIGNED_SHORT)
        return QAttribute::UnsignedShort;
    else if (componentType == GL_UNSIGNED_INT)
        return QAttribute::UnsignedInt;
    else if (componentType == GL_FLOAT)
        return QAttribute::Float;

    // There shouldn't be an invalid case here
    qCWarning(Jobs, "unsupported accessor type %d", componentType);
    return QAttribute::Float;
}

uint GLTFSkeletonLoader::accessorTypeSize(QAttribute::VertexBaseType componentType)
{
    switch (componentType) {
    case QAttribute::Byte:
    case QAttribute::UnsignedByte:
         return 1;

    case QAttribute::Short:
    case QAttribute::UnsignedShort:
        return 2;

    case QAttribute::Int:
    case QAttribute::Float:
        return 4;

    default:
        qCWarning(Jobs, "Unhandled accessor data type %d", componentType);
        return 0;
    }
}

uint GLTFSkeletonLoader::accessorDataSizeFromJson(const QString &type)
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

GLTFSkeletonLoader::GLTFSkeletonLoader()
{
}

bool GLTFSkeletonLoader::load(QIODevice *ioDev)
{
    QByteArray jsonData = ioDev->readAll();
    QJsonDocument sceneDocument = QJsonDocument::fromBinaryData(jsonData);
    if (sceneDocument.isNull())
        sceneDocument = QJsonDocument::fromJson(jsonData);

    if (Q_UNLIKELY(!setJSON(sceneDocument))) {
        qCWarning(Jobs, "not a JSON document");
        return false;
    }

    auto file = qobject_cast<QFile*>(ioDev);
    if (file) {
        QFileInfo finfo(file->fileName());
        setBasePath(finfo.dir().absolutePath());
    }

    return parse();
}

SkeletonData GLTFSkeletonLoader::createSkeleton(const QString &skeletonName)
{
    if (m_skins.isEmpty()) {
        qCWarning(Jobs, "glTF file does not contain any skins");
        return SkeletonData();
    }

    Skin *skin = m_skins.begin();
    if (!skeletonName.isNull()) {
        const auto result = std::find_if(m_skins.begin(), m_skins.end(),
            [skeletonName](const Skin &skin) { return skin.name == skeletonName; });
        if (result != m_skins.end())
            skin = result;
    }

    Q_ASSERT(skin != nullptr);
    return createSkeletonFromSkin(skin);
}

SkeletonData GLTFSkeletonLoader::createSkeletonFromSkin(Skin *skin) const
{
    SkeletonData skel;

    const int jointCount = skin->jointNodeIndices.size();
    skel.reserve(jointCount);

    QHash<const Node *, int> jointIndexMap;
    for (int i = 0; i < jointCount; ++i) {
        // Get a pointer to the node for this joint and store it in
        // a map to the JointInfo index. We can later use this to set
        // the parent indices of the joints
        const Node *node = &m_nodes[skin->jointNodeIndices[i]];
        jointIndexMap.insert(node, i);

        JointInfo joint;
        joint.inverseBindPose = inverseBindMatrix(skin, i);
        joint.parentIndex = jointIndexMap.value(&m_nodes[node->parentNodeIndex], -1);
        if (joint.parentIndex == -1 && i != 0)
            qCDebug(Jobs) << "Cannot find parent joint for joint" << i;

        skel.joints.push_back(joint);
        skel.localPoses.push_back(node->localTransform);
        skel.jointNames.push_back(node->name);
    }

    return skel;
}

QMatrix4x4 GLTFSkeletonLoader::inverseBindMatrix(Skin *skin, int jointIndex) const
{
    // Create a matrix and copy the data into it
    RawData rawData = accessorData(skin->inverseBindAccessorIndex, jointIndex);
    QMatrix4x4 m;
    memcpy(m.data(), rawData.data, rawData.byteLength);
    return m;
}

GLTFSkeletonLoader::RawData GLTFSkeletonLoader::accessorData(int accessorIndex, int index) const
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
        qCWarning(Jobs, "Attempting to access data beyond end of buffer");
        return RawData{ nullptr, 0 };
    }

    const quint64 byteLength = accessor.dataSize * typeSize;
    RawData rd{ data, byteLength };

    return rd;
}

void GLTFSkeletonLoader::setBasePath(const QString &path)
{
    m_basePath = path;
}

bool GLTFSkeletonLoader::setJSON(const QJsonDocument &json)
{
    if (!json.isObject())
        return false;
    m_json = json;
    cleanup();
    return true;
}

bool GLTFSkeletonLoader::parse()
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

bool GLTFSkeletonLoader::parseGLTF2()
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

    const QJsonArray nodes = m_json.object().value(KEY_NODES).toArray();
    for (const auto &nodeValue : nodes)
        success &= processJSONNode(nodeValue.toObject());
    setupNodeParentLinks();

    // TODO: Make a complete GLTF 2 parser by extending to other top level elements:
    // scenes, animations, meshes etc.

    return success;
}

void GLTFSkeletonLoader::cleanup()
{
    m_accessors.clear();
    m_bufferViews.clear();
    m_bufferDatas.clear();
}

bool GLTFSkeletonLoader::processJSONBuffer(const QJsonObject &json)
{
    // Store buffer details and load data into memory
    BufferData buffer(json);
    buffer.data = resolveLocalData(buffer.path);
    if (buffer.data.isEmpty())
        return false;

    m_bufferDatas.push_back(buffer);
    return true;
}

bool GLTFSkeletonLoader::processJSONBufferView(const QJsonObject &json)
{
    BufferView bufferView(json);

    // Perform sanity checks
    const auto bufferIndex = bufferView.bufferIndex;
    if (Q_UNLIKELY(bufferIndex) >= m_bufferDatas.size()) {
        qCWarning(Jobs, "Unknown buffer %d when processing buffer view", bufferIndex);
        return false;
    }

    const auto &bufferData = m_bufferDatas[bufferIndex];
    if (bufferView.byteOffset > bufferData.byteLength) {
        qCWarning(Jobs, "Bufferview has offset greater than buffer %d length", bufferIndex);
        return false;
    }

    if (Q_UNLIKELY(bufferView.byteOffset + bufferView.byteLength > bufferData.byteLength)) {
        qCWarning(Jobs, "BufferView extends beyond end of buffer %d", bufferIndex);
        return false;
    }

    m_bufferViews.push_back(bufferView);
    return true;
}

bool GLTFSkeletonLoader::processJSONAccessor(const QJsonObject &json)
{
    AccessorData accessor(json);

    // TODO: Perform sanity checks

    m_accessors.push_back(accessor);
    return true;
}

bool GLTFSkeletonLoader::processJSONSkin(const QJsonObject &json)
{
    Skin skin(json);

    // TODO: Perform sanity checks

    m_skins.push_back(skin);
    return true;
}

bool GLTFSkeletonLoader::processJSONNode(const QJsonObject &json)
{
    Node node(json);

    // TODO: Perform sanity checks

    m_nodes.push_back(node);
    return true;
}

void GLTFSkeletonLoader::setupNodeParentLinks()
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

QByteArray GLTFSkeletonLoader::resolveLocalData(const QString &path) const
{
    QDir d(m_basePath);
    Q_ASSERT(d.exists());

    QString absPath = d.absoluteFilePath(path);
    QFile f(absPath);
    f.open(QIODevice::ReadOnly);
    return f.readAll();
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
