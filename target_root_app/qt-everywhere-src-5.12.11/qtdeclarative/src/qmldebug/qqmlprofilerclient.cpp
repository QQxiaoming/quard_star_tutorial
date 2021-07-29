/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmlprofilerclient_p_p.h"
#include "qqmldebugconnection_p.h"

QT_BEGIN_NAMESPACE

QQmlProfilerClientPrivate::~QQmlProfilerClientPrivate()
{
}

int QQmlProfilerClientPrivate::resolveType(const QQmlProfilerTypedEvent &event)
{
    int typeIndex = -1;
    if (event.serverTypeId != 0) {
        QHash<qint64, int>::ConstIterator it = serverTypeIds.constFind(event.serverTypeId);

        if (it != serverTypeIds.constEnd()) {
            typeIndex = it.value();
        } else {
            typeIndex = eventReceiver->numLoadedEventTypes();
            eventReceiver->addEventType(event.type);
            serverTypeIds[event.serverTypeId] = typeIndex;
        }
    } else {
        QHash<QQmlProfilerEventType, int>::ConstIterator it = eventTypeIds.constFind(event.type);

        if (it != eventTypeIds.constEnd()) {
            typeIndex = it.value();
        } else {
            typeIndex = eventReceiver->numLoadedEventTypes();
            eventReceiver->addEventType(event.type);
            eventTypeIds[event.type] = typeIndex;
        }
    }
    return typeIndex;
}

int QQmlProfilerClientPrivate::resolveStackTop()
{
    if (rangesInProgress.isEmpty())
        return -1;

    QQmlProfilerTypedEvent &typedEvent = rangesInProgress.top();
    int typeIndex = typedEvent.event.typeIndex();
    if (typeIndex >= 0)
        return typeIndex;

    typeIndex = resolveType(typedEvent);
    typedEvent.event.setTypeIndex(typeIndex);
    while (!pendingMessages.isEmpty()
           && pendingMessages.head().timestamp() < typedEvent.event.timestamp()) {
        forwardEvents(pendingMessages.dequeue());
    }
    forwardEvents(typedEvent.event);
    return typeIndex;
}

void QQmlProfilerClientPrivate::forwardEvents(const QQmlProfilerEvent &last)
{
    forwardDebugMessages(last.timestamp());
    eventReceiver->addEvent(last);
}

void QQmlProfilerClientPrivate::forwardDebugMessages(qint64 untilTimestamp)
{
    while (!pendingDebugMessages.isEmpty()
           && pendingDebugMessages.front().timestamp() <= untilTimestamp) {
        eventReceiver->addEvent(pendingDebugMessages.dequeue());
    }
}

void QQmlProfilerClientPrivate::processCurrentEvent()
{
    // RangeData and RangeLocation always apply to the range on the top of the stack. Furthermore,
    // all ranges are perfectly nested. This is why we can defer the type resolution until either
    // the range ends or a child range starts. With only the information in RangeStart we wouldn't
    // be able to uniquely identify the event type.
    Message rangeStage = currentEvent.type.rangeType() == MaximumRangeType ?
                currentEvent.type.message() : currentEvent.event.rangeStage();
    switch (rangeStage) {
    case RangeStart:
        resolveStackTop();
        rangesInProgress.push(currentEvent);
        break;
    case RangeEnd: {
        int typeIndex = resolveStackTop();
        if (typeIndex == -1)
            break;
        currentEvent.event.setTypeIndex(typeIndex);
        while (!pendingMessages.isEmpty())
            forwardEvents(pendingMessages.dequeue());
        forwardEvents(currentEvent.event);
        rangesInProgress.pop();
        break;
    }
    case RangeData:
        if (!rangesInProgress.isEmpty())
            rangesInProgress.top().type.setData(currentEvent.type.data());
        break;
    case RangeLocation:
        if (!rangesInProgress.isEmpty())
            rangesInProgress.top().type.setLocation(currentEvent.type.location());
        break;
    case DebugMessage:
        currentEvent.event.setTypeIndex(resolveType(currentEvent));
        pendingDebugMessages.enqueue(currentEvent.event);
        break;
    default: {
        int typeIndex = resolveType(currentEvent);
        currentEvent.event.setTypeIndex(typeIndex);
        if (rangesInProgress.isEmpty())
            forwardEvents(currentEvent.event);
        else
            pendingMessages.enqueue(currentEvent.event);
        break;
    }
    }
}

