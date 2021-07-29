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

#include "qmetalroughmaterial.h"
#include "qmetalroughmaterial_p.h"
#include <Qt3DRender/qfilterkey.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qshaderprogram.h>
#include <Qt3DRender/qshaderprogrambuilder.h>
#include <Qt3DRender/qparameter.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qgraphicsapifilter.h>
#include <QUrl>
#include <QVector3D>
#include <QVector4D>

QT_BEGIN_NAMESPACE

using namespace Qt3DRender;

namespace Qt3DExtras {

QMetalRoughMaterialPrivate::QMetalRoughMaterialPrivate()
    : QMaterialPrivate()
    , m_baseColorParameter(new QParameter(QStringLiteral("baseColor"), QColor("grey")))
    , m_metalnessParameter(new QParameter(QStringLiteral("metalness"), 0.0f))
    , m_roughnessParameter(new QParameter(QStringLiteral("roughness"), 0.0f))
    , m_baseColorMapParameter(new QParameter(QStringLiteral("baseColorMap"), QVariant()))
    , m_metalnessMapParameter(new QParameter(QStringLiteral("metalnessMap"), QVariant()))
    , m_roughnessMapParameter(new QParameter(QStringLiteral("roughnessMap"), QVariant()))
    , m_ambientOcclusionMapParameter(new QParameter(QStringLiteral("ambientOcclusionMap"), QVariant()))
    , m_normalMapParameter(new QParameter(QStringLiteral("normalMap"), QVariant()))
    , m_textureScaleParameter(new QParameter(QStringLiteral("texCoordScale"), 1.0f))
    , m_metalRoughEffect(new QEffect())
    , m_metalRoughGL3Technique(new QTechnique())
    , m_metalRoughGL3RenderPass(new QRenderPass())
    , m_metalRoughGL3Shader(new QShaderProgram())
    , m_metalRoughGL3ShaderBuilder(new QShaderProgramBuilder())
    , m_metalRoughES3Technique(new QTechnique())
    , m_metalRoughES3RenderPass(new QRenderPass())
    , m_metalRoughES3Shader(new QShaderProgram())
    , m_metalRoughES3ShaderBuilder(new QShaderProgramBuilder())
    , m_filterKey(new QFilterKey)
{
}

void QMetalRoughMaterialPrivate::init()
{
    Q_Q(QMetalRoughMaterial);

    QObject::connect(m_baseColorParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QMetalRoughMaterial::baseColorChanged);
    QObject::connect(m_metalnessParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QMetalRoughMaterial::metalnessChanged);
    QObject::connect(m_roughnessParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QMetalRoughMaterial::roughnessChanged);
    QObject::connect(m_ambientOcclusionMapParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QMetalRoughMaterial::roughnessChanged);
    QObject::connect(m_normalMapParameter, &Qt3DRender::QParameter::valueChanged,
                     q, &QMetalRoughMaterial::normalChanged);
    connect(m_textureScaleParameter, &Qt3DRender::QParameter::valueChanged,
            this, &QMetalRoughMaterialPrivate::handleTextureScaleChanged);

    m_metalRoughGL3Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/shaders/gl3/default.vert"))));
    m_metalRoughGL3ShaderBuilder->setParent(q);
    m_metalRoughGL3ShaderBuilder->setShaderProgram(m_metalRoughGL3Shader);
    m_metalRoughGL3ShaderBuilder->setFragmentShaderGraph(QUrl(QStringLiteral("qrc:/shaders/graphs/metalrough.frag.json")));
    m_metalRoughGL3ShaderBuilder->setEnabledLayers({QStringLiteral("baseColor"),
                                                    QStringLiteral("metalness"),
                                                    QStringLiteral("roughness"),
                                                    QStringLiteral("ambientOcclusion"),
                                                    QStringLiteral("normal")});

    m_metalRoughES3Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/shaders/es3/default.vert"))));
    m_metalRoughES3ShaderBuilder->setParent(q);
    m_metalRoughES3ShaderBuilder->setShaderProgram(m_metalRoughES3Shader);
    m_metalRoughES3ShaderBuilder->setFragmentShaderGraph(QUrl(QStringLiteral("qrc:/shaders/graphs/metalrough.frag.json")));
    m_metalRoughES3ShaderBuilder->setEnabledLayers({QStringLiteral("baseColor"),
                                                    QStringLiteral("metalness"),
                                                    QStringLiteral("roughness"),
                                                    QStringLiteral("ambientOcclusion"),
                                                    QStringLiteral("normal")});

    m_metalRoughGL3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    m_metalRoughGL3Technique->graphicsApiFilter()->setMajorVersion(3);
    m_metalRoughGL3Technique->graphicsApiFilter()->setMinorVersion(1);
    m_metalRoughGL3Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);

