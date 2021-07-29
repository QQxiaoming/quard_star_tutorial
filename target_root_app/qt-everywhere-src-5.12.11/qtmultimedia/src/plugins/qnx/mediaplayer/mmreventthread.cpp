/****************************************************************************
**
** Copyright (C) 2017 QNX Software Systems. All rights reserved.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
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

#include "mmreventthread.h"

#include <QtCore/QDebug>

#include <errno.h>
#include <mm/renderer/events.h>
#include <sys/neutrino.h>

static const int c_mmrCode = _PULSE_CODE_MINAVAIL + 0;
static const int c_readCode = _PULSE_CODE_MINAVAIL + 1;
static const int c_quitCode = _PULSE_CODE_MINAVAIL + 2;

MmrEventThread::MmrEventThread(mmr_context_t *context)
    : QThread(),
      m_mmrContext(context)
{
    if (Q_UNLIKELY((m_channelId = ChannelCreate(_NTO_CHF_DISCONNECT
                                                | _NTO_CHF_UNBLOCK
                                                | _NTO_CHF_PRIVATE)) == -1)) {
        qFatal("MmrEventThread: Can't continue without a channel");
    }

    if (Q_UNLIKELY((m_connectionId = ConnectAttach(0, 0, m_channelId,
                                                   _NTO_SIDE_CHANNEL, 0)) == -1)) {
        ChannelDestroy(m_channelId);
        qFatal("MmrEventThread: Can't continue without a channel connection");
    }

    SIGEV_PULSE_INIT(&m_mmrEvent, m_connectionId, SIGEV_PULSE_PRIO_INHERIT, c_mmrCode, 0);
}

MmrEventThread::~MmrEventThread()
{
    // block until thread terminates
    shutdown();

    ConnectDetach(m_connectionId);
    ChannelDestroy(m_channelId);
}

void MmrEventThread::run()
{
    int armResult = mmr_event_arm(m_mmrContext, &m_mmrEvent);
    if (armResult > 0)
        emit eventPending();

    while (1) {
        struct _pulse msg;
        memset(&msg, 0, sizeof(msg));
        int receiveId = MsgReceive(m_channelId, &msg, sizeof(msg), nullptr);
        if (receiveId == 0) {
            if (msg.code == c_mmrCode) {
                emit eventPending();
            } else if (msg.code == c_readCode) {
                armResult = mmr_event_arm(m_mmrContext, &m_mmrEvent);
                if (armResult > 0)
                    emit eventPending();
            } else if (msg.code == c_quitCode) {
                break;
            } else {
                qWarning() << Q_FUNC_INFO << "Unexpected pulse" << msg.code;
            }
        } else if (receiveId > 0) {
            qWarning() << Q_FUNC_INFO << "Unexpected message" << msg.code;
        } else {
            qWarning() << Q_FUNC_INFO << "MsgReceive error" << strerror(errno);
        }
    }
}

void MmrEventThread::signalRead()
{
    MsgSendPulse(m_connectionId, SIGEV_PULSE_PRIO_INHERIT, c_readCode, 0);
}

void MmrEventThread::shutdown()
{
    MsgSendPulse(m_connectionId, SIGEV_PULSE_PRIO_INHERIT, c_quitCode, 0);

    // block until thread terminates
    wait();
}
