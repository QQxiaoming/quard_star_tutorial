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

#ifndef VIRTUALCANBACKEND_H
#define VIRTUALCANBACKEND_H

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <QtCore/qlist.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QTcpServer;
class QTcpSocket;

class VirtualCanServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VirtualCanServer)

public:
    explicit VirtualCanServer(QObject *parent = nullptr);
    ~VirtualCanServer() override;

    void start(quint16 port);

private:
    void connected();
    void disconnected();
    void readyRead();

    QTcpServer *m_server = nullptr;
    QList<QTcpSocket *> m_serverSockets;
};

class VirtualCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DISABLE_COPY(VirtualCanBackend)

public:
    explicit VirtualCanBackend(const QString &interface, QObject *parent = nullptr);
    ~VirtualCanBackend() override;

    bool open() override;
    void close() override;

    void setConfigurationParameter(int key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &frame) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static QList<QCanBusDeviceInfo> interfaces();

private:
    void clientConnected();
    void clientDisconnected();
    void clientReadyRead();

    QUrl m_url;
    uint m_channel = 0;
    QTcpSocket *m_clientSocket = nullptr;
};

QT_END_NAMESPACE

#endif // VIRTUALCANBACKEND_H
