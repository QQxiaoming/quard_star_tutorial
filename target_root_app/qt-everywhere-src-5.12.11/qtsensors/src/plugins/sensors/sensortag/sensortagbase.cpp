/****************************************************************************
**
** Copyright (C) 2017 Lorn Potter
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

#include "sensortagbase.h"

#include <QLowEnergyCharacteristic>
#include <QtMath>
#include <QTimer>
#include <QDeadlineTimer>

Q_GLOBAL_STATIC(SensorTagBasePrivate, sensortagBasePrivate)

SensorTagBasePrivate::SensorTagBasePrivate(QObject *parent)
    : QObject(parent)
{
    QTimer::singleShot(50, this, &SensorTagBasePrivate::deviceSearch);
}

void SensorTagBasePrivate::deviceSearch()
{
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &SensorTagBasePrivate::deviceFound);
    connect(m_deviceDiscoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &SensorTagBasePrivate::deviceScanError);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &SensorTagBasePrivate::scanFinished);

    QTimer::singleShot(20000, this, &SensorTagBasePrivate::deviceSearchTimeout); //make sure to timeout
    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void SensorTagBasePrivate::deviceSearchTimeout()
{
    if (m_deviceDiscoveryAgent->isActive() && m_control == nullptr) {
        m_deviceDiscoveryAgent->stop();
        qWarning("No Sensor Tag devices found");
    }
}

void SensorTagBasePrivate::deviceFound(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        const QString idString = QString::fromLatin1(qgetenv("QT_SENSORTAG_ID"));

        const QBluetoothAddress watchForAddress(idString);
        //mac uses deviceUuid
        const QUuid watchForId(idString);

        bool ok;

        if ((!watchForAddress.isNull() && watchForAddress == device.address()) ||
              (!watchForId.isNull() && watchForId == device.deviceUuid())) {
            ok = true;
        }
        if (ok || device.name().contains("SensorTag")) {

            m_deviceDiscoveryAgent->stop();

            m_control = new QLowEnergyController(device.address(), this);

            connect(m_control, &QLowEnergyController::discoveryFinished,
                    this, &SensorTagBasePrivate::serviceDiscoveryFinished);
            connect(m_control, &QLowEnergyController::serviceDiscovered,
                    this, &SensorTagBasePrivate::serviceDiscovered);
            connect(m_control, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),
                    this, &SensorTagBasePrivate::controllerError);
            connect(m_control, &QLowEnergyController::connected,
                    this, &SensorTagBasePrivate::sensortagDeviceConnected);
            connect(m_control, &QLowEnergyController::disconnected,
                    this, &SensorTagBasePrivate::deviceDisconnected);

            m_control->connectToDevice();
        }
    }
}

void SensorTagBasePrivate::serviceDiscoveryFinished()
{
    discoveryDone = true;
}

void SensorTagBasePrivate::scanFinished()
{
    if (m_control == nullptr)
        qWarning("No Sensor Tag devices found");
}

void SensorTagBasePrivate::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        qWarning("The Bluetooth adaptor is powered off, power it on before doing discovery.");
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        qWarning("Writing or reading from the device resulted in an error.");
        break;
    default:
        qWarning("An unknown error has occurred.");
        break;
    };
}

void SensorTagBasePrivate::serviceError(QLowEnergyService::ServiceError e)
{
    switch (e) {
    case QLowEnergyService::DescriptorWriteError:
        qWarning("Cannot obtain SensorTag notifications");
        break;
    default:
    case QLowEnergyService::CharacteristicWriteError:
        qWarning() << "SensorTag service error:" << e;
        break;
    };
}

void SensorTagBasePrivate::controllerError(QLowEnergyController::Error error)
{
    qWarning("Cannot connect to remote device.");
    qWarning() << "Controller Error:" << error;
}

void SensorTagBasePrivate::sensortagDeviceConnected()
{
    m_control->discoverServices();
}

void SensorTagBasePrivate::deviceDisconnected()
{
    if (q_ptr && q_ptr->sensor()->isActive())
        q_ptr->sensorStopped();
}

void SensorTagBasePrivate::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (enabledServiceUuids.contains(gatt)) {

        if (gatt == TI_SENSORTAG_LIGHT_SERVICE) {
            lightService = m_control->createServiceObject(gatt, this);
            doConnections(lightService);
        } else if (gatt == TI_SENSORTAG_TEMPERATURE_SERVICE) {
            temperatureService = m_control->createServiceObject(gatt, this);
            doConnections(temperatureService);
        } else if (gatt == TI_SENSORTAG_BAROMETER_SERVICE) {
            barometerService = m_control->createServiceObject(gatt, this);
            doConnections(barometerService);
        } else if (gatt == TI_SENSORTAG_HUMIDTIY_SERVICE) {
            humidityService = m_control->createServiceObject(gatt, this);
            doConnections(humidityService);
        } else if (gatt == TI_SENSORTAG_INFO_SERVICE) {
            infoService = m_control->createServiceObject(gatt, this);
            doConnections(infoService);
        } else if (gatt == TI_SENSORTAG_ACCELEROMETER_SERVICE) {
            acceleratorService = m_control->createServiceObject(gatt, this);
            doConnections(acceleratorService);
        } else if (gatt == TI_SENSORTAG_GYROSCOPE_SERVICE) {
            gyroscopeService = m_control->createServiceObject(gatt, this);
            doConnections(gyroscopeService);
        } else if (gatt == TI_SENSORTAG_MAGNETOMETER_SERVICE) {
            magnetometerService = m_control->createServiceObject(gatt, this);
            doConnections(magnetometerService);
        } else if (movementService == nullptr) {
            if (gatt == TI_SENSORTAG_MOVEMENT_SERVICE) {
                movementService = m_control->createServiceObject(gatt, this);
                doConnections(movementService);
            }
        }
    }
}

void SensorTagBasePrivate::doConnections(QLowEnergyService *service)
{
    if (service) {
        connect(service, &QLowEnergyService::stateChanged,
                this, &SensorTagBasePrivate::serviceStateChanged);

        connect(service, &QLowEnergyService::characteristicChanged,
                this, &SensorTagBasePrivate::updateCharacteristic);

        connect(service,SIGNAL(error(QLowEnergyService::ServiceError)),
                this,SLOT(serviceError(QLowEnergyService::ServiceError)));

        if (service->state() == QLowEnergyService::DiscoveryRequired) {
            service->discoverDetails();
        } else if (!enabledServiceUuids.isEmpty()
                   && enabledServiceUuids.contains(service->serviceUuid())) {
            enableService(service->serviceUuid());
        }
    }
}

void SensorTagBasePrivate::serviceStateChanged(QLowEnergyService::ServiceState newState)
{
    if (newState != QLowEnergyService::ServiceDiscovered)
        return;

    QLowEnergyService *m_service = qobject_cast<QLowEnergyService *>(sender());

    if (!m_service)
        return;

    if (!enabledServiceUuids.isEmpty()
            && enabledServiceUuids.contains(m_service->serviceUuid())) {
        enableService(m_service->serviceUuid());
    }
}

void SensorTagBasePrivate::enableLight(bool on)
{
    if (!lightService && discoveryDone)
        serviceDiscovered(TI_SENSORTAG_LIGHT_SERVICE);
    if (!lightService)
        return;

    const QLowEnergyCharacteristic hrChar = lightService->characteristic(TI_SENSORTAG_LIGHT_CONTROL);
    lightService->writeCharacteristic(hrChar, on ? enableSensorCharacteristic : disableSensorCharacteristic);

    const QLowEnergyCharacteristic hrChar2 = lightService->characteristic(TI_SENSORTAG_LIGHT_DATA);

    if (hrChar2.descriptors().count() > 0) {
        const QLowEnergyDescriptor m_notificationDesc = hrChar2.descriptors().at(0);
        lightService->writeDescriptor(m_notificationDesc,
                                      on ? enableNotificationsCharacteristic : disableNotificationsCharacteristic);
    }
}

void SensorTagBasePrivate::enableTemp(bool on)
{
    if (!temperatureService && discoveryDone)
        serviceDiscovered(TI_SENSORTAG_TEMPERATURE_SERVICE);

    if (!temperatureService)
        return;

    const QLowEnergyCharacteristic hrChar = temperatureService->characteristic(TI_SENSORTAG_IR_TEMPERATURE_CONTROL);
    temperatureService->writeCharacteristic(hrChar,on ? enableSensorCharacteristic : disableSensorCharacteristic);

    const QLowEnergyCharacteristic hrChar2 = temperatureService->characteristic(TI_SENSORTAG_IR_TEMPERATURE_DATA);

    if (hrChar2.descriptors().count() > 0) {
        const QLowEnergyDescriptor m_notificationDesc = hrChar2.descriptors().at(0);
        temperatureService->writeDescriptor(m_notificationDesc,
                                            on ? enableNotificationsCharacteristic : disableNotificationsCharacteristic);
    }
}

void SensorTagBasePrivate::enablePressure(bool on)
{
    if (!barometerService && discoveryDone)
        serviceDiscovered(TI_SENSORTAG_BAROMETER_SERVICE);
    if (!barometerService)
        return;

    const QLowEnergyCharacteristic hrChar = barometerService->characteristic(TI_SENSORTAG_BAROMETER_CONTROL);
    barometerService->writeCharacteristic(hrChar, on ? enableSensorCharacteristic : disableSensorCharacteristic);

    const QLowEnergyCharacteristic hrChar2 = barometerService->characteristic(TI_SENSORTAG_BAROMETER_DATA);
    if (hrChar2.descriptors().count() > 0) {
        const QLowEnergyDescriptor m_notificationDesc = hrChar2.descriptors().at(0);

        barometerService->writeDescriptor(m_notificationDesc,
                                          on ? enableNotificationsCharacteristic : disableNotificationsCharacteristic);
    }
}

void SensorTagBasePrivate::enableHumidity(bool on)
{
    if (!humidityService && discoveryDone)
        serviceDiscovered(TI_SENSORTAG_HUMIDTIY_SERVICE);
    if (!humidityService)
        return;

    const QLowEnergyCharacteristic hrChar = humidityService->characteristic(TI_SENSORTAG_HUMIDTIY_CONTROL);
    humidityService->writeCharacteristic(hrChar, on ? enableSensorCharacteristic : disableSensorCharacteristic);
    const QLowEnergyCharacteristic hrChar2 = humidityService->characteristic(TI_SENSORTAG_HUMIDTIY_DATA);
    if (hrChar2.descriptors().count() > 0) {
        const QLowEnergyDescriptor m_notificationDesc = hrChar2.descriptors().at(0);
        humidityService->writeDescriptor(m_notificationDesc,
                                         on ? enableNotificationsCharacteristic : disableNotificationsCharacteristic);
    }
}

void SensorTagBasePrivate::enableMovement(bool on)
{
    if (!movementService && discoveryDone)
        serviceDiscovered(TI_SENSORTAG_MOVEMENT_SERVICE);
    if (!movementService)
        return;

    QByteArray controlCharacteristic;

    int movementControl = 0;
    //movement service has different syntax here
    if (on) {
        if (gyroscopeEnabled)
            movementControl += 7;
        if (accelerometerEnabled)
            movementControl += 56;
        if (magnetometerEnabled)
            movementControl += 64;

        controlCharacteristic = QByteArray::number(movementControl, 16);
        controlCharacteristic.append("04");

    } else {
        controlCharacteristic = "00";
    }

    const QLowEnergyCharacteristic hrChar = movementService->characteristic(TI_SENSORTAG_MOVEMENT_CONTROL);
    movementService->writeCharacteristic(hrChar, QByteArray::fromHex(controlCharacteristic));

    const QLowEnergyCharacteristic hrChar2 = movementService->characteristic(TI_SENSORTAG_MOVEMENT_DATA);
    if (hrChar2.descriptors().count() > 0) {
        QLowEnergyDescriptor m_notificationDesc = hrChar2.descriptors().at(0);

        movementService->writeDescriptor(m_notificationDesc,
                                         on ? enableNotificationsCharacteristic : disableNotificationsCharacteristic);
    }
}

void SensorTagBasePrivate::enableService(const QBluetoothUuid &uuid)
{
    if (uuid == TI_SENSORTAG_ACCELEROMETER_SERVICE
            || uuid == TI_SENSORTAG_MAGNETOMETER_SERVICE
            || uuid == TI_SENSORTAG_GYROSCOPE_SERVICE) {

        if ((uuid != TI_SENSORTAG_MOVEMENT_SERVICE)
                && (accelerometerEnabled || magnetometerEnabled || gyroscopeEnabled))
            return;

        if (!enabledServiceUuids.contains(TI_SENSORTAG_MOVEMENT_SERVICE))
            enabledServiceUuids.append(TI_SENSORTAG_MOVEMENT_SERVICE);

        if (uuid == TI_SENSORTAG_ACCELEROMETER_SERVICE)
            accelerometerEnabled = true;
        else if (uuid == TI_SENSORTAG_MAGNETOMETER_SERVICE)
            magnetometerEnabled = true;
        else if (uuid == TI_SENSORTAG_GYROSCOPE_SERVICE)
            gyroscopeEnabled = true;

    } else if (!enabledServiceUuids.contains(uuid))
        enabledServiceUuids.append(uuid);

    if (discoveryDone) {

        if (uuid == TI_SENSORTAG_LIGHT_SERVICE)
            enableLight(true);
        else if (uuid == TI_SENSORTAG_TEMPERATURE_SERVICE)
            enableTemp(true);
        else if (uuid == TI_SENSORTAG_BAROMETER_SERVICE)
            enablePressure(true);
        else if (uuid == TI_SENSORTAG_HUMIDTIY_SERVICE)
            enableHumidity(true);
        else if (uuid == TI_SENSORTAG_MOVEMENT_SERVICE)
            enableMovement(true);
        else if (uuid == TI_SENSORTAG_ACCELEROMETER_SERVICE)
            enableMovement(true);
        else if (uuid == TI_SENSORTAG_MAGNETOMETER_SERVICE)
            enableMovement(true);
        else if (uuid == TI_SENSORTAG_GYROSCOPE_SERVICE)
            enableMovement(true);
    }
}

void SensorTagBasePrivate::disableService(const QBluetoothUuid &uuid)
{
    enabledServiceUuids.removeOne(uuid);

    if (uuid == TI_SENSORTAG_LIGHT_SERVICE) {
        enableLight(false);
    } else if (uuid == TI_SENSORTAG_TEMPERATURE_SERVICE) {
        enableTemp(false);
    } else if (uuid == TI_SENSORTAG_BAROMETER_SERVICE) {
        enablePressure(false);
    } else if (uuid == TI_SENSORTAG_HUMIDTIY_SERVICE) {
        enableHumidity(false);
    } else if (uuid == TI_SENSORTAG_MOVEMENT_SERVICE) {
        enableMovement(false);
    } else if (uuid == TI_SENSORTAG_ACCELEROMETER_SERVICE) {
        enableMovement(false);
        accelerometerEnabled = false;
    } else if (uuid == TI_SENSORTAG_MAGNETOMETER_SERVICE) {
        enableMovement(false);
        magnetometerEnabled = false;
    } else if (uuid == TI_SENSORTAG_GYROSCOPE_SERVICE) {
        enableMovement(false);
        gyroscopeEnabled = false;
    }
}

void SensorTagBasePrivate::updateCharacteristic(const QLowEnergyCharacteristic &c,
                                                const QByteArray &value)
{
    if (c.uuid() == TI_SENSORTAG_LIGHT_DATA) {
        convertLux(value);
    } else if (c.uuid()== TI_SENSORTAG_IR_TEMPERATURE_DATA) {
        convertTemperature(value);
    } else if (c.uuid() == TI_SENSORTAG_BAROMETER_DATA) {
        convertBarometer(value);
    } else if (c.uuid()== TI_SENSORTAG_HUMIDTIY_DATA) {
        convertHumidity(value);
    } else if (c.uuid()== TI_SENSORTAG_BAROMETER_DATA) {
        convertBarometer(value);
    } else if ((c.uuid() == TI_SENSORTAG_ACCELEROMETER_DATA
              || c.uuid() == TI_SENSORTAG_MOVEMENT_DATA) && accelerometerEnabled) {
        convertAccelerometer(value);
    } else if ((c.uuid() == TI_SENSORTAG_MAGNETOMETER_DATA
                || c.uuid()== TI_SENSORTAG_MOVEMENT_DATA) && magnetometerEnabled) {
        convertMagnetometer(value);
    } else if ((c.uuid() == TI_SENSORTAG_GYROSCOPE_DATA
                || c.uuid() == TI_SENSORTAG_MOVEMENT_DATA) && gyroscopeEnabled) {
        convertGyroscope(value);
    }
}

void SensorTagBasePrivate::convertLux(const QByteArray &bytes)
{
    if (bytes.size() < 1)
        return;

    quint16 dat = ((quint16)bytes[1] & 0xFF) << 8;
    dat |= (quint16)(bytes[0] & 0xFF);

    qreal lux = dat * .01;

    emit luxDataAvailable(lux);
}

void SensorTagBasePrivate::convertTemperature(const QByteArray &bytes)
{
    if (bytes.size() < 3)
        return;

    qint16 objTemp = ((bytes[0] & 0xff) | ((bytes[1] << 8) & 0xff00));
    objTemp >>= 2;
    qreal objectTemperature = objTemp * 0.03125;
   // change to this if you want to use the ambient/die temp sensor
   // qreal ambientTemperature = ambTemp / 128.0;

    emit tempDataAvailable(objectTemperature);
}

void SensorTagBasePrivate::convertHumidity(const QByteArray &bytes)
{
    if (bytes.size() < 3)
        return;
    quint16 rawH = (bytes[2] & 0xff) | ((bytes[3] << 8) & 0xff00);
    qreal rHumidity = (qreal)(rawH / 65535) * 100.0;

    emit humidityDataAvailable(rHumidity);
}

void SensorTagBasePrivate::convertBarometer(const QByteArray &bytes)
{
    if (bytes.size() < 5)
        return;
    quint32 pressure = (bytes[3] & 0xff) | ((bytes[4] << 8) & 0xff00) | ((bytes[5] << 16) & 0xff0000);

    qreal mbars = (qreal)pressure / 100.0;
    emit pressureDataAvailable(mbars);
}

void SensorTagBasePrivate::convertAccelerometer(const QByteArray &bytes)
{
    if (bytes.size() < 3)
        return;

    int range = 8;

    qint16 X = (qint16)((bytes[8]) + ((bytes[9] << 8)));
    qint16 Y = (qint16)((bytes[6]) + ((bytes[7] << 8)));
    qint16 Z = (qint16)((bytes[10]) + ((bytes[11] << 8)));

    accelReading.setX((qreal)(X * 1.0) / (32768 / range) * 9.80665);
    accelReading.setY(-(qreal)(Y * 1.0) / (32768 / range) * 9.80665);
    accelReading.setZ((qreal)(Z * 1.0) / (32768 / range) * 9.80665);
    // TODO needs calibration

    emit accelDataAvailable(accelReading);
}

void SensorTagBasePrivate::convertMagnetometer(const QByteArray &bytes)
{
    if (bytes.size() < 3)
        return;

    qreal scale = 6.67100977199; // 32768 / 4912;
    qint16 X = (qint16)((bytes[12]) + ((bytes[13] << 8)));
    qint16 Y = (qint16)((bytes[14]) + ((bytes[15] << 8)));
    qint16 Z = (qint16)((bytes[16]) + ((bytes[17] << 8)));
    // TODO needs calibration

    magReading.setX((qreal)(X / scale));
    magReading.setY((qreal)(Y / scale));
    magReading.setZ((qreal)(Z / scale));

    emit magDataAvailable(magReading);
}

void SensorTagBasePrivate::convertGyroscope(const QByteArray &bytes)
{
    if (bytes.size() < 3)
        return;

    qreal scale = 128.0;
    qint16 X = (qint16)((bytes[2]) + ((bytes[3] << 8)));
    qint16 Y = (qint16)((bytes[0]) + ((bytes[1] << 8)));
    qint16 Z = (qint16)((bytes[4]) + ((bytes[5] << 8)));

    gyroReading.setX((qreal)(X / scale));
    gyroReading.setY((qreal)(Y / scale));
    gyroReading.setZ((qreal)(Z / scale));

    emit gyroDataAvailable(gyroReading);
}

SensorTagBasePrivate * SensorTagBasePrivate::instance()
{
    SensorTagBasePrivate *priv = sensortagBasePrivate();
    return priv;
}

SensorTagBase::SensorTagBase(QSensor *sensor)
    : QSensorBackend(sensor),
      leService(nullptr),
      serviceId(nullptr),
      d_ptr(SensorTagBasePrivate::instance())
{
    connect(d_ptr, &SensorTagBasePrivate::luxDataAvailable,
            this, &SensorTagBase::luxDataAvailable);
    connect(d_ptr, &SensorTagBasePrivate::tempDataAvailable,
            this, &SensorTagBase::tempDataAvailable);
    connect(d_ptr, &SensorTagBasePrivate::humidityDataAvailable,
            this, &SensorTagBase::humidityDataAvailable);
    connect(d_ptr, &SensorTagBasePrivate::pressureDataAvailable,
            this, &SensorTagBase::pressureDataAvailable);
    connect(d_ptr, &SensorTagBasePrivate::accelDataAvailable,
            this, &SensorTagBase::accelDataAvailable);
    connect(d_ptr, &SensorTagBasePrivate::gyroDataAvailable,
            this, &SensorTagBase::gyroDataAvailable);
    connect(d_ptr, &SensorTagBasePrivate::magDataAvailable,
            this, &SensorTagBase::magDataAvailable);
}

SensorTagBase::~SensorTagBase()
{
}

void SensorTagBase::start()
{
}

void SensorTagBase::stop()
{
}

quint64 SensorTagBase::produceTimestamp()
{
    return QDeadlineTimer::current().deadlineNSecs() / 1000;
}


