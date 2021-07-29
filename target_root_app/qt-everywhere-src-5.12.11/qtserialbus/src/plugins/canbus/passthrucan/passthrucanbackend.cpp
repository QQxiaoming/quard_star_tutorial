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

#include "passthrucanbackend.h"
#include "passthrucanio.h"

#include <QEventLoop>
#include <QSettings>

namespace {

#ifdef Q_OS_WIN32

static inline QString registryPath()
{
    return QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\PassThruSupport.04.04");
}

static QString canAdapterName(const QSettings &entries)
{
    const int supportsCan = entries.value(QStringLiteral("CAN")).toInt();
    if (supportsCan)
        return entries.value(QStringLiteral("Name")).toString();
    return {};
}

static QString libraryForAdapter(const QString &adapterName)
{
    QString library;
    QSettings entries (registryPath(), QSettings::NativeFormat);
    const QStringList groups = entries.childGroups();

    for (const auto &group : groups) {
        entries.beginGroup(group);

        const QString name = canAdapterName(entries);
        if (!name.isEmpty() && (adapterName.isEmpty() ||
                                name.compare(adapterName, Qt::CaseInsensitive) == 0))
            library = entries.value(QStringLiteral("FunctionLibrary")).toString();

        entries.endGroup();

        if (!library.isEmpty())
            break;
    }
    return library;
}

#else // !Q_OS_WIN32

static QString libraryForAdapter(const QString &adapterName)
{
    // Insert system-specific device name to J2534 library name mapping here.
    // For now, allow the path to the J2534 library to be specified directly
    // as the adapter name.
    return adapterName;
}

#endif // !Q_OS_WIN32

} // anonymous namespace

PassThruCanBackend::PassThruCanBackend(const QString &name, QObject *parent)
    : QCanBusDevice(parent)
    , m_deviceName (name)
    , m_canIO (new PassThruCanIO())
{
    m_canIO->moveToThread(&m_ioThread);

    // Signals emitted by the I/O thread, to be queued.
    connect(m_canIO, &PassThruCanIO::errorOccurred,
            this, &PassThruCanBackend::setError);
    connect(m_canIO, &PassThruCanIO::openFinished,
            this, &PassThruCanBackend::ackOpenFinished);
    connect(m_canIO, &PassThruCanIO::closeFinished,
            this, &PassThruCanBackend::ackCloseFinished);
    connect(m_canIO, &PassThruCanIO::messagesReceived,
            this, &PassThruCanBackend::enqueueReceivedFrames);
    connect(m_canIO, &PassThruCanIO::messagesSent,
            this, &QCanBusDevice::framesWritten);
}

