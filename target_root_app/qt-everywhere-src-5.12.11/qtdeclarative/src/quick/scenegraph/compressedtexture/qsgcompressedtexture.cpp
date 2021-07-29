/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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


#include "qsgcompressedtexture_p.h"
#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QtQuick/private/qquickwindow_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QSG_LOG_TEXTUREIO, "qt.scenegraph.textureio");

QSGCompressedTexture::QSGCompressedTexture(const QTextureFileData &texData)
    : m_textureData(texData)
{
    m_size = m_textureData.size();
    m_hasAlpha = !formatIsOpaque(m_textureData.glInternalFormat());
}

QSGCompressedTexture::~QSGCompressedTexture()
{
#if QT_CONFIG(opengl)
    if (m_textureId) {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        QOpenGLFunctions *funcs = ctx ? ctx->functions() : nullptr;
        if (!funcs)
            return;

        funcs->glDeleteTextures(1, &m_textureId);
    }
#endif
}

int QSGCompressedTexture::textureId() const
{
#if QT_CONFIG(opengl)
    if (!m_textureId) {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        QOpenGLFunctions *funcs = ctx ? ctx->functions() : nullptr;
        if (!funcs)
            return 0;

        funcs->glGenTextures(1, &m_textureId);
    }
#endif
    return m_textureId;
}

QSize QSGCompressedTexture::textureSize() const
{
    return m_size;
}

bool QSGCompressedTexture::hasAlphaChannel() const
{
    return m_hasAlpha;
}

bool QSGCompressedTexture::hasMipmaps() const
{
    return false;
}

void QSGCompressedTexture::bind()
{
#if QT_CONFIG(opengl)
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions *funcs = ctx ? ctx->functions() : nullptr;
    if (!funcs)
        return;

    if (!textureId())
        return;

    funcs->glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_uploaded)
        return;

    if (!m_textureData.isValid()) {
        qCDebug(QSG_LOG_TEXTUREIO, "Invalid texture data for %s", m_textureData.logName().constData());
        funcs->glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    if (Q_UNLIKELY(QSG_LOG_TEXTUREIO().isDebugEnabled())) {
        qCDebug(QSG_LOG_TEXTUREIO) << "Uploading texture" << m_textureData;
        while (funcs->glGetError() != GL_NO_ERROR);
    }

    funcs->glCompressedTexImage2D(GL_TEXTURE_2D, 0, m_textureData.glInternalFormat(),
                                  m_size.width(), m_size.height(), 0, m_textureData.dataLength(),
                                  m_textureData.data().constData() + m_textureData.dataOffset());

    if (Q_UNLIKELY(QSG_LOG_TEXTUREIO().isDebugEnabled())) {
        GLuint error = funcs->glGetError();
        if (error != GL_NO_ERROR) {
            qCDebug(QSG_LOG_TEXTUREIO, "glCompressedTexImage2D failed for %s, error 0x%x", m_textureData.logName().constData(), error);
        }
    }

    m_textureData = QTextureFileData();  // Release this memory, not needed anymore

    updateBindOptions(true);
    m_uploaded = true;
#endif // QT_CONFIG(opengl)
}

QTextureFileData QSGCompressedTexture::textureData() const
{
    return m_textureData;
}

bool QSGCompressedTexture::formatIsOpaque(quint32 glTextureFormat)
{
    switch (glTextureFormat) {
    case QOpenGLTexture::RGB_DXT1:
    case QOpenGLTexture::R_ATI1N_UNorm:
    case QOpenGLTexture::R_ATI1N_SNorm:
    case QOpenGLTexture::RG_ATI2N_UNorm:
    case QOpenGLTexture::RG_ATI2N_SNorm:
    case QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT:
    case QOpenGLTexture::RGB_BP_SIGNED_FLOAT:
    case QOpenGLTexture::R11_EAC_UNorm:
    case QOpenGLTexture::R11_EAC_SNorm:
    case QOpenGLTexture::RG11_EAC_UNorm:
    case QOpenGLTexture::RG11_EAC_SNorm:
    case QOpenGLTexture::RGB8_ETC2:
    case QOpenGLTexture::SRGB8_ETC2:
    case QOpenGLTexture::RGB8_ETC1:
    case QOpenGLTexture::SRGB_DXT1:
        return true;
        break;
    default:
        return false;
    }
}

QSGCompressedTextureFactory::QSGCompressedTextureFactory(const QTextureFileData &texData)
    : m_textureData(texData)
{
}

QSGTexture *QSGCompressedTextureFactory::createTexture(QQuickWindow *window) const
{
    if (!m_textureData.isValid())
        return nullptr;

    // attempt to atlas the texture
    QSGRenderContext *context = QQuickWindowPrivate::get(window)->context;
    QSGTexture *t = context->compressedTextureForFactory(this);
    if (t)
        return t;

    return new QSGCompressedTexture(m_textureData);
}

int QSGCompressedTextureFactory::textureByteCount() const
{
    return qMax(0, m_textureData.data().size() - m_textureData.dataOffset());
}

QSize QSGCompressedTextureFactory::textureSize() const
{
    return m_textureData.size();
}

QT_END_NAMESPACE
