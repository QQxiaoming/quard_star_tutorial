/****************************************************************************
**
** Copyright (C) 2017 BogDan Vatra <bogdan@kde.org>
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qandroidfunctions.h"
#include "qandroidactivityresultreceiver.h"
#include "qandroidactivityresultreceiver_p.h"
#include "qandroidintent.h"
#include "qandroidserviceconnection.h"

#include <QtCore/private/qjni_p.h>
#include <QtCore/private/qjnihelpers_p.h>

#include <jni/qandroidjnienvironment.h>

QT_BEGIN_NAMESPACE

/*!
    \namespace QtAndroid
    \inmodule QtAndroidExtras
    \since 5.3
    \brief The QtAndroid namespace provides miscellaneous functions to aid Android development.
    \inheaderfile QtAndroid
*/

/*!
    \since 5.10
    \enum QtAndroid::BindFlag

    This enum is used with QtAndroid::bindService to describe the mode in which the
    binding is performed.

    \value None                 No options.
    \value AutoCreate           Automatically creates the service as long as the binding exist.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_AUTO_CREATE}
                                {BIND_AUTO_CREATE} documentation for more details.
    \value DebugUnbind          Include debugging help for mismatched calls to unbind.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_DEBUG_UNBIND}
                                {BIND_DEBUG_UNBIND} documentation for more details.
    \value NotForeground        Don't allow this binding to raise the target service's process to the foreground scheduling priority.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_NOT_FOREGROUND}
                                {BIND_NOT_FOREGROUND} documentation for more details.
    \value AboveClient          Indicates that the client application binding to this service considers the service to be more important than the app itself.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_ABOVE_CLIENT}
                                {BIND_ABOVE_CLIENT} documentation for more details.
    \value AllowOomManagement   Allow the process hosting the bound service to go through its normal memory management.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_ALLOW_OOM_MANAGEMENT}
                                {BIND_ALLOW_OOM_MANAGEMENT} documentation for more details.
    \value WaivePriority        Don't impact the scheduling or memory management priority of the target service's hosting process.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_WAIVE_PRIORITY}
                                {BIND_WAIVE_PRIORITY} documentation for more details.
    \value Important            This service is assigned a higher priority so that it is available to the client when needed.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_IMPORTANT}
                                {BIND_IMPORTANT} documentation for more details.
    \value AdjustWithActivity   If binding from an activity, allow the target service's process importance to be raised based on whether the activity is visible to the user.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_ADJUST_WITH_ACTIVITY}
                                {BIND_ADJUST_WITH_ACTIVITY} documentation for more details.
    \value ExternalService      The service being bound is an isolated, external service.
                                See \l {https://developer.android.com/reference/android/content/Context.html#BIND_EXTERNAL_SERVICE}
                                {BIND_EXTERNAL_SERVICE} documentation for more details.
*/

/*!
    \since 5.3
    \fn QAndroidJniObject QtAndroid::androidActivity()

    Returns a handle to this application's main Activity

    \sa QAndroidJniObject, androidService(), androidContext()
*/
QAndroidJniObject QtAndroid::androidActivity()
{
    return QtAndroidPrivate::activity();
}

/*!
    \since 5.7
    \fn QAndroidJniObject QtAndroid::androidService()

    Returns a handle to this application's main Service

    \sa QAndroidJniObject, androidActivity(), androidContext()
*/
QAndroidJniObject QtAndroid::androidService()
{
    return QtAndroidPrivate::service();
}

/*!
    \since 5.8
    \fn QAndroidJniObject QtAndroid::androidContext()

    Returns a handle to this application's main Context. Depending on the nature of
    the application the Context object is either the main Service or Activity
    object.

    \sa QAndroidJniObject, androidActivity(), androidService()
*/
QAndroidJniObject QtAndroid::androidContext()
{
    return QtAndroidPrivate::context();
}

/*!
    \since 5.3
    \fn int QtAndroid::androidSdkVersion()

    Returns the Android SDK version. This is also known as the API level.
*/
int QtAndroid::androidSdkVersion()
{
    return QtAndroidPrivate::androidSdkVersion();
}

/*!
  \since 5.3

  Starts the activity given by \a intent and provides the result asynchronously through the
  \a resultReceiver if this is non-null.

  If \a resultReceiver is null, then the \c startActivity() method in the \c androidActivity()
  will be called. Otherwise \c startActivityForResult() will be called.

  The \a receiverRequestCode is a request code unique to the \a resultReceiver, and will be
  returned along with the result, making it possible to use the same receiver for more than
  one intent.

 */
