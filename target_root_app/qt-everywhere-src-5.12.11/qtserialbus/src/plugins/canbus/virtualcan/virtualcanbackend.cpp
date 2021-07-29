/****************************************************************************
**
** Copyright (C) 2018 Andre Hartmann <aha_1980@gmx.de>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "virtualcanbackend.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qregularexpression.h>

#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_VIRTUALCAN)

enum {
    ServerDefaultTcpPort = 35468,
    VirtualChannels = 2
};

static const char RemoteRequestFlag    = 'R';
static const char ExtendedFormatFlag   = 'X';
static const char FlexibleDataRateFlag = 'F';
static const char BitRateSwitchFlag    = 'B';
static const char ErrorStateFlag       = 'E';
static const char LocalEchoFlag        = 'L';

VirtualCanServer::VirtualCanServer(QObject *parent)
    : QObject(parent)
{
    qCDebug(QT_CANBUS_PLUGINS_VIRTUALCAN, "Server [%p] constructed.", this);
}

VirtualCanServer::~VirtualCanServer()
{
    qCDebug(QT_CANBUS_PLUGINS_VIRTUALCAN, "Server [%p] destructed.", this);
}

void VirtualCanServer::start(quint16 port)
{
    // If there is already a server object, return immediately
    if (m_server) {
        qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN, "Server [%p] is already running.", this);
        return;
    }

    // Otherwise try to start a new server. If there is already
    // another server listen on the specified port, give up.
    m_server = new QTcpServer(this);
    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN,
               "Server [%p] could not be started, port %d is already in use.", this, port);
        m_server->deleteLater();
        m_server = nullptr;
        return;
    }

    // Server successfully started
    connect(m_server, &QTcpServer::newConnection, this, &VirtualCanServer::connected);
    qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN,
            "Server [%p] started and listening on port %d.", this, port);
    return;
}

void VirtualCanServer::connected()
{
    while (m_server->hasPendingConnections()) {
        qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN, "Server [%p] client connected.", this);
        QTcpSocket *next = m_server->nextPendingConnection();
        m_serverSockets.append(next);
        connect(next, &QIODevice::readyRead, this, &VirtualCanServer::readyRead);
        connect(next, &QTcpSocket::disconnected, this, &VirtualCanServer::disconnected);
    }
}

void VirtualCanServer::disconnected()
{
    qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN, "Server [%p] client disconnected.", this);

    auto socket = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(socket);

    m_serverSockets.removeOne(socket);
    socket->deleteLater();
}

void VirtualCanServer::readyRead()
{
    auto readSocket = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(readSocket);

    while (readSocket->canReadLine()) {
        const QByteArray command = readSocket->readLine().trimmed();
        qCDebug(QT_CANBUS_PLUGINS_VIRTUALCAN,
                "Server [%p] received: '%s'.", this, command.constData());

        if (command.startsWith("connect:")) {
            const QVariant interfaces = readSocket->property("interfaces");
            QStringList list = interfaces.toStringList();
            list.append(command.mid(int(strlen("connect:"))));
            readSocket->setProperty("interfaces", list);

        } else if (command.startsWith("disconnect:")) {
            const QVariant interfaces = readSocket->property("interfaces");
            QStringList list = interfaces.toStringList();
            list.removeAll(command.mid(int(strlen("disconnect:"))));
            readSocket->setProperty("interfaces", list);
            readSocket->disconnectFromHost();

        } else {
            const QByteArrayList commandList = command.split(':');
            Q_ASSERT(commandList.size() == 2);

            for (QTcpSocket *writeSocket : qAsConst(m_serverSockets)) {
                // Don't send the frame back to its origin
                if (writeSocket == readSocket)
                    continue;

                // Send frame to all clients registered to the same interface as sender
                const QVariant property = writeSocket->property("interfaces");
                if (!property.isValid())
                    continue;

                const QStringList propertyList = property.toStringList();
                if (propertyList.contains(commandList.first()))
                    writeSocket->write(commandList.last() + '\n');
            }
        }
    }
}

Q_GLOBAL_STATIC(VirtualCanServer, g_server)

VirtualCanBackend::VirtualCanBackend(const QString &interface, QObject *parent)
    : QCanBusDevice(parent)
{
    m_url = QUrl(interface);
    const QString canDevice = m_url.fileName();

    const QRegularExpression re(QStringLiteral("can(\\d)"));
    const QRegularExpressionMatch match = re.match(canDevice);

    if (Q_UNLIKELY(!match.hasMatch())) {
        qCWarning(QT_CANBUS_PLUGINS_VIRTUALCAN,
                "Invalid interface '%ls'.", qUtf16Printable(interface));
        setError(tr("Invalid interface '%1'.").arg(interface), QCanBusDevice::ConnectionError);
        return;
    }

    const uint channel = match.captured(1).toUInt();
    if (Q_UNLIKELY(channel >= VirtualChannels)) {
        qCWarning(QT_CANBUS_PLUGINS_VIRTUALCAN,
                "Invalid interface '%ls'.", qUtf16Printable(interface));
        setError(tr("Invalid interface '%1'.").arg(interface), QCanBusDevice::ConnectionError);
        return;
    }

    m_channel = channel;
}

VirtualCanBackend::~VirtualCanBackend()
{
    qCDebug(QT_CANBUS_PLUGINS_VIRTUALCAN, "Client [%p] socket destructed.", this);
}

bool VirtualCanBackend::open()
{
    setState(QCanBusDevice::ConnectingState);

    const QString host = m_url.host();
    const QHostAddress address = host.isEmpty() ? QHostAddress::LocalHost : QHostAddress(host);
    const quint16 port = static_cast<quint16>(m_url.port(ServerDefaultTcpPort));

    if (address.isLoopback())
        g_server->start(port);

    m_clientSocket = new QTcpSocket(this);
    m_clientSocket->connectToHost(address, port, QIODevice::ReadWrite);
    connect(m_clientSocket, &QAbstractSocket::connected, this, &VirtualCanBackend::clientConnected);
    connect(m_clientSocket, &QAbstractSocket::disconnected, this, &VirtualCanBackend::clientDisconnected);
    connect(m_clientSocket, &QIODevice::readyRead, this, &VirtualCanBackend::clientReadyRead);
    qCDebug(QT_CANBUS_PLUGINS_VIRTUALCAN, "Client [%p] socket created.", this);
    return true;
}

void VirtualCanBackend::close()
{
    setState(ClosingState);

    m_clientSocket->write("disconnect:can" + QByteArray::number(m_channel) + '\n');
}

void VirtualCanBackend::setConfigurationParameter(int key, const QVariant &value)
{
    if (key == QCanBusDevice::ReceiveOwnKey || key == QCanBusDevice::CanFdKey)
        QCanBusDevice::setConfigurationParameter(key, value);
}

/*
    Protocol format: All data is in ASCII, one CAN message per line,
    each line ends with line feed '\n'.

    Format:  "<CAN-Channel>:<Flags>#<CAN-ID>#<Data-Bytes>\n"
    Example: "can0:XF#123#123456\n"

    The first part is the destination CAN channel, "can0" or "can1",
    followed by the flags list:

    * R - Remote Request
    * X - Extended Frame Format
    * F - Flexible Data Rate Format
    * B - Bitrate Switch
    * E - Error State Indicator
    * L - Local Echo

    Afterwards the CAN-ID and the data follows, both separated by '#'.
*/

