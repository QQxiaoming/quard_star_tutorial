/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "qshaderprogram.h"
#include "qshaderprogram_p.h"
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DRender/private/qurlhelper_p.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

/*!
    \class Qt3DRender::QShaderProgram
    \inmodule Qt3DRender
    \brief Encapsulates a Shader Program.
    \inherits Qt3DCore::QNode
    \since 5.5

    A shader program consists of several different shaders, such as vertex and fragment shaders.

    Qt3D will automatically populate a set of default uniforms if they are
    encountered during the shader instrospection phase.

    \table
    \header
        \li {1, 1} Default Uniform
        \li {2, 1} Associated Qt3D Parameter name
        \li {3, 1} GLSL declaration

    \row
        \li {1, 1} ModelMatrix
        \li {2, 1} modelMatrix
        \li {3, 1} uniform mat4 modelMatrix;

    \row
        \li {1, 1} ViewMatrix
        \li {2, 1} viewMatrix
        \li {3, 1} uniform mat4 viewMatrix;

    \row
        \li {1, 1} ProjectionMatrix
        \li {2, 1} projectionMatrix
        \li {3, 1} uniform mat4 projectionMatrix;

    \row
        \li {1, 1} ModelViewMatrix
        \li {2, 1} modelView
        \li {3, 1} uniform mat4 modelView;

    \row
        \li {1, 1} ViewProjectionMatrix
        \li {2, 1} viewProjectionMatrix
        \li {3, 1} uniform mat4 viewProjectionMatrix;

    \row
        \li {1, 1} ModelViewProjectionMatrix
        \li {2, 1} modelViewProjection \br mvp
        \li {3, 1} uniform mat4 modelViewProjection; \br uniform mat4 mvp;

    \row
        \li {1, 1} InverseModelMatrix
        \li {2, 1} inverseModelMatrix
        \li {3, 1} uniform mat4 inverseModelMatrix;

    \row
        \li {1, 1} InverseViewMatrix
        \li {2, 1} inverseViewMatrix
        \li {3, 1} uniform mat4 inverseViewMatrix;

    \row
        \li {1, 1} InverseProjectionMatrix
        \li {2, 1} inverseProjectionMatrix
        \li {3, 1} uniform mat4 inverseProjectionMatrix;

    \row
        \li {1, 1} InverseModelViewMatrix
        \li {2, 1} inverseModelView
        \li {3, 1} uniform mat4 inverseModelView;

    \row
        \li {1, 1} InverseViewProjectionMatrix
        \li {2, 1} inverseViewProjectionMatrix
        \li {3, 1} uniform mat4 inverseViewProjectionMatrix;

    \row
        \li {1, 1} InverseModelViewProjectionMatrix
        \li {2, 1} inverseModelViewProjection
        \li {3, 1} uniform mat4 inverseModelViewProjection;

    \row
        \li {1, 1} ModelNormalMatrix
        \li {2, 1} modelNormalMatrix
        \li {3, 1} uniform mat3 modelNormalMatrix;

    \row
        \li {1, 1} ModelViewNormalMatrix
        \li {2, 1} modelViewNormal
        \li {3, 1} uniform mat3 modelViewNormal;

    \row
        \li {1, 1} ViewportMatrix
        \li {2, 1} viewportMatrix
        \li {3, 1} uniform mat4 viewportMatrix;

    \row
        \li {1, 1} InverseViewportMatrix
        \li {2, 1} inverseViewportMatrix
        \li {3, 1} uniform mat4 inverseViewportMatrix;

    \row
        \li {1, 1} AspectRatio \br (surface width / surface height)
        \li {2, 1} aspectRatio
        \li {3, 1} uniform float aspectRatio;

    \row
        \li {1, 1} Exposure
        \li {2, 1} exposure
        \li {3, 1} uniform float exposure;

    \row
        \li {1, 1} Gamma
        \li {2, 1} gamma
        \li {3, 1} uniform float gamma;

    \row
        \li {1, 1} Time \br (in nano seconds)
        \li {2, 1} time
        \li {3, 1} uniform float time;

    \row
        \li {1, 1} EyePosition
        \li {2, 1} eyePosition
        \li {3, 1} uniform vec3 eyePosition;

    \row
        \li {1, 1} SkinningPalette
        \li {2, 1} skinningPalette[0]
        \li {3, 1} const int maxJoints = 100; \br uniform mat4 skinningPalette[maxJoints];

    \endtable
*/

