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

#include "androidaccelerometer.h"
#include <QDebug>

AndroidAccelerometer::AndroidAccelerometer(int accelerationModes, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QAccelerometerReading>(ASENSOR_TYPE_ACCELEROMETER, sensor, parent)
    , m_accelerationModes(accelerationModes)
{
    auto accelerometer = qobject_cast<QAccelerometer *>(sensor);
    if (accelerometer) {
        connect(accelerometer, &QAccelerometer::accelerationModeChanged,
                this, &AndroidAccelerometer::applyAccelerationMode);
        applyAccelerationMode(accelerometer->accelerationMode());
    }
}

bool AndroidAccelerometer::isFeatureSupported(QSensor::Feature feature) const
{
    return (feature == QSensor::AccelerationMode) ? m_accelerationModes == AllModes : SensorEventQueue<QAccelerometerReading>::isFeatureSupported(feature);
}

void AndroidAccelerometer::dataReceived(const ASensorEvent &event)
{
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_accelerometer:
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_linear_acceleration:
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_gravity:
    const auto &acc = event.acceleration;
    auto x = qreal(acc.x);
    auto y = qreal(acc.y);
    auto z = qreal(acc.z);
    if (sensor()->skipDuplicates() && qFuzzyCompare(m_reader.x(), x) &&
            qFuzzyCompare(m_reader.y(), y) &&
            qFuzzyCompare(m_reader.z(), z)) {
        return;
    }
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setX(x);
    m_reader.setY(y);
    m_reader.setZ(z);
    newReadingAvailable();
}

void AndroidAccelerometer::applyAccelerationMode(QAccelerometer::AccelerationMode accelerationMode)
{
    switch (accelerationMode) {
    case QAccelerometer::Gravity:
        if (!(m_accelerationModes & Gravity)) {
            qWarning() << "Gravity sensor missing";
            return;
        }
        setSensorType(ASENSOR_TYPE_GRAVITY);
        break;
    case QAccelerometer::User:
        if (!(m_accelerationModes & LinearAcceleration)) {
            qWarning() << "Linear acceleration sensor missing";
            return;
        }
        setSensorType(ASENSOR_TYPE_LINEAR_ACCELERATION);
        break;
    case QAccelerometer::Combined:
        if (!(m_accelerationModes & Accelerometer)) {
            qWarning() << "Accelerometer sensor missing";
            return;
        }
        setSensorType(ASENSOR_TYPE_ACCELEROMETER);
        break;
    }
}
