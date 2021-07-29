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

#include "qsgcompressedatlastexture_p.h"

#include <QtCore/QVarLengthArray>
#include <QtCore/QElapsedTimer>
#include <QtCore/QtMath>

#include <QtGui/QOpenGLContext>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QSurface>
#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLTexture>
#include <QDebug>

#include <private/qqmlglobal_p.h>
#include <private/qquickprofiler_p.h>
#include <private/qsgtexture_p.h>
#include <private/qsgcompressedtexture_p.h>

QT_BEGIN_NAMESPACE

static QElapsedTimer qsg_renderer_timer;

namespace QSGCompressedAtlasTexture
{

Atlas::Atlas(const QSize &size, uint format)
    : QSGAtlasTexture::AtlasBase(size)
    , m_format(format)
{
}

Atlas::~Atlas()
{
}

Texture *Atlas::create(const QByteArray &data, int dataLength, int dataOffset, const QSize &size, const QSize &paddedSize)
{
    // No need to lock, as manager already locked it.
    QRect rect = m_allocator.allocate(paddedSize);
    if (rect.width() > 0 && rect.height() > 0) {
        Texture *t = new Texture(this, rect, data, dataLength, dataOffset, size);
        m_pending_uploads << t;
        return t;
    }
    return nullptr;
}

void Atlas::generateTexture()
{
    QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
    funcs->glCompressedTexImage2D(GL_TEXTURE_2D, 0, m_format,
                                  m_size.width(), m_size.height(), 0,
                                  (m_size.width() * m_size.height()) / 2,
                                  nullptr);
}

void Atlas::uploadPendingTexture(int i)
{
    Texture *texture = static_cast<Texture*>(m_pending_uploads.at(i));

    const QRect &r = texture->atlasSubRect();

    QOpenGLFunctions *funcs = QOpenGLContext::currentContext()->functions();
    funcs->glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,
                                     r.x(), r.y(), r.width(), r.height(), m_format,
                                     texture->sizeInBytes(),
                                     texture->data().constData() + texture->dataOffset());

    qCDebug(QSG_LOG_TIME_TEXTURE).nospace() << "compressed atlastexture uploaded in: " << qsg_renderer_timer.elapsed()
                                       << "ms (" << texture->textureSize().width() << "x"
                                       << texture->textureSize().height() << ")";

    // TODO: consider releasing the data (as is done in the regular atlas)?
    // The advantage of keeping this data around is that it makes it much easier
    // to remove the texture from the atlas
}

Texture::Texture(Atlas *atlas, const QRect &textureRect, const QByteArray &data, int dataLength, int dataOffset, const QSize &size)
    : QSGAtlasTexture::TextureBase(atlas, textureRect)
    , m_nonatlas_texture(nullptr)
    , m_data(data)
    , m_size(size)
    , m_dataLength(dataLength)
    , m_dataOffset(dataOffset)
{
    float w = atlas->size().width();
    float h = atlas->size().height();
    QRect nopad = atlasSubRect();
    // offset by half-pixel to prevent bleeding when scaling
    m_texture_coords_rect = QRectF((nopad.x() + .5) / w,
                                   (nopad.y() + .5) / h,
                                   (nopad.width() - 1.) / w,
                                   (nopad.height() - 1.) / h);
}

Texture::~Texture()
{
    delete m_nonatlas_texture;
}

bool Texture::hasAlphaChannel() const
{
    return QSGCompressedTexture::formatIsOpaque(static_cast<Atlas*>(m_atlas)->format());
}

QSGTexture *Texture::removedFromAtlas() const
{
    if (m_nonatlas_texture) {
        m_nonatlas_texture->setMipmapFiltering(mipmapFiltering());
        m_nonatlas_texture->setFiltering(filtering());
        return m_nonatlas_texture;
    }

    if (!m_data.isEmpty()) {
        QTextureFileData texData;
        texData.setData(m_data);
        texData.setSize(m_size);
        texData.setGLInternalFormat(static_cast<Atlas*>(m_atlas)->format());
        texData.setDataLength(m_dataLength);
        texData.setDataOffset(m_dataOffset);
        m_nonatlas_texture = new QSGCompressedTexture(texData);
        m_nonatlas_texture->setMipmapFiltering(mipmapFiltering());
        m_nonatlas_texture->setFiltering(filtering());
    }

    return m_nonatlas_texture;
}

}

QT_END_NAMESPACE
