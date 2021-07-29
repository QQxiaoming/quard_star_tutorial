/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include "qdiffusespecularmaterial.h"
#include "qdiffusespecularmaterial_p.h"

#include <Qt3DRender/qfilterkey.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qshaderprogram.h>
#include <Qt3DRender/qshaderprogrambuilder.h>
#include <Qt3DRender/qparameter.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qgraphicsapifilter.h>
#include <Qt3DRender/qblendequation.h>
#include <Qt3DRender/qblendequationarguments.h>
#include <Qt3DRender/qnodepthmask.h>
#include <QtCore/QUrl>
#include <QtGui/QVector3D>
#include <QtGui/QVector4D>


QT_BEGIN_NAMESPACE

using namespace Qt3DRender;

namespace Qt3DExtras {

QDiffuseSpecularMaterialPrivate::QDiffuseSpecularMaterialPrivate()
    : QMaterialPrivate()
    , m_effect(new QEffect())
    , m_ambientParameter(new QParameter(QStringLiteral("ka"), QColor::fromRgbF(0.05f, 0.05f, 0.05f, 1.0f)))
    , m_diffuseParameter(new QParameter(QStringLiteral("kd"), QColor::fromRgbF(0.7f, 0.7f, 0.7f, 1.0f)))
    , m_specularParameter(new QParameter(QStringLiteral("ks"), QColor::fromRgbF(0.01f, 0.01f, 0.01f, 1.0f)))
    , m_diffuseTextureParameter(new QParameter(QStringLiteral("diffuseTexture"), QVariant()))
    , m_specularTextureParameter(new QParameter(QStringLiteral("specularTexture"), QVariant()))
    , m_shininessParameter(new QParameter(QStringLiteral("shininess"), 150.0f))
    , m_normalTextureParameter(new QParameter(QStringLiteral("normalTexture"), QVariant()))
    , m_textureScaleParameter(new QParameter(QStringLiteral("texCoordScale"), 1.0f))
    , m_gl3Technique(new QTechnique())
    , m_gl2Technique(new QTechnique())
    , m_es2Technique(new QTechnique())
    , m_gl3RenderPass(new QRenderPass())
    , m_gl2RenderPass(new QRenderPass())
    , m_es2RenderPass(new QRenderPass())
    , m_gl3Shader(new QShaderProgram())
    , m_gl3ShaderBuilder(new QShaderProgramBuilder())
    , m_gl2es2Shader(new QShaderProgram())
    , m_gl2es2ShaderBuilder(new QShaderProgramBuilder())
    , m_noDepthMask(new QNoDepthMask())
    , m_blendState(new QBlendEquationArguments())
    , m_blendEquation(new QBlendEquation())
    , m_filterKey(new QFilterKey)
{
}

void QDiffuseSpecularMaterialPrivate::init()
{
    Q_Q(QDiffuseSpecularMaterial);

    connect(m_ambientParameter, &Qt3DRender::QParameter::valueChanged,
            this, &QDiffuseSpecularMaterialPrivate::handleAmbientChanged);
    QObject::connect(m_diffuseParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QDiffuseSpecularMaterial::diffuseChanged);
    QObject::connect(m_specularParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QDiffuseSpecularMaterial::specularChanged);
    connect(m_shininessParameter, &Qt3DRender::QParameter::valueChanged,
            this, &QDiffuseSpecularMaterialPrivate::handleShininessChanged);
    QObject::connect(m_normalTextureParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QDiffuseSpecularMaterial::normalChanged);
    connect(m_textureScaleParameter, &Qt3DRender::QParameter::valueChanged,
            this, &QDiffuseSpecularMaterialPrivate::handleTextureScaleChanged);
    QObject::connect(m_noDepthMask, &QNoDepthMask::enabledChanged,
                     q, &QDiffuseSpecularMaterial::alphaBlendingEnabledChanged);

    m_gl3Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/shaders/gl3/default.vert"))));
    m_gl3ShaderBuilder->setParent(q);
    m_gl3ShaderBuilder->setShaderProgram(m_gl3Shader);
    m_gl3ShaderBuilder->setFragmentShaderGraph(QUrl(QStringLiteral("qrc:/shaders/graphs/phong.frag.json")));
    m_gl3ShaderBuilder->setEnabledLayers({QStringLiteral("diffuse"),
                                               QStringLiteral("specular"),
                                               QStringLiteral("normal")});

    m_gl2es2Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/shaders/es2/default.vert"))));
    m_gl2es2ShaderBuilder->setParent(q);
    m_gl2es2ShaderBuilder->setShaderProgram(m_gl2es2Shader);
    m_gl2es2ShaderBuilder->setFragmentShaderGraph(QUrl(QStringLiteral("qrc:/shaders/graphs/phong.frag.json")));
    m_gl2es2ShaderBuilder->setEnabledLayers({QStringLiteral("diffuse"),
                                                  QStringLiteral("specular"),
                                                  QStringLiteral("normal")});


    m_gl3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    m_gl3Technique->graphicsApiFilter()->setMajorVersion(3);
    m_gl3Technique->graphicsApiFilter()->setMinorVersion(1);
    m_gl3Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);