bool VirtualCanBackend::writeFrame(const QCanBusFrame &frame)
{
    if (Q_UNLIKELY(state() != ConnectedState)) {
        qCWarning(QT_CANBUS_PLUGINS_VIRTUALCAN, "Error: Cannot write frame as client is not connected!");
        return false;
    }

    bool canFdEnabled = configurationParameter(QCanBusDevice::CanFdKey).toBool();
    if (Q_UNLIKELY(frame.hasFlexibleDataRateFormat() && !canFdEnabled)) {
        qCWarning(QT_CANBUS_PLUGINS_VIRTUALCAN,
                "Error: Cannot write CAN FD frame as CAN FD is not enabled!");
        return false;
    }

    QByteArray flags;
    if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
        flags.append(RemoteRequestFlag);
    if (frame.hasExtendedFrameFormat())
        flags.append(ExtendedFormatFlag);
    if (frame.hasFlexibleDataRateFormat())
        flags.append(FlexibleDataRateFlag);
    if (frame.hasBitrateSwitch())
        flags.append(BitRateSwitchFlag);
    if (frame.hasErrorStateIndicator())
        flags.append(ErrorStateFlag);
    if (frame.hasLocalEcho())
        flags.append(LocalEchoFlag);
    const QByteArray frameId = QByteArray::number(frame.frameId());
    const QByteArray command = "can" + QByteArray::number(m_channel)
            + ':' + frameId + '#' + flags + '#' + frame.payload().toHex() + '\n';
    m_clientSocket->write(command);

    if (configurationParameter(QCanBusDevice::ReceiveOwnKey).toBool()) {
        const qint64 timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        QCanBusFrame echoFrame = frame;
        echoFrame.setLocalEcho(true);
        echoFrame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(timeStamp * 1000));
        enqueueReceivedFrames({echoFrame});
    }

    return true;
}

