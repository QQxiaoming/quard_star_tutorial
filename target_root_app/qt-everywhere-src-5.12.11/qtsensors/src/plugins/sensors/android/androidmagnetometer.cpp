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

#include "androidmagnetometer.h"

AndroidMagnetometer::AndroidMagnetometer(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QMagnetometerReading>(type, sensor, parent)
{}

void AndroidMagnetometer::dataReceived(const ASensorEvent &event)
{
    const auto &mag = event.magnetic;
    qreal accuracy = mag.status == ASENSOR_STATUS_NO_CONTACT ? 0 : mag.status / 3.0;
    // check https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_magnetic_field:
    // Android uses micro-Tesla, Qt uses Tesla
    qreal x = qreal(mag.x) / 1e6;
    qreal y = qreal(mag.y) / 1e6;
    qreal z = qreal(mag.z) / 1e6;
    if (sensor()->skipDuplicates() && qFuzzyCompare(accuracy, m_reader.calibrationLevel()) &&
            qFuzzyCompare(x, m_reader.x()) &&
            qFuzzyCompare(y, m_reader.y()) &&
            qFuzzyCompare(z, m_reader.z())) {
        return;
    }
    m_reader.setCalibrationLevel(accuracy);
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setX(x);
    m_reader.setY(y);
    m_reader.setZ(z);
    newReadingAvailable();
}
