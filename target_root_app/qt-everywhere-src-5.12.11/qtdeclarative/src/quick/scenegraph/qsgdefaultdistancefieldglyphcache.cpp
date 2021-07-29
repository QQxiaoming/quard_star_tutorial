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

#include "qsgdefaultdistancefieldglyphcache_p.h"

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qbuffer.h>
#include <QtQml/qqmlfile.h>

#include <QtGui/private/qdistancefield_p.h>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtQml/private/qqmlglobal_p.h>
#include <qopenglfunctions.h>
#include <qopenglframebufferobject.h>
#include <qmath.h>
#include "qsgcontext_p.h"


#if !defined(QT_OPENGL_ES_2)
#include <QtGui/qopenglfunctions_3_2_core.h>
#endif

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(qmlUseGlyphCacheWorkaround, QML_USE_GLYPHCACHE_WORKAROUND)
DEFINE_BOOL_CONFIG_OPTION(qsgPreferFullSizeGlyphCacheTextures, QSG_PREFER_FULLSIZE_GLYPHCACHE_TEXTURES)

#if !defined(QSG_DEFAULT_DISTANCEFIELD_GLYPH_CACHE_PADDING)
#  define QSG_DEFAULT_DISTANCEFIELD_GLYPH_CACHE_PADDING 2
#endif

QSGDefaultDistanceFieldGlyphCache::QSGDefaultDistanceFieldGlyphCache(QOpenGLContext *c,
                                                                     const QRawFont &font)
    : QSGDistanceFieldGlyphCache(font)
    , m_maxTextureWidth(0)
    , m_maxTextureHeight(0)
    , m_maxTextureCount(3)
    , m_areaAllocator(nullptr)
    , m_blitProgram(nullptr)
    , m_blitBuffer(QOpenGLBuffer::VertexBuffer)
    , m_fboGuard(nullptr)
    , m_funcs(c->functions())
#if !defined(QT_OPENGL_ES_2)
    , m_coreFuncs(nullptr)
#endif
{
    if (Q_LIKELY(m_blitBuffer.create())) {
        m_blitBuffer.bind();
        static const GLfloat buffer[16] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
                                           0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
        m_blitBuffer.allocate(buffer, sizeof(buffer));
        m_blitBuffer.release();
    } else {
        qWarning("Buffer creation failed");
    }

    m_coreProfile = (c->format().profile() == QSurfaceFormat::CoreProfile);

    // Load a pregenerated cache if the font contains one
    loadPregeneratedCache(font);
}

QSGDefaultDistanceFieldGlyphCache::~QSGDefaultDistanceFieldGlyphCache()
{
    for (int i = 0; i < m_textures.count(); ++i)
        m_funcs->glDeleteTextures(1, &m_textures[i].texture);

    if (m_fboGuard != nullptr)
        m_fboGuard->free();

    delete m_blitProgram;
    delete m_areaAllocator;
}

