/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qbluetoothsocket.h"
#include "qbluetoothsocket_bluezdbus_p.h"

#include "bluez/bluez_data_p.h"
#include "bluez/bluez5_helper_p.h"
#include "bluez/adapter1_bluez5_p.h"
#include "bluez/device1_bluez5_p.h"
#include "bluez/objectmanager_p.h"
#include "bluez/profile1_p.h"
#include "bluez/profile1context_p.h"
#include "bluez/profilemanager1_p.h"

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qrandom.h>

#include <QtNetwork/qlocalsocket.h>

#include <unistd.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

static const QLatin1String profilePathTemplate("/qt/btsocket/%1%2/%3");

QBluetoothSocketPrivateBluezDBus::QBluetoothSocketPrivateBluezDBus()
{
    secFlags = QBluetooth::NoSecurity;

    profileManager = new OrgBluezProfileManager1Interface(
                            QStringLiteral("org.bluez"),
                            QStringLiteral("/org/bluez"),
                            QDBusConnection::systemBus(),
                            this);
}

QBluetoothSocketPrivateBluezDBus::~QBluetoothSocketPrivateBluezDBus()
{
}

bool QBluetoothSocketPrivateBluezDBus::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    switch (type) {
    case QBluetoothServiceInfo::UnknownProtocol:
        break;
    case QBluetoothServiceInfo::RfcommProtocol:
    case QBluetoothServiceInfo::L2capProtocol:
        socketType = type;
        return true;
    }

    return false;
}

void QBluetoothSocketPrivateBluezDBus::connectToServiceHelper(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    // TODO Remove when Bluez4 support dropped
    // Only used by QBluetoothSocketPrivateBluez
    Q_UNUSED(openMode);
    Q_UNUSED(address);
    Q_UNUSED(port);
}

static QString findRemoteDevicePath(const QBluetoothAddress &address)
{
    OrgFreedesktopDBusObjectManagerInterface manager(QStringLiteral("org.bluez"),
                                                     QStringLiteral("/"),
                                                     QDBusConnection::systemBus());

    bool ok = false;
    const QString adapterPath = findAdapterForAddress(QBluetoothAddress(), &ok);
    if (!ok)
        return QString();

    auto reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError())
        return QString();

    QString remoteDevicePath;

    ManagedObjectList objectList = reply.value();
    for (ManagedObjectList::const_iterator it = objectList.constBegin();
                                           it != objectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator ifaceIter = ifaceList.constBegin();
                                           ifaceIter != ifaceList.constEnd(); ++ifaceIter) {
            if (ifaceIter.key() == QStringLiteral("org.bluez.Device1")) {
                if (path.path().indexOf(adapterPath) != 0)
                    continue; // devices whose path does not start with same path we skip

                OrgBluezDevice1Interface device(QStringLiteral("org.bluez"),
                                                path.path(), QDBusConnection::systemBus());
                if (device.adapter().path() != adapterPath)
                    continue;

                const QBluetoothAddress btAddress(device.address());
                if (btAddress.isNull() || btAddress != address)
                    continue;

                return path.path();
            }
        }
    }

    return QString();
}

