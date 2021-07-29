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

#include "qspritesheet.h"
#include "qspritesheetitem.h"
#include "qspritesheet_p.h"

#include <Qt3DRender/qabstracttexture.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DExtras {

QSpriteSheetPrivate::QSpriteSheetPrivate()
    : QAbstractSpriteSheetPrivate()
{
}

int QSpriteSheetPrivate::maxIndex() const
{
    return m_sprites.count() - 1;
}

void QSpriteSheetPrivate::updateSizes()
{
    Q_Q(QSpriteSheet);
    if (m_texture)
        m_textureSize = QSize(m_texture->width(), m_texture->height());
    else
        m_textureSize = QSize();

    if (m_textureSize.isEmpty() || m_sprites.isEmpty()) {
        if (m_currentIndex != -1) {
            m_currentIndex = -1;
            emit q->currentIndexChanged(m_currentIndex);
        }
        m_textureTransform.setToIdentity();
        emit q->textureTransformChanged(m_textureTransform);
        return;
    }

    if (m_currentIndex < 0 || m_currentIndex >= m_sprites.size()) {
        m_currentIndex = 0;
        emit q->currentIndexChanged(m_currentIndex);
    }
    updateTransform();
}

void QSpriteSheetPrivate::updateTransform()
{
    Q_Q(QSpriteSheet);
    if (m_currentIndex < 0 || m_currentIndex >= m_sprites.size())
        return;

    const QSpriteSheetItem *r = m_sprites.at(m_currentIndex);
    const float xScale = static_cast<float>(r->width()) / static_cast<float>(m_textureSize.width());
    const float yScale = static_cast<float>(r->height()) / static_cast<float>(m_textureSize.height());

    const float xTranslate = static_cast<float>(r->x()) / static_cast<float>(m_textureSize.width());
    const float yTranslate = static_cast<float>(r->y()) / static_cast<float>(m_textureSize.height());

    m_textureTransform.setToIdentity();
    m_textureTransform(0, 0) = xScale;
    m_textureTransform(1, 1) = yScale;
    m_textureTransform(0, 2) = xTranslate;
    m_textureTransform(1, 2) = yTranslate;
    emit q->textureTransformChanged(m_textureTransform);
}

/*!
    Constructs a new QSpriteSheet instance with parent object \a parent.
 */
QSpriteSheet::QSpriteSheet(QNode *parent)
    : QAbstractSpriteSheet(*new QSpriteSheetPrivate, parent)
{
}

QSpriteSheet::~QSpriteSheet()
{
}

QVector<QSpriteSheetItem *> QSpriteSheet::sprites() const
{
    Q_D(const QSpriteSheet);
    return d->m_sprites;
}

QSpriteSheetItem *QSpriteSheet::addSprite(int x, int y, int width, int height)
{
    QSpriteSheetItem *item = new QSpriteSheetItem(this);
    item->setX(x);
    item->setX(y);
    item->setWidth(width);
    item->setHeight(height);
    addSprite(item);
    return item;
}

void QSpriteSheet::addSprite(QSpriteSheetItem *sprite)
{
    Q_ASSERT(sprite);
    Q_D(QSpriteSheet);
    if (!d->m_sprites.contains(sprite)) {
        d->m_sprites << sprite;

        // Ensures proper bookkeeping
        d->registerDestructionHelper(sprite, &QSpriteSheet::removeSprite, d->m_sprites);
        if (!sprite->parent())
            sprite->setParent(this);

        emit spritesChanged(d->m_sprites);
        d->updateSizes();
    }
}

void QSpriteSheet::removeSprite(QSpriteSheetItem *sprite)
{
    Q_ASSERT(sprite);
    Q_D(QSpriteSheet);
    d->m_sprites.removeOne(sprite);
    // Remove bookkeeping connection
    d->unregisterDestructionHelper(sprite);
}

void QSpriteSheet::setSprites(QVector<QSpriteSheetItem *> sprites)
{
    Q_D(QSpriteSheet);
    d->m_sprites = sprites;
    emit spritesChanged(sprites);
    d->updateSizes();
}

} // namespace Qt3DExtras

QT_END_NAMESPACE
