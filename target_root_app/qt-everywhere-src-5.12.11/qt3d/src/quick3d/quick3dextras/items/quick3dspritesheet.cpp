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

#include "quick3dspritesheet_p.h"
#include <Qt3DExtras/qspritesheetitem.h>

QT_BEGIN_NAMESPACE

namespace Qt3DExtras {
namespace Extras {
namespace Quick {

Quick3DSpriteSheet::Quick3DSpriteSheet(QObject *parent)
    : QObject(parent)
{
}

Quick3DSpriteSheet::~Quick3DSpriteSheet()
{
}

QQmlListProperty<Qt3DExtras::QSpriteSheetItem> Quick3DSpriteSheet::sprites()
{
    return QQmlListProperty<Qt3DExtras::QSpriteSheetItem>(this, 0,
                                                          &Quick3DSpriteSheet::appendSprite,
                                                          &Quick3DSpriteSheet::spriteCount,
                                                          &Quick3DSpriteSheet::spriteAt,
                                                          &Quick3DSpriteSheet::clearSprites);

}

void Quick3DSpriteSheet::appendSprite(QQmlListProperty<Qt3DExtras::QSpriteSheetItem> *list,
                                      Qt3DExtras::QSpriteSheetItem *sprite)
{
    Quick3DSpriteSheet *spritesheet = qobject_cast<Quick3DSpriteSheet *>(list->object);
    spritesheet->parentSpriteSheet()->addSprite(sprite);
}

Qt3DExtras::QSpriteSheetItem *Quick3DSpriteSheet::spriteAt(QQmlListProperty<Qt3DExtras::QSpriteSheetItem> *list, int index)
{
    Quick3DSpriteSheet *spritesheet = qobject_cast<Quick3DSpriteSheet *>(list->object);
    return spritesheet->parentSpriteSheet()->sprites().at(index);
}

int Quick3DSpriteSheet::spriteCount(QQmlListProperty<Qt3DExtras::QSpriteSheetItem> *list)
{
    Quick3DSpriteSheet *spritesheet = qobject_cast<Quick3DSpriteSheet *>(list->object);
    return spritesheet->parentSpriteSheet()->sprites().count();
}

void Quick3DSpriteSheet::clearSprites(QQmlListProperty<Qt3DExtras::QSpriteSheetItem> *list)
{
    Quick3DSpriteSheet *spritesheet = qobject_cast<Quick3DSpriteSheet *>(list->object);
    const auto sprites = spritesheet->parentSpriteSheet()->sprites();
    for (QSpriteSheetItem *sprite : sprites)
        spritesheet->parentSpriteSheet()->removeSprite(sprite);
}

} // namespace Quick
} // namespace Extras
} // namespace Qt3DExtras

QT_END_NAMESPACE