void QSGDefaultDistanceFieldGlyphCache::requestGlyphs(const QSet<glyph_t> &glyphs)
{
    QList<GlyphPosition> glyphPositions;
    QVector<glyph_t> glyphsToRender;

    const int padding = QSG_DEFAULT_DISTANCEFIELD_GLYPH_CACHE_PADDING;
    const qreal scaleFactor = qreal(1) / QT_DISTANCEFIELD_SCALE(m_doubleGlyphResolution);

    if (m_maxTextureHeight == 0) {
        m_funcs->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureWidth);

        // We need to add a buffer to avoid glyphs that overlap the border between two
        // textures causing the height of the textures to extend beyond the limit.
        m_maxTextureHeight = m_maxTextureWidth - (qCeil(m_referenceFont.pixelSize() * scaleFactor) + distanceFieldRadius() * 2 + padding * 2);
    }

    if (m_areaAllocator == nullptr)
        m_areaAllocator = new QSGAreaAllocator(QSize(m_maxTextureWidth, m_maxTextureCount * m_maxTextureHeight));

    for (QSet<glyph_t>::const_iterator it = glyphs.constBegin(); it != glyphs.constEnd() ; ++it) {
        glyph_t glyphIndex = *it;

        QRectF boundingRect = glyphData(glyphIndex).boundingRect;
        int glyphWidth = qCeil(boundingRect.width()) + distanceFieldRadius() * 2;
        int glyphHeight = qCeil(boundingRect.height()) + distanceFieldRadius() * 2;
        QSize glyphSize(glyphWidth + padding * 2, glyphHeight + padding * 2);
        QRect alloc = m_areaAllocator->allocate(glyphSize);

        if (alloc.isNull()) {
            // Unallocate unused glyphs until we can allocated the new glyph
            while (alloc.isNull() && !m_unusedGlyphs.isEmpty()) {
                glyph_t unusedGlyph = *m_unusedGlyphs.constBegin();

                TexCoord unusedCoord = glyphTexCoord(unusedGlyph);
                QRectF unusedGlyphBoundingRect = glyphData(unusedGlyph).boundingRect;
                int unusedGlyphWidth = qCeil(unusedGlyphBoundingRect.width()) + distanceFieldRadius() * 2;
                int unusedGlyphHeight = qCeil(unusedGlyphBoundingRect.height())  + distanceFieldRadius() * 2;
                m_areaAllocator->deallocate(QRect(unusedCoord.x - padding,
                                                  unusedCoord.y - padding,
                                                  padding * 2 + unusedGlyphWidth,
                                                  padding * 2 + unusedGlyphHeight));

                m_unusedGlyphs.remove(unusedGlyph);
                m_glyphsTexture.remove(unusedGlyph);
                removeGlyph(unusedGlyph);

                alloc = m_areaAllocator->allocate(glyphSize);
            }

            // Not enough space left for this glyph... skip to the next one
            if (alloc.isNull())
                continue;
        }

        TextureInfo *tex = textureInfo(alloc.y() / m_maxTextureHeight);
        alloc = QRect(alloc.x(), alloc.y() % m_maxTextureHeight, alloc.width(), alloc.height());

        tex->allocatedArea |= alloc;
        Q_ASSERT(tex->padding == padding || tex->padding < 0);
        tex->padding = padding;

        GlyphPosition p;
        p.glyph = glyphIndex;
        p.position = alloc.topLeft() + QPoint(padding, padding);

        glyphPositions.append(p);
        glyphsToRender.append(glyphIndex);
        m_glyphsTexture.insert(glyphIndex, tex);
    }

    setGlyphsPosition(glyphPositions);
    markGlyphsToRender(glyphsToRender);
}

void QSGDefaultDistanceFieldGlyphCache::storeGlyphs(const QList<QDistanceField> &glyphs)
{
    typedef QHash<TextureInfo *, QVector<glyph_t> > GlyphTextureHash;
    typedef GlyphTextureHash::const_iterator GlyphTextureHashConstIt;

    GlyphTextureHash glyphTextures;

    GLint alignment = 4; // default value
    m_funcs->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    // Distance field data is always tightly packed
    m_funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (int i = 0; i < glyphs.size(); ++i) {
        QDistanceField glyph = glyphs.at(i);
        glyph_t glyphIndex = glyph.glyph();
        TexCoord c = glyphTexCoord(glyphIndex);
        TextureInfo *texInfo = m_glyphsTexture.value(glyphIndex);

        resizeTexture(texInfo, texInfo->allocatedArea.width(), texInfo->allocatedArea.height());
        m_funcs->glBindTexture(GL_TEXTURE_2D, texInfo->texture);

        glyphTextures[texInfo].append(glyphIndex);

        int padding = texInfo->padding;
        int expectedWidth = qCeil(c.width + c.xMargin * 2);
        glyph = glyph.copy(-padding, -padding,
                           expectedWidth + padding  * 2, glyph.height() + padding * 2);

        if (useTextureResizeWorkaround()) {
            uchar *inBits = glyph.scanLine(0);
            uchar *outBits = texInfo->image.scanLine(int(c.y) - padding) + int(c.x) - padding;
            for (int y = 0; y < glyph.height(); ++y) {
                memcpy(outBits, inBits, glyph.width());
                inBits += glyph.width();
                outBits += texInfo->image.width();
            }
        }

#if !defined(QT_OPENGL_ES_2)
        const GLenum format = isCoreProfile() ? GL_RED : GL_ALPHA;
#else
        const GLenum format = GL_ALPHA;
#endif
        if (useTextureUploadWorkaround()) {
            for (int i = 0; i < glyph.height(); ++i) {
                m_funcs->glTexSubImage2D(GL_TEXTURE_2D, 0,
                                         c.x - padding, c.y + i - padding, glyph.width(),1,
                                         format, GL_UNSIGNED_BYTE,
                                         glyph.scanLine(i));
            }
        } else {
            m_funcs->glTexSubImage2D(GL_TEXTURE_2D, 0,
                                     c.x - padding, c.y - padding, glyph.width(), glyph.height(),
                                     format, GL_UNSIGNED_BYTE,
                                     glyph.constBits());
        }
    }

    // restore to previous alignment
    m_funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    for (GlyphTextureHashConstIt i = glyphTextures.constBegin(), cend = glyphTextures.constEnd(); i != cend; ++i) {
        Texture t;
        t.textureId = i.key()->texture;
        t.size = i.key()->size;
        setGlyphsTexture(i.value(), t);
    }
}