void QQmlProfilerClientPrivate::sendRecordingStatus(int engineId)
{
    Q_Q(QQmlProfilerClient);
    QPacket stream(connection->currentDataStreamVersion());
    stream << recording << engineId; // engineId -1 is OK. It means "all of them"
    if (recording) {
        stream << requestedFeatures << flushInterval;
        stream << true; // yes, we support type IDs
    }
    q->sendMessage(stream.data());
}

QQmlProfilerClient::QQmlProfilerClient(QQmlDebugConnection *connection,
                                       QQmlProfilerEventReceiver *eventReceiver,
                                       quint64 features)
    : QQmlDebugClient(*(new QQmlProfilerClientPrivate(connection, eventReceiver)))
{
    Q_D(QQmlProfilerClient);
    setRequestedFeatures(features);
    connect(this, &QQmlDebugClient::stateChanged, this, &QQmlProfilerClient::onStateChanged);
    connect(d->engineControl.data(), &QQmlEngineControlClient::engineAboutToBeAdded,
            this, &QQmlProfilerClient::sendRecordingStatus);
    connect(d->engineControl.data(), &QQmlEngineControlClient::engineAboutToBeRemoved,
            this, [d](int engineId) {
        // We may already be done with that engine. Then we don't need to block it.
        if (d->trackedEngines.contains(engineId))
            d->engineControl->blockEngine(engineId);
    });
    connect(this, &QQmlProfilerClient::traceFinished,
            d->engineControl.data(), [d](qint64 timestamp, const QList<int> &engineIds) {
        Q_UNUSED(timestamp);
        // The engines might not be blocked because the trace can get finished before engine control
        // sees them.
        for (int blocked : d->engineControl->blockedEngines()) {
            if (engineIds.contains(blocked))
                d->engineControl->releaseEngine(blocked);
        }
    });
}

QQmlProfilerClient::~QQmlProfilerClient()
{
    //Disable profiling if started by client
    //Profiling data will be lost!!
    if (isRecording())
        setRecording(false);
}

void QQmlProfilerClient::clearEvents()
{
    Q_D(QQmlProfilerClient);
    d->rangesInProgress.clear();
    d->pendingMessages.clear();
    d->pendingDebugMessages.clear();
    if (d->recordedFeatures != 0) {
        d->recordedFeatures = 0;
        emit recordedFeaturesChanged(0);
    }
    emit cleared();
}

void QQmlProfilerClient::clearAll()
{
    Q_D(QQmlProfilerClient);
    d->serverTypeIds.clear();
    d->eventTypeIds.clear();
    d->trackedEngines.clear();
    clearEvents();
}

void QQmlProfilerClientPrivate::finalize()
{
    while (!rangesInProgress.isEmpty()) {
        currentEvent = rangesInProgress.top();
        currentEvent.event.setRangeStage(RangeEnd);
        currentEvent.event.setTimestamp(maximumTime);
        processCurrentEvent();
    }
    forwardDebugMessages(std::numeric_limits<qint64>::max());
}


void QQmlProfilerClient::sendRecordingStatus(int engineId)
{
    Q_D(QQmlProfilerClient);
    d->sendRecordingStatus(engineId);
}

bool QQmlProfilerClient::isRecording() const
{
    Q_D(const QQmlProfilerClient);
    return d->recording;
}

void QQmlProfilerClient::setRecording(bool v)
{
    Q_D(QQmlProfilerClient);
    if (v == d->recording)
        return;

    d->recording = v;

    if (state() == Enabled)
        sendRecordingStatus();

    emit recordingChanged(v);
}

