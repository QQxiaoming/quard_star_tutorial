/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company.
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

#include "passthrucanio.h"

#include <QLoggingCategory>
#include <QtEndian>

#include <cstring>

PassThruCanIO::PassThruCanIO(QObject *parent)
    : QObject(parent)
    , m_ioBuffer (8, J2534::Message(J2534::Protocol::CAN))
{
}

PassThruCanIO::~PassThruCanIO()
{
}

void PassThruCanIO::open(const QString &library, const QByteArray &subDev, uint bitRate)
{
    if (Q_UNLIKELY(m_passThru)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Pass-thru interface already open");
        emit openFinished(false);
        return;
    }
    qCDebug(QT_CANBUS_PLUGINS_PASSTHRU, "Loading interface library: %ls",
            qUtf16Printable(library));

    m_passThru = new J2534::PassThru(library, this);
    J2534::PassThru::Status openStatus = m_passThru->lastError();

    if (openStatus == J2534::PassThru::NoError)
        openStatus = m_passThru->open(subDev, &m_deviceId);

    if (openStatus == J2534::PassThru::NoError
            && m_passThru->connect(m_deviceId, J2534::Protocol::CAN,
                                   J2534::PassThru::CAN29BitID | J2534::PassThru::CANIDBoth,
                                   bitRate, &m_channelId) == J2534::PassThru::NoError) {
        emit openFinished(true);
        return;
    }
    emit errorOccurred(m_passThru->lastErrorString(),
                       QCanBusDevice::ConnectionError);

    if (openStatus == J2534::PassThru::NoError
            && m_passThru->close(m_deviceId) != J2534::PassThru::NoError)
        qCWarning(QT_CANBUS_PLUGINS_PASSTHRU, "Failed to close pass-thru device");

    delete m_passThru;
    m_passThru = nullptr;

    emit openFinished(false);
}

void PassThruCanIO::close()
{
    if (Q_LIKELY(m_passThru)) {
        delete m_idleNotifier;
        m_idleNotifier = nullptr;

        if (m_passThru->disconnect(m_channelId) != J2534::PassThru::NoError
                || m_passThru->close(m_deviceId) != J2534::PassThru::NoError) {

            qCWarning(QT_CANBUS_PLUGINS_PASSTHRU, "Failed to close pass-thru device");
            emit errorOccurred(m_passThru->lastErrorString(),
                               QCanBusDevice::ConnectionError);
        }
        delete m_passThru;
        m_passThru = nullptr;
    }
    emit closeFinished();
}

void PassThruCanIO::applyConfig(int key, const QVariant &value)
{
    if (Q_UNLIKELY(!m_passThru)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Pass-thru interface not open");
        return;
    }
    bool success = true;

    switch (key) {
    case QCanBusDevice::RawFilterKey:
        success = setMessageFilters(qvariant_cast<QList<QCanBusDevice::Filter>>(value));
        break;
    case QCanBusDevice::LoopbackKey:
        success = setConfigValue(J2534::Config::Loopback, value.toBool());
        break;
    case QCanBusDevice::BitRateKey:
        success = setConfigValue(J2534::Config::DataRate, value.toUInt());
        break;
    default:
        emit errorOccurred(tr("Unsupported configuration key: %1").arg(key),
                           QCanBusDevice::ConfigurationError);
        break;
    }
    if (!success) {
        emit errorOccurred(tr("Configuration failed: %1").arg(m_passThru->lastErrorString()),
                           QCanBusDevice::ConfigurationError);
    }
}

void PassThruCanIO::listen()
{
    if (Q_UNLIKELY(!m_passThru)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Pass-thru interface not open");
        return;
    }
    if (Q_UNLIKELY(m_idleNotifier)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Idle notifier already created");
        return;
    }
    m_idleNotifier = new QTimer(this);
    connect(m_idleNotifier, &QTimer::timeout, this, &PassThruCanIO::pollForMessages);

    m_idleNotifier->start(0);
}

bool PassThruCanIO::enqueueMessage(const QCanBusFrame &frame)
{
    const QMutexLocker lock (&m_writeGuard);
    m_writeQueue.append(frame);
    return true;
}

bool PassThruCanIO::setMessageFilters(const QList<QCanBusDevice::Filter> &filters)
{
    if (m_passThru->clear(m_channelId, J2534::PassThru::MsgFilters) != J2534::PassThru::NoError)
        return false;

    J2534::Message pattern {J2534::Protocol::CAN};
    pattern.setSize(4);
    J2534::Message mask {J2534::Protocol::CAN};
    mask.setSize(4);

    for (const auto &filter : filters) {
        if (filter.type != QCanBusFrame::DataFrame
                && filter.type != QCanBusFrame::InvalidFrame) {
            emit errorOccurred(tr("Configuration failed: unsupported filter type"),
                               QCanBusDevice::ConfigurationError);
            break;
        }
        if (filter.format & QCanBusDevice::Filter::MatchExtendedFormat)
            pattern.setRxStatus(J2534::Message::InCAN29BitID);
        else
            pattern.setRxStatus({});

        if (filter.format != QCanBusDevice::Filter::MatchBaseAndExtendedFormat)
            mask.setRxStatus(J2534::Message::InCAN29BitID);
        else
            mask.setRxStatus({});

        qToBigEndian<quint32>(filter.frameId & filter.frameIdMask, pattern.data());
        qToBigEndian<quint32>(filter.frameIdMask, mask.data());

        if (m_passThru->startMsgFilter(m_channelId, J2534::PassThru::PassFilter,
                                       mask, pattern) != J2534::PassThru::NoError)
            return false;
    }
    return true;
}

