/****************************************************************************
**
** Copyright (C) 2019 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSensors module of the Qt Toolkit.
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

#ifndef SENSOREVENTQUEUE_H
#define SENSOREVENTQUEUE_H

#include "sensormanager.h"

#include <functional>

#include <QCoreApplication>
#include <QEvent>
#include <QSensorBackend>

class FunctionEvent : public QEvent
{
public:
    using Function = std::function<void()>;
    enum {
        FunctionEventType = User + 1
    };
    explicit FunctionEvent(const Function &function)
        : QEvent(Type(FunctionEventType))
        , m_function(function)
    {}

    void callFunction() const
    {
        m_function();
    }
private:
    Function m_function;
};

class ThreadSafeSensorBackend : public QSensorBackend
{
public:
    ThreadSafeSensorBackend(QSensor *sensor, QObject *parent = nullptr)
        : QSensorBackend(sensor, parent)
        , m_sensorManager(SensorManager::instance())
    {}

    // QObject interface
    bool event(QEvent *event) override
    {
        if (event->type() == FunctionEvent::FunctionEventType) {
            static_cast<FunctionEvent*>(event)->callFunction();
            event->accept();
            return true;
        }
        return QSensorBackend::event(event);
    }
protected:
    QSharedPointer<SensorManager> m_sensorManager;
};



template <typename T>
class SensorEventQueue : public ThreadSafeSensorBackend
{
public:
    explicit SensorEventQueue(int androidSensorType, QSensor *sensor, QObject *parent = nullptr)
        : ThreadSafeSensorBackend(sensor, parent)
    {
        setReading<T>(&m_reader);
        m_sensorEventQueue = ASensorManager_createEventQueue(m_sensorManager->manager(), m_sensorManager->looper(), -1, &looperCallback, this);
        setSensorType(androidSensorType);
    }

    ~SensorEventQueue() override
    {
        stop();
        ASensorManager_destroyEventQueue(m_sensorManager->manager(), m_sensorEventQueue);
    }

    void setSensorType(int type)
    {
        bool started = m_started;
        if (started)
            stop();
        m_sensor = ASensorManager_getDefaultSensor(m_sensorManager->manager(), type);
        setDescription(m_sensorManager->description(m_sensor));
        if (started)
            start();
    }

    // QSensorBackend interface
    void start() override
    {
        ASensorEventQueue_enableSensor(m_sensorEventQueue, m_sensor);
        if (sensor()->dataRate() > 0)
            ASensorEventQueue_setEventRate(m_sensorEventQueue, m_sensor, std::max(ASensor_getMinDelay(m_sensor), sensor()->dataRate()));
        m_started = true;
    }
    void stop() override
    {
        ASensorEventQueue_disableSensor(m_sensorEventQueue, m_sensor);
        m_started = false;
    }
    bool isFeatureSupported(QSensor::Feature feature) const override
    {
        switch (feature) {
        case QSensor::SkipDuplicates:
            return true;
        default:
            return false;
        }
    }

protected:
    virtual void dataReceived(const ASensorEvent &event) = 0;
    static int looperCallback(int /*fd*/, int /*events*/, void* data)
    {
        ASensorEvent sensorEvent;
        auto self = reinterpret_cast<SensorEventQueue*>(data);
        while (ASensorEventQueue_getEvents(self->m_sensorEventQueue, &sensorEvent, 1))
            QCoreApplication::postEvent(self, new FunctionEvent{[=]{self->dataReceived(sensorEvent);}});

        return 1; // 1 means keep receiving events
    }

protected:
    T m_reader;
    const ASensor *m_sensor = nullptr;
    ASensorEventQueue* m_sensorEventQueue = nullptr;
    bool m_started = false;
};

#endif // SENSOREVENTQUEUE_H
