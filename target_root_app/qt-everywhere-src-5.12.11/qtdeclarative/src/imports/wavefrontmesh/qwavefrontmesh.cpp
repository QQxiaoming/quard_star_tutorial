/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwavefrontmesh.h"

#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/private/qobject_p.h>

#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>

#include <QtQml/qqmlfile.h>
#include <QtQml/qqmlcontext.h>

#include <QtQuick/qsggeometry.h>

QT_BEGIN_NAMESPACE

class QWavefrontMeshPrivate : public QObjectPrivate
{
public:
    QWavefrontMeshPrivate()
        : lastError(QWavefrontMesh::NoError)
    {}

    Q_DECLARE_PUBLIC(QWavefrontMesh)

    static QWavefrontMeshPrivate *get(QWavefrontMesh *mesh)
    {
        return mesh->d_func();
    }

    static const QWavefrontMeshPrivate *get(const QWavefrontMesh *mesh)
    {
        return mesh->d_func();
    }

    QVector<QPair<ushort, ushort> > indexes;
    QVector<QVector3D> vertexes;
    QVector<QVector2D> textureCoordinates;

    QUrl source;
    QWavefrontMesh::Error lastError;

    QVector3D planeV;
    QVector3D planeW;
};

/*!
    \qmlmodule Qt.labs.wavefrontmesh 1.\QtMinorVersion
    \title Qt Labs WavefrontMesh QML Types
    \ingroup qmlmodules
    \brief The WavefrontMesh provides a mesh based on a Wavefront .obj file.

    To use this module, import the module with the following line:

    \qml \QtMinorVersion
    import Qt.labs.wavefrontmesh 1.\1
    \endqml
*/

/*!
    \qmltype WavefrontMesh
    \inqmlmodule Qt.labs.wavefrontmesh
    \instantiates QWavefrontMesh
    \ingroup qtquick-effects
    \brief The WavefrontMesh provides a mesh based on a Wavefront .obj file.
    \since 5.12

    WavefrontMesh reads the geometry from a Wavefront .obj file and generates
    a two-dimensional \l{QSGGeometry}{geometry} from this. If the .obj file
    contains a three-dimensional shape, it will be orthographically projected,
    onto a plane. If defined, this is given by \l projectionPlaneV
    and \l projectionPlaneW. Otherwise, the first face encountered in the data
    will be used to determine the projection plane.

    If the file contains texture coordinates, these will also be used. Otherwise,
    the vertexes of the object will be normalized and used.

    The mesh can be used in a ShaderEffect to define the shaded geometry. The
    geometry will be normalized before use, so the position and scale of the
    input objects have no impact on the result.

    \note Some Wavefront exporters will change the source scene's coordinate system
    before exporting it. This can cause unexpected results when Qt applies the
    projection. If the visual results are not as you expect, try checking the export
    parameters and the documentation of the editor tool to see if this is the case.

    For instance, the following example takes an .obj file containing a standard torus
    and visualizes the automatically generated texture coordinates.

    \table
    \row
    \li \image qtlabs-wavefrontmesh.png
    \li \qml
        import QtQuick 2.\1
        import Qt.labs.wavefrontmesh 1.\1

        ShaderEffect {
            width: 200
            height: 200
            mesh: WavefrontMesh {
                source: "torus.obj"
                projectionPlaneV: Qt.vector3d(0, 1, 0)
                projectionPlaneW: Qt.vector3d(1, 0, 0)
            }
            vertexShader: "
                    uniform highp mat4 qt_Matrix;
                    attribute highp vec4 qt_Vertex;
                    attribute highp vec2 qt_MultiTexCoord0;
                    varying highp vec2 coord;
                    void main() {
                        coord = qt_MultiTexCoord0;
                        gl_Position = qt_Matrix * qt_Vertex;
                    }"
            fragmentShader: "
                    varying highp vec2 coord;
                    uniform lowp float qt_Opacity;
                    void main() {
                        gl_FragColor = vec4(coord.x, coord.y, 0.0, 1.0);
                    }"

        }
        \endqml
    \endtable

    \note Since the input is a 3D torus, we need to define the projection plane. This would not be necessary when
    using a 2D shape as input. We use the XY plane in this case, because of the orientation of the input.
*/

QWavefrontMesh::QWavefrontMesh(QObject *parent)
    : QQuickShaderEffectMesh(*(new QWavefrontMeshPrivate), parent)
{
    connect(this, &QWavefrontMesh::sourceChanged, this, &QWavefrontMesh::readData);
    connect(this, &QWavefrontMesh::projectionPlaneVChanged, this, &QQuickShaderEffectMesh::geometryChanged);
    connect(this, &QWavefrontMesh::projectionPlaneWChanged, this, &QQuickShaderEffectMesh::geometryChanged);
}

