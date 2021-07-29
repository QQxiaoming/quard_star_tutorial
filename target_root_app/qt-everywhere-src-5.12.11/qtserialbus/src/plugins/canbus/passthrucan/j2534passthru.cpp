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

#include "j2534passthru.h"

#include <QtEndian>

#include <cstring>

namespace {

enum Ioctl {
    GetConfig = 1,
    SetConfig = 2
};

// Template to model the structs SCONFIG_LIST, SBYTE_ARRAY etc as defined
// in the J2534 spec.
template <typename T>
struct SArray
{
    SArray(ulong n, T *p) : num(n), ptr(p) {}
    // On Windows x64, ulong is 32 bit wide and thus the value would normally
    // be padded so that the pointer begins on a 64-bit boundary. It is not
    // clear from the J2534 spec whether structs should be packed or not on
    // x64. Most vendors still only provide a 32-bit DLL, but there is at
    // least one x64 implementation (from Hatteland Display) out there which
    // does not pack this struct.
    ulong num;
    T *ptr;
};

// Fixed-length string buffers must be at least 80 bytes according to the spec.
// The example code in the spec document uses 256 bytes though -- let's play it
// safe and do so, too.
const int StringBufferSize = 256;

} // anonymous namespace

Q_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_PASSTHRU, "qt.canbus.plugins.passthru", QtWarningMsg)

namespace J2534 {

Message::Message()
{
    std::memset(m_data, 0, sizeof(m_data));
}

Message::Message(Protocol proto)
    : m_protocolId (ulong(proto))
{
    std::memset(m_data, 0, sizeof(m_data));
}

PassThru::PassThru(const QString &libraryPath, QObject *parent)
    : QObject(parent)
    , m_libJ2534 (libraryPath, this)
{
    if (!m_libJ2534.load()
            || !resolveApiFunction(&m_ptOpen, "PassThruOpen")
            || !resolveApiFunction(&m_ptClose, "PassThruClose")
            || !resolveApiFunction(&m_ptConnect, "PassThruConnect")
            || !resolveApiFunction(&m_ptDisconnect, "PassThruDisconnect")
            || !resolveApiFunction(&m_ptReadMsgs, "PassThruReadMsgs")
            || !resolveApiFunction(&m_ptWriteMsgs, "PassThruWriteMsgs")
            || !resolveApiFunction(&m_ptStartMsgFilter, "PassThruStartMsgFilter")
            || !resolveApiFunction(&m_ptGetLastError, "PassThruGetLastError")
            || !resolveApiFunction(&m_ptIoctl, "PassThruIoctl")) {

        m_lastError = LoadFailed;
        m_lastErrorString = m_libJ2534.errorString();

        qCWarning(QT_CANBUS_PLUGINS_PASSTHRU, "%ls", qUtf16Printable(m_lastErrorString));
    }
}

PassThru::~PassThru()
{
    m_libJ2534.unload();
}

PassThru::Status PassThru::open(const QByteArray &name, Handle *deviceId)
{
    Q_ASSERT(m_ptOpen);

    const char *const devName = (name.isEmpty()) ? nullptr : name.data();
    const long status = (*m_ptOpen)(devName, deviceId);
    return handleResult(status);
}

PassThru::Status PassThru::close(Handle deviceId)
{
    Q_ASSERT(m_ptClose);

    const long status = (*m_ptClose)(deviceId);
    return handleResult(status);
}

PassThru::Status PassThru::connect(Handle deviceId, Protocol protocolId,
                                   ConnectFlags flags, uint baudRate, Handle *channelId)
{
    Q_ASSERT(m_ptConnect);

    const long status = (*m_ptConnect)(deviceId, ulong(protocolId),
                                       flags, baudRate, channelId);
    return handleResult(status);
}

PassThru::Status PassThru::disconnect(Handle channelId)
{
    Q_ASSERT(m_ptDisconnect);

    const long status = (*m_ptDisconnect)(channelId);
    return handleResult(status);
}

PassThru::Status PassThru::readMsgs(Handle channelId, Message *msgs,
                                    ulong *numMsgs, uint timeout)
{
    Q_ASSERT(m_ptReadMsgs);

    const long status = (*m_ptReadMsgs)(channelId, msgs, numMsgs, timeout);
    return handleResult(status);
}

PassThru::Status PassThru::writeMsgs(Handle channelId, const Message *msgs,
                                     ulong *numMsgs, uint timeout)
{
    Q_ASSERT(m_ptWriteMsgs);

    const long status = (*m_ptWriteMsgs)(channelId, msgs, numMsgs, timeout);
    return handleResult(status);
}

PassThru::Status PassThru::startMsgFilter(Handle channelId, FilterType filterType,
                                          const Message &maskMsg, const Message &patternMsg)
{
    Q_ASSERT(m_ptStartMsgFilter);

    // The CAN pass-thru plugin implementation does not need the filter ID.
    Handle filterId = 0;

    const long status = (*m_ptStartMsgFilter)(channelId, filterType, &maskMsg,
                                              &patternMsg, nullptr, &filterId);
    return handleResult(status);
}

PassThru::Status PassThru::setConfig(Handle channelId, const Config *params, ulong numParams)
{
    Q_ASSERT(m_ptIoctl);

    const SArray<const Config> configList {numParams, params};
    const long status = (*m_ptIoctl)(channelId, SetConfig, &configList, nullptr);
    return handleResult(status);
}

PassThru::Status PassThru::clear(Handle channelId, ClearTarget target)
{
    Q_ASSERT(m_ptIoctl);

    const long status = (*m_ptIoctl)(channelId, target, nullptr, nullptr);
    return handleResult(status);
}

QString PassThru::lastErrorString() const
{
    return m_lastErrorString;
}

PassThru::Status PassThru::handleResult(long statusCode)
{
    if (Q_UNLIKELY(statusCode != NoError)) {
        m_lastError = Status(statusCode);

        QByteArray description (StringBufferSize, 0);
        Q_ASSERT(m_ptGetLastError);
        const long descStatus = (*m_ptGetLastError)(description.data());

        if (Q_LIKELY(descStatus == NoError)) {
            m_lastErrorString = QString::fromLatin1(description);
        } else {
            m_lastErrorString = tr("Command failed with status code %1").arg(statusCode);
            qCWarning(QT_CANBUS_PLUGINS_PASSTHRU, "GetLastError failed with code %ld", descStatus);
        }
    }
    return Status(statusCode);
}

} // namespace J2534
