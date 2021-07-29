/****************************************************************************
**
** Copyright (C) 2017 QNX Software Systems. All rights reserved.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
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

#include "qmediacontrol_p.h"
#include "qcustomaudiorolecontrol.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCustomAudioRoleControl
    \inmodule QtMultimedia
    \ingroup multimedia_control
    \since 5.11

    \brief The QCustomAudioRoleControl class provides control over the audio role of a media object.

    If a QMediaService supports audio roles it may implement QCustomAudioRoleControl in order to
    provide access to roles unknown to Qt.

    The functionality provided by this control is exposed to application code through the
    QMediaPlayer class.

    The interface name of QCustomAudioRoleControl is \c org.qt-project.qt.customaudiorolecontrol/5.11 as
    defined in QCustomAudioRoleControl_iid.

    \sa QMediaService::requestControl(), QMediaPlayer
*/

/*!
    \macro QCustomAudioRoleControl_iid

    \c org.qt-project.qt.customaudiorolecontrol/5.11

    Defines the interface name of the QCustomAudioRoleControl class.

    \relates QCustomAudioRoleControl
*/

/*!
    Construct a QCustomAudioRoleControl with the given \a parent.
*/
QCustomAudioRoleControl::QCustomAudioRoleControl(QObject *parent)
    : QMediaControl(*new QMediaControlPrivate, parent)
{
}

/*!
    Destroys the audio role control.
*/
QCustomAudioRoleControl::~QCustomAudioRoleControl()
{
}

/*!
    \fn QAudio::Role QCustomAudioRoleControl::customAudioRole() const

    Returns the audio role of the media played by the media service.
*/

/*!
    \fn void QCustomAudioRoleControl::setCustomAudioRole(const QString &role)

    Sets the audio \a role of the media played by the media service.
*/

/*!
    \fn QStringList QCustomAudioRoleControl::supportedCustomAudioRoles() const

    Returns a list of custom audio roles that the media service supports. An
    empty list may indicate that the supported custom audio roles aren't known.
    The list may not be complete.
*/

/*!
    \fn void QCustomAudioRoleControl::customAudioRoleChanged(const QString &role)

    Signal emitted when the audio \a role has changed.
 */

#include "moc_qcustomaudiorolecontrol.cpp"
QT_END_NAMESPACE
