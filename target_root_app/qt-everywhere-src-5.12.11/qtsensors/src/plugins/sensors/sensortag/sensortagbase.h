/****************************************************************************
**
** Copyright (C) 2017 Lorn Potter
** Copyright (C) 2017 Canonical, Ltd.
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

#ifndef SENSORTAGBASEPRIVATE_H
#define SENSORTAGBASEPRIVATE_H

#include <QObject>
#include <QSensorBackend>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>

#include <QAccelerometerReading>
#include <QGyroscopeReading>
#include <QMagnetometerReading>

class SensorTagBase;

class SensorTagBasePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(SensorTagBase)
public:
    explicit SensorTagBasePrivate(QObject *parent = 0);

    QLowEnergyService *lightService = nullptr;
    QLowEnergyService *temperatureService = nullptr;
    QLowEnergyService *barometerService = nullptr;
    QLowEnergyService *humidityService = nullptr;
    QLowEnergyService *movementService = nullptr;
    QLowEnergyService *acceleratorService = nullptr;
    QLowEnergyService *magnetometerService = nullptr;
    QLowEnergyService *gyroscopeService = nullptr;

    void enableService(const QBluetoothUuid &uuid);
    void disableService(const QBluetoothUuid &uuid);

    static SensorTagBasePrivate *instance();

signals:
    void accelDataAvailable(const QAccelerometerReading &);
    void gyroDataAvailable(const QGyroscopeReading &);
    void magDataAvailable(const QMagnetometerReading &);

    void luxDataAvailable(qreal);
    void tempDataAvailable(qreal);
    void humidityDataAvailable(qreal);
    void pressureDataAvailable(qreal);
    void compassDataAvailable(qreal);

public slots:
    void enableLight(bool);
    void enableTemp(bool);
    void enablePressure(bool);
    void enableHumidity(bool);
    void enableMovement(bool);

    void deviceFound(const QBluetoothDeviceInfo&);
    void scanFinished();

    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);
    void serviceDiscovered(const QBluetoothUuid &);

    void controllerError(QLowEnergyController::Error);

    void sensortagDeviceConnected();
    void deviceDisconnected();

    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateCharacteristic(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void serviceError(QLowEnergyService::ServiceError e);
    void doConnections(QLowEnergyService *service);

private slots:
    void deviceSearch();
    void serviceDiscoveryFinished();
    void deviceSearchTimeout();

private:

    void convertLux(const QByteArray &value);
    void convertTemperature(const QByteArray &value);
    void convertBarometer(const QByteArray &value);
    void convertHumidity(const QByteArray &value);
    void convertAccelerometer(const QByteArray &value);
    void convertMagnetometer(const QByteArray &value);
    void convertGyroscope(const QByteArray &value);

    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent = nullptr;
    QLowEnergyDescriptor m_notificationDesc;

    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *infoService = nullptr;

    QAccelerometerReading accelReading;
    QGyroscopeReading gyroReading;
    QMagnetometerReading magReading;

    bool discoveryDone = false;
    QList <QBluetoothUuid> enabledServiceUuids;
    SensorTagBase *q_ptr;

    bool accelerometerEnabled = false;
    bool gyroscopeEnabled = false;
    bool magnetometerEnabled = false;

    const QByteArray enableSensorCharacteristic = QByteArrayLiteral("\x01");
    const QByteArray disableSensorCharacteristic = QByteArrayLiteral("\x00");

    const QByteArray enableNotificationsCharacteristic = QByteArrayLiteral("\x01\x00");
    const QByteArray disableNotificationsCharacteristic = QByteArrayLiteral("\x00\x00");
};

const QBluetoothUuid TI_SENSORTAG_TEMPERATURE_SERVICE(QUuid("{f000aa00-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_IR_TEMPERATURE_DATA(QUuid("{f000aa01-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_IR_TEMPERATURE_CONTROL(QUuid("{f000aa02-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_IR_TEMPERATURE_PERIOD(QUuid("{f000aa03-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_ACCELEROMETER_SERVICE(QUuid("{f000aa10-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_ACCELEROMETER_DATA(QUuid("{f000aa11-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_ACCELEROMETER_CONTROL(QUuid("{f000aa12-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_ACCELEROMETER_PERIOD(QUuid("{f000aa13-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_HUMIDTIY_SERVICE(QUuid("{f000aa20-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_HUMIDTIY_DATA(QUuid("{f000aa21-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_HUMIDTIY_CONTROL(QUuid("{f000aa22-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_HUMIDTIY_PERIOD(QUuid("{f000aa23-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_MAGNETOMETER_SERVICE(QUuid("{f000aa30-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_MAGNETOMETER_DATA(QUuid("{f000aa31-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_MAGNETOMETER_CONTROL(QUuid("{f000aa32-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_MAGNETOMETER_PERIOD(QUuid("{f000aa33-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_BAROMETER_SERVICE(QUuid("{f000aa40-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_BAROMETER_DATA(QUuid("{f000aa41-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_BAROMETER_CONTROL(QUuid("{f000aa42-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_BAROMETER_CALIBRATION(QUuid("{f000aa43-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_BAROMETER_PERIOD(QUuid("{f000aa44-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_GYROSCOPE_SERVICE(QUuid("{f000aa50-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_GYROSCOPE_DATA(QUuid("{f000aa51-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_GYROSCOPE_CONTROL(QUuid("{f000aa52-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_GYROSCOPE_PERIOD(QUuid("{f000aa53-0451-4000-b000-000000000000}"));

// test service                              "{f000aa60-0451-4000-b000-000000000000}"

const QBluetoothUuid TI_SENSORTAG_IO_SERVICE(QUuid("{f000aa64-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_IO_DATA(QUuid("{f000aa65-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_IO_CONTROL(QUuid("{f000aa66-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_LIGHT_SERVICE(QUuid("{f000aa70-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_LIGHT_DATA(QUuid("{f000aa71-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_LIGHT_CONTROL(QUuid("{f000aa72-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_LIGHT_PERIOD(QUuid("{f000aa73-0451-4000-b000-000000000000}"));

const QBluetoothUuid TI_SENSORTAG_MOVEMENT_SERVICE(QUuid("{f000aa80-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_MOVEMENT_DATA(QUuid("{f000aa81-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_MOVEMENT_CONTROL(QUuid("{f000aa82-0451-4000-b000-000000000000}"));
const QBluetoothUuid TI_SENSORTAG_MOVEMENT_PERIOD(QUuid("{f000aa83-0451-4000-b000-000000000000}"));

// keypress                                  "{f000ffe0-0000-1000-8000-00805f9b34fb"

// Connection Control Service                "{f000ccc0-0451-4000-b000-000000000000}"
// OAD (over air download) Service           "{f000ffc0-0451-4000-b000-000000000000}"

const QBluetoothUuid TI_SENSORTAG_INFO_SERVICE(QUuid("{0000180a-0000-1000-8000-00805f9b34fb}"));
//we might want to use these somehow
//const QBluetoothUuid TI_SENSORTAG_BATTERY_SERVICE         "180F"
//const QBluetoothUuid TI_SENSORTAG_BATTERY_BATTERY_LEVEL   "2A19"

// {00001800-0000-1000-8000-00805f9b34fb} //Generic Access Service
// {00001801-0000-1000-8000-00805f9b34fb} //Generic Attribute Service

class SensorTagBase : public QSensorBackend
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SensorTagBase)
public:
    SensorTagBase(QSensor *sensor);
    virtual ~SensorTagBase();

    quint64 produceTimestamp();
signals:
    void accelDataAvailable(const QAccelerometerReading &);
    void luxDataAvailable(qreal);
    void tempDataAvailable(qreal);
    void humidityDataAvailable(qreal);
    void pressureDataAvailable(qreal);
    void gyroDataAvailable(const QGyroscopeReading &);
    void magDataAvailable(const QMagnetometerReading &);

protected:
    void start() override;
    void stop() override;
    QLowEnergyService *leService;
    QBluetoothUuid *serviceId;

private:
    SensorTagBasePrivate *d_ptr;
    friend class SensorTagTemperatureSensor;
    friend class SensorTagAls;
    friend class SensorTagHumiditySensor;
    friend class SensorTagLightSensor;

    friend class SensorTagPressureSensor;
    friend class SensorTagAccelerometer;

    friend class SensorTagGyroscope;
    friend class SensorTagMagnetometer;
};

#endif // SENSORTAGBASEPRIVATE_H