    m_gl2Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    m_gl2Technique->graphicsApiFilter()->setMajorVersion(2);
    m_gl2Technique->graphicsApiFilter()->setMinorVersion(0);
    m_gl2Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::NoProfile);

    m_es2Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGLES);
    m_es2Technique->graphicsApiFilter()->setMajorVersion(2);
    m_es2Technique->graphicsApiFilter()->setMinorVersion(0);
    m_es2Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::NoProfile);

    m_noDepthMask->setEnabled(false);
    m_blendState->setEnabled(false);
    m_blendState->setSourceRgb(QBlendEquationArguments::SourceAlpha);
    m_blendState->setDestinationRgb(QBlendEquationArguments::OneMinusSourceAlpha);
    m_blendEquation->setEnabled(false);
    m_blendEquation->setBlendFunction(QBlendEquation::Add);

    m_gl3RenderPass->setShaderProgram(m_gl3Shader);
    m_gl2RenderPass->setShaderProgram(m_gl2es2Shader);
    m_es2RenderPass->setShaderProgram(m_gl2es2Shader);

    m_gl3RenderPass->addRenderState(m_noDepthMask);
    m_gl3RenderPass->addRenderState(m_blendState);
    m_gl3RenderPass->addRenderState(m_blendEquation);

    m_gl2RenderPass->addRenderState(m_noDepthMask);
    m_gl2RenderPass->addRenderState(m_blendState);
    m_gl2RenderPass->addRenderState(m_blendEquation);

    m_es2RenderPass->addRenderState(m_noDepthMask);
    m_es2RenderPass->addRenderState(m_blendState);
    m_es2RenderPass->addRenderState(m_blendEquation);

    m_gl3Technique->addRenderPass(m_gl3RenderPass);
    m_gl2Technique->addRenderPass(m_gl2RenderPass);
    m_es2Technique->addRenderPass(m_es2RenderPass);

    m_filterKey->setParent(q);
    m_filterKey->setName(QStringLiteral("renderingStyle"));
    m_filterKey->setValue(QStringLiteral("forward"));

    m_gl3Technique->addFilterKey(m_filterKey);
    m_gl2Technique->addFilterKey(m_filterKey);
    m_es2Technique->addFilterKey(m_filterKey);

    m_effect->addTechnique(m_gl3Technique);
    m_effect->addTechnique(m_gl2Technique);
    m_effect->addTechnique(m_es2Technique);

    m_effect->addParameter(m_ambientParameter);
    m_effect->addParameter(m_diffuseParameter);
    m_effect->addParameter(m_specularParameter);
    m_effect->addParameter(m_shininessParameter);
    m_effect->addParameter(m_textureScaleParameter);

    q->setEffect(m_effect);
}

void QDiffuseSpecularMaterialPrivate::handleAmbientChanged(const QVariant &var)
{
    Q_Q(QDiffuseSpecularMaterial);
    emit q->ambientChanged(var.value<QColor>());
}

void QDiffuseSpecularMaterialPrivate::handleShininessChanged(const QVariant &var)
{
    Q_Q(QDiffuseSpecularMaterial);
    emit q->shininessChanged(var.toFloat());
}

void QDiffuseSpecularMaterialPrivate::handleTextureScaleChanged(const QVariant &var)
{
    Q_Q(QDiffuseSpecularMaterial);
    emit q->textureScaleChanged(var.toFloat());
}