bool PassThruCanIO::setConfigValue(J2534::Config::Parameter param, ulong value)
{
    const J2534::Config config {param, value};

    return (m_passThru->setConfig(m_channelId, &config) == J2534::PassThru::NoError);
}

void PassThruCanIO::pollForMessages()
{
    if (Q_UNLIKELY(!m_passThru)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Pass-thru interface not open");
        return;
    }
    const bool writePending = writeMessages();
    readMessages(writePending);
}

bool PassThruCanIO::writeMessages()
{
    ulong numMsgs = m_ioBuffer.size();
    {
        const QMutexLocker lock (&m_writeGuard);
        numMsgs = qMin<ulong>(m_writeQueue.size(), numMsgs);

        for (ulong i = 0; i < numMsgs; ++i) {
            const QCanBusFrame &frame = m_writeQueue.at(i);
            J2534::Message &msg = m_ioBuffer[i];

            const QByteArray payload = frame.payload();
            const ulong payloadSize = qMin<ulong>(payload.size(),
                                                  J2534::Message::maxSize - 4);
            msg.setRxStatus({});
            msg.setTimestamp(0);
            msg.setSize(4 + payloadSize);
            msg.setExtraDataIndex(0);

            if (frame.hasExtendedFrameFormat())
                msg.setTxFlags(J2534::Message::OutCAN29BitID);
            else
                msg.setTxFlags({});

            qToBigEndian<quint32>(frame.frameId(), msg.data());
            std::memcpy(msg.data() + 4, payload.data(), payloadSize);
        }
    }
    if (numMsgs == 0)
        return false;

    const auto status = m_passThru->writeMsgs(m_channelId, m_ioBuffer.constData(),
                                              &numMsgs, pollTimeout);
    if (status == J2534::PassThru::BufferFull)
        return false;

    if (status != J2534::PassThru::NoError && status != J2534::PassThru::Timeout) {
        emit errorOccurred(tr("Message write failed: %1").arg(m_passThru->lastErrorString()),
                           QCanBusDevice::WriteError);
        return false;
    }
    if (numMsgs == 0)
        return false;

    bool morePending;
    {
        const QMutexLocker lock (&m_writeGuard);
        // De-queue successfully written frames.
        m_writeQueue.erase(m_writeQueue.begin(), m_writeQueue.begin() + numMsgs);
        morePending = !m_writeQueue.isEmpty();
    }
    emit messagesSent(numMsgs);

    return morePending;
}

void PassThruCanIO::readMessages(bool writePending)
{
    // If there are outgoing messages waiting to be written, just check
    // for already received messages but do not block waiting for more.
    const uint timeout = (writePending) ? 0 : pollTimeout;

    ulong numMsgs = m_ioBuffer.size();
    const auto status = m_passThru->readMsgs(m_channelId, m_ioBuffer.data(),
                                             &numMsgs, timeout);
    if (status == J2534::PassThru::BufferEmpty)
        return;

    if (status != J2534::PassThru::NoError && status != J2534::PassThru::Timeout) {
        emit errorOccurred(tr("Message read failed: %1").arg(m_passThru->lastErrorString()),
                           QCanBusDevice::ReadError);
        if (status != J2534::PassThru::BufferOverflow)
            return;
    }
    const int numFrames = qMin<ulong>(m_ioBuffer.size(), numMsgs);
    QVector<QCanBusFrame> frames;
    frames.reserve(numFrames);

    for (int i = 0; i < numFrames; ++i) {
        const J2534::Message &msg = m_ioBuffer.at(i);
        if (Q_UNLIKELY(msg.size() < 4)
                || Q_UNLIKELY(msg.size() > J2534::Message::maxSize)) {
            // This normally shouldn't happen, so a log message is appropriate.
            qCWarning(QT_CANBUS_PLUGINS_PASSTHRU,
                      "Message with invalid size %lu received", msg.size());
            continue;
        }
        const quint32 msgId = qFromBigEndian<quint32>(msg.data());
        const QByteArray payload (msg.data() + 4, msg.size() - 4);

        QCanBusFrame frame (msgId, payload);
        frame.setExtendedFrameFormat((msg.rxStatus() & J2534::Message::InCAN29BitID) != 0);
        frame.setLocalEcho((msg.rxStatus() & J2534::Message::InTxMsgType) != 0);
        frame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(msg.timestamp()));

        frames.append(std::move(frame));
    }
    if (!frames.isEmpty())
        emit messagesReceived(std::move(frames));
}
