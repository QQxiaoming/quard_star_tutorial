/****************************************************************************
**
** Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "qbluetoothdevicediscoveryagent_p.h"
#include <QtCore/QLoggingCategory>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtCore/private/qjnihelpers_p.h>
#include "android/devicediscoverybroadcastreceiver_p.h"
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroid>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

enum {
    NoScanActive = 0,
    SDPScanActive = 1,
    BtleScanActive = 2
};

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
    const QBluetoothAddress &deviceAdapter, QBluetoothDeviceDiscoveryAgent *parent) :
    inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry),
    lastError(QBluetoothDeviceDiscoveryAgent::NoError),
    receiver(0),
    m_adapterAddress(deviceAdapter),
    m_active(NoScanActive),
    leScanTimeout(0),
    pendingCancel(false),
    pendingStart(false),
    lowEnergySearchTimeout(25000),
    q_ptr(parent)
{
    QAndroidJniEnvironment env;
    adapter = QAndroidJniObject::callStaticObjectMethod("android/bluetooth/BluetoothAdapter",
                                                        "getDefaultAdapter",
                                                        "()Landroid/bluetooth/BluetoothAdapter;");
    if (!adapter.isValid()) {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        qCWarning(QT_BT_ANDROID) << "Device does not support Bluetooth";
    }
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    if (m_active != NoScanActive)
        stop();

    if (leScanner.isValid())
        leScanner.setField<jlong>("qtObject", reinterpret_cast<jlong>(nullptr));

    if (receiver) {
        receiver->unregisterReceiver();
        delete receiver;
    }
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    if (pendingStart)
        return true;
    if (pendingCancel)
        return false;
    return m_active != NoScanActive;
}

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return (LowEnergyMethod | ClassicMethod);
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
{
    //TODO Implement discovery method handling (see input parameter)
    requestedMethods = methods;

    if (pendingCancel) {
        pendingStart = true;
        return;
    }

    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (!adapter.isValid()) {
        qCWarning(QT_BT_ANDROID) << "Device does not support Bluetooth";
        lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device does not support Bluetooth");
        emit q->error(lastError);
        return;
    }

    if (!m_adapterAddress.isNull()
        && adapter.callObjectMethod<jstring>("getAddress").toString()
        != m_adapterAddress.toString()) {
        qCWarning(QT_BT_ANDROID) << "Incorrect local adapter passed.";
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Passed address is not a local device.");
        emit q->error(lastError);
        return;
    }

    const int state = adapter.callMethod<jint>("getState");
    if (state != 12) {  // BluetoothAdapter.STATE_ON
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Device is powered off");
        emit q->error(lastError);
        return;
    }

    // check Android v23+ permissions
    // -> any device search requires android.permission.ACCESS_COARSE_LOCATION or android.permission.ACCESS_FINE_LOCATION
    if (QtAndroid::androidSdkVersion() >= 23) {
        const QString coarsePermission(QLatin1String("android.permission.ACCESS_COARSE_LOCATION"));
        const QString finePermission(QLatin1String("android.permission.ACCESS_FINE_LOCATION"));

        // do we have required permission already, if so nothing to do
        if (QtAndroidPrivate::checkPermission(coarsePermission) == QtAndroidPrivate::PermissionsResult::Denied
            && QtAndroidPrivate::checkPermission(finePermission) == QtAndroidPrivate::PermissionsResult::Denied) {
            qCWarning(QT_BT_ANDROID) << "Requesting ACCESS_*_LOCATION permission";

            QAndroidJniEnvironment env;
            const QHash<QString, QtAndroidPrivate::PermissionsResult> results =
                    QtAndroidPrivate::requestPermissionsSync(env, QStringList() << coarsePermission << finePermission);

            bool permissionReceived = false;
            for (const QString &permission: results.keys()) {
                qCDebug(QT_BT_ANDROID) << permission << (results[permission] == QtAndroidPrivate::PermissionsResult::Denied);
                if ((permission == coarsePermission || permission == finePermission)
                    && results[permission] == QtAndroidPrivate::PermissionsResult::Granted) {
                        permissionReceived = true;
                        break;
                }
            }
            if (!permissionReceived) {
                qCWarning(QT_BT_ANDROID) << "Search not possible due to missing permission (ACCESS_COARSE|FINE_LOCATION)";
                lastError = QBluetoothDeviceDiscoveryAgent::UnknownError;
                errorString = QBluetoothDeviceDiscoveryAgent::tr("Missing Location permission. Search is not possible.");
                emit q->error(lastError);
                return;
            }
        }

        qCWarning(QT_BT_ANDROID) << "ACCESS_COARSE|FINE_LOCATION permission available";
    }

    // Double check Location service is turned on
    bool locationTurnedOn = true; // backwards compatible behavior to previous Qt versions
    const  QAndroidJniObject locString = QAndroidJniObject::getStaticObjectField(
                "android/content/Context", "LOCATION_SERVICE", "Ljava/lang/String;");
    const QAndroidJniObject locService = QtAndroid::androidContext().callObjectMethod(
                "getSystemService",
                "(Ljava/lang/String;)Ljava/lang/Object;",
                locString.object<jstring>());

    if (locService.isValid()) {
        if (QtAndroid::androidSdkVersion() >= 28) {
            locationTurnedOn = bool(locService.callMethod<jboolean>("isLocationEnabled"));
        } else {
            // try GPS and network provider
            QAndroidJniObject provider = QAndroidJniObject::getStaticObjectField(
                        "android/location/LocationManager", "GPS_PROVIDER", "Ljava/lang/String;");
            bool gpsTurnedOn = bool(locService.callMethod<jboolean>("isProviderEnabled",
                                      "(Ljava/lang/String;)Z", provider.object<jstring>()));

            provider = QAndroidJniObject::getStaticObjectField(
                       "android/location/LocationManager", "NETWORK_PROVIDER", "Ljava/lang/String;");
            bool providerTurnedOn = bool(locService.callMethod<jboolean>("isProviderEnabled",
                                          "(Ljava/lang/String;)Z", provider.object<jstring>()));

            locationTurnedOn = gpsTurnedOn || providerTurnedOn;
        }
    }

    if (!locationTurnedOn) {
        qCWarning(QT_BT_ANDROID) << "Search not possible due to turned off Location service";
        lastError = QBluetoothDeviceDiscoveryAgent::UnknownError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Location service turned off. Search is not possible.");
        emit q->error(lastError);
        return;
    }

    qCDebug(QT_BT_ANDROID) << "Location turned on";

    // install Java BroadcastReceiver
    if (!receiver) {
        // SDP based device discovery
        receiver = new DeviceDiscoveryBroadcastReceiver();
        qRegisterMetaType<QBluetoothDeviceInfo>();
        QObject::connect(receiver, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo,bool)),
                         this, SLOT(processDiscoveredDevices(QBluetoothDeviceInfo,bool)));
        QObject::connect(receiver, SIGNAL(finished()), this, SLOT(processSdpDiscoveryFinished()));
    }

    discoveredDevices.clear();

    // by arbitrary definition we run classic search first
    if (requestedMethods & QBluetoothDeviceDiscoveryAgent::ClassicMethod) {
        const bool success = adapter.callMethod<jboolean>("startDiscovery");
        if (!success) {
            qCDebug(QT_BT_ANDROID) << "Classic Discovery cannot be started";
            if (requestedMethods == QBluetoothDeviceDiscoveryAgent::ClassicMethod) {
                //only classic discovery requested -> error out
                lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
                errorString = QBluetoothDeviceDiscoveryAgent::tr("Classic Discovery cannot be started");

                emit q->error(lastError);
                return;
            } // else fall through to LE discovery
        } else {
            m_active = SDPScanActive;
            qCDebug(QT_BT_ANDROID)
                << "QBluetoothDeviceDiscoveryAgentPrivate::start() - Classic search successfully started.";
            return;
        }
    }

    if (requestedMethods & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) {
        // LE search only requested or classic discovery failed but lets try LE scan anyway
        Q_ASSERT(requestedMethods & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

        if (QtAndroidPrivate::androidSdkVersion() < 18) {
            qCDebug(QT_BT_ANDROID) << "Skipping Bluetooth Low Energy device scan due to"
                                      "insufficient Android version.";
            m_active = NoScanActive;
            lastError = QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Low Energy Discovery not supported");
            emit q->error(lastError);
            return;
        }

        startLowEnergyScan();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (m_active == NoScanActive)
        return;

    if (m_active == SDPScanActive) {
        if (pendingCancel)
            return;

        pendingCancel = true;
        pendingStart = false;
        bool success = adapter.callMethod<jboolean>("cancelDiscovery");
        if (!success) {
            lastError = QBluetoothDeviceDiscoveryAgent::InputOutputError;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Discovery cannot be stopped");
            emit q->error(lastError);
            return;
        }
    } else if (m_active == BtleScanActive) {
        stopLowEnergyScan();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::processSdpDiscoveryFinished()
{
    // We need to guard because Android sends two DISCOVERY_FINISHED when cancelling
    // Also if we have two active agents both receive the same signal.
    // If this one is not active ignore the device information
    if (m_active != SDPScanActive)
        return;

    Q_Q(QBluetoothDeviceDiscoveryAgent);

    if (pendingCancel && !pendingStart) {
        m_active = NoScanActive;
        pendingCancel = false;
        emit q->canceled();
    } else if (pendingStart) {
        pendingStart = pendingCancel = false;
        start(requestedMethods);
    } else {
        // check that it didn't finish due to turned off Bluetooth Device
        const int state = adapter.callMethod<jint>("getState");
        if (state != 12) {  // BluetoothAdapter.STATE_ON
            m_active = NoScanActive;
            lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
            errorString = QBluetoothDeviceDiscoveryAgent::tr("Device is powered off");
            emit q->error(lastError);
            return;
        }

        // no BTLE scan requested
        if (!(requestedMethods & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod)) {
            m_active = NoScanActive;
            emit q->finished();
            return;
        }

        // start LE scan if supported
        if (QtAndroidPrivate::androidSdkVersion() < 18) {
            qCDebug(QT_BT_ANDROID) << "Skipping Bluetooth Low Energy device scan";
            m_active = NoScanActive;
            emit q->finished();
        } else {
            startLowEnergyScan();
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::processDiscoveredDevices(
    const QBluetoothDeviceInfo &info, bool isLeResult)
{
    // If we have two active agents both receive the same signal.
    // If this one is not active ignore the device information
    if (m_active != SDPScanActive && !isLeResult)
        return;
    if (m_active != BtleScanActive && isLeResult)
        return;

    Q_Q(QBluetoothDeviceDiscoveryAgent);

    // Android Classic scan and LE scan can find the same device under different names
    // The classic name finds the SDP based device name, the LE scan finds the name in
    // the advertisement package.
    // If address is same but name different then we keep both entries.

    for (int i = 0; i < discoveredDevices.size(); i++) {
        if (discoveredDevices[i].address() == info.address()) {
            QBluetoothDeviceInfo::Fields updatedFields = QBluetoothDeviceInfo::Field::None;
            if (discoveredDevices[i].rssi() != info.rssi()) {
                qCDebug(QT_BT_ANDROID) << "Updating RSSI for" << info.address()
                                       << info.rssi();
                discoveredDevices[i].setRssi(info.rssi());
                updatedFields.setFlag(QBluetoothDeviceInfo::Field::RSSI);
            }
            if (discoveredDevices[i].manufacturerData() != info.manufacturerData()) {
                qCDebug(QT_BT_ANDROID) << "Updating manufacturer data for" << info.address();
                const QVector<quint16> keys = info.manufacturerIds();
                for (auto key: keys)
                    discoveredDevices[i].setManufacturerData(key, info.manufacturerData(key));
                updatedFields.setFlag(QBluetoothDeviceInfo::Field::ManufacturerData);
            }

            if (lowEnergySearchTimeout > 0) {
                if (discoveredDevices[i] != info) {
                    if (discoveredDevices.at(i).name() == info.name()) {
                        qCDebug(QT_BT_ANDROID) << "Almost Duplicate " << info.address()
                                               << info.name() << "- replacing in place";
                        discoveredDevices.replace(i, info);
                        emit q->deviceDiscovered(info);
                    }
                } else {
                    if (!updatedFields.testFlag(QBluetoothDeviceInfo::Field::None))
                        emit q->deviceUpdated(discoveredDevices[i], updatedFields);
                }

                return;
            }

            discoveredDevices.replace(i, info);
            emit q->deviceDiscovered(info);

            if (!updatedFields.testFlag(QBluetoothDeviceInfo::Field::None))
                emit q->deviceUpdated(discoveredDevices[i], updatedFields);

            return;
        }
    }

    discoveredDevices.append(info);
    qCDebug(QT_BT_ANDROID) << "Device found: " << info.name() << info.address().toString()
                           << "isLeScanResult:" << isLeResult;
    emit q->deviceDiscovered(info);
}

void QBluetoothDeviceDiscoveryAgentPrivate::startLowEnergyScan()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    m_active = BtleScanActive;

    QAndroidJniEnvironment env;
    if (!leScanner.isValid()) {
        leScanner = QAndroidJniObject("org/qtproject/qt5/android/bluetooth/QtBluetoothLE");
        if (env->ExceptionCheck() || !leScanner.isValid()) {
            qCWarning(QT_BT_ANDROID) << "Cannot load BTLE device scan class";
            env->ExceptionDescribe();
            env->ExceptionClear();
            m_active = NoScanActive;
            emit q->finished();
            return;
        }

        leScanner.setField<jlong>("qtObject", reinterpret_cast<long>(receiver));
    }

    jboolean result = leScanner.callMethod<jboolean>("scanForLeDevice", "(Z)Z", true);
    if (!result) {
        qCWarning(QT_BT_ANDROID) << "Cannot start BTLE device scanner";
        m_active = NoScanActive;
        emit q->finished();
        return;
    }

    // wait interval and sum up what was found
    if (!leScanTimeout) {
        leScanTimeout = new QTimer(this);
        leScanTimeout->setSingleShot(true);
        connect(leScanTimeout, &QTimer::timeout,
                this, &QBluetoothDeviceDiscoveryAgentPrivate::stopLowEnergyScan);
    }

    if (lowEnergySearchTimeout > 0) { // otherwise no timeout and stop() required
        leScanTimeout->setInterval(lowEnergySearchTimeout);
        leScanTimeout->start();
    }

    qCDebug(QT_BT_ANDROID)
        << "QBluetoothDeviceDiscoveryAgentPrivate::start() - Low Energy search successfully started.";
}

void QBluetoothDeviceDiscoveryAgentPrivate::stopLowEnergyScan()
{
    jboolean result = leScanner.callMethod<jboolean>("scanForLeDevice", "(Z)Z", false);
    if (!result)
        qCWarning(QT_BT_ANDROID) << "Cannot stop BTLE device scanner";

    m_active = NoScanActive;

    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (leScanTimeout->isActive()) {
        // still active if this function was called from stop()
        leScanTimeout->stop();
        emit q->canceled();
    } else {
        // timeout -> regular stop
        emit q->finished();
    }
}
QT_END_NAMESPACE