void QSGDefaultDistanceFieldGlyphCache::referenceGlyphs(const QSet<glyph_t> &glyphs)
{
    m_unusedGlyphs -= glyphs;
}

void QSGDefaultDistanceFieldGlyphCache::releaseGlyphs(const QSet<glyph_t> &glyphs)
{
    m_unusedGlyphs += glyphs;
}

void QSGDefaultDistanceFieldGlyphCache::createTexture(TextureInfo *texInfo,
                                                      int width,
                                                      int height)
{
    QByteArray zeroBuf(width * height, 0);
    createTexture(texInfo, width, height, zeroBuf.constData());
}

void QSGDefaultDistanceFieldGlyphCache::createTexture(TextureInfo *texInfo,
                                                      int width,
                                                      int height,
                                                      const void *pixels)
{
    if (useTextureResizeWorkaround() && texInfo->image.isNull()) {
        texInfo->image = QDistanceField(width, height);
        memcpy(texInfo->image.bits(), pixels, width * height);
    }

    while (m_funcs->glGetError() != GL_NO_ERROR) { }

    m_funcs->glGenTextures(1, &texInfo->texture);
    m_funcs->glBindTexture(GL_TEXTURE_2D, texInfo->texture);

    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if !defined(QT_OPENGL_ES_2)
    if (!QOpenGLContext::currentContext()->isOpenGLES())
        m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    const GLint internalFormat = isCoreProfile() ? GL_R8 : GL_ALPHA;
    const GLenum format = isCoreProfile() ? GL_RED : GL_ALPHA;
#else
    const GLint internalFormat = GL_ALPHA;
    const GLenum format = GL_ALPHA;
#endif

    m_funcs->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

    texInfo->size = QSize(width, height);

    GLuint error = m_funcs->glGetError();
    if (error != GL_NO_ERROR) {
        m_funcs->glBindTexture(GL_TEXTURE_2D, 0);
        m_funcs->glDeleteTextures(1, &texInfo->texture);
        texInfo->texture = 0;
    }

}

static void freeFramebufferFunc(QOpenGLFunctions *funcs, GLuint id)
{
    funcs->glDeleteFramebuffers(1, &id);
}

void QSGDefaultDistanceFieldGlyphCache::resizeTexture(TextureInfo *texInfo, int width, int height)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    Q_ASSERT(ctx);

    int oldWidth = texInfo->size.width();
    int oldHeight = texInfo->size.height();
    if (width == oldWidth && height == oldHeight)
        return;

    GLuint oldTexture = texInfo->texture;
    createTexture(texInfo, width, height);

    if (!oldTexture)
        return;

    updateTexture(oldTexture, texInfo->texture, texInfo->size);

