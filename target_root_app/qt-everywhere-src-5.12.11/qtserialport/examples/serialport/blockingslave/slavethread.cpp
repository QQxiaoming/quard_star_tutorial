/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "slavethread.h"

#include <QSerialPort>
#include <QTime>

SlaveThread::SlaveThread(QObject *parent) :
    QThread(parent)
{
}

//! [0]
SlaveThread::~SlaveThread()
{
    m_mutex.lock();
    m_quit = true;
    m_mutex.unlock();
    wait();
}
//! [0]

//! [1] //! [2]
void SlaveThread::startSlave(const QString &portName, int waitTimeout, const QString &response)
{
    //! [1]
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
    m_response = response;
    //! [3]
    if (!isRunning())
        start();
}
//! [2] //! [3]

//! [4]
void SlaveThread::run()
{
    bool currentPortNameChanged = false;

    m_mutex.lock();
//! [4] //! [5]
    QString currentPortName;
    if (currentPortName != m_portName) {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = m_waitTimeout;
    QString currentRespone = m_response;
    m_mutex.unlock();
//! [5] //! [6]
    QSerialPort serial;

    while (!m_quit) {
//![6] //! [7]
        if (currentPortNameChanged) {
            serial.close();
            serial.setPortName(currentPortName);

            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(tr("Can't open %1, error code %2")
                           .arg(m_portName).arg(serial.error()));
                return;
            }
        }

        if (serial.waitForReadyRead(currentWaitTimeout)) {
//! [7] //! [8]
            // read request
            QByteArray requestData = serial.readAll();
            while (serial.waitForReadyRead(10))
                requestData += serial.readAll();
//! [8] //! [10]
            // write response
            const QByteArray responseData = currentRespone.toUtf8();
            serial.write(responseData);
            if (serial.waitForBytesWritten(m_waitTimeout)) {
                const QString request = QString::fromUtf8(requestData);
//! [12]
                emit this->request(request);
//! [10] //! [11] //! [12]
            } else {
                emit timeout(tr("Wait write response timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
//! [9] //! [11]
        } else {
            emit timeout(tr("Wait read request timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
//! [9]  //! [13]
        m_mutex.lock();
        if (currentPortName != m_portName) {
            currentPortName = m_portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = m_waitTimeout;
        currentRespone = m_response;
        m_mutex.unlock();
    }
//! [13]
}
