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

#include "mmreventmediaplayercontrol.h"
#include "mmreventthread.h"
#include "mmrenderervideowindowcontrol.h"

#include <mm/renderer.h>
#include <tuple>

QT_BEGIN_NAMESPACE

static std::tuple<int, int, bool> parseBufferLevel(const QByteArray &value)
{
    const int slashPos = value.indexOf('/');
    if (slashPos <= 0)
        return std::make_tuple(0, 0, false);

    bool ok = false;
    const int level = value.left(slashPos).toInt(&ok);
    if (!ok || level < 0)
        return std::make_tuple(0, 0, false);

    const int capacity = value.mid(slashPos + 1).toInt(&ok);
    if (!ok || capacity < 0)
        return std::make_tuple(0, 0, false);

    return std::make_tuple(level, capacity, true);
}

MmrEventMediaPlayerControl::MmrEventMediaPlayerControl(QObject *parent)
    : MmRendererMediaPlayerControl(parent)
    , m_eventThread(nullptr)
    , m_bufferStatus("")
    , m_bufferLevel(0)
    , m_bufferCapacity(0)
    , m_position(0)
    , m_suspended(false)
    , m_suspendedReason("unknown")
    , m_state(MMR_STATE_IDLE)
    , m_speed(0)
{
    openConnection();
}

MmrEventMediaPlayerControl::~MmrEventMediaPlayerControl()
{
    destroy();
}

void MmrEventMediaPlayerControl::startMonitoring()
{
    m_eventThread = new MmrEventThread(m_context);

    connect(m_eventThread, &MmrEventThread::eventPending,
            this, &MmrEventMediaPlayerControl::readEvents);

    m_eventThread->setObjectName(QStringLiteral("MmrEventThread-") + QString::number(m_id));
    m_eventThread->start();
}

void MmrEventMediaPlayerControl::stopMonitoring()
{
    delete m_eventThread;
    m_eventThread = nullptr;
}

void MmrEventMediaPlayerControl::resetMonitoring()
{
    m_bufferStatus = "";
    m_bufferLevel = 0;
    m_bufferCapacity = 0;
    m_position = 0;
    m_suspended = false;
    m_suspendedReason = "unknown";
    m_state = MMR_STATE_IDLE;
    m_speed = 0;
}

bool MmrEventMediaPlayerControl::nativeEventFilter(const QByteArray &eventType,
                                                   void *message,
                                                   long *result)
{
    Q_UNUSED(result)
    if (eventType == "screen_event_t") {
        screen_event_t event = static_cast<screen_event_t>(message);
        if (MmRendererVideoWindowControl *control = videoWindowControl())
            control->screenEventHandler(event);
    }

    return false;
}

void MmrEventMediaPlayerControl::readEvents()
{
    const mmr_event_t *event;

    while ((event = mmr_event_get(m_context))) {
        if (event->type == MMR_EVENT_NONE)
            break;

        switch (event->type) {
        case MMR_EVENT_STATUS: {
            if (event->data) {
                const strm_string_t *value;
                value = strm_dict_find_rstr(event->data, "bufferstatus");
                if (value) {
                    m_bufferStatus = QByteArray(strm_string_get(value));
                    if (!m_suspended)
                        setMmBufferStatus(m_bufferStatus);
                }

                value = strm_dict_find_rstr(event->data, "bufferlevel");
                if (value) {
                    const char *cstrValue = strm_string_get(value);
                    int level;
                    int capacity;
                    bool ok;
                    std::tie(level, capacity, ok) = parseBufferLevel(QByteArray(cstrValue));
                    if (!ok) {
                        qCritical("Could not parse buffer capacity from '%s'", cstrValue);
                    } else {
                        m_bufferLevel = level;
                        m_bufferCapacity = capacity;
                        setMmBufferLevel(level, capacity);
                    }
                }

                value = strm_dict_find_rstr(event->data, "suspended");
                if (value) {
                    if (!m_suspended) {
                        m_suspended = true;
                        m_suspendedReason = strm_string_get(value);
                        handleMmSuspend(m_suspendedReason);
                    }
                } else if (m_suspended) {
                    m_suspended = false;
                    handleMmSuspendRemoval(m_bufferStatus);
                }
            }

            if (event->pos_str) {
                const QByteArray valueBa = QByteArray(event->pos_str);
                bool ok;
                m_position = valueBa.toLongLong(&ok);
                if (!ok) {
                    qCritical("Could not parse position from '%s'", valueBa.constData());
                } else {
                    setMmPosition(m_position);
                }
            }
            break;
        }
        case MMR_EVENT_STATE: {
            if (event->state == MMR_STATE_PLAYING && m_speed != event->speed) {
                m_speed = event->speed;
                if (m_speed == 0)
                    handleMmPause();
                else
                    handleMmPlay();
            }
            break;
        }
        case MMR_EVENT_METADATA: {
            updateMetaData(event->data);
            break;
        }
        case MMR_EVENT_ERROR:
        case MMR_EVENT_NONE:
        case MMR_EVENT_OVERFLOW:
        case MMR_EVENT_WARNING:
        case MMR_EVENT_PLAYLIST:
        case MMR_EVENT_INPUT:
        case MMR_EVENT_OUTPUT:
        case MMR_EVENT_CTXTPAR:
        case MMR_EVENT_TRKPAR:
        case MMR_EVENT_OTHER: {
            break;
        }
        }

        // Currently, any exit from the playing state is considered a stop (end-of-media).
        // If you ever need to separate end-of-media from things like "stopped unexpectedly"
        // or "stopped because of an error", you'll find that end-of-media is signaled by an
        // MMR_EVENT_ERROR of MMR_ERROR_NONE with state changed to MMR_STATE_STOPPED.
        if (event->state != m_state && m_state == MMR_STATE_PLAYING)
            handleMmStopped();
        m_state = event->state;
    }

    if (m_eventThread)
        m_eventThread->signalRead();
}

QT_END_NAMESPACE