QWavefrontMesh::~QWavefrontMesh()
{
}

/*!
    \qmlproperty enumeration WavefrontMesh::lastError

    This property holds the last error, if any, that occurred when parsing the
    source or building the mesh.

    \list
    \li WavefrontMesh.NoError No error has occurred.
    \li WavefrontMesh.InvalidSourceError The source was not recognized as a valid .obj file.
    \li WavefrontMesh.UnsupportedFaceShapeError The faces in the source is of an unsupported type.
                                                WavefrontMesh only supports triangles and convex quads.
    \li WavefrontMesh.UnsupportedIndexSizeError The source shape is too large. Only 16 bit indexes are supported.
    \li WavefrontMesh.FileNotFoundError The source file was not found.
    \li WavefrontMesh.MissingPositionAttributeError The 'qt_Vertex' attribute is missing from the shaders.
    \li WavefrontMesh.MissingTextureCoordinateAttributeError The texture coordinate attribute in the shaders is wrongly named. Use 'qt_MultiTexCoord0'.
    \li WavefrontMesh.MissingPositionAndTextureCoordinateAttributesError Both the 'qt_Vertex' and 'qt_MultiTexCoord0' attributes are missing from the shaders.
    \li WavefrontMesh.TooManyAttributesError The shaders expect too many attributes (maximum is two: Position, 'qt_Vertex', and texture coordinate, 'qt_MultiTexCoord0').
    \li WavefrontMesh.InvalidPlaneDefinitionError The V and W vectors in the plane cannot be null, nor parallel to each other.
    \endlist
*/

QWavefrontMesh::Error QWavefrontMesh::lastError() const
{
    Q_D(const QWavefrontMesh);
    return d->lastError;
}

void QWavefrontMesh::setLastError(Error lastError)
{
    Q_D(QWavefrontMesh);
    if (d->lastError == lastError)
        return;

    d->lastError = lastError;
    emit lastErrorChanged();
}

/*!
    \qmlproperty url WavefrontMesh::source

    This property holds the URL of the source. This must be either a local file or in qrc. The source will
    be read as a Wavefront .obj file and the geometry will be updated.
*/
QUrl QWavefrontMesh::source() const
{
    Q_D(const QWavefrontMesh);
    return d->source;
}

void QWavefrontMesh::setSource(const QUrl &source)
{
    Q_D(QWavefrontMesh);
    if (d->source == source)
        return;

    d->source = source;
    emit sourceChanged();
}