#if !defined(QT_OPENGL_ES_2)
    if (isCoreProfile() && !useTextureResizeWorkaround()) {
        // For an OpenGL Core Profile we can use http://www.opengl.org/wiki/Framebuffer#Blitting
        // to efficiently copy the contents of the old texture to the new texture
        // TODO: Use ARB_copy_image if available of if we have >=4.3 context
        if (!m_coreFuncs) {
            m_coreFuncs = ctx->versionFunctions<QOpenGLFunctions_3_2_Core>();
            Q_ASSERT(m_coreFuncs);
            m_coreFuncs->initializeOpenGLFunctions();
        }

        // Create a framebuffer object to which we can attach our old and new textures (to
        // the first two color buffer attachment points)
        if (!m_fboGuard) {
            GLuint fbo;
            m_coreFuncs->glGenFramebuffers(1, &fbo);
            m_fboGuard = new QOpenGLSharedResourceGuard(ctx, fbo, freeFramebufferFunc);
        }

        // Bind the FBO to both the GL_READ_FRAMEBUFFER? and GL_DRAW_FRAMEBUFFER targets
        m_coreFuncs->glBindFramebuffer(GL_FRAMEBUFFER, m_fboGuard->id());

        // Bind the old texture to GL_COLOR_ATTACHMENT0
        m_coreFuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                        GL_TEXTURE_2D, oldTexture, 0);

        // Bind the new texture to GL_COLOR_ATTACHMENT1
        m_coreFuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                        GL_TEXTURE_2D, texInfo->texture, 0);

        // Set the source and destination buffers
        m_coreFuncs->glReadBuffer(GL_COLOR_ATTACHMENT0);
        m_coreFuncs->glDrawBuffer(GL_COLOR_ATTACHMENT1);

        // Do the blit
        m_coreFuncs->glBlitFramebuffer(0, 0, oldWidth, oldHeight,
                                   0, 0, oldWidth, oldHeight,
                                   GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Reset the default framebuffer
        QOpenGLFramebufferObject::bindDefault();

        return;
    } else if (useTextureResizeWorkaround()) {
#else
    if (useTextureResizeWorkaround()) {
#endif
        GLint alignment = 4; // default value
        m_funcs->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
        m_funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#if !defined(QT_OPENGL_ES_2)
        const GLenum format = isCoreProfile() ? GL_RED : GL_ALPHA;
#else
        const GLenum format = GL_ALPHA;
#endif

        if (useTextureUploadWorkaround()) {
            for (int i = 0; i < texInfo->image.height(); ++i) {
                m_funcs->glTexSubImage2D(GL_TEXTURE_2D, 0,
                                         0, i, oldWidth, 1,
                                         format, GL_UNSIGNED_BYTE,
                                         texInfo->image.scanLine(i));
            }
        } else {
            m_funcs->glTexSubImage2D(GL_TEXTURE_2D, 0,
                                     0, 0, oldWidth, oldHeight,
                                     format, GL_UNSIGNED_BYTE,
                                     texInfo->image.constBits());
        }

        m_funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment); // restore to previous value

        texInfo->image = texInfo->image.copy(0, 0, width, height);
        m_funcs->glDeleteTextures(1, &oldTexture);
        return;
    }

    if (!m_blitProgram)
        createBlitProgram();

    Q_ASSERT(m_blitProgram);

    if (!m_fboGuard) {
        GLuint fbo;
        m_funcs->glGenFramebuffers(1, &fbo);
        m_fboGuard = new QOpenGLSharedResourceGuard(ctx, fbo, freeFramebufferFunc);
    }
    m_funcs->glBindFramebuffer(GL_FRAMEBUFFER, m_fboGuard->id());

    GLuint tmp_texture;
    m_funcs->glGenTextures(1, &tmp_texture);
    m_funcs->glBindTexture(GL_TEXTURE_2D, tmp_texture);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if !defined(QT_OPENGL_ES_2)
    if (!ctx->isOpenGLES())
        m_funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