PassThruCanBackend::~PassThruCanBackend()
{
    if (state() != UnconnectedState) {
        // If the I/O thread is still active at this point, we will have to
        // wait for it to finish.
        QEventLoop loop;
        connect(&m_ioThread, &QThread::finished, &loop, &QEventLoop::quit);

        if (state() != ClosingState)
            disconnectDevice();

        while (!m_ioThread.isFinished())
            loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    m_canIO->deleteLater();
}

void PassThruCanBackend::setConfigurationParameter(int key, const QVariant &value)
{
    QCanBusDevice::setConfigurationParameter(key, value);

    if (state() == ConnectedState)
        applyConfig(key, value);
}

bool PassThruCanBackend::writeFrame(const QCanBusFrame &frame)
{
    if (state() != ConnectedState) {
        setError(tr("Device is not connected"), WriteError);
        return false;
    }
    if (!frame.isValid()) {
        setError(tr("Invalid CAN bus frame"), WriteError);
        return false;
    }
    if (frame.frameType() != QCanBusFrame::DataFrame) {
        setError(tr("Unsupported CAN frame type"), WriteError);
        return false;
    }
    // Push the frame directly to the write queue of the worker thread,
    // bypassing the QCanBusDevice output queue. Despite the duplicated
    // queue, things are cleaner this way as it avoids a reverse dependency
    // from the worker object on the QCanBusDevice object.
    return m_canIO->enqueueMessage(frame);
}

QString PassThruCanBackend::interpretErrorFrame(const QCanBusFrame &)
{
    // J2534 Pass-thru v04.04 does not seem to support error frames.
    return {};
}

QList<QCanBusDeviceInfo> PassThruCanBackend::interfaces()
{
    QList<QCanBusDeviceInfo> list;
#ifdef Q_OS_WIN32
    QSettings entries (registryPath(), QSettings::NativeFormat);
    const QStringList groups = entries.childGroups();

    for (const auto &group : groups) {
        entries.beginGroup(group);

        const QString name = canAdapterName(entries);
        if (!name.isEmpty())
            list.append(createDeviceInfo(name));

        entries.endGroup();
    }
#endif
    return list;
}

bool PassThruCanBackend::open()
{
    if (Q_UNLIKELY(state() != ConnectingState)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Unexpected state on open");
        return false;
    }
    // Support a special "adapter%subdevice" syntax to allow control of the
    // device name passed to the J2534 library's PassThruOpen() function.
    // If the "%subdevice" suffix is not used, the J2534 interface library
    // will choose a default or ask the user.
    const int splitPos = m_deviceName.indexOf(QChar::fromLatin1('%'));
    const QString adapter = m_deviceName.left(splitPos);
    QByteArray subDev;

    if (splitPos >= 0)
      subDev = m_deviceName.midRef(splitPos + 1).toLatin1();

    const QString library = libraryForAdapter(adapter);
    if (library.isEmpty()) {
        setError(tr("Adapter not found: %1").arg(adapter), ConnectionError);
        return false;
    }
    bool ok = false;
    uint bitRate = configurationParameter(BitRateKey).toUInt(&ok);
    if (!ok) {
        bitRate = 500*1000; // default initial bit rate
        setConfigurationParameter(BitRateKey, bitRate);
    }
    m_ioThread.start();

    return QMetaObject::invokeMethod(m_canIO, "open", Qt::QueuedConnection,
                                     Q_ARG(QString, library),
                                     Q_ARG(QByteArray, subDev),
                                     Q_ARG(uint, bitRate));
}

void PassThruCanBackend::close()
{
    if (Q_UNLIKELY(state() != ClosingState)) {
        qCCritical(QT_CANBUS_PLUGINS_PASSTHRU, "Unexpected state on close");
        return;
    }
    QMetaObject::invokeMethod(m_canIO, "close", Qt::QueuedConnection);
}

void PassThruCanBackend::ackOpenFinished(bool success)
{
    // Do not transition to connected state if close() has been called
    // in the meantime.
    if (state() != ConnectingState)
        return;

    if (success) {
        const QVariant loopback = configurationParameter(LoopbackKey);
        if (loopback.toBool())
            applyConfig(LoopbackKey, loopback);

        QVariant filters = configurationParameter(RawFilterKey);
        if (!filters.isValid()) {
            // Configure default match-all filter.
            filters = QVariant::fromValue(QList<Filter>{Filter{}});
            setConfigurationParameter(RawFilterKey, filters);
        }
        applyConfig(RawFilterKey, filters);

        QMetaObject::invokeMethod(m_canIO, "listen", Qt::QueuedConnection);

        setState(ConnectedState);
    } else {
        setState(UnconnectedState);
    }
}

void PassThruCanBackend::ackCloseFinished()
{
    m_ioThread.exit(0);
    m_ioThread.wait();

    setState(UnconnectedState);
}

void PassThruCanBackend::applyConfig(int key, const QVariant &value)
{
    QMetaObject::invokeMethod(m_canIO, "applyConfig", Qt::QueuedConnection,
                              Q_ARG(int, key), Q_ARG(QVariant, value));
}