void QWavefrontMesh::readData()
{
    Q_D(QWavefrontMesh);
    d->vertexes.clear();
    d->textureCoordinates.clear();
    d->indexes.clear();

    QString localFile = QQmlFile::urlToLocalFileOrQrc(d->source);
    if (!localFile.isEmpty()) {
        QFile file(localFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);

            QString buffer;
            buffer.reserve(256);

            static QChar space(QLatin1Char(' '));
            static QChar slash(QLatin1Char('/'));

            while (!stream.atEnd()) {
                stream.readLineInto(&buffer);
                QVector<QStringRef> tokens = buffer.splitRef(space, QString::SkipEmptyParts);
                if (tokens.size() < 2)
                    continue;

                QByteArray command = tokens.at(0).toLatin1();

                if (command == "vt") {
                    bool ok;
                    float u = tokens.at(1).toFloat(&ok);
                    if (!ok) {
                        setLastError(InvalidSourceError);
                        return;
                    }

                    float v = tokens.size() > 2 ? tokens.at(2).toFloat(&ok) : 0.0;
                    if (!ok) {
                        setLastError(InvalidSourceError);
                        return;
                    }

                    d->textureCoordinates.append(QVector2D(u, v));
                } else if (command == "v") {
                    // Format: v <x> <y> <z> [w]
                    if (tokens.length() < 4 || tokens.length() > 5) {
                        setLastError(InvalidSourceError);
                        return;
                    }

                    bool ok;

                    float x = tokens.at(1).toFloat(&ok);
                    if (!ok) {
                        setLastError(InvalidSourceError);
                        return;
                    }

                    float y = tokens.at(2).toFloat(&ok);
                    if (!ok) {
                        setLastError(InvalidSourceError);
                        return;
                    }

                    float z = tokens.at(3).toFloat(&ok);
                    if (!ok) {
                        setLastError(InvalidSourceError);
                        return;
                    }

                    d->vertexes.append(QVector3D(x, y, z));
                } else if (command == "f") {
                    // The scenegraph only supports triangles, so we
                    // support triangles and quads (which we split up)
                    int p1, p2, p3;
                    int t1 = 0;
                    int t2 = 0;
                    int t3 = 0;
                    if (tokens.size() >= 4 && tokens.size() <= 5) {
                        {
                            bool ok;
                            QVector<QStringRef> faceTokens = tokens.at(1).split(slash, QString::SkipEmptyParts);
                            Q_ASSERT(!faceTokens.isEmpty());

                            p1 = faceTokens.at(0).toInt(&ok) - 1;
                            if (!ok) {
                                setLastError(InvalidSourceError);
                                return;
                            }

                            if (faceTokens.size() > 1) {
                                t1 = faceTokens.at(1).toInt(&ok) - 1;
                                if (!ok) {
                                    setLastError(InvalidSourceError);
                                    return;
                                }
                            }
                        }

                        {
                            bool ok;
                            QVector<QStringRef> faceTokens = tokens.at(2).split(slash, QString::SkipEmptyParts);
                            Q_ASSERT(!faceTokens.isEmpty());

                            p2 = faceTokens.at(0).toInt(&ok) - 1;
                            if (!ok) {
                                setLastError(InvalidSourceError);
                                return;
                            }

                            if (faceTokens.size() > 1) {
                                t2 = faceTokens.at(1).toInt(&ok) - 1;
                                if (!ok) {
                                    setLastError(InvalidSourceError);
                                    return;
                                }
                            }
                        }

                        {
                            bool ok;
                            QVector<QStringRef> faceTokens = tokens.at(3).split(slash, QString::SkipEmptyParts);
                            Q_ASSERT(!faceTokens.isEmpty());

                            p3 = faceTokens.at(0).toInt(&ok) - 1;
                            if (!ok) {
                                setLastError(InvalidSourceError);
                                return;
                            }

                            if (faceTokens.size() > 1) {
                                t3 = faceTokens.at(1).toInt(&ok) - 1;
                                if (!ok) {
                                    setLastError(InvalidSourceError);
                                    return;
                                }
                            }
                        }

                        if (Q_UNLIKELY(p1 < 0 || p1 > UINT16_MAX
                                        || p2 < 0 || p2 > UINT16_MAX
                                        || p3 < 0 || p3 > UINT16_MAX
                                        || t1 < 0 || t1 > UINT16_MAX
                                        || t2 < 0 || t2 > UINT16_MAX
                                        || t3 < 0 || t3 > UINT16_MAX)) {
                            setLastError(UnsupportedIndexSizeError);
                            return;
                        }

                        d->indexes.append(qMakePair(ushort(p1), ushort(t1)));
                        d->indexes.append(qMakePair(ushort(p2), ushort(t2)));
                        d->indexes.append(qMakePair(ushort(p3), ushort(t3)));
                    } else {
                        setLastError(UnsupportedFaceShapeError);
                        return;
                    }

                    if (tokens.size() == 5) {
                        bool ok;
                        QVector<QStringRef> faceTokens = tokens.at(4).split(slash, QString::SkipEmptyParts);
                        Q_ASSERT(!faceTokens.isEmpty());

                        int p4 = faceTokens.at(0).toInt(&ok) - 1;
                        if (!ok) {
                            setLastError(InvalidSourceError);
                            return;
                        }

                        int t4 = 0;
                        if (faceTokens.size() > 1) {
                            t4 = faceTokens.at(1).toInt(&ok) - 1;
                            if (!ok) {
                                setLastError(InvalidSourceError);
                                return;
                            }
                        }

                        if (Q_UNLIKELY(p4 < 0 || p4 > UINT16_MAX || t4 < 0 || t4 > UINT16_MAX)) {
                            setLastError(UnsupportedIndexSizeError);
                            return;
                        }

                        // ### Assumes convex quad, correct algorithm is to find the concave corner,
                        // and if there is one, do the split on the line between this and the corner it is
                        // not connected to. Also assumes order of vertices is counter clockwise.
                        d->indexes.append(qMakePair(ushort(p3), ushort(t3)));
                        d->indexes.append(qMakePair(ushort(p4), ushort(t4)));
                        d->indexes.append(qMakePair(ushort(p1), ushort(t1)));
                    }
                }
            }
        } else {
            setLastError(FileNotFoundError);
        }
    } else {
        setLastError(InvalidSourceError);
    }

    emit geometryChanged();
}

