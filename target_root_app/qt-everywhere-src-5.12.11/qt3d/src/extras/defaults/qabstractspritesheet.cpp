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

#include "qabstractspritesheet.h"
#include "qabstractspritesheet_p.h"

#include <Qt3DRender/qabstracttexture.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DExtras {

QAbstractSpriteSheetPrivate::QAbstractSpriteSheetPrivate()
    : QNodePrivate()
    , m_texture(nullptr)
    , m_currentIndex(-1)
{
}

void QAbstractSpriteSheetPrivate::init()
{
    m_textureTransform.setToIdentity();
}

void QAbstractSpriteSheetPrivate::updateTexture(Qt3DRender::QAbstractTexture *texture)
{
    if (m_texture) {
        disconnect(m_texture, &Qt3DRender::QAbstractTexture::widthChanged,
                   this, &QAbstractSpriteSheetPrivate::updateSizes);
        disconnect(m_texture, &Qt3DRender::QAbstractTexture::heightChanged,
                   this, &QAbstractSpriteSheetPrivate::updateSizes);
    }
    m_texture = texture;
    if (m_texture) {
        connect(m_texture, &Qt3DRender::QAbstractTexture::widthChanged,
                this, &QAbstractSpriteSheetPrivate::updateSizes);
        connect(m_texture, &Qt3DRender::QAbstractTexture::heightChanged,
                this, &QAbstractSpriteSheetPrivate::updateSizes);
    }
}

void QAbstractSpriteSheetPrivate::updateIndex(int newIndex)
{
    Q_Q(QAbstractSpriteSheet);
    if (newIndex > maxIndex())
        newIndex = 0;

    m_currentIndex = newIndex;
    emit q->currentIndexChanged(newIndex);
    updateTransform();
}

QAbstractSpriteSheet::QAbstractSpriteSheet(QAbstractSpriteSheetPrivate &dd, QNode *parent)
    : Qt3DCore::QNode(dd, parent)
{
    Q_D(QAbstractSpriteSheet);
    d->init();
}

QAbstractSpriteSheet::~QAbstractSpriteSheet()
{
}

/*!
    \property QAbstractSpriteSheet::texture

    Holds the current texture used by the material.
*/
Qt3DRender::QAbstractTexture *QAbstractSpriteSheet::texture() const
{
    Q_D(const QAbstractSpriteSheet);
    return d->m_texture;
}

void QAbstractSpriteSheet::setTexture(Qt3DRender::QAbstractTexture *texture)
{
    Q_D(QAbstractSpriteSheet);
    if (d->m_texture != texture) {
        d->updateTexture(texture);
        d->updateSizes();
        emit textureChanged(d->m_texture);
    }
}

QMatrix3x3 QAbstractSpriteSheet::textureTransform() const
{
    Q_D(const QAbstractSpriteSheet);
    return d->m_textureTransform;
}

int QAbstractSpriteSheet::currentIndex() const
{
    Q_D(const QAbstractSpriteSheet);
    return d->m_currentIndex;
}

void QAbstractSpriteSheet::setCurrentIndex(int currentIndex)
{
    Q_D(QAbstractSpriteSheet);
    d->updateIndex(currentIndex);
}

} // namespace Qt3DExtras

QT_END_NAMESPACE