/*!
    \class Qt3DExtras::QDiffuseSpecularMaterial
    \ingroup qt3d-extras-materials
    \brief The QDiffuseSpecularMaterial class provides a default implementation
    of the phong lighting effect.
    \inmodule Qt3DExtras
    \since 5.10
    \inherits Qt3DRender::QMaterial

    The phong lighting effect is based on the combination of 3 lighting
    components ambient, diffuse and specular. The relative strengths of these
    components are controlled by means of their reflectivity coefficients which
    are modelled as RGB triplets:

    \list
    \li Ambient is the color that is emitted by an object without any other
        light source.
    \li Diffuse is the color that is emitted for rought surface reflections
        with the lights.
    \li Specular is the color emitted for shiny surface reflections with the
        lights.
    \li The shininess of a surface is controlled by a float property.
    \endlist

    This material uses an effect with a single render pass approach and
    performs per fragment lighting. Techniques are provided for OpenGL 2,
    OpenGL 3 or above as well as OpenGL ES 2.
*/
/*!
    \qmltype DiffuseSpecularMaterial
    \brief The DiffuseSpecularMaterial class provides a default implementation
    of the phong lighting effect.
    \since 5.10
    \inqmlmodule Qt3D.Extras
    \instantiates Qt3DExtras::QDiffuseSpecularMaterial

    The phong lighting effect is based on the combination of 3 lighting
    components ambient, diffuse and specular. The relative strengths of these
    components are controlled by means of their reflectivity coefficients which
    are modelled as RGB triplets:

    \list
    \li Ambient is the color that is emitted by an object without any other
        light source.
    \li Diffuse is the color that is emitted for rough surface reflections
        with the lights.
    \li Specular is the color emitted for shiny surface reflections with the
        lights.
    \li The shininess of a surface is controlled by a float property.
    \endlist

    This material uses an effect with a single render pass approach and
    performs per fragment lighting. Techniques are provided for OpenGL 2,
    OpenGL 3 or above as well as OpenGL ES 2.
 */

/*!
    Constructs a new QDiffuseSpecularMaterial instance with parent object \a parent.
*/
QDiffuseSpecularMaterial::QDiffuseSpecularMaterial(QNode *parent)
    : QMaterial(*new QDiffuseSpecularMaterialPrivate, parent)
{
    Q_D(QDiffuseSpecularMaterial);
    d->init();
}

/*!
   Destroys the QDiffuseSpecularMaterial.
*/
QDiffuseSpecularMaterial::~QDiffuseSpecularMaterial()
{
}

/*!
    \property QDiffuseSpecularMaterial::ambient

    Holds the ambient color that is emitted by an object without any other
    light source.
*/
/*!
    \qmlproperty color DiffuseSpecularMaterial::ambient

    Holds the ambient color that is emitted by an object without any other
    light source.
*/
QColor QDiffuseSpecularMaterial::ambient() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_ambientParameter->value().value<QColor>();
}

/*!
    \property QDiffuseSpecularMaterial::diffuse

    Holds the diffuse color of the material that is emitted for rough surface
    reflections with the lights. This can be either a plain color value or a
    texture.
*/
/*!
    \qmlproperty var DiffuseSpecularMaterial::diffuse

    Holds the diffuse color of the material that is emitted for rough surface
    reflections with the lights. This can be either a plain color value or a
    texture.
*/
QVariant QDiffuseSpecularMaterial::diffuse() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_diffuseParameter->value();
}

/*!
    \property QDiffuseSpecularMaterial::specular

    Holds the specular color of the material that is emitted for shiny surface
    reflections with the lights. This can be either a plain color value or a
    texture.
*/
/*!
    \qmlproperty var DiffuseSpecularMaterial::specular

    Holds the specular color of the material that is emitted for shiny surface
    reflections with the lights. This can be either a plain color value or a
    texture.
*/
QVariant QDiffuseSpecularMaterial::specular() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_specularParameter->value();
}

/*!
    \property QDiffuseSpecularMaterial::shininess

    Holds the shininess exponent. Higher values of shininess result in
    a smaller and brighter highlight.

    Defaults to 150.0.
*/
/*!
    \qmlproperty real DiffuseSpecularMaterial::shininess

    Holds the shininess exponent. Higher values of shininess result in
    a smaller and brighter highlight.

    Defaults to 150.0.
*/
float QDiffuseSpecularMaterial::shininess() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_shininessParameter->value().toFloat();
}

/*!
    \property QDiffuseSpecularMaterial::normal

    Holds the current normal map texture of the material. This can only be a
    texture, otherwise it is ignored. By default this map is not set.
*/
/*!
    \qmlproperty var DiffuseSpecularMaterial::normal

    Holds the current normal map texture of the material. This can only be a
    texture, otherwise it is ignored. By default this map is not set.
*/
QVariant QDiffuseSpecularMaterial::normal() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_normalTextureParameter->value();
}