void QBluetoothSocketPrivateBluezDBus::connectToServiceHelper(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid,
        QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    int i = 0;
    bool success = false;
    profileUuid = uuid.toString(QUuid::WithoutBraces);

    if (profileContext) {
        qCDebug(QT_BT_BLUEZ) << "Profile context still active. close socket first.";
        q->setSocketError(QBluetoothSocket::UnknownSocketError);
        return;
    }


    profileContext = new OrgBluezProfile1ContextInterface(this);
    connect(profileContext, &OrgBluezProfile1ContextInterface::newConnection,
            this, &QBluetoothSocketPrivateBluezDBus::remoteConnected);

    for (i = 0; i < 10 && !success; i++) {
        // profile registration might fail in case other service uses same path
        // try 10 times and otherwise abort

        profilePath = QString(profilePathTemplate).
                                  arg(sanitizeNameForDBus(QCoreApplication::applicationName())).
                                  arg(QCoreApplication::applicationPid()).
                                  arg(QRandomGenerator::global()->generate());

        success = QDBusConnection::systemBus().registerObject(
                                    profilePath, profileContext, QDBusConnection::ExportAllSlots);
    }

    if (!success) {
        // we could not register the profile
        qCWarning(QT_BT_BLUEZ) << "Cannot export serial client profile on DBus";

        delete profileContext;
        profileContext = nullptr;

        errorString = QBluetoothSocket::tr("Cannot export profile on DBus");
        q->setSocketError(QBluetoothSocket::UnknownSocketError);

        return;
    }

    QVariantMap profileOptions;
    profileOptions.insert(QStringLiteral("Role"), QStringLiteral("client"));
    profileOptions.insert(QStringLiteral("Service"), profileUuid);
    profileOptions.insert(QStringLiteral("Name"),
                          QStringLiteral("QBluetoothSocket-%1").arg(QCoreApplication::applicationPid()));

    // TODO support more profile parameter
    // profileOptions.insert(QStringLiteral("Channel"), 0);

    qCDebug(QT_BT_BLUEZ) << "Registering client profile on" << profilePath << "with options:";
    qCDebug(QT_BT_BLUEZ) << profileOptions;
    QDBusPendingReply<> reply = profileManager->RegisterProfile(
                                    QDBusObjectPath(profilePath),
                                    profileUuid,
                                    profileOptions);
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Client profile registration failed:"
                               << reply.error().message();

        QDBusConnection::systemBus().unregisterObject(profilePath);
        errorString = QBluetoothSocket::tr("Cannot register profile on DBus");
        q->setSocketError(QBluetoothSocket::UnknownSocketError);
        return;
    }

    remoteDevicePath = findRemoteDevicePath(address);
    if (remoteDevicePath.isEmpty()) {
        qCWarning(QT_BT_BLUEZ) << "Unknown remote device:" << address
                               << "Try device discovery first";
        clearSocket();

        errorString = QBluetoothSocket::tr("Cannot find remote device");
        q->setSocketError(QBluetoothSocket::HostNotFoundError);
        return;
    }

    OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), remoteDevicePath,
                                    QDBusConnection::systemBus());
    reply = device.ConnectProfile(profileUuid);
    if (reply.isError()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot connect to profile/service:" << uuid;

        clearSocket();

        errorString = QBluetoothSocket::tr("Cannot connect to remote profile");
        q->setSocketError(QBluetoothSocket::HostNotFoundError);
        return;
    }

    q->setOpenMode(openMode);
    q->setSocketState(QBluetoothSocket::ConnectingState);
}

void QBluetoothSocketPrivateBluezDBus::connectToService(
        const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    QBluetoothUuid targetService;

    targetService = service.serviceUuid();
    if (targetService.isNull()) {
        // Do we have serialport service class?
        if (service.serviceClassUuids().contains(QBluetoothUuid::SerialPort))
            targetService = QBluetoothUuid::SerialPort;
    }

    if (targetService.isNull()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot find appropriate serviceUuid"
                               << "or SerialPort service class uuid";
        errorString = QBluetoothSocket::tr("Missing serviceUuid or Serial Port service class uuid");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    connectToService(service.device().address(), targetService, openMode);
}

void QBluetoothSocketPrivateBluezDBus::connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (address.isNull()) {
        qCWarning(QT_BT_BLUEZ) << "Invalid address to remote address passed.";
        errorString = QBluetoothSocket::tr("Invalid Bluetooth address passed to connectToService()");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    if (uuid.isNull()) {
        qCWarning(QT_BT_BLUEZ) << "Cannot find appropriate serviceUuid"
                               << "or SerialPort service class uuid";
        errorString = QBluetoothSocket::tr("Missing serviceUuid or Serial Port service class uuid");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    if (q->state() != QBluetoothSocket::UnconnectedState) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluezDBus::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::OperationError);
        return;
    }

    if (q->socketType() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_BLUEZ) << "QBluetoothSocketPrivateBluezDBus::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::UnsupportedProtocolError);
        return;
    }

    if (!ensureNativeSocket(q->socketType())) {
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::UnsupportedProtocolError);
        return;
    }
    connectToServiceHelper(address, uuid, openMode);
}

