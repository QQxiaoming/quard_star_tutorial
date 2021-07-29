/****************************************************************************
**
** Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSensors module of the Qt Toolkit.
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
#include "sensormanager.h"
#include <dlfcn.h>

SensorManager::SensorManager()
{
    auto sensorService =  QJNIObjectPrivate::getStaticObjectField("android.content.Context", "SENSOR_SERVICE", "Ljava/lang/String;");
    m_sensorManager = QJNIObjectPrivate{QtAndroidPrivate::context()}.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", sensorService.object());
    setObjectName("QtSensorsLooperThread");
    start();
    m_waitForStart.acquire();
}

SensorManager::~SensorManager()
{
    m_quit.store(1);
    wait();
}

QJNIObjectPrivate SensorManager::javaSensor(const ASensor *sensor) const
{
    return m_sensorManager.callObjectMethod("getDefaultSensor", "(I)Landroid/hardware/Sensor;", ASensor_getType(sensor));
}

QSharedPointer<SensorManager> &SensorManager::instance()
{
    static QSharedPointer<SensorManager> looper{new SensorManager};
    return looper;
}

ALooper *SensorManager::looper() const
{
    return m_looper;
}

static inline ASensorManager* androidManager()
{
#if __ANDROID_API__ >= 26
    return ASensorManager_getInstanceForPackage(QJNIObjectPrivate{QtAndroidPrivate::context()}
                                                .callObjectMethod("getPackageName", "()Ljava/lang/String;")
                                                .toString().toUtf8().constData());
#else
    if (QtAndroidPrivate::androidSdkVersion() >= 26) {
        using GetInstanceForPackage = ASensorManager *(*)(const char *);
        auto handler = dlopen("libandroid.so", RTLD_NOW);
        auto function = GetInstanceForPackage(dlsym(handler, "ASensorManager_getInstanceForPackage"));
        if (function) {
            auto res = function(QJNIObjectPrivate{QtAndroidPrivate::context()}
                                .callObjectMethod("getPackageName", "()Ljava/lang/String;")
                                .toString().toUtf8().constData());
            dlclose(handler);
            return res;
        }
        dlclose(handler);
    }
    return ASensorManager_getInstance();
#endif
}
ASensorManager *SensorManager::manager() const
{
    static auto sensorManger = androidManager();
    return sensorManger;
}

QString SensorManager::description(const ASensor *sensor) const
{
    return QString::fromUtf8(ASensor_getName(sensor)) + " " + ASensor_getVendor(sensor) + " v" + QString::number(javaSensor(sensor).callMethod<jint>("getVersion"));
}

double SensorManager::getMaximumRange(const ASensor *sensor) const
{
    return qreal(javaSensor(sensor).callMethod<jfloat>("getMaximumRange"));
}

void SensorManager::run()
{
    m_looper = ALooper_prepare(0);
    m_waitForStart.release();
    do {
        if (ALooper_pollAll(5 /*ms*/, nullptr, nullptr, nullptr) == ALOOPER_POLL_TIMEOUT)
            QThread::yieldCurrentThread();
    } while (!m_quit.load());
}
