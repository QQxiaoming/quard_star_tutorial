/****************************************************************************
**
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

#include <QtCore/qcoreapplication.h>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>
#include <QtCore/private/qjnihelpers_p.h>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtBluetooth/QBluetoothHostInfo>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothServiceDiscoveryAgent>

#include "qbluetoothservicediscoveryagent_p.h"
#include "qbluetoothsocket_android_p.h"
#include "android/servicediscoverybroadcastreceiver_p.h"
#include "android/localdevicebroadcastreceiver_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
        QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    : error(QBluetoothServiceDiscoveryAgent::NoError),
      m_deviceAdapterAddress(deviceAdapter),
      state(Inactive),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      q_ptr(qp)

{
    // If a specific adapter address is requested we need to check it matches
    // the current local adapter. If it does not match we emit
    // InvalidBluetoothAdapterError when calling start()

    bool createAdapter = true;
    if (!deviceAdapter.isNull()) {
        const QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
        if (devices.isEmpty()) {
            createAdapter = false;
        } else {
            auto match = [deviceAdapter](const QBluetoothHostInfo& info) {
                return info.address() == deviceAdapter;
            };

            auto result = std::find_if(devices.begin(), devices.end(), match);
            if (result == devices.end())
                createAdapter = false;
        }
    }

    if (QtAndroidPrivate::androidSdkVersion() < 15)
        qCWarning(QT_BT_ANDROID)
            << "SDP not supported by Android API below version 15. Detected version: "
            << QtAndroidPrivate::androidSdkVersion()
            << "Service discovery will return empty list.";


    /*
      We assume that the current local adapter has been passed.
      The logic below must change once there is more than one adapter.
    */

    if (createAdapter)
        btAdapter = QAndroidJniObject::callStaticObjectMethod("android/bluetooth/BluetoothAdapter",
                                                           "getDefaultAdapter",
                                                           "()Landroid/bluetooth/BluetoothAdapter;");
    if (!btAdapter.isValid())
        qCWarning(QT_BT_ANDROID) << "Platform does not support Bluetooth";

    qRegisterMetaType<QList<QBluetoothUuid> >();
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    if (receiver) {
        receiver->unregisterReceiver();
        delete receiver;
    }
    if (localDeviceReceiver) {
        localDeviceReceiver->unregisterReceiver();
        delete localDeviceReceiver;
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (!btAdapter.isValid()) {
        if (m_deviceAdapterAddress.isNull()) {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Platform does not support Bluetooth");
        } else {
            // specific adapter was requested which does not match the locally
            // existing adapter
            error = QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Invalid Bluetooth adapter address");
        }

        //abort any outstanding discoveries
        discoveredDevices.clear();
        emit q->error(error);
        _q_serviceDiscoveryFinished();

        return;
    }

    /* SDP discovery was officially added by Android API v15
     * BluetoothDevice.getUuids() existed in earlier APIs already and in the future we may use
     * reflection to support earlier Android versions than 15. Unfortunately
     * BluetoothDevice.fetchUuidsWithSdp() and related APIs had some structure changes
     * over time. Therefore we won't attempt this with reflection.
     *
     * TODO: Use reflection to support getUuuids() where possible.
     * */
    if (QtAndroidPrivate::androidSdkVersion() < 15) {
        qCWarning(QT_BT_ANDROID) << "Aborting SDP enquiry due to too low Android API version (requires v15+)";

        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        errorString = QBluetoothServiceDiscoveryAgent::tr("Android API below v15 does not support SDP discovery");

        //abort any outstanding discoveries
        sdpCache.clear();
        discoveredDevices.clear();
        emit q->error(error);
        _q_serviceDiscoveryFinished();

        return;
    }

    QAndroidJniObject inputString = QAndroidJniObject::fromString(address.toString());
    QAndroidJniObject remoteDevice =
            btAdapter.callObjectMethod("getRemoteDevice",
                                               "(Ljava/lang/String;)Landroid/bluetooth/BluetoothDevice;",
                                               inputString.object<jstring>());
    QAndroidJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->ExceptionDescribe();

        //if it was only device then its error -> otherwise go to next device
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Cannot create Android BluetoothDevice");

            qCWarning(QT_BT_ANDROID) << "Cannot start SDP for" << discoveredDevices.at(0).name()
                                     << "(" << address.toString() << ")";
            emit q->error(error);
        }
        _q_serviceDiscoveryFinished();
        return;
    }


    if (mode == QBluetoothServiceDiscoveryAgent::MinimalDiscovery) {
        qCDebug(QT_BT_ANDROID) << "Minimal discovery on (" << discoveredDevices.at(0).name()
                               << ")" << address.toString() ;

        //Minimal discovery uses BluetoothDevice.getUuids()
        QAndroidJniObject parcelUuidArray = remoteDevice.callObjectMethod(
                    "getUuids", "()[Landroid/os/ParcelUuid;");

        if (!parcelUuidArray.isValid()) {
            if (singleDevice) {
                error = QBluetoothServiceDiscoveryAgent::InputOutputError;
                errorString = QBluetoothServiceDiscoveryAgent::tr("Cannot obtain service uuids");
                emit q->error(error);
            }
            qCWarning(QT_BT_ANDROID) << "Cannot retrieve SDP UUIDs for" << discoveredDevices.at(0).name()
                                     << "(" << address.toString() << ")";
            _q_serviceDiscoveryFinished();
            return;
        }

        const QList<QBluetoothUuid> results = ServiceDiscoveryBroadcastReceiver::convertParcelableArray(parcelUuidArray);
        populateDiscoveredServices(discoveredDevices.at(0), results);

        _q_serviceDiscoveryFinished();
    } else {
        qCDebug(QT_BT_ANDROID) << "Full discovery on (" << discoveredDevices.at(0).name()
                               << ")" << address.toString();

        //Full discovery uses BluetoothDevice.fetchUuidsWithSdp()
        if (!receiver) {
            receiver = new ServiceDiscoveryBroadcastReceiver();
            QObject::connect(receiver, &ServiceDiscoveryBroadcastReceiver::uuidFetchFinished,
                             q, [this](const QBluetoothAddress &address, const QList<QBluetoothUuid>& uuids) {
                this->_q_processFetchedUuids(address, uuids);
            });
        }

        if (!localDeviceReceiver) {
            localDeviceReceiver = new LocalDeviceBroadcastReceiver();
            QObject::connect(localDeviceReceiver, &LocalDeviceBroadcastReceiver::hostModeStateChanged,
                             q, [this](QBluetoothLocalDevice::HostMode state){
                this->_q_hostModeStateChanged(state);
            });
        }

        jboolean result = remoteDevice.callMethod<jboolean>("fetchUuidsWithSdp");
        if (!result) {
            //kill receiver to limit load of signals
            receiver->unregisterReceiver();
            receiver->deleteLater();
            receiver = nullptr;
            qCWarning(QT_BT_ANDROID) << "Cannot start dynamic fetch.";
            _q_serviceDiscoveryFinished();
        }
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    sdpCache.clear();
    discoveredDevices.clear();

    //kill receiver to limit load of signals
    receiver->unregisterReceiver();
    receiver->deleteLater();
    receiver = nullptr;

    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();

}