void QBluetoothSocketPrivateBluezDBus::connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{

    Q_UNUSED(port);
    Q_UNUSED(address);
    Q_UNUSED(openMode);
    Q_Q(QBluetoothSocket);

    errorString = tr("Connecting to port is not supported via Bluez DBus");
    q->setSocketError(QBluetoothSocket::ServiceNotFoundError);
    qCWarning(QT_BT_BLUEZ) << "Connecting to port is not supported (Uuid required)";
}

void QBluetoothSocketPrivateBluezDBus::abort()
{
    if (localSocket) {
        localSocket->close();
        // delayed disconnected signal emission when localSocket closes
    } else {
        Q_Q(QBluetoothSocket);

        clearSocket();
        q->setOpenMode(QIODevice::NotOpen);
        q->setSocketState(QBluetoothSocket::UnconnectedState);
        emit q->readChannelFinished();
    }
}

QString QBluetoothSocketPrivateBluezDBus::localName() const
{
    bool ok = false;
    const QString adapterPath = findAdapterForAddress(QBluetoothAddress(), &ok);
    if (!ok)
        return QString();

    OrgBluezAdapter1Interface adapter(QStringLiteral("org.bluez"), adapterPath,
                                      QDBusConnection::systemBus());
    return QString(adapter.alias());
}

QBluetoothAddress QBluetoothSocketPrivateBluezDBus::localAddress() const
{
    bool ok = false;
    const QString adapterPath = findAdapterForAddress(QBluetoothAddress(), &ok);
    if (!ok)
        return QBluetoothAddress();

    OrgBluezAdapter1Interface adapter(QStringLiteral("org.bluez"), adapterPath,
                                      QDBusConnection::systemBus());
    return QBluetoothAddress(adapter.address());
}

quint16 QBluetoothSocketPrivateBluezDBus::localPort() const
{
    int descriptor = -1;

    if (localSocket)
        descriptor = int(localSocket->socketDescriptor());
    if (descriptor == -1)
        return 0;

     if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(descriptor, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return (addr.rc_channel);
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getsockname(descriptor, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.l2_psm;
    }

    return 0;
}

QString QBluetoothSocketPrivateBluezDBus::peerName() const
{
    if (remoteDevicePath.isEmpty())
        return QString();

    OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), remoteDevicePath,
                                            QDBusConnection::systemBus());
    return device.alias();
}

QBluetoothAddress QBluetoothSocketPrivateBluezDBus::peerAddress() const
{
    if (remoteDevicePath.isEmpty())
        return QBluetoothAddress();

    OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), remoteDevicePath,
                                            QDBusConnection::systemBus());
    return QBluetoothAddress(device.address());
}

quint16 QBluetoothSocketPrivateBluezDBus::peerPort() const
{
    int descriptor = -1;

    if (localSocket)
        descriptor = int(localSocket->socketDescriptor());
    if (descriptor == -1)
        return 0;

    if (socketType == QBluetoothServiceInfo::RfcommProtocol) {
        sockaddr_rc addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(descriptor, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.rc_channel;
    } else if (socketType == QBluetoothServiceInfo::L2capProtocol) {
        sockaddr_l2 addr;
        socklen_t addrLength = sizeof(addr);

        if (::getpeername(descriptor, reinterpret_cast<sockaddr *>(&addr), &addrLength) == 0)
            return addr.l2_psm;
    }

    return 0;
}

qint64 QBluetoothSocketPrivateBluezDBus::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::OperationError);
        return -1;
    }

    if (localSocket)
        return localSocket->write(data, maxSize);

    return -1;
}