#endif
    m_funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, oldWidth, oldHeight, 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_funcs->glBindTexture(GL_TEXTURE_2D, 0);
    m_funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                    GL_TEXTURE_2D, tmp_texture, 0);

    m_funcs->glActiveTexture(GL_TEXTURE0);
    m_funcs->glBindTexture(GL_TEXTURE_2D, oldTexture);

    // save current render states
    GLboolean stencilTestEnabled;
    GLboolean depthTestEnabled;
    GLboolean scissorTestEnabled;
    GLboolean blendEnabled;
    GLint viewport[4];
    GLint oldProgram;
    m_funcs->glGetBooleanv(GL_STENCIL_TEST, &stencilTestEnabled);
    m_funcs->glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    m_funcs->glGetBooleanv(GL_SCISSOR_TEST, &scissorTestEnabled);
    m_funcs->glGetBooleanv(GL_BLEND, &blendEnabled);
    m_funcs->glGetIntegerv(GL_VIEWPORT, &viewport[0]);
    m_funcs->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    m_funcs->glDisable(GL_STENCIL_TEST);
    m_funcs->glDisable(GL_DEPTH_TEST);
    m_funcs->glDisable(GL_SCISSOR_TEST);
    m_funcs->glDisable(GL_BLEND);

    m_funcs->glViewport(0, 0, oldWidth, oldHeight);

    const bool vaoInit = m_vao.isCreated();
    if (isCoreProfile()) {
        if ( !vaoInit )
            m_vao.create();
        m_vao.bind();
    }
    m_blitProgram->bind();
    if (!vaoInit || !isCoreProfile()) {
        m_blitBuffer.bind();

        m_blitProgram->enableAttributeArray(int(QT_VERTEX_COORDS_ATTR));
        m_blitProgram->enableAttributeArray(int(QT_TEXTURE_COORDS_ATTR));
        m_blitProgram->setAttributeBuffer(int(QT_VERTEX_COORDS_ATTR), GL_FLOAT, 0, 2);
        m_blitProgram->setAttributeBuffer(int(QT_TEXTURE_COORDS_ATTR), GL_FLOAT, 32, 2);
    }
    m_blitProgram->disableAttributeArray(int(QT_OPACITY_ATTR));
    m_blitProgram->setUniformValue("imageTexture", GLuint(0));

    m_funcs->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    m_funcs->glBindTexture(GL_TEXTURE_2D, texInfo->texture);

    if (useTextureUploadWorkaround()) {
        for (int i = 0; i < oldHeight; ++i)
            m_funcs->glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, 0, i, oldWidth, 1);
    } else {
        m_funcs->glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, oldWidth, oldHeight);
    }

    m_funcs->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                       GL_RENDERBUFFER, 0);
    m_funcs->glDeleteTextures(1, &tmp_texture);
    m_funcs->glDeleteTextures(1, &oldTexture);

    QOpenGLFramebufferObject::bindDefault();

    // restore render states
    if (stencilTestEnabled)
        m_funcs->glEnable(GL_STENCIL_TEST);
    if (depthTestEnabled)
        m_funcs->glEnable(GL_DEPTH_TEST);
    if (scissorTestEnabled)
        m_funcs->glEnable(GL_SCISSOR_TEST);
    if (blendEnabled)
        m_funcs->glEnable(GL_BLEND);
    m_funcs->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    m_funcs->glUseProgram(oldProgram);

    m_blitProgram->disableAttributeArray(int(QT_VERTEX_COORDS_ATTR));
    m_blitProgram->disableAttributeArray(int(QT_TEXTURE_COORDS_ATTR));
    if (isCoreProfile())
        m_vao.release();
}

bool QSGDefaultDistanceFieldGlyphCache::useTextureResizeWorkaround() const
{
    static bool set = false;
    static bool useWorkaround = false;
    if (!set) {
        QOpenGLContextPrivate *ctx_p = static_cast<QOpenGLContextPrivate *>(QOpenGLContextPrivate::get(QOpenGLContext::currentContext()));
        useWorkaround = ctx_p->workaround_brokenFBOReadBack
                || qmlUseGlyphCacheWorkaround(); // on some hardware the workaround is faster (see QTBUG-29264)
        set = true;
    }
    return useWorkaround;
}

bool QSGDefaultDistanceFieldGlyphCache::useTextureUploadWorkaround() const
{
    static bool set = false;
    static bool useWorkaround = false;
    if (!set) {
        useWorkaround = qstrcmp(reinterpret_cast<const char*>(m_funcs->glGetString(GL_RENDERER)),
                                "Mali-400 MP") == 0;
        set = true;
    }
    return useWorkaround;
}

bool QSGDefaultDistanceFieldGlyphCache::createFullSizeTextures() const
{
    return qsgPreferFullSizeGlyphCacheTextures() && glyphCount() > QT_DISTANCEFIELD_HIGHGLYPHCOUNT();
}

namespace {
    struct Qtdf {
        // We need these structs to be tightly packed, but some compilers we use do not
        // support #pragma pack(1), so we need to hardcode the offsets/sizes in the
        // file format
        enum TableSize {
            HeaderSize = 14,
            GlyphRecordSize = 46,
            TextureRecordSize = 17
        };

        enum Offset {
            // Header
            majorVersion        = 0,
            minorVersion        = 1,
            pixelSize           = 2,
            textureSize         = 4,
            flags               = 8,
            headerPadding       = 9,
            numGlyphs           = 10,

            // Glyph record
            glyphIndex          = 0,
            textureOffsetX      = 4,
            textureOffsetY      = 8,
            textureWidth        = 12,
            textureHeight       = 16,
            xMargin             = 20,
            yMargin             = 24,
            boundingRectX       = 28,
            boundingRectY       = 32,
            boundingRectWidth   = 36,
            boundingRectHeight  = 40,
            textureIndex        = 44,

