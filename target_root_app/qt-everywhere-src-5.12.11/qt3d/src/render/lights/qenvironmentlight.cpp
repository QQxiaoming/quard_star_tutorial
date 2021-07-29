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

#include "qenvironmentlight.h"
#include "qenvironmentlight_p.h"
#include "qabstracttexture.h"
#include <QVector3D>

QT_BEGIN_NAMESPACE

namespace Qt3DRender
{

/*!
 * \qmltype EnvironmentLight
 * \inqmlmodule Qt3D.Render
 * \instantiates Qt3DRender::QEnvironmentLight
 * \brief Encapsulate an environment light object in a Qt 3D scene.
 * \since 5.9
 *
 * EnvironmentLight uses cubemaps to implement image-based lighting (IBL), a technique
 * often used in conjunction with physically-based rendering (PBR). The cubemaps are
 * typically expected be based on high dynamic range (HDR) images, with a suitable
 * OpenGL format (such as RGBA16F) that can handle the increased range of values.
 *
 * There are a variety of tools that can be used to produce the cubemaps needed by
 * EnvironmentLight. Some examples include
 *
 * \list
 * \li \l {https://github.com/dariomanesku/cmftStudio}{cmftStudio}
 * \li \l {https://github.com/derkreature/IBLBaker}{IBLBaker}
 * \li \l {https://www.knaldtech.com/lys/}{Lys}
 * \endlist
 *
 * \l {https://hdrihaven.com/hdris/}{HDRI Haven} provides many CC0-licensed HDR images
 * that can be used as source material for the above tools.
 */

QEnvironmentLightPrivate::QEnvironmentLightPrivate()
    : m_shaderData(new QShaderData)
    , m_irradiance(nullptr)
    , m_specular(nullptr)
{
}

QEnvironmentLightPrivate::~QEnvironmentLightPrivate()
{
}

void QEnvironmentLightPrivate::_q_updateEnvMapsSize()
{
    QVector3D irradianceSize;
    if (m_irradiance != nullptr)
        irradianceSize = QVector3D(m_irradiance->width(),
                                   m_irradiance->height(),
                                   m_irradiance->depth());
    m_shaderData->setProperty("irradianceSize", QVariant::fromValue(irradianceSize));

    QVector3D specularSize;
    if (m_specular != nullptr)
        specularSize = QVector3D(m_specular->width(),
                                 m_specular->height(),
                                 m_specular->depth());
    m_shaderData->setProperty("specularSize", QVariant::fromValue(specularSize));
}

Qt3DCore::QNodeCreatedChangeBasePtr QEnvironmentLight::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QEnvironmentLightData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QEnvironmentLight);
    data.shaderDataId = qIdForNode(d->m_shaderData);
    return creationChange;
}

/*!
    \class Qt3DRender::QEnvironmentLight
    \inmodule Qt3DRender
    \brief Encapsulate an environment light object in a Qt 3D scene.
    \since 5.9

    QEnvironmentLight uses cubemaps to implement image-based lighting (IBL), a technique
    often used in conjunction with physically-based rendering (PBR). The cubemaps are
    typically expected be based on high dynamic range (HDR) images, with a suitable
    OpenGL format (such as RGBA16F) that can handle the increased range of values.

    There are a variety of tools that can be used to produce the cubemaps needed by
    QEnvironmentLight. Some examples include

    \list
    \li \l {https://github.com/dariomanesku/cmftStudio}{cmftStudio}
    \li \l {https://github.com/derkreature/IBLBaker}{IBLBaker}
    \li \l {https://www.knaldtech.com/lys/}{Lys}
    \endlist

    \l {https://hdrihaven.com/hdris/}{HDRI Haven} provides many CC0-licensed HDR images
    that can be used as source material for the above tools.
*/

QEnvironmentLight::QEnvironmentLight(Qt3DCore::QNode *parent)
    : QComponent(*new QEnvironmentLightPrivate, parent)
{
    Q_D(QEnvironmentLight);
    d->m_shaderData->setParent(this);
}

/*! \internal */
QEnvironmentLight::QEnvironmentLight(QEnvironmentLightPrivate &dd, QNode *parent)
    : QComponent(dd, parent)
{
    Q_D(QEnvironmentLight);
    d->m_shaderData->setParent(this);
}

QEnvironmentLight::~QEnvironmentLight()
{
}

/*!
    \qmlproperty Texture EnvironmentLight::irradiance

    Holds the current environment irradiance map texture.

    By default, the environment irradiance texture is null.
*/

/*!
    \property QEnvironmentLight::irradiance

    Holds the current environment irradiance map texture.

    By default, the environment irradiance texture is null.
*/
QAbstractTexture *QEnvironmentLight::irradiance() const
{
    Q_D(const QEnvironmentLight);
    return d->m_irradiance;
}

/*!
    \qmlproperty Texture EnvironmentLight::specular

    Holds the current environment specular map texture.

    By default, the environment specular texture is null.
*/

/*!
    \property QEnvironmentLight::specular

    Holds the current environment specular map texture.

    By default, the environment specular texture is null.
*/
QAbstractTexture *QEnvironmentLight::specular() const
{
    Q_D(const QEnvironmentLight);
    return d->m_specular;
}

void QEnvironmentLight::setIrradiance(QAbstractTexture *i)
{
    Q_D(QEnvironmentLight);
    if (irradiance() == i)
        return;

    if (irradiance()) {
        d->unregisterDestructionHelper(d->m_irradiance);
        QObject::disconnect(d->m_irradiance, SIGNAL(widthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::disconnect(d->m_irradiance, SIGNAL(heightChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::disconnect(d->m_irradiance, SIGNAL(depthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
    }

    if (i && !i->parent())
        i->setParent(this);

    d->m_irradiance = i;
    d->m_shaderData->setProperty("irradiance", QVariant::fromValue(i));
    d->_q_updateEnvMapsSize();

    if (i) {
        d->registerDestructionHelper(i, &QEnvironmentLight::setIrradiance, i);
        QObject::connect(d->m_irradiance, SIGNAL(widthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::connect(d->m_irradiance, SIGNAL(heightChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::connect(d->m_irradiance, SIGNAL(depthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
    }

    emit irradianceChanged(i);
}

void QEnvironmentLight::setSpecular(QAbstractTexture *s)
{
    Q_D(QEnvironmentLight);
    if (specular() == s)
        return;

    if (specular()) {
        d->unregisterDestructionHelper(d->m_specular);
        QObject::disconnect(d->m_specular, SIGNAL(widthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::disconnect(d->m_specular, SIGNAL(heightChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::disconnect(d->m_specular, SIGNAL(depthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
    }

    if (s && !s->parent())
        s->setParent(this);

    d->m_specular = s;
    d->m_shaderData->setProperty("specular", QVariant::fromValue(s));
    d->_q_updateEnvMapsSize();

    if (s) {
        d->registerDestructionHelper(s, &QEnvironmentLight::setSpecular, s);
        QObject::connect(d->m_specular, SIGNAL(widthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::connect(d->m_specular, SIGNAL(heightChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
        QObject::connect(d->m_specular, SIGNAL(depthChanged(int)), this, SLOT(_q_updateEnvMapsSize()));
    }

    emit specularChanged(s);
}

} // namespace Qt3DRender

QT_END_NAMESPACE

#include "moc_qenvironmentlight.cpp"