    m_metalRoughES3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGLES);
    m_metalRoughES3Technique->graphicsApiFilter()->setMajorVersion(3);
    m_metalRoughES3Technique->graphicsApiFilter()->setMinorVersion(0);

    m_filterKey->setParent(q);
    m_filterKey->setName(QStringLiteral("renderingStyle"));
    m_filterKey->setValue(QStringLiteral("forward"));

    m_metalRoughGL3Technique->addFilterKey(m_filterKey);
    m_metalRoughGL3RenderPass->setShaderProgram(m_metalRoughGL3Shader);
    m_metalRoughGL3Technique->addRenderPass(m_metalRoughGL3RenderPass);
    m_metalRoughEffect->addTechnique(m_metalRoughGL3Technique);

    m_metalRoughES3Technique->addFilterKey(m_filterKey);
    m_metalRoughES3RenderPass->setShaderProgram(m_metalRoughES3Shader);
    m_metalRoughES3Technique->addRenderPass(m_metalRoughES3RenderPass);
    m_metalRoughEffect->addTechnique(m_metalRoughES3Technique);

    m_metalRoughEffect->addParameter(m_baseColorParameter);
    m_metalRoughEffect->addParameter(m_metalnessParameter);
    m_metalRoughEffect->addParameter(m_roughnessParameter);
    m_metalRoughEffect->addParameter(m_textureScaleParameter);

    q->setEffect(m_metalRoughEffect);
}

void QMetalRoughMaterialPrivate::handleTextureScaleChanged(const QVariant &var)
{
    Q_Q(QMetalRoughMaterial);
    emit q->textureScaleChanged(var.toFloat());
}

/*!
    \class Qt3DExtras::QMetalRoughMaterial
    \ingroup qt3d-extras-materials
    \brief The QMetalRoughMaterial provides a default implementation of PBR
    lighting.
    \inmodule Qt3DExtras
    \since 5.9
    \inherits Qt3DRender::QMaterial

    This material uses an effect with a single render pass approach and performs per fragment
    lighting. Techniques are provided for OpenGL 3 and OpenGL ES 3.
*/

/*!
   \qmltype MetalRoughMaterial
   \instantiates Qt3DExtras::QMetalRoughMaterial
   \inqmlmodule Qt3D.Extras

   \brief This material uses an effect with a single render pass approach and
          performs per fragment lighting. Techniques are provided for OpenGL 3
          and OpenGL ES 3.
*/

/*!
    Constructs a new QMetalRoughMaterial instance with parent object \a parent.
*/
QMetalRoughMaterial::QMetalRoughMaterial(QNode *parent)
    : QMaterial(*new QMetalRoughMaterialPrivate, parent)
{
    Q_D(QMetalRoughMaterial);
    d->init();
}

/*! \internal */
QMetalRoughMaterial::QMetalRoughMaterial(QMetalRoughMaterialPrivate &dd, QNode *parent)
    : QMaterial(dd, parent)
{
    Q_D(QMetalRoughMaterial);
    d->init();
}

/*!
    Destroys the QMetalRoughMaterial instance.
*/
QMetalRoughMaterial::~QMetalRoughMaterial()
{
}

/*!
    \property QMetalRoughMaterial::baseColor

    Holds the current base color of the material. This can be either a plain
    color value or a texture. By default the value of this property is "grey".
*/
/*!
    \qmlproperty variant Qt3D.Extras::MetalRoughMaterial::baseColor

    Holds the current base color of the material. This can be either a plain
    color value or a texture. By default the value of this property is "grey".
*/
QVariant QMetalRoughMaterial::baseColor() const
{
    Q_D(const QMetalRoughMaterial);
    return d->m_baseColorParameter->value();
}

