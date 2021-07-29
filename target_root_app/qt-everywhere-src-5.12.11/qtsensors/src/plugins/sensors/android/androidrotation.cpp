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

#include "androidrotation.h"

#include <QtCore/qmath.h>

AndroidRotation::AndroidRotation(int type, QSensor *sensor, QObject *parent)
    : SensorEventQueue<QRotationReading>(type, sensor, parent)
{}


void AndroidRotation::dataReceived(const ASensorEvent &event)
{
    // ### Check me, at first look it seems wrong,
    // here https://developer.android.com/reference/android/hardware/SensorEvent.html#sensor.type_rotation_vector:
    // are the Android values
    qreal rz = -qRadiansToDegrees(qreal(event.data[0])); // event.data[0] corresponds to x
    qreal rx = -qRadiansToDegrees(qreal(event.data[1])); // event.data[1] corresponds to y
    qreal ry =  qRadiansToDegrees(qreal(event.data[2])); // event.data[2] corresponds to z
    if (sensor()->skipDuplicates() && qFuzzyCompare(m_reader.x(), rx) &&
            qFuzzyCompare(m_reader.y(), ry) &&
            qFuzzyCompare(m_reader.z(), rz)) {
        return;
    }
    m_reader.setTimestamp(uint64_t(event.timestamp / 1000));
    m_reader.setFromEuler(rx, ry, rz);
    newReadingAvailable();
}
