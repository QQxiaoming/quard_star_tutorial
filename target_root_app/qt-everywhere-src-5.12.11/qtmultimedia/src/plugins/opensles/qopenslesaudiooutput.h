/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef QOPENSLESAUDIOOUTPUT_H
#define QOPENSLESAUDIOOUTPUT_H

#include <qaudiosystem.h>
#include <SLES/OpenSLES.h>
#include <qbytearray.h>
#include <qmap.h>
#include <QTime>
#include <QIODevice>

QT_BEGIN_NAMESPACE

class QOpenSLESAudioOutput : public QAbstractAudioOutput
{
    Q_OBJECT

public:
    QOpenSLESAudioOutput(const QByteArray &device);
    ~QOpenSLESAudioOutput();

    void start(QIODevice *device) override;
    QIODevice *start() override;
    void stop() override;
    void reset() override;
    void suspend() override;
    void resume() override;
    int bytesFree() const override;
    int periodSize() const override;
    void setBufferSize(int value) override;
    int bufferSize() const override;
    void setNotifyInterval(int milliSeconds) override;
    int notifyInterval() const override;
    qint64 processedUSecs() const override;
    qint64 elapsedUSecs() const override;
    QAudio::Error error() const override;
    QAudio::State state() const override;
    void setFormat(const QAudioFormat &format) override;
    QAudioFormat format() const override;

    void setVolume(qreal volume) override;
    qreal volume() const override;

    void setCategory(const QString &category) override;
    QString category() const override;

private:
    friend class SLIODevicePrivate;

    Q_INVOKABLE void onEOSEvent();
    Q_INVOKABLE void onBytesProcessed(qint64 bytes);
    void bufferAvailable(quint32 count, quint32 playIndex);

    static void playCallback(SLPlayItf playItf, void *ctx, SLuint32 event);
    static void bufferQueueCallback(SLBufferQueueItf bufferQueue, void *ctx);

    bool preparePlayer();
    void destroyPlayer();
    void stopPlayer();
    void startPlayer();
    qint64 writeData(const char *data, qint64 len);

    void setState(QAudio::State state);
    void setError(QAudio::Error error);

    SLmillibel adjustVolume(qreal vol);

    QByteArray m_deviceName;
    QAudio::State m_state;
    QAudio::Error m_error;
    SLObjectItf m_outputMixObject;
    SLObjectItf m_playerObject;
    SLPlayItf m_playItf;
    SLVolumeItf m_volumeItf;
    SLBufferQueueItf m_bufferQueueItf;
    QIODevice *m_audioSource;
    char *m_buffers;
    qreal m_volume;
    bool m_pullMode;
    int m_nextBuffer;
    int m_bufferSize;
    int m_notifyInterval;
    int m_periodSize;
    qint64 m_elapsedTime;
    qint64 m_processedBytes;
    QAtomicInt m_availableBuffers;
    SLuint32 m_eventMask;
    bool m_startRequiresInit;

    qint32 m_streamType;
    QTime m_clockStamp;
    QAudioFormat m_format;
    QString m_category;
    static QMap<QString, qint32> m_categories;
};

class SLIODevicePrivate : public QIODevice
{
    Q_OBJECT

public:
    inline SLIODevicePrivate(QOpenSLESAudioOutput *audio) : m_audioDevice(audio) {}
    inline ~SLIODevicePrivate() override {}

protected:
    inline qint64 readData(char *, qint64) override { return 0; }
    inline qint64 writeData(const char *data, qint64 len) override;

private:
    QOpenSLESAudioOutput *m_audioDevice;
};

qint64 SLIODevicePrivate::writeData(const char *data, qint64 len)
{
    Q_ASSERT(m_audioDevice);
    return m_audioDevice->writeData(data, len);
}

QT_END_NAMESPACE

#endif // QOPENSLESAUDIOOUTPUT_H