/*!
    \property QMetalRoughMaterial::metalness

    Holds the current metalness level of the material, as a value between
    0 (purely dielectric, the default) and 1 (purely metallic). This can be
    either a plain uniform value or a texture. By default the value of this
    property is 0.
*/
/*!
    \qmlproperty variant Qt3D.Extras::MetalRoughMaterial::metalness

    Holds the current metalness level of the material, as a value between
    0 (purely dielectric, the default) and 1 (purely metallic). This can be
    either a plain uniform value or a texture. By default the value of this
    property is 0.
*/
QVariant QMetalRoughMaterial::metalness() const
{
    Q_D(const QMetalRoughMaterial);
    return d->m_metalnessParameter->value();
}

/*!
    \property QMetalRoughMaterial::roughness

    Holds the current roughness level of the material. This can be either a
    plain uniform value or a texture. By default the value of this property is
    0.
*/
/*!
    \qmlproperty variant Qt3D.Extras::MetalRoughMaterial::roughness

    Holds the current roughness level of the material. This can be either a
    plain uniform value or a texture. By default the value of this property is
    0.
*/
QVariant QMetalRoughMaterial::roughness() const
{
    Q_D(const QMetalRoughMaterial);
    return d->m_roughnessParameter->value();
}

/*!
    \property QMetalRoughMaterial::ambientOcclusion

    Holds the current ambient occlusion map texture of the material. This can
    only be a texture, otherwise it is ignored. By default this map is not set.
*/
/*!
    \qmlproperty Texture Qt3D.Extras::MetalRoughMaterial::ambientOcclusion

    Holds the current ambient occlusion map texture of the material. This can
    only be a texture, otherwise it is ignored. By default this map is not set.
*/
QVariant QMetalRoughMaterial::ambientOcclusion() const
{
    Q_D(const QMetalRoughMaterial);
    return d->m_ambientOcclusionMapParameter->value();
}

/*!
    \property QMetalRoughMaterial::normal

    Holds the current normal map texture of the material. This can only be a
    texture, otherwise it is ignored. By default this map is not set.
*/
/*!
    \qmlproperty Texture Qt3D.Extras::MetalRoughMaterial::normal

    Holds the current normal map texture of the material. This can only be a
    texture, otherwise it is ignored. By default this map is not set.
*/
QVariant QMetalRoughMaterial::normal() const
{
    Q_D(const QMetalRoughMaterial);
    return d->m_normalMapParameter->value();
}

/*!
    \property QMetalRoughMaterial::textureScale

    Holds the current texture scale. It is applied as a multiplier to texture
    coordinates at render time. Defaults to 1.0.

    When used in conjunction with QTextureWrapMode::Repeat, textureScale provides a simple
    way to tile a texture across a surface. For example, a texture scale of \c 4.0
    would result in 16 (4x4) tiles.
*/
/*!
    \qmlproperty real Qt3D.Extras::MetalRoughMaterial::textureScale

    Holds the current texture scale. It is applied as a multiplier to texture
    coordinates at render time. Defaults to 1.0.

    When used in conjunction with WrapMode.Repeat, textureScale provides a simple
    way to tile a texture across a surface. For example, a texture scale of \c 4.0
    would result in 16 (4x4) tiles.
*/
float QMetalRoughMaterial::textureScale() const
{
    Q_D(const QMetalRoughMaterial);
    return d->m_textureScaleParameter->value().toFloat();
}

void QMetalRoughMaterial::setBaseColor(const QVariant &baseColor)
{
    Q_D(QMetalRoughMaterial);
    d->m_baseColorParameter->setValue(baseColor);
    d->m_baseColorMapParameter->setValue(baseColor);

    auto layers = d->m_metalRoughGL3ShaderBuilder->enabledLayers();
    if (baseColor.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("baseColor"));
        layers.append(QStringLiteral("baseColorMap"));
        d->m_metalRoughEffect->addParameter(d->m_baseColorMapParameter);
        d->m_metalRoughEffect->removeParameter(d->m_baseColorParameter);
    } else {
        layers.removeAll(QStringLiteral("baseColorMap"));
        layers.append(QStringLiteral("baseColor"));
        d->m_metalRoughEffect->removeParameter(d->m_baseColorMapParameter);
        d->m_metalRoughEffect->addParameter(d->m_baseColorParameter);
    }
    d->m_metalRoughGL3ShaderBuilder->setEnabledLayers(layers);
    d->m_metalRoughES3ShaderBuilder->setEnabledLayers(layers);
}