/*!
    \property QDiffuseSpecularMaterial::textureScale

    Holds the current texture scale. It is applied as a multiplier to texture
    coordinates at render time. Defaults to 1.0.

    When used in conjunction with QTextureWrapMode::Repeat, textureScale provides a simple
    way to tile a texture across a surface. For example, a texture scale of \c 4.0
    would result in 16 (4x4) tiles.
*/
/*!
    \qmlproperty real DiffuseSpecularMaterial::textureScale

    Holds the current texture scale. It is applied as a multiplier to texture
    coordinates at render time. Defaults to 1.0.

    When used in conjunction with WrapMode.Repeat, textureScale provides a simple
    way to tile a texture across a surface. For example, a texture scale of \c 4.0
    would result in 16 (4x4) tiles.
*/
float QDiffuseSpecularMaterial::textureScale() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_textureScaleParameter->value().toFloat();
}

/*!
    \property QDiffuseSpecularMaterial::alphaBlending

    Indicates if the alpha information coming from the diffuse property will
    be taken into account during rendering. Defaults to false.
*/
/*!
    \qmlproperty bool DiffuseSpecularMaterial::alphaBlending

    Indicates if the alpha information coming from the diffuse property will
    be taken into account during rendering. Defaults to false.
*/
bool QDiffuseSpecularMaterial::isAlphaBlendingEnabled() const
{
    Q_D(const QDiffuseSpecularMaterial);
    return d->m_noDepthMask->isEnabled();
}

void QDiffuseSpecularMaterial::setAmbient(const QColor &ambient)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_ambientParameter->setValue(ambient);
}

void QDiffuseSpecularMaterial::setDiffuse(const QVariant &diffuse)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_diffuseParameter->setValue(diffuse);
    d->m_diffuseTextureParameter->setValue(diffuse);

    auto layers = d->m_gl3ShaderBuilder->enabledLayers();
    if (diffuse.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("diffuse"));
        layers.append(QStringLiteral("diffuseTexture"));
        d->m_effect->addParameter(d->m_diffuseTextureParameter);
        d->m_effect->removeParameter(d->m_diffuseParameter);
    } else {
        layers.removeAll(QStringLiteral("diffuseTexture"));
        layers.append(QStringLiteral("diffuse"));
        d->m_effect->removeParameter(d->m_diffuseTextureParameter);
        d->m_effect->addParameter(d->m_diffuseParameter);
    }
    d->m_gl3ShaderBuilder->setEnabledLayers(layers);
    d->m_gl2es2ShaderBuilder->setEnabledLayers(layers);
}

void QDiffuseSpecularMaterial::setSpecular(const QVariant &specular)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_specularParameter->setValue(specular);
    d->m_specularTextureParameter->setValue(specular);

    auto layers = d->m_gl3ShaderBuilder->enabledLayers();
    if (specular.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("specular"));
        layers.append(QStringLiteral("specularTexture"));
        d->m_effect->addParameter(d->m_specularTextureParameter);
        d->m_effect->removeParameter(d->m_specularParameter);
    } else {
        layers.removeAll(QStringLiteral("specularTexture"));
        layers.append(QStringLiteral("specular"));
        d->m_effect->removeParameter(d->m_specularTextureParameter);
        d->m_effect->addParameter(d->m_specularParameter);
    }
    d->m_gl3ShaderBuilder->setEnabledLayers(layers);
    d->m_gl2es2ShaderBuilder->setEnabledLayers(layers);
}

void QDiffuseSpecularMaterial::setShininess(float shininess)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_shininessParameter->setValue(shininess);
}

void QDiffuseSpecularMaterial::setNormal(const QVariant &normal)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_normalTextureParameter->setValue(normal);

    auto layers = d->m_gl3ShaderBuilder->enabledLayers();
    if (normal.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("normal"));
        layers.append(QStringLiteral("normalTexture"));
        d->m_effect->addParameter(d->m_normalTextureParameter);
    } else {
        layers.removeAll(QStringLiteral("normalTexture"));
        layers.append(QStringLiteral("normal"));
        d->m_effect->removeParameter(d->m_normalTextureParameter);
    }
    d->m_gl3ShaderBuilder->setEnabledLayers(layers);
}

void QDiffuseSpecularMaterial::setTextureScale(float textureScale)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_textureScaleParameter->setValue(textureScale);
}

void QDiffuseSpecularMaterial::setAlphaBlendingEnabled(bool enabled)
{
    Q_D(QDiffuseSpecularMaterial);
    d->m_noDepthMask->setEnabled(enabled);
    d->m_blendState->setEnabled(enabled);
    d->m_blendEquation->setEnabled(enabled);
}

} // namespace Qt3DExtras

QT_END_NAMESPACE