QString QWavefrontMesh::log() const
{
    Q_D(const QWavefrontMesh);
    switch (d->lastError) {
    case NoError: return QStringLiteral("No error");
    case InvalidSourceError: return QStringLiteral("Error: Invalid source");
    case UnsupportedFaceShapeError: return QStringLiteral("Error: Unsupported face shape in source");
    case UnsupportedIndexSizeError: return QStringLiteral("Error: Unsupported index size in source");
    case FileNotFoundError: return QStringLiteral("Error: File not found");
    case MissingPositionAttributeError: return QStringLiteral("Error: Missing '%1' attribute").arg(qtPositionAttributeName());
    case MissingTextureCoordinateAttributeError: return QStringLiteral("Error: Missing '%1' attribute").arg(qtTexCoordAttributeName());
    case MissingPositionAndTextureCoordinateAttributesError: return QStringLiteral("Error: Missing '%1' and '%2' attributes").arg(qtPositionAttributeName()).arg(qtTexCoordAttributeName());
    case TooManyAttributesError: return QStringLiteral("Error: Too many attributes");
    case InvalidPlaneDefinitionError: return QStringLiteral("Error: Invalid plane. V and W must be non-null and cannot be parallel");
    default: return QStringLiteral("Unknown error");
    };
}

bool QWavefrontMesh::validateAttributes(const QVector<QByteArray> &attributes, int *posIndex)
{
    Q_D(QWavefrontMesh);
    const int attrCount = attributes.count();
    int positionIndex = attributes.indexOf(qtPositionAttributeName());
    int texCoordIndex = attributes.indexOf(qtTexCoordAttributeName());

    switch (attrCount) {
    case 0:
        d->lastError = NoAttributesError;
        return false;
    case 1:
        if (positionIndex < 0) {
            d->lastError = MissingPositionAttributeError;
            return false;
        }
        break;
    case 2:
        if (positionIndex < 0 || texCoordIndex < 0) {
            if (positionIndex < 0 && texCoordIndex < 0)
                d->lastError = MissingPositionAndTextureCoordinateAttributesError;
            else if (positionIndex < 0)
                d->lastError = MissingPositionAttributeError;
            else if (texCoordIndex < 0)
                d->lastError = MissingTextureCoordinateAttributeError;
            return false;
        }
        break;
    default:
        d->lastError = TooManyAttributesError;
        return false;
    }

    if (posIndex)
        *posIndex = positionIndex;

    return true;

}

