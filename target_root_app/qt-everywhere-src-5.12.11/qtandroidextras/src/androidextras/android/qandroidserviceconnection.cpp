/****************************************************************************
**
** Copyright (C) 2017 BogDan Vatra <bogdan@kde.org>
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

#include "qandroidserviceconnection.h"
#include "qandroidjniobject.h"

#include <QAndroidJniEnvironment>

QT_BEGIN_NAMESPACE
/*!
    \class QAndroidServiceConnection
    \inmodule QtAndroidExtras

    The QAndroidServiceConnection is a convenience abstract class which wraps the
    \l {https://developer.android.com/reference/android/content/ServiceConnection.html} {Android ServiceConnection} interface.

    It is useful when you perform a QtAndroid::bindService operation.

    \since 5.10
*/


/*!
    Creates a new object
 */
QAndroidServiceConnection::QAndroidServiceConnection()
    : m_handle("org/qtproject/qt5/android/extras/QtAndroidServiceConnection", "(J)V", jlong(this))
{
}

/*!
    Creates a new object from an existing \a serviceConnection.

    It's useful when you have your own Java implementation.
    Of course onServiceConnected()/onServiceDisconnected()
    will not be called anymore.
 */
QAndroidServiceConnection::QAndroidServiceConnection(const QAndroidJniObject &serviceConnection)
    : m_handle(serviceConnection)
{
}

QAndroidServiceConnection::~QAndroidServiceConnection()
{
    m_handle.callMethod<void>("setId", "(J)V", jlong(this));
}

/*!
    returns the underline QAndroidJniObject
 */
QAndroidJniObject QAndroidServiceConnection::handle() const
{
    return m_handle;
}

/*!
    \fn void QAndroidServiceConnection::onServiceConnected(const QString &name, const QAndroidBinder &serviceBinder)

    This notification is called when the client managed to connect to the service.
    The \a name contains the server name, the \a serviceBinder is the binder that the client
    uses to perform IPC operations.

    \warning This method is called from Binder's thread which is different
    from the thread that this object was created.

    returns the underline QAndroidJniObject
 */

/*!
    \fn void QAndroidServiceConnection::onServiceDisconnected(const QString &name)

    Called when a connection to the Service has been lost.
    The \a name parameter specifies which connectioen was lost.

    \warning This method is called from Binder's thread which is different
    from the thread that this object was created.

    returns the underline QAndroidJniObject
 */

QT_END_NAMESPACE
