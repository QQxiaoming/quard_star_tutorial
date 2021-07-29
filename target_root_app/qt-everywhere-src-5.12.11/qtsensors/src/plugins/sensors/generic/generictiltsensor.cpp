/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "generictiltsensor.h"
#include <QDebug>
#include <qmath.h>

char const * const GenericTiltSensor::id("generic.tilt");

GenericTiltSensor::GenericTiltSensor(QSensor *sensor)
    : QSensorBackend(sensor)
    , radAccuracy(qDegreesToRadians(qreal(1)))
    , pitch(0)
    , roll(0)
    , calibratedPitch(0)
    , calibratedRoll(0)
    , xRotation(0)
    , yRotation(0)
{
    accelerometer = new QAccelerometer(this);
    accelerometer->addFilter(this);
    accelerometer->connectToBackend();

    setReading<QTiltReading>(&m_reading);
    setDataRates(accelerometer);
}

void GenericTiltSensor::start()
{
    accelerometer->setDataRate(sensor()->dataRate());
    accelerometer->setAlwaysOn(sensor()->isAlwaysOn());
    accelerometer->start();
    if (!accelerometer->isActive())
        sensorStopped();
    if (accelerometer->isBusy())
        sensorBusy();
}

void GenericTiltSensor::stop()
{
    accelerometer->stop();
}

/*
  Angle between Ground and X
*/
static inline qreal calcPitch(double Ax, double Ay, double Az)
{
    return qAtan2(-Ax, qSqrt(Ay * Ay + Az * Az));
}

/*
  Angle between Ground and Y
*/
static inline qreal calcRoll(double /*Ax*/, double Ay, double Az)
{
    return qAtan2(Ay, Az);
}

void GenericTiltSensor::calibrate()
{
    calibratedPitch = pitch;
    calibratedRoll = roll;
}

bool GenericTiltSensor::filter(QAccelerometerReading *reading)
{
    /*
      z  y
      | /
      |/___ x
    */

    qreal ax = reading->x();
    qreal ay = reading->y();
    qreal az = reading->z();
#ifdef LOGCALIBRATION
    qDebug() << "------------ new value -----------";
    qDebug() << "old _pitch: " << pitch;
    qDebug() << "old _roll: " << roll;
    qDebug() << "_calibratedPitch: " << calibratedPitch;
    qDebug() << "_calibratedRoll: " << calibratedRoll;
#endif
    pitch = calcPitch(ax, ay, az);
    roll  = calcRoll (ax, ay, az);
#ifdef LOGCALIBRATION
    qDebug() << "_pitch: " << pitch;
    qDebug() << "_roll: " << roll;
#endif
    qreal xrot = roll - calibratedRoll;
    qreal yrot = pitch - calibratedPitch;
    //get angle between 0 and 180 or 0 -180
    xrot = qAtan2(qSin(xrot), qCos(xrot));
    yrot = qAtan2(qSin(yrot), qCos(yrot));

#ifdef LOGCALIBRATION
    qDebug() << "new xrot: " << xrot;
    qDebug() << "new yrot: " << yrot;
    qDebug() << "----------------------------------";
#endif
    qreal dxrot = qRadiansToDegrees(xrot) - xRotation;
    qreal dyrot = qRadiansToDegrees(yrot) - yRotation;
    if (dxrot < 0) dxrot = -dxrot;
    if (dyrot < 0) dyrot = -dyrot;

    bool setNewReading = false;
    if (dxrot >= qRadiansToDegrees(radAccuracy) || !sensor()->skipDuplicates()) {
        xRotation = qRadiansToDegrees(xrot);
        setNewReading = true;
    }
    if (dyrot >= qRadiansToDegrees(radAccuracy) || !sensor()->skipDuplicates()) {
        yRotation = qRadiansToDegrees(yrot);
        setNewReading = true;
    }

    if (setNewReading || m_reading.timestamp() == 0) {
        m_reading.setTimestamp(reading->timestamp());
        m_reading.setXRotation(xRotation);
        m_reading.setYRotation(yRotation);
        newReadingAvailable();
    }

    return false;
}

bool GenericTiltSensor::isFeatureSupported(QSensor::Feature feature) const
{
    return (feature == QSensor::SkipDuplicates);
}