/*!
    \qmltype ShaderProgram
    \instantiates Qt3DRender::QShaderProgram
    \inqmlmodule Qt3D.Render
    \brief Encapsulates a Shader Program.
    \since 5.5

    ShaderProgram class encapsulates a shader program. A shader program consists of several
    different shaders, such as vertex and fragment shaders.

    Qt3D will automatically populate a set of default uniforms if they are
    encountered during the shader instrospection phase.

    \table
    \header
        \li {1, 1} Default Uniform
        \li {2, 1} Associated Qt3D Parameter name
        \li {3, 1} GLSL declaration

    \row
        \li {1, 1} ModelMatrix
        \li {2, 1} modelMatrix
        \li {3, 1} uniform mat4 modelMatrix;

    \row
        \li {1, 1} ViewMatrix
        \li {2, 1} viewMatrix
        \li {3, 1} uniform mat4 viewMatrix;

    \row
        \li {1, 1} ProjectionMatrix
        \li {2, 1} projectionMatrix
        \li {3, 1} uniform mat4 projectionMatrix;

    \row
        \li {1, 1} ModelViewMatrix
        \li {2, 1} modelView
        \li {3, 1} uniform mat4 modelView;

    \row
        \li {1, 1} ViewProjectionMatrix
        \li {2, 1} viewProjectionMatrix
        \li {3, 1} uniform mat4 viewProjectionMatrix;

    \row
        \li {1, 1} ModelViewProjectionMatrix
        \li {2, 1} modelViewProjection \br mvp
        \li {3, 1} uniform mat4 modelViewProjection; \br uniform mat4 mvp;

    \row
        \li {1, 1} InverseModelMatrix
        \li {2, 1} inverseModelMatrix
        \li {3, 1} uniform mat4 inverseModelMatrix;

    \row
        \li {1, 1} InverseViewMatrix
        \li {2, 1} inverseViewMatrix
        \li {3, 1} uniform mat4 inverseViewMatrix;

    \row
        \li {1, 1} InverseProjectionMatrix
        \li {2, 1} inverseProjectionMatrix
        \li {3, 1} uniform mat4 inverseProjectionMatrix;

    \row
        \li {1, 1} InverseModelViewMatrix
        \li {2, 1} inverseModelView
        \li {3, 1} uniform mat4 inverseModelView;

    \row
        \li {1, 1} InverseViewProjectionMatrix
        \li {2, 1} inverseViewProjectionMatrix
        \li {3, 1} uniform mat4 inverseViewProjectionMatrix;

    \row
        \li {1, 1} InverseModelViewProjectionMatrix
        \li {2, 1} inverseModelViewProjection
        \li {3, 1} uniform mat4 inverseModelViewProjection;

    \row
        \li {1, 1} ModelNormalMatrix
        \li {2, 1} modelNormalMatrix
        \li {3, 1} uniform mat3 modelNormalMatrix;

    \row
        \li {1, 1} ModelViewNormalMatrix
        \li {2, 1} modelViewNormal
        \li {3, 1} uniform mat3 modelViewNormal;

    \row
        \li {1, 1} ViewportMatrix
        \li {2, 1} viewportMatrix
        \li {3, 1} uniform mat4 viewportMatrix;

    \row
        \li {1, 1} InverseViewportMatrix
        \li {2, 1} inverseViewportMatrix
        \li {3, 1} uniform mat4 inverseViewportMatrix;

    \row
        \li {1, 1} AspectRatio \br (surface width / surface height)
        \li {2, 1} aspectRatio
        \li {3, 1} uniform float aspectRatio;

    \row
        \li {1, 1} Exposure
        \li {2, 1} exposure
        \li {3, 1} uniform float exposure;

    \row
        \li {1, 1} Gamma
        \li {2, 1} gamma
        \li {3, 1} uniform float gamma;

    \row
        \li {1, 1} Time \br (in nano seconds)
        \li {2, 1} time
        \li {3, 1} uniform float time;

    \row
        \li {1, 1} EyePosition
        \li {2, 1} eyePosition
        \li {3, 1} uniform vec3 eyePosition;

    \row
        \li {1, 1} SkinningPalette
        \li {2, 1} skinningPalette[0]
        \li {3, 1} const int maxJoints = 100; \br uniform mat4 skinningPalette[maxJoints];

    \endtable
*/