void QtAndroid::startActivity(const QAndroidJniObject &intent,
                              int receiverRequestCode,
                              QAndroidActivityResultReceiver *resultReceiver)
{
    QAndroidJniObject activity = androidActivity();
    if (resultReceiver != 0) {
        QAndroidActivityResultReceiverPrivate *resultReceiverD = QAndroidActivityResultReceiverPrivate::get(resultReceiver);
        activity.callMethod<void>("startActivityForResult",
                                  "(Landroid/content/Intent;I)V",
                                  intent.object<jobject>(),
                                  resultReceiverD->globalRequestCode(receiverRequestCode));
    } else {
        activity.callMethod<void>("startActivity",
                                  "(Landroid/content/Intent;)V",
                                  intent.object<jobject>());
    }
}

/*!
  \since 5.3

  Starts the activity given by \a intentSender and provides the result asynchronously through the
  \a resultReceiver if this is non-null.

  If \a resultReceiver is null, then the \c startIntentSender() method in the \c androidActivity()
  will be called. Otherwise \c startIntentSenderForResult() will be called.

  The \a receiverRequestCode is a request code unique to the \a resultReceiver, and will be
  returned along with the result, making it possible to use the same receiver for more than
  one intent.

*/
void QtAndroid::startIntentSender(const QAndroidJniObject &intentSender,
                                  int receiverRequestCode,
                                  QAndroidActivityResultReceiver *resultReceiver)
{
    QAndroidJniObject activity = androidActivity();
    if (resultReceiver != 0) {
        QAndroidActivityResultReceiverPrivate *resultReceiverD = QAndroidActivityResultReceiverPrivate::get(resultReceiver);
        activity.callMethod<void>("startIntentSenderForResult",
                                  "(Landroid/content/IntentSender;ILandroid/content/Intent;III)V",
                                  intentSender.object<jobject>(),
                                  resultReceiverD->globalRequestCode(receiverRequestCode),
                                  0,  // fillInIntent
                                  0,  // flagsMask
                                  0,  // flagsValues
                                  0); // extraFlags
    } else {
        activity.callMethod<void>("startIntentSender",
                                  "(Landroid/content/IntentSender;Landroid/content/Intent;III)V",
                                  intentSender.object<jobject>(),
                                  0,  // fillInIntent
                                  0,  // flagsMask
                                  0,  // flagsValues
                                  0); // extraFlags

    }

}

/*!
    \typedef QtAndroid::Runnable

    Synonym for std::function<void()>.
*/

/*!
  \since 5.7
  \fn void QtAndroid::runOnAndroidThread(const Runnable &runnable)

  Posts the given \a runnable on the android thread.
  The \a runnable will be queued and executed on the Android UI thread, unless it called on the
  Android UI thread, in which case the runnable will be executed immediately.

  This function is useful to set asynchronously properties of objects that must be set on on Android UI thread.
*/
void QtAndroid::runOnAndroidThread(const QtAndroid::Runnable &runnable)
{
    QtAndroidPrivate::runOnAndroidThread(runnable, QJNIEnvironmentPrivate());
}

/*!
  \since 5.7
  \fn void QtAndroid::runOnAndroidThreadSync(const Runnable &runnable, int timeoutMs)

  Posts the \a runnable on the Android UI thread and waits until the runnable is executed,
  or until \a timeoutMs has passed

  This function is useful to create objects, or get properties on Android UI thread:

  \code
    QAndroidJniObject javaControl;
    QtAndroid::runOnAndroidThreadSync([&javaControl](){

        // create our Java control on Android UI thread.
        javaControl = QAndroidJniObject("android/webkit/WebView",
                                                    "(Landroid/content/Context;)V",
                                                    QtAndroid::androidActivity().object<jobject>());
        javaControl.callMethod<void>("setWebViewClient",
                                       "(Landroid/webkit/WebViewClient;)V",
                                       QAndroidJniObject("android/webkit/WebViewClient").object());
    });

    // Continue the execution normally
    qDebug() << javaControl.isValid();
  \endcode
*/
void QtAndroid::runOnAndroidThreadSync(const QtAndroid::Runnable &runnable, int timeoutMs)
{
    QtAndroidPrivate::runOnAndroidThreadSync(runnable, QJNIEnvironmentPrivate(), timeoutMs);
}


/*!
  \since 5.7
  \fn void QtAndroid::hideSplashScreen()

  Hides the splash screen immediately.
*/
void QtAndroid::hideSplashScreen()
{
    hideSplashScreen(0);
}