QString VirtualCanBackend::interpretErrorFrame(const QCanBusFrame &errorFrame)
{
    Q_UNUSED(errorFrame);
    return QString();
}

QList<QCanBusDeviceInfo> VirtualCanBackend::interfaces()
{
    QList<QCanBusDeviceInfo> result;

    for (int channel = 0; channel < VirtualChannels; ++channel) {
        result.append(std::move(createDeviceInfo(
                                    QStringLiteral("can%1").arg(channel), QString(),
                                    QStringLiteral("Qt Virtual CAN bus"), channel,
                                    true, true)));
    }

    return result;
}

void VirtualCanBackend::clientConnected()
{
    qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN, "Client [%p] socket connected.", this);
    m_clientSocket->write("connect:can" + QByteArray::number(m_channel) + '\n');

    setState(QCanBusDevice::ConnectedState);
}

void VirtualCanBackend::clientDisconnected()
{
    qCInfo(QT_CANBUS_PLUGINS_VIRTUALCAN, "Client [%p] socket disconnected.", this);

    setState(UnconnectedState);
}

void VirtualCanBackend::clientReadyRead()
{
    while (m_clientSocket->canReadLine()) {
        const QByteArray answer = m_clientSocket->readLine().trimmed();
        qCDebug(QT_CANBUS_PLUGINS_VIRTUALCAN, "Client [%p] received: '%s'.",
                this, answer.constData());

        if (answer.startsWith("disconnect:can" + QByteArray::number(m_channel))) {
            m_clientSocket->disconnectFromHost();
            continue;
        }

        const QByteArrayList list = answer.split('#');
        Q_ASSERT(list.size() == 3);

        const quint32 id = list.at(0).toUInt();
        const QByteArray flags = list.at(1);
        const QByteArray data = QByteArray::fromHex(list.at(2));
        const qint64 timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        QCanBusFrame frame(id, data);
        frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(timeStamp * 1000));
        if (flags.contains(RemoteRequestFlag))
            frame.setFrameType(QCanBusFrame::RemoteRequestFrame);
        frame.setExtendedFrameFormat(flags.contains(ExtendedFormatFlag));
        frame.setFlexibleDataRateFormat(flags.contains(FlexibleDataRateFlag));
        frame.setBitrateSwitch(flags.contains(BitRateSwitchFlag));
        frame.setErrorStateIndicator(flags.contains(ErrorStateFlag));
        frame.setLocalEcho(flags.contains(LocalEchoFlag));
        enqueueReceivedFrames({frame});
    }
}

QT_END_NAMESPACE
