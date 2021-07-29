/****************************************************************************
**
** Copyright (C) 2019 Klaralvdalens Datakonsult AB (KDAB).
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

#include "texturesubmissioncontext_p.h"

#include <Qt3DRender/private/graphicscontext_p.h>
#include <Qt3DRender/private/gltexture_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {

class TextureExtRendererLocker
{
public:
    static void lock(GLTexture *tex)
    {
        if (!tex->isExternalRenderingEnabled())
            return;
        if (s_lockHash.keys().contains(tex)) {
            ++s_lockHash[tex];
        } else {
            tex->externalRenderingLock()->lock();
            s_lockHash[tex] = 1;
        }
    }
    static void unlock(GLTexture *tex)
    {
        if (!tex->isExternalRenderingEnabled())
            return;
        if (!s_lockHash.keys().contains(tex))
            return;

        --s_lockHash[tex];
        if (s_lockHash[tex] == 0) {
            s_lockHash.remove(tex);
            tex->externalRenderingLock()->unlock();
        }
    }
private:
    static QHash<GLTexture*, int> s_lockHash;
};

QHash<GLTexture*, int> TextureExtRendererLocker::s_lockHash = QHash<GLTexture*, int>();


TextureSubmissionContext::TextureSubmissionContext()
{

}

TextureSubmissionContext::~TextureSubmissionContext()
{

}

void TextureSubmissionContext::initialize(GraphicsContext *context)
{
    m_activeTextures.resize(context->maxTextureUnitsCount());
}

void TextureSubmissionContext::endDrawing()
{
    decayTextureScores();
    for (int i = 0; i < m_activeTextures.size(); ++i)
        if (m_activeTextures[i].texture)
            TextureExtRendererLocker::unlock(m_activeTextures[i].texture);
}

int TextureSubmissionContext::activateTexture(TextureSubmissionContext::TextureScope scope, GLTexture *tex)
{
    // Returns the texture unit to use for the texture
    // This always return a valid unit, unless there are more textures than
    // texture unit available for the current material
    const int onUnit = assignUnitForTexture(tex);

    // check we didn't overflow the available units
    if (onUnit == -1)
        return -1;

    // Texture must have been created and updated at this point
    QOpenGLTexture *glTex = tex->getGLTexture();
    if (glTex == nullptr)
        return -1;
    glTex->bind(uint(onUnit));
    if (m_activeTextures[onUnit].texture != tex) {
        if (m_activeTextures[onUnit].texture)
            TextureExtRendererLocker::unlock(m_activeTextures[onUnit].texture);
        m_activeTextures[onUnit].texture = tex;
        TextureExtRendererLocker::lock(tex);
    }

#if defined(QT3D_RENDER_ASPECT_OPENGL_DEBUG)
    int err = m_gl->functions()->glGetError();
    if (err)
        qCWarning(Backend) << "GL error after activating texture" << QString::number(err, 16)
                           << tex->getGLTexture()->textureId() << "on unit" << onUnit;
#endif

    m_activeTextures[onUnit].score = 200;
    m_activeTextures[onUnit].pinned = true;
    m_activeTextures[onUnit].scope = scope;

    return onUnit;
}

void TextureSubmissionContext::deactivateTexturesWithScope(TextureSubmissionContext::TextureScope ts)
{
    for (int u=0; u<m_activeTextures.size(); ++u) {
        if (!m_activeTextures[u].pinned)
            continue; // inactive, ignore

        if (m_activeTextures[u].scope == ts) {
            m_activeTextures[u].pinned = false;
            m_activeTextures[u].score = qMax(m_activeTextures[u].score, 1) - 1;
        }
    } // of units iteration
}

void TextureSubmissionContext::deactivateTexture(GLTexture* tex)
{
    for (int u=0; u<m_activeTextures.size(); ++u) {
        if (m_activeTextures[u].texture == tex) {
            Q_ASSERT(m_activeTextures[u].pinned);
            m_activeTextures[u].pinned = false;
            return;
        }
    } // of units iteration

    qCWarning(Backend) << Q_FUNC_INFO << "texture not active:" << tex;
}

/*!
    \internal
    Returns a texture unit for a texture, -1 if all texture units are assigned.
    Tries to use the texture unit with the texture that hasn't been used for the longest time
    if the texture happens not to be already pinned on a texture unit.
 */
int TextureSubmissionContext::assignUnitForTexture(GLTexture *tex)
{
    int lowestScoredUnit = -1;
    int lowestScore = 0xfffffff;

    for (int u=0; u<m_activeTextures.size(); ++u) {
        if (m_activeTextures[u].texture == tex)
            return u;
    }

    for (int u=0; u<m_activeTextures.size(); ++u) {
        // No texture is currently active on the texture unit
        // we save the texture unit with the texture that has been on there
        // the longest time while not being used
        if (!m_activeTextures[u].pinned) {
            int score = m_activeTextures[u].score;
            if (score < lowestScore) {
                lowestScore = score;
                lowestScoredUnit = u;
            }
        }
    } // of units iteration

    if (lowestScoredUnit == -1)
        qCWarning(Backend) << Q_FUNC_INFO << "No free texture units!";

    return lowestScoredUnit;
}

void TextureSubmissionContext::decayTextureScores()
{
    for (int u = 0; u < m_activeTextures.size(); u++)
        m_activeTextures[u].score = qMax(m_activeTextures[u].score - 1, 0);
}

} // namespace Render
} // namespace Qt3DRender of namespace

QT_END_NAMESPACE
