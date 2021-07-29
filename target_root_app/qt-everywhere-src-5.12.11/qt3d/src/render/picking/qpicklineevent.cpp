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

#include "qpicklineevent.h"
#include "qpickevent_p.h"
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

class QPickLineEventPrivate : public QPickEventPrivate
{
public:
    QPickLineEventPrivate()
        : QPickEventPrivate()
        , m_edgeIndex(0)
        , m_vertex1Index(0)
        , m_vertex2Index(0)
    {
    }

    uint m_edgeIndex;
    uint m_vertex1Index;
    uint m_vertex2Index;
};

/*!
    \class Qt3DRender::QPickLineEvent
    \inmodule Qt3DRender

    \brief The QPickLineEvent class holds information when a segment of a line is picked.

    \sa QPickEvent
    \since 5.10
*/

/*!
 * \qmltype PickLineEvent
 * \instantiates Qt3DRender::QPickLineEvent
 * \inqmlmodule Qt3D.Render
 * \brief PickLineEvent holds information when a segment of a line is picked.
 * \sa ObjectPicker
 */


/*!
  \fn Qt3DRender::QPickLineEvent::QPickLineEvent()
  Constructs a new QPickEvent.
 */
QPickLineEvent::QPickLineEvent()
    : QPickEvent(*new QPickLineEventPrivate())
{
}

QPickLineEvent::QPickLineEvent(const QPointF &position, const QVector3D &worldIntersection,
                               const QVector3D &localIntersection, float distance,
                               uint edgeIndex, uint vertex1Index, uint vertex2Index,
                               QPickEvent::Buttons button, int buttons, int modifiers)
    : QPickEvent(*new QPickLineEventPrivate())
{
    Q_D(QPickLineEvent);
    d->m_position = position;
    d->m_distance = distance;
    d->m_worldIntersection = worldIntersection;
    d->m_localIntersection = localIntersection;
    d->m_edgeIndex = edgeIndex;
    d->m_vertex1Index = vertex1Index;
    d->m_vertex2Index = vertex2Index;
    d->m_button = button;
    d->m_buttons = buttons;
    d->m_modifiers = modifiers;
}

/*! \internal */
QPickLineEvent::~QPickLineEvent()
{
}

/*!
    \qmlproperty uint Qt3D.Render::PickLineEvent::triangleIndex
    Specifies the triangle index of the event
*/
/*!
  \property Qt3DRender::QPickLineEvent::edgeIndex
    Specifies the index of the edge that was picked
 */
/*!
 * \brief QPickLineEvent::edgeIndex
 * Returns the index of the picked edge
 */
uint QPickLineEvent::edgeIndex() const
{
    Q_D(const QPickLineEvent);
    return d->m_edgeIndex;
}

/*!
    \qmlproperty uint Qt3D.Render::PickLineEvent::vertex1Index
    Specifies the index of the first point of the picked edge
*/
/*!
  \property Qt3DRender::QPickLineEvent::vertex1Index
    Specifies the index of the first point of the picked edge
 */
/*!
 * \brief QPickLineEvent::vertex1Index
 * Returns the index of the first point of the picked edge
 */
uint QPickLineEvent::vertex1Index() const
{
    Q_D(const QPickLineEvent);
    return d->m_vertex1Index;
}

/*!
    \qmlproperty uint Qt3D.Render::PickLineEvent::vertex2Index
    Specifies the index of the second point of the picked edge
*/
/*!
  \property Qt3DRender::QPickLineEvent::vertex2Index
    Specifies the index of the second point of the picked edge
 */
/*!
 * \brief QPickLineEvent::vertex2Index
 * Returns the index of the second point of the picked triangle
 */
uint QPickLineEvent::vertex2Index() const
{
    Q_D(const QPickLineEvent);
    return d->m_vertex2Index;
}

} // Qt3DRender

QT_END_NAMESPACE

