/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2016 Canonical, Ltd.
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

#include "sensortagaccelerometer.h"
#include "sensortagals.h"
#include "sensortagbase.h"
#include "sensortaggyroscope.h"
#include "sensortaghumiditysensor.h"
#include "sensortaglightsensor.h"
#include "sensortagmagnetometer.h"
#include "sensortagpressuresensor.h"
#include "sensortagtemperaturesensor.h"

#include <QtSensors/qsensorplugin.h>
#include <QtSensors/qsensorbackend.h>
#include <QtSensors/qsensormanager.h>
#include <QSettings>

class SensortagSensorPlugin : public QObject, public QSensorPluginInterface, public QSensorBackendFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.qt-project.Qt.QSensorPluginInterface/1.0" FILE "plugin.json")
    Q_INTERFACES(QSensorPluginInterface)

public:

    void registerSensors()
    {
        if (!QSensorManager::isBackendRegistered(QAccelerometer::type, SensorTagAccelerometer::id))
            QSensorManager::registerBackend(QAccelerometer::type, SensorTagAccelerometer::id, this);

        if (!QSensorManager::isBackendRegistered(QAmbientLightSensor::type, SensorTagAls::id))
            QSensorManager::registerBackend(QAmbientLightSensor::type, SensorTagAls::id, this);

        if (!QSensorManager::isBackendRegistered(QLightSensor::type, SensorTagLightSensor::id))
            QSensorManager::registerBackend(QLightSensor::type, SensorTagLightSensor::id, this);

        if (!QSensorManager::isBackendRegistered(QAmbientTemperatureSensor::type, SensorTagTemperatureSensor::id))
            QSensorManager::registerBackend(QAmbientTemperatureSensor::type, SensorTagTemperatureSensor::id, this);

        if (!QSensorManager::isBackendRegistered(QHumiditySensor::type, SensorTagHumiditySensor::id))
            QSensorManager::registerBackend(QHumiditySensor::type, SensorTagHumiditySensor::id, this);

        if (!QSensorManager::isBackendRegistered(QPressureSensor::type, SensorTagPressureSensor::id))
            QSensorManager::registerBackend(QPressureSensor::type, SensorTagPressureSensor::id, this);

        if (!QSensorManager::isBackendRegistered(QGyroscope::type, SensorTagGyroscope::id))
            QSensorManager::registerBackend(QGyroscope::type, SensorTagGyroscope::id, this);

        if (!QSensorManager::isBackendRegistered(QMagnetometer::type, SensorTagMagnetometer::id))
            QSensorManager::registerBackend(QMagnetometer::type, SensorTagMagnetometer::id, this);
    }

    QSensorBackend *createBackend(QSensor *sensor)
    {
        if (sensor->identifier() == SensorTagAccelerometer::id)
            return new SensorTagAccelerometer(sensor);
        if (sensor->identifier() == SensorTagAls::id)
            return new SensorTagAls(sensor);
        if (sensor->identifier() == SensorTagLightSensor::id)
            return new SensorTagLightSensor(sensor);
        if (sensor->identifier() == SensorTagTemperatureSensor::id)
            return new SensorTagTemperatureSensor(sensor);
        if (sensor->identifier() == SensorTagHumiditySensor::id)
            return new SensorTagHumiditySensor(sensor);
        if (sensor->identifier() == SensorTagPressureSensor::id)
            return new SensorTagPressureSensor(sensor);
        if (sensor->identifier() == SensorTagGyroscope::id)
            return new SensorTagGyroscope(sensor);
        if (sensor->identifier() == SensorTagMagnetometer::id)
            return new SensorTagMagnetometer(sensor);
        return nullptr;
    }
};

#include "main.moc"