/*!
    \enum QShaderProgram::ShaderType

    This enum identifies the type of shader used.

    \value Vertex                  Vertex shader
    \value Fragment                Fragment shader
    \value TessellationControl     Tesselation control shader
    \value TessellationEvaluation  Tesselation evaluation shader
    \value Geometry                Geometry shader
    \value Compute                 Compute shader
*/

/*!
    \enum QShaderProgram::Status

    This enum identifies the status of shader used.

    \value NotReady              The shader hasn't been compiled and linked yet
    \value Ready                 The shader was successfully compiled
    \value Error                 An error occurred while compiling the shader
*/

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

QShaderProgramPrivate::QShaderProgramPrivate()
    : QNodePrivate()
    , m_status(QShaderProgram::NotReady)
{
}

void QShaderProgramPrivate::setLog(const QString &log)
{
    Q_Q(QShaderProgram);
    if (log != m_log) {
        m_log = log;
        const bool blocked = q->blockNotifications(true);
        emit q->logChanged(m_log);
        q->blockNotifications(blocked);
    }
}

void QShaderProgramPrivate::setStatus(QShaderProgram::Status status)
{
    Q_Q(QShaderProgram);
    if (status != m_status) {
        m_status = status;
        const bool blocked = q->blockNotifications(true);
        emit q->statusChanged(m_status);
        q->blockNotifications(blocked);
    }
}

QShaderProgram::QShaderProgram(QNode *parent)
    : QNode(*new QShaderProgramPrivate, parent)
{
}

QShaderProgram::~QShaderProgram()
{
}

/*! \internal */
QShaderProgram::QShaderProgram(QShaderProgramPrivate &dd, QNode *parent)
    : QNode(dd, parent)
{
}

/*!
    Posts a scene change with parameter \a change.
*/
void QShaderProgram::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    Q_D(QShaderProgram);
    if (change->type() == Qt3DCore::PropertyUpdated) {
        const Qt3DCore::QPropertyUpdatedChangePtr e = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
        if (e->propertyName() == QByteArrayLiteral("log"))
            d->setLog(e->value().toString());
        else if (e->propertyName() == QByteArrayLiteral("status"))
            d->setStatus(static_cast<QShaderProgram::Status>(e->value().toInt()));
    }
}

/*!
    \qmlproperty string ShaderProgram::vertexShaderCode

    Holds the vertex shader code used by this shader program.
*/
/*!
    \property QShaderProgram::vertexShaderCode

    Holds the vertex shader code used by this shader program.
*/
void QShaderProgram::setVertexShaderCode(const QByteArray &vertexShaderCode)
{
    Q_D(QShaderProgram);
    if (vertexShaderCode != d->m_vertexShaderCode) {
        d->m_vertexShaderCode = vertexShaderCode;
        emit vertexShaderCodeChanged(vertexShaderCode);
    }
}

QByteArray QShaderProgram::vertexShaderCode() const
{
    Q_D(const QShaderProgram);
    return d->m_vertexShaderCode;
}

/*!
    \qmlproperty string ShaderProgram::tessellationControlShaderCode

    Holds the tesselation control shader code used by this shader program.
*/
/*!
    \property QShaderProgram::tessellationControlShaderCode

    Holds the tesselation control shader code used by this shader program.
*/
void QShaderProgram::setTessellationControlShaderCode(const QByteArray &tessellationControlShaderCode)
{
    Q_D(QShaderProgram);
    if (tessellationControlShaderCode != d->m_tessControlShaderCode) {
        d->m_tessControlShaderCode = tessellationControlShaderCode;
        emit tessellationControlShaderCodeChanged(tessellationControlShaderCode);
    }
}