void QBluetoothServiceDiscoveryAgentPrivate::_q_processFetchedUuids(
    const QBluetoothAddress &address, const QList<QBluetoothUuid> &uuids)
{
    //don't leave more data through if we are not interested anymore
    if (discoveredDevices.count() == 0)
        return;

    //could not find any service for the current address/device -> go to next one
    if (address.isNull() || uuids.isEmpty()) {
        if (discoveredDevices.count() == 1) {
            Q_Q(QBluetoothServiceDiscoveryAgent);
            QTimer::singleShot(4000, q, [this]() {
                this->_q_fetchUuidsTimeout();
            });
        }
        _q_serviceDiscoveryFinished();
        return;
    }

    if (QT_BT_ANDROID().isDebugEnabled()) {
        qCDebug(QT_BT_ANDROID) << "Found UUID for" << address.toString()
                               << "\ncount: " << uuids.count();

        QString result;
        for (int i = 0; i<uuids.count(); i++)
            result += uuids.at(i).toString() + QStringLiteral("**");
        qCDebug(QT_BT_ANDROID) << result;
    }

    /* In general there are two uuid events per device.
     * We'll wait for the second event to arrive before we process the UUIDs.
     * We utilize a timeout to catch cases when the second
     * event doesn't arrive at all.
     * Generally we assume that the second uuid event carries the most up-to-date
     * set of uuids and discard the first events results.
    */

    if (sdpCache.contains(address)) {
        //second event
        QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > pair = sdpCache.take(address);

        //prefer second uuid set over first
        populateDiscoveredServices(pair.first, uuids);

        if (discoveredDevices.count() == 1 && sdpCache.isEmpty()) {
            //last regular uuid data set from OS -> we finish here
            _q_serviceDiscoveryFinished();
        }
    } else {
        //first event
        QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > pair;
        pair.first = discoveredDevices.at(0);
        pair.second = uuids;

        if (pair.first.address() != address)
            return;

        sdpCache.insert(address, pair);

        //the discovery on the last device cannot immediately finish
        //we have to grant the 2 seconds timeout delay
        if (discoveredDevices.count() == 1) {
            Q_Q(QBluetoothServiceDiscoveryAgent);
            QTimer::singleShot(4000, q, [this]() {
                this->_q_fetchUuidsTimeout();
            });
            return;
        }

        _q_serviceDiscoveryFinished();
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::populateDiscoveredServices(const QBluetoothDeviceInfo &remoteDevice, const QList<QBluetoothUuid> &uuids)
{
    /* Android doesn't provide decent SDP data. A flat list of uuids is all we get.
     *
     * The following approach is chosen:
     * - If we see an SPP service class and we see
     *   one or more custom uuids we match them up. Such services will always
     *   be SPP services. There is the chance that a custom uuid is eronously
     *   mapped as being an SPP service. In addition, the SPP uuid will be mapped as
     *   standalone SPP service.
     * - If we see a custom uuid but no SPP uuid then we return
     *   BluetoothServiceInfo instance with just a serviceUuid (no service class set)
     * - If we don't find any custom uuid but the SPP uuid, we return a
     *   BluetoothServiceInfo instance where classId and serviceUuid() are set to SPP.
     * - Any other service uuid will stand on its own.
     * */

    Q_Q(QBluetoothServiceDiscoveryAgent);

    //find SPP and custom uuid
    bool haveSppClass = false;
    QVector<int> customUuids;

    for (int i = 0; i < uuids.count(); i++) {
        const QBluetoothUuid uuid = uuids.at(i);

        if (uuid.isNull())
            continue;

        //check for SPP protocol
        bool ok = false;
        auto uuid16 = uuid.toUInt16(&ok);
        haveSppClass |= ok && uuid16 == QBluetoothUuid::SerialPort;

        //check for custom uuid
        if (uuid.minimumSize() == 16)
            customUuids.append(i);
    }

    auto rfcommProtocolDescriptorList = []() -> QBluetoothServiceInfo::Sequence {
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
                     << QVariant::fromValue(0);
            return protocol;
    };

    auto sppProfileDescriptorList = []() -> QBluetoothServiceInfo::Sequence {
        QBluetoothServiceInfo::Sequence profileSequence;
        QBluetoothServiceInfo::Sequence classId;
        classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
        classId << QVariant::fromValue(quint16(0x100));
        profileSequence.append(QVariant::fromValue(classId));
        return profileSequence;
    };

    for (int i = 0; i < uuids.count(); i++) {
        const QBluetoothUuid &uuid = uuids.at(i);
        if (uuid.isNull())
            continue;

        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setDevice(remoteDevice);

        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        {
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
            protocolDescriptorList.append(QVariant::fromValue(protocol));
        }

        if (customUuids.contains(i) && haveSppClass) {
            //we have a custom uuid of service class type SPP

            //set rfcomm protocol
            protocolDescriptorList.append(QVariant::fromValue(rfcommProtocolDescriptorList()));

            //set SPP profile descriptor list
            serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                                     sppProfileDescriptorList());

            QBluetoothServiceInfo::Sequence classId;
            //set SPP service class uuid
            classId << QVariant::fromValue(uuid);
            classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

            serviceInfo.setServiceName(QBluetoothServiceDiscoveryAgent::tr("Serial Port Profile"));
            serviceInfo.setServiceUuid(uuid);
        } else if (uuid == QBluetoothUuid{QBluetoothUuid::SerialPort}) {
            //set rfcomm protocol
            protocolDescriptorList.append(QVariant::fromValue(rfcommProtocolDescriptorList()));

            //set SPP profile descriptor list
            serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                                     sppProfileDescriptorList());

            //also we need to set the custom uuid to the SPP uuid
            //otherwise QBluetoothSocket::connectToService() would fail due to a missing service uuid
            serviceInfo.setServiceUuid(uuid);
        } else if (customUuids.contains(i)) {
            //custom uuid but no serial port
            serviceInfo.setServiceUuid(uuid);
        }

        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);
        QBluetoothServiceInfo::Sequence publicBrowse;
        publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

        if (!customUuids.contains(i)) {
            //if we don't have custom uuid use it as class id as well
            QBluetoothServiceInfo::Sequence classId;
            classId << QVariant::fromValue(uuid);
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
            auto clsId = QBluetoothUuid::ServiceClassUuid(uuid.toUInt16());
            serviceInfo.setServiceName(QBluetoothUuid::serviceClassToString(clsId));
        }

        //Check if the service is in the uuidFilter
        if (!uuidFilter.isEmpty()) {
            bool match = uuidFilter.contains(serviceInfo.serviceUuid());
            match |= uuidFilter.contains(QBluetoothSocketPrivateAndroid::reverseUuid(serviceInfo.serviceUuid()));
            for (const auto &uuid : qAsConst(uuidFilter)) {
                match |= serviceInfo.serviceClassUuids().contains(uuid);
                match |= serviceInfo.serviceClassUuids().contains(QBluetoothSocketPrivateAndroid::reverseUuid(uuid));
            }

            if (!match)
                continue;
        }

        //don't include the service if we already discovered it before
        if (!isDuplicatedService(serviceInfo)) {
            discoveredServices << serviceInfo;
            //qCDebug(QT_BT_ANDROID) << serviceInfo;
            emit q->serviceDiscovered(serviceInfo);
        }
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_fetchUuidsTimeout()
{
    if (sdpCache.isEmpty())
        return;

    QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > pair;
    const QList<QBluetoothAddress> keys = sdpCache.keys();
    for (const QBluetoothAddress &key : keys) {
        pair = sdpCache.take(key);
        populateDiscoveredServices(pair.first, pair.second);
    }

    Q_ASSERT(sdpCache.isEmpty());

    //kill receiver to limit load of signals
    receiver->unregisterReceiver();
    receiver->deleteLater();
    receiver = nullptr;
    _q_serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_hostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
    if (discoveryState() == QBluetoothServiceDiscoveryAgentPrivate::ServiceDiscovery &&
            state == QBluetoothLocalDevice::HostPoweredOff ) {

        discoveredDevices.clear();
        sdpCache.clear();
        error = QBluetoothServiceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothServiceDiscoveryAgent::tr("Device is powered off");

        //kill receiver to limit load of signals
        receiver->unregisterReceiver();
        receiver->deleteLater();
        receiver = nullptr;

        Q_Q(QBluetoothServiceDiscoveryAgent);
        emit q->error(error);
        _q_serviceDiscoveryFinished();
    }
}

QT_END_NAMESPACE
