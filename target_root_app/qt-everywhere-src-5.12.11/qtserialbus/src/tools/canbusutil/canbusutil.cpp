/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the tools applications of the QtSerialBus module.
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

#include "canbusutil.h"

#include <QCoreApplication>
#include <QTextStream>

CanBusUtil::CanBusUtil(QTextStream &output, QCoreApplication &app, QObject *parent) :
    QObject(parent),
    m_canBus(QCanBus::instance()),
    m_output(output),
    m_app(app),
    m_readTask(new ReadTask(output, this))
{
}

void CanBusUtil::setShowTimeStamp(bool showTimeStamp)
{
    m_readTask->setShowTimeStamp(showTimeStamp);
}

void CanBusUtil::setShowFdFlags(bool showFdFlags)
{
    m_readTask->setShowFdFlags(showFdFlags);
}

void CanBusUtil::setConfigurationParameter(QCanBusDevice::ConfigurationKey key,
                                           const QVariant &value)
{
    m_configurationParameter[key] = value;
}

bool CanBusUtil::start(const QString &pluginName, const QString &deviceName, const QString &data)
{
    if (!m_canBus) {
        m_output << tr("Error: Cannot create QCanBus.") << endl;
        return false;
    }

    m_pluginName = pluginName;
    m_deviceName = deviceName;
    m_data = data;
    m_listening = data.isEmpty();

    if (!connectCanDevice())
        return false;

    if (m_listening) {
        if (m_readTask->isShowFdFlags())
             m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);
        connect(m_canDevice.data(), &QCanBusDevice::framesReceived,
                m_readTask, &ReadTask::handleFrames);
    } else {
        if (!sendData())
            return false;
        QTimer::singleShot(0, &m_app, QCoreApplication::quit);
    }

    return true;
}

int CanBusUtil::printPlugins()
{
    if (!m_canBus) {
        m_output << tr("Error: Cannot create QCanBus.") << endl;
        return 1;
    }

    const QStringList plugins = m_canBus->plugins();
    for (const QString &plugin : plugins)
        m_output << plugin << endl;
    return 0;
}

int CanBusUtil::printDevices(const QString &pluginName)
{
    if (!m_canBus) {
        m_output << tr("Error: Cannot create QCanBus.") << endl;
        return 1;
    }

    QString errorMessage;
    const QList<QCanBusDeviceInfo> devices = m_canBus->availableDevices(pluginName, &errorMessage);
    if (!errorMessage.isEmpty()) {
        m_output << tr("Error gathering available devices: '%1'").arg(errorMessage) << endl;
        return 1;
    }

    for (const QCanBusDeviceInfo &info : devices)
        m_output << info.name() << endl;
    return 0;
}

bool CanBusUtil::parseDataField(quint32 &id, QString &payload)
{
    int hashMarkPos = m_data.indexOf('#');
    if (hashMarkPos < 0) {
        m_output << tr("Data field invalid: No hash mark found!") << endl;
        return false;
    }

    id = m_data.leftRef(hashMarkPos).toUInt(nullptr, 16);
    payload = m_data.right(m_data.size() - hashMarkPos - 1);

    return true;
}

bool CanBusUtil::setFrameFromPayload(QString payload, QCanBusFrame *frame)
{
    if (!payload.isEmpty() && payload.at(0).toUpper() == 'R') {
        frame->setFrameType(QCanBusFrame::RemoteRequestFrame);

        if (payload.size() == 1) // payload = "R"
            return true;

        bool ok = false;
        int rtrFrameLength = payload.midRef(1).toInt(&ok);
        if (ok && rtrFrameLength >= 0 && rtrFrameLength <= 8) { // payload = "R8"
            frame->setPayload(QByteArray(rtrFrameLength, 0));
            return true;
        }

        m_output << tr("Error: RTR frame length must be between 0 and 8 (including).") << endl;
        return false;
    }

    if (!payload.isEmpty() && payload.at(0) == '#') {
        frame->setFlexibleDataRateFormat(true);
        payload.remove(0, 1);
    }

    const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f]*$"));
    if (!re.match(payload).hasMatch()) {
        m_output << tr("Data field invalid: Only hex numbers allowed.") << endl;
        return false;
    }

    if (payload.size() % 2 != 0) {
        if (frame->hasFlexibleDataRateFormat()) {
            enum { BitrateSwitchFlag = 1, ErrorStateIndicatorFlag = 2 };
            const int flags = payload.leftRef(1).toInt(nullptr, 16);
            frame->setBitrateSwitch(flags & BitrateSwitchFlag);
            frame->setErrorStateIndicator(flags & ErrorStateIndicatorFlag);
            payload.remove(0, 1);
        } else {
            m_output << tr("Data field invalid: Size is not multiple of two.") << endl;
            return false;
        }
    }

    QByteArray bytes = QByteArray::fromHex(payload.toLatin1());

    const int maxSize = frame->hasFlexibleDataRateFormat() ? 64 : 8;
    if (bytes.size() > maxSize) {
        m_output << tr("Data field invalid: Size is longer than %1 bytes.").arg(maxSize) << endl;
        return false;
    }

    frame->setPayload(bytes);

    return true;
}

bool CanBusUtil::connectCanDevice()
{
    if (!m_canBus->plugins().contains(m_pluginName)) {
        m_output << tr("Cannot find CAN bus plugin '%1'.").arg(m_pluginName) << endl;
        return false;
    }

    m_canDevice.reset(m_canBus->createDevice(m_pluginName, m_deviceName));
    if (!m_canDevice) {
        m_output << tr("Cannot create CAN bus device: '%1'").arg(m_deviceName) << endl;
        return false;
    }

    const auto constEnd = m_configurationParameter.constEnd();
    for (auto i = m_configurationParameter.constBegin(); i != constEnd; ++i)
        m_canDevice->setConfigurationParameter(i.key(), i.value());

    connect(m_canDevice.data(), &QCanBusDevice::errorOccurred, m_readTask, &ReadTask::handleError);
    if (!m_canDevice->connectDevice()) {
        m_output << tr("Cannot create CAN bus device: '%1'").arg(m_deviceName) << endl;
        return false;
    }

    return true;
}

bool CanBusUtil::sendData()
{
    quint32 id;
    QString payload;
    QCanBusFrame frame;

    if (!parseDataField(id, payload))
        return false;

    if (!setFrameFromPayload(payload, &frame))
        return false;

    if (id > 0x1FFFFFFF) { // 29 bits
        m_output << tr("Cannot send invalid frame ID: '%1'").arg(id, 0, 16) << endl;
        return false;
    }

    frame.setFrameId(id);

    if (frame.hasFlexibleDataRateFormat())
        m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);

    return m_canDevice->writeFrame(frame);
}