void QMetalRoughMaterial::setMetalness(const QVariant &metalness)
{
    Q_D(QMetalRoughMaterial);
    d->m_metalnessParameter->setValue(metalness);
    d->m_metalnessMapParameter->setValue(metalness);

    auto layers = d->m_metalRoughGL3ShaderBuilder->enabledLayers();
    if (metalness.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("metalness"));
        layers.append(QStringLiteral("metalnessMap"));
        d->m_metalRoughEffect->addParameter(d->m_metalnessMapParameter);
        d->m_metalRoughEffect->removeParameter(d->m_metalnessParameter);
    } else {
        layers.removeAll(QStringLiteral("metalnessMap"));
        layers.append(QStringLiteral("metalness"));
        d->m_metalRoughEffect->removeParameter(d->m_metalnessMapParameter);
        d->m_metalRoughEffect->addParameter(d->m_metalnessParameter);
    }
    d->m_metalRoughGL3ShaderBuilder->setEnabledLayers(layers);
    d->m_metalRoughES3ShaderBuilder->setEnabledLayers(layers);
}

void QMetalRoughMaterial::setRoughness(const QVariant &roughness)
{
    Q_D(QMetalRoughMaterial);
    d->m_roughnessParameter->setValue(roughness);
    d->m_roughnessMapParameter->setValue(roughness);

    auto layers = d->m_metalRoughGL3ShaderBuilder->enabledLayers();
    if (roughness.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("roughness"));
        layers.append(QStringLiteral("roughnessMap"));
        d->m_metalRoughEffect->addParameter(d->m_roughnessMapParameter);
        d->m_metalRoughEffect->removeParameter(d->m_roughnessParameter);
    } else {
        layers.removeAll(QStringLiteral("roughnessMap"));
        layers.append(QStringLiteral("roughness"));
        d->m_metalRoughEffect->removeParameter(d->m_roughnessMapParameter);
        d->m_metalRoughEffect->addParameter(d->m_roughnessParameter);
    }
    d->m_metalRoughGL3ShaderBuilder->setEnabledLayers(layers);
    d->m_metalRoughES3ShaderBuilder->setEnabledLayers(layers);
}

void QMetalRoughMaterial::setAmbientOcclusion(const QVariant &ambientOcclusion)
{
    Q_D(QMetalRoughMaterial);
    d->m_ambientOcclusionMapParameter->setValue(ambientOcclusion);

    auto layers = d->m_metalRoughGL3ShaderBuilder->enabledLayers();
    if (ambientOcclusion.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("ambientOcclusion"));
        layers.append(QStringLiteral("ambientOcclusionMap"));
        d->m_metalRoughEffect->addParameter(d->m_ambientOcclusionMapParameter);
    } else {
        layers.removeAll(QStringLiteral("ambientOcclusionMap"));
        layers.append(QStringLiteral("ambientOcclusion"));
        d->m_metalRoughEffect->removeParameter(d->m_ambientOcclusionMapParameter);
    }
    d->m_metalRoughGL3ShaderBuilder->setEnabledLayers(layers);
    d->m_metalRoughES3ShaderBuilder->setEnabledLayers(layers);
}

void QMetalRoughMaterial::setNormal(const QVariant &normal)
{
    Q_D(QMetalRoughMaterial);
    d->m_normalMapParameter->setValue(normal);

    auto layers = d->m_metalRoughGL3ShaderBuilder->enabledLayers();
    if (normal.value<QAbstractTexture *>()) {
        layers.removeAll(QStringLiteral("normal"));
        layers.append(QStringLiteral("normalMap"));
        d->m_metalRoughEffect->addParameter(d->m_normalMapParameter);
    } else {
        layers.removeAll(QStringLiteral("normalMap"));
        layers.append(QStringLiteral("normal"));
        d->m_metalRoughEffect->removeParameter(d->m_normalMapParameter);
    }
    d->m_metalRoughGL3ShaderBuilder->setEnabledLayers(layers);
    d->m_metalRoughES3ShaderBuilder->setEnabledLayers(layers);
}

void QMetalRoughMaterial::setTextureScale(float textureScale)
{
    Q_D(QMetalRoughMaterial);
    d->m_textureScaleParameter->setValue(textureScale);
}

} // namespace Qt3DExtras

QT_END_NAMESPACE