QByteArray QShaderProgram::tessellationControlShaderCode() const
{
    Q_D(const QShaderProgram);
    return d->m_tessControlShaderCode;
}

/*!
    \qmlproperty string ShaderProgram::tessellationEvaluationShaderCode

    Holds the tesselation evaluation shader code used by this shader program.
*/
/*!
    \property QShaderProgram::tessellationEvaluationShaderCode

    Holds the tesselation evaluation shader code used by this shader program.
*/
void QShaderProgram::setTessellationEvaluationShaderCode(const QByteArray &tessellationEvaluationShaderCode)
{
    Q_D(QShaderProgram);
    if (tessellationEvaluationShaderCode != d->m_tessEvalShaderCode) {
        d->m_tessEvalShaderCode = tessellationEvaluationShaderCode;
        emit tessellationEvaluationShaderCodeChanged(tessellationEvaluationShaderCode);
    }
}

QByteArray QShaderProgram::tessellationEvaluationShaderCode() const
{
    Q_D(const QShaderProgram);
    return d->m_tessEvalShaderCode;
}

/*!
    \qmlproperty string ShaderProgram::geometryShaderCode

    Holds the geometry shader code used by this shader program.
*/
/*!
    \property QShaderProgram::geometryShaderCode

    Holds the geometry shader code used by this shader program.
*/
void QShaderProgram::setGeometryShaderCode(const QByteArray &geometryShaderCode)
{
    Q_D(QShaderProgram);
    if (geometryShaderCode != d->m_geometryShaderCode) {
        d->m_geometryShaderCode = geometryShaderCode;
        emit geometryShaderCodeChanged(geometryShaderCode);
    }
}

QByteArray QShaderProgram::geometryShaderCode() const
{
    Q_D(const QShaderProgram);
    return d->m_geometryShaderCode;
}

/*!
    \qmlproperty string ShaderProgram::fragmentShaderCode

    Holds the fragment shader code used by this shader program.
*/
/*!
    \property QShaderProgram::fragmentShaderCode

    Holds the fragment shader code used by this shader program.
*/
void QShaderProgram::setFragmentShaderCode(const QByteArray &fragmentShaderCode)
{
    Q_D(QShaderProgram);
    if (fragmentShaderCode != d->m_fragmentShaderCode) {
        d->m_fragmentShaderCode = fragmentShaderCode;
        emit fragmentShaderCodeChanged(fragmentShaderCode);
    }
}

QByteArray QShaderProgram::fragmentShaderCode() const
{
    Q_D(const QShaderProgram);
    return d->m_fragmentShaderCode;
}

/*!
    \qmlproperty string ShaderProgram::computeShaderCode

    Holds the compute shader code used by this shader program.
*/
/*!
    \property QShaderProgram::computeShaderCode

    Holds the compute shader code used by this shader program.
*/
void QShaderProgram::setComputeShaderCode(const QByteArray &computeShaderCode)
{
    Q_D(QShaderProgram);
    if (computeShaderCode != d->m_computeShaderCode) {
        d->m_computeShaderCode = computeShaderCode;
        emit computeShaderCodeChanged(computeShaderCode);
    }
}

QByteArray QShaderProgram::computeShaderCode() const
{
    Q_D(const QShaderProgram);
    return d->m_computeShaderCode;
}


/*!
    Sets the shader code for \a type of shader to the \a shaderCode.
*/
void QShaderProgram::setShaderCode(ShaderType type, const QByteArray &shaderCode)
{
    switch (type) {
    case Vertex:
        setVertexShaderCode(shaderCode);
        break;
    case TessellationControl:
        setTessellationControlShaderCode(shaderCode);
        break;
    case TessellationEvaluation:
        setTessellationEvaluationShaderCode(shaderCode);
        break;
    case Geometry:
        setGeometryShaderCode(shaderCode);
        break;
    case Fragment:
        setFragmentShaderCode(shaderCode);
        break;
    case Compute:
        setComputeShaderCode(shaderCode);
        break;
    default:
        Q_UNREACHABLE();
    }
}

