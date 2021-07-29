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

#include "qandroidintent.h"

#include <jni/qandroidjnienvironment.h>
#include <QtCore/private/qjni_p.h>
#include <QBuffer>
#include <QDataStream>
#include <QVariant>

QT_BEGIN_NAMESPACE

/*!
    \class QAndroidIntent
    \inmodule QtAndroidExtras

    The QAndroidIntent is a convenience class that wraps the most important
    \l {https://developer.android.com/reference/android/content/Intent.html} {Android Intent} methods.

    \since 5.10
*/

/*!
    Create a new intent
 */
QAndroidIntent::QAndroidIntent()
    : m_handle("android.content.Intent", "()V")
{

}

QAndroidIntent::~QAndroidIntent()
{}

/*!
    Wraps the provided \a intent java object.
 */
QAndroidIntent::QAndroidIntent(const QAndroidJniObject &intent)
    : m_handle(intent)
{
}

/*!
    Creates a new intent and sets the provided \a action.
 */
QAndroidIntent::QAndroidIntent(const QString &action)
    : m_handle("android.content.Intent", "(Ljava/lang/String;)V",
                        QAndroidJniObject::fromString(action).object())
{
    QAndroidJniExceptionCleaner cleaner;
}

/*!
    Creates a new intent and sets the provided \a packageContext and the service \a className.
    Example:
    \code
        auto serviceIntent = QAndroidIntent(QtAndroid::androidActivity().object(), "com.example.MyService");
    \endcode

    \sa QtAndroid::androidActivity, QtAndroid::bindService
 */
QAndroidIntent::QAndroidIntent(const QAndroidJniObject &packageContext, const char *className)
    : m_handle("android/content/Intent", "(Landroid/content/Context;Ljava/lang/Class;)V",
                        packageContext.object(), QJNIEnvironmentPrivate::findClass(className))
{
    QAndroidJniExceptionCleaner cleaner;
}

/*!
    Sets the \a key with the \a data in the Intent extras
 */
void QAndroidIntent::putExtra(const QString &key, const QByteArray &data)
{
    QAndroidJniExceptionCleaner cleaner;
    QAndroidJniEnvironment env;
    jbyteArray array = env->NewByteArray(data.size());
    env->SetByteArrayRegion(array, 0, data.length(), reinterpret_cast<const jbyte*>(data.constData()));
    m_handle.callObjectMethod("putExtra", "(Ljava/lang/String;[B)Landroid/content/Intent;",
                              QAndroidJniObject::fromString(key).object(), array);
    env->DeleteLocalRef(array);
}

/*!
    Returns the extra \a key data from the Intent extras
 */
QByteArray QAndroidIntent::extraBytes(const QString &key)
{
    QAndroidJniExceptionCleaner cleaner;
    auto array = m_handle.callObjectMethod("getByteArrayExtra", "(Ljava/lang/String;)[B",
                                           QAndroidJniObject::fromString(key).object());
    if (!array.isValid() || !array.object())
        return QByteArray();
    QAndroidJniEnvironment env;
    auto sz = env->GetArrayLength(jarray(array.object()));
    QByteArray res(sz, Qt::Initialization::Uninitialized);
    env->GetByteArrayRegion(jbyteArray(array.object()), 0, sz, reinterpret_cast<jbyte *>(res.data()));
    return res;
}

/*!
    Sets the \a key with the \a value in the Intent extras.
 */
void QAndroidIntent::putExtra(const QString &key, const QVariant &value)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::WriteOnly);
    stream << value;
    putExtra(key, buff);
}

/*!
    Returns the extra \a key data from the Intent extras as a QVariant
 */
QVariant QAndroidIntent::extraVariant(const QString &key)
{
    QDataStream stream(extraBytes(key));
    QVariant res;
    stream >> res;
    return res;
}

/*!
    The return value is useful to call other Java API which are not covered by this wrapper
 */
QAndroidJniObject QAndroidIntent::handle() const
{
    return m_handle;
}
QT_END_NAMESPACE