QSGGeometry *QWavefrontMesh::updateGeometry(QSGGeometry *geometry, int attributeCount, int positionIndex,
                                            const QRectF &sourceRect, const QRectF &destinationRect)
{
    Q_D(QWavefrontMesh);

    if (geometry == nullptr) {
        Q_ASSERT(attributeCount == 1 || attributeCount == 2);
        geometry = new QSGGeometry(attributeCount == 1
                                   ? QSGGeometry::defaultAttributes_Point2D()
                                   : QSGGeometry::defaultAttributes_TexturedPoint2D(),
                                   d->indexes.size(),
                                   d->indexes.size(),
                                   QSGGeometry::UnsignedShortType);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);

    } else {
        geometry->allocate(d->indexes.size(), d->indexes.size());
    }

    // If there is not at least a full triangle in the data set, skip out
    if (d->indexes.size() < 3) {
        geometry->allocate(0, 0);
        return geometry;
    }

    QVector3D planeV = d->planeV;
    QVector3D planeW = d->planeW;

    // Automatically detect plane based on first face if none is set
    if (planeV.isNull() || planeW.isNull()) {
        QVector3D p = d->vertexes.at(d->indexes.at(0).first);
        planeV = (d->vertexes.at(d->indexes.at(1).first) - p);
        planeW = (p - d->vertexes.at(d->indexes.at(2).first)).normalized();
    }

    planeV.normalize();
    planeW.normalize();

    QVector3D planeNormal = QVector3D::crossProduct(planeV, planeW).normalized();
    if (planeNormal.isNull()) { // V and W are either parallel or null
        setLastError(InvalidPlaneDefinitionError);
        geometry->allocate(0, 0);
        return geometry;
    }

    QVector3D planeAxes1 = planeV;
    QVector3D planeAxes2 = QVector3D::crossProduct(planeAxes1, planeNormal).normalized();

    ushort *indexData = static_cast<ushort *>(geometry->indexData());
    QSGGeometry::Point2D *vertexData = static_cast<QSGGeometry::Point2D *>(geometry->vertexData());

    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
    for (ushort i = 0; i < ushort(d->indexes.size()); ++i) {
        *(indexData + i) = i;

        QVector3D v = d->vertexes.at(d->indexes.at(i).first);

        // Project onto plane
        QVector2D w;
        v -= QVector3D::dotProduct(planeNormal, v) * planeNormal;
        w.setX(QVector3D::dotProduct(v, planeAxes1));
        w.setY(QVector3D::dotProduct(v, planeAxes2));

        QSGGeometry::Point2D *positionData = vertexData + (i * attributeCount + positionIndex);
        positionData->x = w.x();
        positionData->y = w.y();

        if (i == 0 || minX > w.x())
            minX = w.x();
        if (i == 0 || maxX < w.x())
            maxX = w.x();
        if (i == 0 || minY > w.y())
            minY = w.y();
        if (i == 0 || maxY < w.y())
            maxY = w.y();

        if (attributeCount > 1 && !d->textureCoordinates.isEmpty()) {
            Q_ASSERT(positionIndex == 0 || positionIndex == 1);

            QVector2D uv = d->textureCoordinates.at(d->indexes.at(i).second);
            QSGGeometry::Point2D *textureCoordinateData = vertexData + (i * attributeCount + (1 - positionIndex));
            textureCoordinateData->x = uv.x();
            textureCoordinateData->y = uv.y();
        }
    }

    float width = maxX - minX;
    float height = maxY - minY;

    QVector2D center(minX + width / 2.0f, minY + height / 2.0f);
    QVector2D scale(1.0f / width, 1.0f / height);

    for (int i = 0; i < geometry->vertexCount(); ++i) {
        float x = ((vertexData + positionIndex)->x - center.x()) * scale.x();
        float y = ((vertexData + positionIndex)->y - center.y()) * scale.y();

        for (int attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex) {
            if (attributeIndex == positionIndex) {
                vertexData->x = float(destinationRect.left()) + x * float(destinationRect.width()) + float(destinationRect.width()) / 2.0f;
                vertexData->y = float(destinationRect.top()) + y * float(destinationRect.height()) + float(destinationRect.height()) / 2.0f;
            } else {
                // If there are no texture coordinates, use the normalized vertex
                float tx = d->textureCoordinates.isEmpty() ? x : vertexData->x;
                float ty = d->textureCoordinates.isEmpty() ? y : vertexData->y;

                vertexData->x = float(sourceRect.left()) + tx * float(sourceRect.width());
                vertexData->y = float(sourceRect.top()) + ty * float(sourceRect.height());
            }

            ++vertexData;
        }
    }

    return geometry;
}

/*!
    \qmlproperty vector3d WavefrontMesh::projectionPlaneV

    Since the Wavefront .obj format describes an object in 3D space, the coordinates
    have to be projected into 2D before they can be displayed in Qt Quick.

    This will be done in WavefrontMesh by an orthographic projection onto an
    appropriate plane.

    The projectionPlaneV is one of two vectors in the plane in 3D space. If
    either this, or \l projectionPlaneW is set to (0, 0, 0) (the default),
    then the plane will be detected based on the first encountered face in the
    data set.

    \note projectionPlaneV and \l projectionPlaneW cannot be parallel vectors.
*/
void QWavefrontMesh::setProjectionPlaneV(const QVector3D &v)
{
    Q_D(QWavefrontMesh);
    if (d->planeV == v)
        return;

    d->planeV = v;
    emit projectionPlaneVChanged();
}

QVector3D QWavefrontMesh::projectionPlaneV() const
{
    Q_D(const QWavefrontMesh);
    return d->planeV;
}

/*!
    \qmlproperty vector3d WavefrontMesh::projectionPlaneW

    Since the Wavefront .obj format describes an object in 3D space, the coordinates
    have to be projected into 2D before they can be displayed in Qt Quick.

    This will be done in WavefrontMesh by an orthographic projection onto an
    appropriate plane.

    The projectionPlaneW is one of two vectors in the plane in 3D space. If
    either this, or \l projectionPlaneV is set to (0, 0, 0) (the default),
    then the plane will be detected based on the first encountered face in the
    data set.

    \note \l projectionPlaneV and projectionPlaneW cannot be parallel vectors.
*/
void QWavefrontMesh::setProjectionPlaneW(const QVector3D &w)
{
    Q_D(QWavefrontMesh);
    if (d->planeW == w)
        return;

    d->planeW = w;
    emit projectionPlaneWChanged();
}

QVector3D QWavefrontMesh::projectionPlaneW() const
{
    Q_D(const QWavefrontMesh);
    return d->planeW;
}


QT_END_NAMESPACE