/*!
    Returns the shader code for \a type.
*/
QByteArray QShaderProgram::shaderCode(ShaderType type) const
{
    Q_D(const QShaderProgram);
    switch (type) {
    case Vertex:
        return d->m_vertexShaderCode;
    case TessellationControl:
        return d->m_tessControlShaderCode;
    case TessellationEvaluation:
        return d->m_tessEvalShaderCode;
    case Geometry:
        return d->m_geometryShaderCode;
    case Fragment:
        return d->m_fragmentShaderCode;
    case Compute:
        return d->m_computeShaderCode;
    default:
        Q_UNREACHABLE();
    }
}

/*!
    \qmlproperty string ShaderProgram::log

    Holds the log of the current shader program. This is useful to diagnose a
    compilation failure of the shader program.
*/
/*!
    \property QShaderProgram::log

    Holds the log of the current shader program. This is useful to diagnose a
    compilation failure of the shader program.
*/
QString QShaderProgram::log() const
{
    Q_D(const QShaderProgram);
    return d->m_log;
}

/*!
    \qmlproperty string ShaderProgram::status

    Holds the status of the current shader program.
*/
/*!
    \property QShaderProgram::status

    Holds the status of the current shader program.
*/
/*!
    Returns the status of the current shader program.
*/
QShaderProgram::Status QShaderProgram::status() const
{
    Q_D(const QShaderProgram);
    return d->m_status;
}

QByteArray QShaderProgramPrivate::deincludify(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not read shader source file:" << f.fileName();
        return QByteArray();
    }

    QByteArray contents = f.readAll();
    return deincludify(contents, filePath);
}

QByteArray QShaderProgramPrivate::deincludify(const QByteArray &contents, const QString &filePath)
{
    QByteArrayList lines = contents.split('\n');
    const QByteArray includeDirective = QByteArrayLiteral("#pragma include");
    for (int i = 0; i < lines.count(); ++i) {
        const auto line = lines[i].simplified();
        if (line.startsWith(includeDirective)) {
            const QString includePartialPath = QString::fromUtf8(line.mid(includeDirective.count() + 1));

            QString includePath = QFileInfo(includePartialPath).isAbsolute() ? includePartialPath
                                : QFileInfo(filePath).absolutePath() + QLatin1Char('/') + includePartialPath;
            if (qEnvironmentVariableIsSet("QT3D_GLSL100_WORKAROUND")) {
                QString candidate = includePath + QLatin1String("100");
                if (QFile::exists(candidate))
                    includePath = candidate;
            }
            lines.removeAt(i);
            QByteArray includedContents = deincludify(includePath);
            lines.insert(i, includedContents);
            QString lineDirective = QString(QStringLiteral("#line %1")).arg(i + 2);
            lines.insert(i + 1, lineDirective.toUtf8());
        }
    }

    return lines.join('\n');
}

/*!
    \qmlmethod string ShaderProgram::loadSource(url sourceUrl)

    Returns the shader code loaded from \a sourceUrl.
*/
/*!
    Returns the shader code loaded from \a sourceUrl.
*/
QByteArray QShaderProgram::loadSource(const QUrl &sourceUrl)
{
    // TO DO: Handle remote path
    return QShaderProgramPrivate::deincludify(Qt3DRender::QUrlHelper::urlToLocalFileOrQrc(sourceUrl));
}

Qt3DCore::QNodeCreatedChangeBasePtr QShaderProgram::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QShaderProgramData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QShaderProgram);
    data.vertexShaderCode = d->m_vertexShaderCode;
    data.tessellationControlShaderCode = d->m_tessControlShaderCode;
    data.tessellationEvaluationShaderCode = d->m_tessEvalShaderCode;
    data.geometryShaderCode = d->m_geometryShaderCode;
    data.fragmentShaderCode = d->m_fragmentShaderCode;
    data.computeShaderCode = d->m_computeShaderCode;
    return creationChange;
}

} // of namespace Qt3DRender

QT_END_NAMESPACE
