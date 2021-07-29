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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "androidcompass.h"

#include <qmath.h>

#include "sensormanager.h"

AndroidCompass::AndroidCompass(QSensor *sensor, QObject *parent)
    : ThreadSafeSensorBackend(sensor, parent)
{
    setDescription("Compass");
    setReading<QCompassReading>(&m_reading);
    memset(&m_accelerometerEvent, 0, sizeof(ASensorVector));
    memset(&m_magneticEvent, 0, sizeof(ASensorVector));
    m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager->manager(), m_sensorManager->looper(), -1, &looperCallback, this);
    m_accelerometer = ASensorManager_getDefaultSensor(m_sensorManager->manager(), ASENSOR_TYPE_ACCELEROMETER);
    m_magnetometer = ASensorManager_getDefaultSensor(m_sensorManager->manager(), ASENSOR_TYPE_MAGNETIC_FIELD);
}

AndroidCompass::~AndroidCompass()
{
    stop();
    ASensorManager_destroyEventQueue(m_sensorManager->manager(), m_sensorEventQueue);
}

void AndroidCompass::start()
{
    ASensorEventQueue_enableSensor(m_sensorEventQueue, m_accelerometer);
    if (sensor()->dataRate() > 0)
        ASensorEventQueue_setEventRate(m_sensorEventQueue, m_accelerometer, std::max(ASensor_getMinDelay(m_accelerometer), sensor()->dataRate()));

    ASensorEventQueue_enableSensor(m_sensorEventQueue, m_magnetometer);
    if (sensor()->dataRate() > 0)
        ASensorEventQueue_setEventRate(m_sensorEventQueue, m_magnetometer, std::max(ASensor_getMinDelay(m_magnetometer), sensor()->dataRate()));
}

void AndroidCompass::stop()
{
    ASensorEventQueue_disableSensor(m_sensorEventQueue, m_accelerometer);
    ASensorEventQueue_disableSensor(m_sensorEventQueue, m_magnetometer);
}

void AndroidCompass::readAllEvents()
{
    {
        ASensorEvent sensorEvent;
        QMutexLocker lock(&m_sensorsMutex);
        while (ASensorEventQueue_getEvents(m_sensorEventQueue, &sensorEvent, 1)) {
            switch (sensorEvent.type) {
            case ASENSOR_TYPE_ACCELEROMETER:
                m_accelerometerEvent = sensorEvent.acceleration;
                m_accelerometerEvent.status = m_accelerometerEvent.status == ASENSOR_STATUS_NO_CONTACT ? 0 : m_accelerometerEvent.status;
                break;
            case ASENSOR_TYPE_MAGNETIC_FIELD:
                m_magneticEvent = sensorEvent.magnetic;
                m_magneticEvent.status = m_magneticEvent.status == ASENSOR_STATUS_NO_CONTACT ? 0 : m_magneticEvent.status;
                break;
            }
        }
    }

    QCoreApplication::postEvent(this, new FunctionEvent{[=]() {
        // merged getRotationMatrix https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/SensorManager.java#1182
        // and getOrientation https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/SensorManager.java#1477
        QMutexLocker lock(&m_sensorsMutex);
        auto Ax = qreal(m_accelerometerEvent.x);
        auto Ay = qreal(m_accelerometerEvent.y);
        auto Az = qreal(m_accelerometerEvent.z);

        const qreal normsqA = (Ax * Ax + Ay * Ay + Az * Az);
        const auto g = qreal(ASENSOR_STANDARD_GRAVITY);
        const qreal freeFallGravitySquared = 0.01 * g * g;
        if (normsqA < freeFallGravitySquared)
            return;

        auto Ex = qreal(m_magneticEvent.x);
        auto Ey = qreal(m_magneticEvent.y);
        auto Ez = qreal(m_magneticEvent.z);
        qreal Hx = Ey * Az - Ez * Ay;
        qreal Hy = Ez * Ax - Ex * Az;
        qreal Hz = Ex * Ay - Ey * Ax;
        const qreal normH = std::sqrt(Hx * Hx + Hy * Hy + Hz * Hz);

        if (normH < 0.1)
            return;
        const qreal invH = 1.0 / normH;
        Hx *= invH;
        Hy *= invH;
        Hz *= invH;
        const qreal invA = 1.0 / std::sqrt(Ax * Ax + Ay * Ay + Az * Az);
        Ax *= invA;
        Ay *= invA;
        Az *= invA;
        const qreal My = Az * Hx - Ax * Hz;
        qreal azimuth = std::atan2(Hy, My);
        qreal accuracyValue = (m_accelerometerEvent.status + m_magneticEvent.status) / 6.0;
        if (sensor()->skipDuplicates() && qFuzzyCompare(azimuth, m_reading.azimuth()) &&
                qFuzzyCompare(accuracyValue, m_reading.calibrationLevel())) {
            return;
        }
        m_reading.setAzimuth(qRadiansToDegrees(azimuth));
        m_reading.setCalibrationLevel(accuracyValue);
        newReadingAvailable();
    }});
}

int AndroidCompass::looperCallback(int, int, void *data)
{
    auto self = reinterpret_cast<AndroidCompass*>(data);
    self->readAllEvents();
    return 1; // 1 means keep receiving events
}