/*!
  \since 5.10
  \fn void QtAndroid::hideSplashScreen(int duration)

  Hides the splash screen, fading it for \a duration milliseconds.
*/
void QtAndroid::hideSplashScreen(int duration)
{
    QtAndroidPrivate::hideSplashScreen(QJNIEnvironmentPrivate(), duration);
}


/*!
    \since 5.10
    \fn bool QtAndroid::bindService(const QAndroidIntent &serviceIntent, const QAndroidServiceConnection &serviceConnection, BindFlags flags = BindFlag::None)

    Binds the service given by \a serviceIntent, \a serviceConnection and \a flags.
    The \a serviceIntent object identifies the service to connect to.
    The \a serviceConnection is a listener that receives the information as the service is started and stopped.

    \return true on success

    See \l {https://developer.android.com/reference/android/content/Context.html#bindService%28android.content.Intent,%20android.content.ServiceConnection,%20int%29}
    {Android documentation} documentation for more details.

    \sa QAndroidIntent, QAndroidServiceConnection, BindFlag
*/
bool QtAndroid::bindService(const QAndroidIntent &serviceIntent,
                            const QAndroidServiceConnection &serviceConnection, BindFlags flags)
{
    QAndroidJniExceptionCleaner cleaner;
    return androidContext().callMethod<jboolean>("bindService", "(Landroid/content/Intent;Landroid/content/ServiceConnection;I)Z",
                                                 serviceIntent.handle().object(), serviceConnection.handle().object(), jint(flags));
}

static QtAndroid::PermissionResultMap privateToPublicPermissionsHash(const QtAndroidPrivate::PermissionsHash &privateHash)
{
    QtAndroid::PermissionResultMap hash;
    for (auto it = privateHash.constBegin(); it != privateHash.constEnd(); ++it)
        hash[it.key()] = QtAndroid::PermissionResult(it.value());
    return hash;
}

/*!
  \since 5.10
  \enum QtAndroid::PermissionResult

  This enum is used to describe the permission status.

  \value Granted    The permission was granted.
  \value Denied     The permission was denied.
*/

/*!
    \typedef QtAndroid::PermissionResultMap

    Synonym for QHash<QString, PermissionResult>.
*/

/*!
    \typedef QtAndroid::PermissionResultCallback

    Synonym for std::function<void(const PermissionResultMap &)>.
*/

/*!
  \since 5.10
  \fn void QtAndroid::requestPermissions(const QStringList &permissions, const PermissionResultCallback &callbackFunc)

  Asynchronously requests \a permissions to be granted to this application, \a callbackFunc will be called with the results.
*/
void QtAndroid::requestPermissions(const QStringList &permissions, const QtAndroid::PermissionResultCallback &callbackFunc)
{
    QtAndroidPrivate::requestPermissions(QJNIEnvironmentPrivate(), permissions,
                                         [callbackFunc](const QtAndroidPrivate::PermissionsHash &privatePerms){
                                            callbackFunc(privateToPublicPermissionsHash(privatePerms));
                                         }, false);
}

/*!
  \since 5.10
  \fn QtAndroid::PermissionResultMap QtAndroid::requestPermissionsSync(const QStringList &permissions, int timeoutMs)

  Synchronously requests \a permissions to be granted to this application, waits \a timeoutMs to complete.
 */
QtAndroid::PermissionResultMap QtAndroid::requestPermissionsSync(const QStringList &permissions, int timeoutMs)
{
    return privateToPublicPermissionsHash(QtAndroidPrivate::requestPermissionsSync(QJNIEnvironmentPrivate(), permissions, timeoutMs));
}

/*!
  \since 5.10
  \fn QtAndroid::PermissionResult QtAndroid::checkPermission(const QString &permission)

  Checks if the \a permission was granted or not. This function should be called every time when
  the application starts for needed permissions, as the users might disable them from Android Settings.
 */
QtAndroid::PermissionResult QtAndroid::checkPermission(const QString &permission)
{
    return QtAndroid::PermissionResult(QtAndroidPrivate::checkPermission(permission));
}

/*!
  \since 5.10
  \fn bool QtAndroid::shouldShowRequestPermissionRationale(const QString &permission)

  Returns \c true if you should show UI with a rationale for requesting a \a permission.
*/
bool QtAndroid::shouldShowRequestPermissionRationale(const QString &permission)
{
    return QtAndroidPrivate::shouldShowRequestPermissionRationale(permission);
}
QT_END_NAMESPACE