qint64 QBluetoothSocketPrivateBluezDBus::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q->setSocketError(QBluetoothSocket::OperationError);
        return -1;
    }

    if (localSocket)
        return localSocket->read(data, maxSize);

    return -1;
}

void QBluetoothSocketPrivateBluezDBus::close()
{
    abort();
}

bool QBluetoothSocketPrivateBluezDBus::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType)
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);
    return false;
}

qint64 QBluetoothSocketPrivateBluezDBus::bytesAvailable() const
{
    if (localSocket)
        return localSocket->bytesAvailable();

    return 0;
}

bool QBluetoothSocketPrivateBluezDBus::canReadLine() const
{
    if (localSocket)
        return localSocket->canReadLine();

    return false;
}

qint64 QBluetoothSocketPrivateBluezDBus::bytesToWrite() const
{
    if (localSocket)
        return localSocket->bytesToWrite();

    return 0;
}

void QBluetoothSocketPrivateBluezDBus::remoteConnected(const QDBusUnixFileDescriptor &fd)
{
    Q_Q(QBluetoothSocket);

    int descriptor = ::dup(fd.fileDescriptor());
    localSocket = new QLocalSocket(this);
    bool success = localSocket->setSocketDescriptor(
                            descriptor, QLocalSocket::ConnectedState, q->openMode());
    if (!success || !localSocket->isValid()) {
        q->setSocketState(QBluetoothSocket::UnconnectedState);
        delete localSocket;
        localSocket = nullptr;
    } else {
        connect(localSocket, &QLocalSocket::readyRead,
                q, &QBluetoothSocket::readyRead);
        connect(localSocket, &QLocalSocket::stateChanged,
                this, &QBluetoothSocketPrivateBluezDBus::socketStateChanged);
        connect(localSocket, &QLocalSocket::bytesWritten,
                q, &QBluetoothSocket::bytesWritten);

        socket = descriptor;
        q->setSocketState(QBluetoothSocket::ConnectedState);
    }
}

void QBluetoothSocketPrivateBluezDBus::socketStateChanged(QLocalSocket::LocalSocketState newState)
{
    Q_Q(QBluetoothSocket);

    switch (newState) {
    case QLocalSocket::ClosingState:
        q->setSocketState(QBluetoothSocket::ClosingState);
        break;
    case QLocalSocket::UnconnectedState:
        clearSocket();
        q->setOpenMode(QIODevice::NotOpen);
        q->setSocketState(QBluetoothSocket::UnconnectedState);
        emit q->readChannelFinished();
        break;
    default:
        // ConnectingState and ConnectedState not mapped
        // (already set at the time when the socket is created)
        break;
    }
}

void QBluetoothSocketPrivateBluezDBus::clearSocket()
{
    Q_Q(QBluetoothSocket);

    if (profilePath.isEmpty())
        return;

    qCDebug(QT_BT_BLUEZ) << "Clearing profile called for" << profilePath;

    if (localSocket) {
        localSocket->close();
        localSocket->deleteLater();
        localSocket = nullptr;
    }

    socket = -1;

    if (q->state() == QBluetoothSocket::ConnectedState) {
        OrgBluezDevice1Interface device(QStringLiteral("org.bluez"), remoteDevicePath,
                                        QDBusConnection::systemBus());
        auto reply = device.DisconnectProfile(profileUuid);
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(QT_BT_BLUEZ) << "Disconnect profile failed:"
                                   << reply.error().message();
        }
    }

    QDBusPendingReply<> reply = profileManager->UnregisterProfile(QDBusObjectPath(profilePath));
    reply.waitForFinished();
    if (reply.isError())
        qCWarning(QT_BT_BLUEZ) << "Unregister profile:" << reply.error().message();

    QDBusConnection::systemBus().unregisterObject(profilePath);

    if (profileContext) {
        delete profileContext;
        profileContext = nullptr;
    }

    remoteDevicePath.clear();
    profileUuid.clear();
    profilePath.clear();
}
QT_END_NAMESPACE