            // Texture record
            allocatedX          = 0,
            allocatedY          = 4,
            allocatedWidth      = 8,
            allocatedHeight     = 12,
            texturePadding      = 16

        };

        template <typename T>
        static inline T fetch(const char *data, Offset offset)
        {
            return qFromBigEndian<T>(data + int(offset));
        }
    };
}

bool QSGDefaultDistanceFieldGlyphCache::loadPregeneratedCache(const QRawFont &font)
{
    // The pregenerated data must be loaded first, otherwise the area allocator
    // will be wrong
    if (m_areaAllocator != nullptr) {
        qWarning("Font cache must be loaded before cache is used");
        return false;
    }

    static QElapsedTimer timer;

    bool profile = QSG_LOG_TIME_GLYPH().isDebugEnabled();
    if (profile)
        timer.start();

    QByteArray qtdfTable = font.fontTable("qtdf");
    if (qtdfTable.isEmpty())
        return false;

    typedef QHash<TextureInfo *, QVector<glyph_t> > GlyphTextureHash;

    GlyphTextureHash glyphTextures;

    if (uint(qtdfTable.size()) < Qtdf::HeaderSize) {
        qWarning("Invalid qtdf table in font '%s'",
                 qPrintable(font.familyName()));
        return false;
    }

    const char *qtdfTableStart = qtdfTable.constData();
    const char *qtdfTableEnd = qtdfTableStart + qtdfTable.size();

    int padding = 0;
    int textureCount = 0;
    {
        quint8 majorVersion = Qtdf::fetch<quint8>(qtdfTableStart, Qtdf::majorVersion);
        quint8 minorVersion = Qtdf::fetch<quint8>(qtdfTableStart, Qtdf::minorVersion);
        if (majorVersion != 5 || minorVersion != 12) {
            qWarning("Invalid version of qtdf table %d.%d in font '%s'",
                     majorVersion,
                     minorVersion,
                     qPrintable(font.familyName()));
            return false;
        }

        qreal pixelSize = qreal(Qtdf::fetch<quint16>(qtdfTableStart, Qtdf::pixelSize));
        m_maxTextureWidth = m_maxTextureHeight = Qtdf::fetch<quint32>(qtdfTableStart, Qtdf::textureSize);
        m_doubleGlyphResolution = Qtdf::fetch<quint8>(qtdfTableStart, Qtdf::flags) == 1;
        padding = Qtdf::fetch<quint8>(qtdfTableStart, Qtdf::headerPadding);

        if (pixelSize <= 0.0) {
            qWarning("Invalid pixel size in '%s'", qPrintable(font.familyName()));
            return false;
        }

        if (m_maxTextureWidth <= 0) {
            qWarning("Invalid texture size in '%s'", qPrintable(font.familyName()));
            return false;
        }

        int systemMaxTextureSize;
        m_funcs->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &systemMaxTextureSize);

        if (m_maxTextureWidth > systemMaxTextureSize) {
            qWarning("System maximum texture size is %d. This is lower than the value in '%s', which is %d",
                     systemMaxTextureSize,
                     qPrintable(font.familyName()),
                     m_maxTextureWidth);
        }

        if (padding != QSG_DEFAULT_DISTANCEFIELD_GLYPH_CACHE_PADDING) {
            qWarning("Padding mismatch in '%s'. Font requires %d, but Qt is compiled with %d.",
                     qPrintable(font.familyName()),
                     padding,
                     QSG_DEFAULT_DISTANCEFIELD_GLYPH_CACHE_PADDING);
        }

        m_referenceFont.setPixelSize(pixelSize);

        quint32 glyphCount = Qtdf::fetch<quint32>(qtdfTableStart, Qtdf::numGlyphs);
        m_unusedGlyphs.reserve(glyphCount);

        const char *allocatorData = qtdfTableStart + Qtdf::HeaderSize;
        {
            m_areaAllocator = new QSGAreaAllocator(QSize(0, 0));
            allocatorData = m_areaAllocator->deserialize(allocatorData, qtdfTableEnd - allocatorData);
            if (allocatorData == nullptr)
                return false;
        }

        if (m_areaAllocator->size().height() % m_maxTextureHeight != 0) {
            qWarning("Area allocator size mismatch in '%s'", qPrintable(font.familyName()));
            return false;
        }

        textureCount = m_areaAllocator->size().height() / m_maxTextureHeight;
        m_maxTextureCount = qMax(m_maxTextureCount, textureCount);

        const char *textureRecord = allocatorData;
        for (int i = 0; i < textureCount; ++i, textureRecord += Qtdf::TextureRecordSize) {
            if (textureRecord + Qtdf::TextureRecordSize > qtdfTableEnd) {
                qWarning("qtdf table too small in font '%s'.",
                         qPrintable(font.familyName()));
                return false;
            }

            TextureInfo *tex = textureInfo(i);
            tex->allocatedArea.setX(Qtdf::fetch<quint32>(textureRecord, Qtdf::allocatedX));
            tex->allocatedArea.setY(Qtdf::fetch<quint32>(textureRecord, Qtdf::allocatedY));
            tex->allocatedArea.setWidth(Qtdf::fetch<quint32>(textureRecord, Qtdf::allocatedWidth));
            tex->allocatedArea.setHeight(Qtdf::fetch<quint32>(textureRecord, Qtdf::allocatedHeight));
            tex->padding = Qtdf::fetch<quint8>(textureRecord, Qtdf::texturePadding);
        }

        const char *glyphRecord = textureRecord;
        for (quint32 i = 0; i < glyphCount; ++i, glyphRecord += Qtdf::GlyphRecordSize) {
            if (glyphRecord + Qtdf::GlyphRecordSize > qtdfTableEnd) {
                qWarning("qtdf table too small in font '%s'.",
                         qPrintable(font.familyName()));
                return false;
            }

            glyph_t glyph = Qtdf::fetch<quint32>(glyphRecord, Qtdf::glyphIndex);
            m_unusedGlyphs.insert(glyph);

            GlyphData &glyphData = emptyData(glyph);

#define FROM_FIXED_POINT(value) \
(((qreal)value)/(qreal)65536)

            glyphData.texCoord.x = FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::textureOffsetX));
            glyphData.texCoord.y = FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::textureOffsetY));
            glyphData.texCoord.width = FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::textureWidth));
            glyphData.texCoord.height = FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::textureHeight));
            glyphData.texCoord.xMargin = FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::xMargin));
            glyphData.texCoord.yMargin = FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::yMargin));
            glyphData.boundingRect.setX(FROM_FIXED_POINT(Qtdf::fetch<qint32>(glyphRecord, Qtdf::boundingRectX)));
            glyphData.boundingRect.setY(FROM_FIXED_POINT(Qtdf::fetch<qint32>(glyphRecord, Qtdf::boundingRectY)));
            glyphData.boundingRect.setWidth(FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::boundingRectWidth)));
            glyphData.boundingRect.setHeight(FROM_FIXED_POINT(Qtdf::fetch<quint32>(glyphRecord, Qtdf::boundingRectHeight)));