quint64 QQmlProfilerClient::recordedFeatures() const
{
    Q_D(const QQmlProfilerClient);
    return d->recordedFeatures;
}

void QQmlProfilerClient::setRequestedFeatures(quint64 features)
{
    Q_D(QQmlProfilerClient);
    d->requestedFeatures = features;
    if (features & static_cast<quint64>(1) << ProfileDebugMessages) {
        if (d->messageClient.isNull()) {
            d->messageClient.reset(new QQmlDebugMessageClient(connection()));
            connect(d->messageClient.data(), &QQmlDebugMessageClient::message, this,
                    [this](QtMsgType type, const QString &text, const QQmlDebugContextInfo &context)
            {
                Q_D(QQmlProfilerClient);
                d->updateFeatures(ProfileDebugMessages);
                d->currentEvent.event.setTimestamp(context.timestamp > 0 ? context.timestamp : 0);
                d->currentEvent.event.setTypeIndex(-1);
                d->currentEvent.event.setString(text);
                d->currentEvent.type = QQmlProfilerEventType(
                            DebugMessage, MaximumRangeType, type,
                            QQmlProfilerEventLocation(context.file, context.line, 1));
                d->currentEvent.serverTypeId = 0;
                d->processCurrentEvent();
            });
        }
    } else {
        d->messageClient.reset();
    }
}

void QQmlProfilerClient::setFlushInterval(quint32 flushInterval)
{
    Q_D(QQmlProfilerClient);
    d->flushInterval = flushInterval;
}

QQmlProfilerClient::QQmlProfilerClient(QQmlProfilerClientPrivate &dd) :
    QQmlDebugClient(dd)
{
    Q_D(QQmlProfilerClient);
    connect(d->engineControl.data(), &QQmlEngineControlClient::engineAboutToBeAdded,
            this, &QQmlProfilerClient::sendRecordingStatus);
}

bool QQmlProfilerClientPrivate::updateFeatures(ProfileFeature feature)
{
    Q_Q(QQmlProfilerClient);
    quint64 flag = 1ULL << feature;
    if (!(requestedFeatures & flag))
        return false;
    if (!(recordedFeatures & flag)) {
        recordedFeatures |= flag;
        emit q->recordedFeaturesChanged(recordedFeatures);
    }
    return true;
}

void QQmlProfilerClient::onStateChanged(State status)
{
    if (status == Enabled) {
        sendRecordingStatus(-1);
    } else {
        Q_D(QQmlProfilerClient);
        d->finalize();
    }

}

void QQmlProfilerClient::messageReceived(const QByteArray &data)
{
    Q_D(QQmlProfilerClient);
    QPacket stream(d->connection->currentDataStreamVersion(), data);

    stream >> d->currentEvent;

    d->maximumTime = qMax(d->currentEvent.event.timestamp(), d->maximumTime);
    if (d->currentEvent.type.message() == Complete) {
        d->finalize();
        emit complete(d->maximumTime);
    } else if (d->currentEvent.type.message() == Event
               && d->currentEvent.type.detailType() == StartTrace) {
        const QList<int> engineIds = d->currentEvent.event.numbers<QList<int>, qint32>();
        d->trackedEngines.append(engineIds);
        d->forwardDebugMessages(d->currentEvent.event.timestamp());
        emit traceStarted(d->currentEvent.event.timestamp(), engineIds);
    } else if (d->currentEvent.type.message() == Event
               && d->currentEvent.type.detailType() == EndTrace) {
        const QList<int> engineIds = d->currentEvent.event.numbers<QList<int>, qint32>();
        for (int engineId : engineIds)
            d->trackedEngines.removeAll(engineId);
        d->forwardDebugMessages(d->currentEvent.event.timestamp());
        emit traceFinished(d->currentEvent.event.timestamp(), engineIds);
    } else if (d->updateFeatures(d->currentEvent.type.feature())) {
        d->processCurrentEvent();
    }
}

QT_END_NAMESPACE

#include "moc_qqmlprofilerclient_p.cpp"
