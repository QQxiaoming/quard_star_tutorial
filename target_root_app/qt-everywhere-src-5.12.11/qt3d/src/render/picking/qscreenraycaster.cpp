/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
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

#include "qscreenraycaster.h"
#include "qabstractraycaster_p.h"
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qcomponent_p.h>
#include <Qt3DCore/private/qscene_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

/*!
    \class Qt3DRender::QScreenRayCaster
    \brief Performe ray casting test based on screen coordinates.
    \inmodule Qt3DRender
    \since 5.11
    \inherits QAbstractRayCaster

    QScreenRayCaster can be used to perform ray casting tests by specifying coordinates in
    screen space, which will be used to construct an actual 3D ray between the near and
    far planes.

    \sa QRayCaster
*/
/*!
    \qmltype ScreenRayCaster
    \brief Performe ray casting test based on screen coordinates.
    \inqmlmodule Qt3D.Render
    \since 5.11
    \instantiates Qt3DRender::QScreenRayCaster

    ScreenRayCaster can be used to perform ray casting tests by specifying coordinates in
    screen space, which will be used to construct an actual 3D ray between the near and
    far planes.

    \sa RayCaster
*/

/*!
    \property Qt3DRender::QScreenRayCaster::position

    Holds the screen space position used to compute the actual 3D ray for intersection tests.

    Note: the coordinates will be used for every available render surface as long as they are
    in the valid range.
*/
/*!
    \qmlproperty point Qt3D.Render::ScreenRayCaster::position

    Holds the length of the 3D ray.

    \note The coordinates will be used for every available render surface as long as they are
    in the valid range.
*/
QScreenRayCaster::QScreenRayCaster(Qt3DCore::QNode *parent)
    : QAbstractRayCaster(parent)
{
    QAbstractRayCasterPrivate::get(this)->m_rayCasterType = QAbstractRayCasterPrivate::ScreenScapeRayCaster;
}

/*! \internal */
QScreenRayCaster::QScreenRayCaster(QAbstractRayCasterPrivate &dd, Qt3DCore::QNode *parent)
    : QAbstractRayCaster(dd, parent)
{
    QAbstractRayCasterPrivate::get(this)->m_rayCasterType = QAbstractRayCasterPrivate::ScreenScapeRayCaster;
}

/*! \internal */
QScreenRayCaster::~QScreenRayCaster()
{
}

QPoint QScreenRayCaster::position() const
{
    auto d = QAbstractRayCasterPrivate::get(this);
    return d->m_position;
}

void QScreenRayCaster::setPosition(const QPoint &position)
{
    auto d = QAbstractRayCasterPrivate::get(this);
    if (d->m_position != position) {
        d->m_position = position;
        emit positionChanged(d->m_position);
    }
}

/*!
  Convenience method to enable the component and trigger tests using the current coordinate value.
*/
void QScreenRayCaster::trigger()
{
    setEnabled(true);
}

/*!
  Convenience method to set the coordinate value \a position and enable the component to trigger tests.
*/
void QScreenRayCaster::trigger(const QPoint &position)
{
    setPosition(position);
    setEnabled(true);
}

} // Qt3DRender

QT_END_NAMESPACE