#undef FROM_FIXED_POINT

            int textureIndex = Qtdf::fetch<quint16>(glyphRecord, Qtdf::textureIndex);
            if (textureIndex < 0 || textureIndex >= textureCount) {
                qWarning("Invalid texture index %d (texture count == %d) in '%s'",
                         textureIndex,
                         textureCount,
                         qPrintable(font.familyName()));
                return false;
            }


            TextureInfo *texInfo = textureInfo(textureIndex);
            m_glyphsTexture.insert(glyph, texInfo);

            glyphTextures[texInfo].append(glyph);
        }

        GLint alignment = 4; // default value
        m_funcs->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

        m_funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        const uchar *textureData = reinterpret_cast<const uchar *>(glyphRecord);
        for (int i = 0; i < textureCount; ++i) {

            TextureInfo *texInfo = textureInfo(i);

            int width = texInfo->allocatedArea.width();
            int height = texInfo->allocatedArea.height();
            qint64 size = width * height;
            if (reinterpret_cast<const char *>(textureData + size) > qtdfTableEnd) {
                qWarning("qtdf table too small in font '%s'.",
                         qPrintable(font.familyName()));
                return false;
            }

            createTexture(texInfo, width, height, textureData);

            QVector<glyph_t> glyphs = glyphTextures.value(texInfo);

            Texture t;
            t.textureId = texInfo->texture;
            t.size = texInfo->size;

            setGlyphsTexture(glyphs, t);

            textureData += size;
        }

        m_funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    }

    if (profile) {
        quint64 now = timer.elapsed();
        qCDebug(QSG_LOG_TIME_GLYPH,
                "distancefield: %d pre-generated glyphs loaded in %dms",
                m_unusedGlyphs.size(),
                (int) now);
    }

    return true;
}

QT_END_NAMESPACE
