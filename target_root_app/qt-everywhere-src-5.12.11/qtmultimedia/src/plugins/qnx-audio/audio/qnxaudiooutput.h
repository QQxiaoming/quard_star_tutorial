/****************************************************************************
**
** Copyright (C) 2016 Research In Motion
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

#ifndef QNXAUDIOOUTPUT_H
#define QNXAUDIOOUTPUT_H

#include "qaudiosystem.h"

#include <QTime>
#include <QTimer>
#include <QIODevice>
#include <QSocketNotifier>

#include <sys/asoundlib.h>
#include <sys/neutrino.h>

QT_BEGIN_NAMESPACE

class QnxPushIODevice;

class QnxAudioOutput : public QAbstractAudioOutput
{
    Q_OBJECT

public:
    QnxAudioOutput();
    ~QnxAudioOutput();

    void start(QIODevice *source) override;
    QIODevice *start() override;
    void stop() override;
    void reset() override;
    void suspend() override;
    void resume() override;
    int bytesFree() const override;
    int periodSize() const override;
    void setBufferSize(int) override {}
    int bufferSize() const override { return 0; }
    void setNotifyInterval(int ms) override;
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

private slots:
    void pullData();

private:
    bool open();
    void close();
    void setError(QAudio::Error error);
    void setState(QAudio::State state);

    void addPcmEventFilter();
    void createPcmNotifiers();
    void destroyPcmNotifiers();
    void setTypeName(snd_pcm_channel_params_t *params);

    void suspendInternal(QAudio::State suspendState);
    void resumeInternal();

    friend class QnxPushIODevice;
    qint64 write(const char *data, qint64 len);

    QIODevice *m_source;
    bool m_pushSource;
    QTimer m_timer;

    int m_notifyInterval;
    QAudio::Error m_error;
    QAudio::State m_state;
    QAudioFormat m_format;
    qreal m_volume;
    QString m_category;
    int m_periodSize;

    snd_pcm_t *m_pcmHandle;
    qint64 m_bytesWritten;
    QTime m_startTimeStamp;
    QTime m_intervalTimeStamp;
    qint64 m_intervalOffset;

#if _NTO_VERSION >= 700
    QSocketNotifier *m_pcmNotifier;

private slots:
    void pcmNotifierActivated(int socket);
#endif
};

class QnxPushIODevice : public QIODevice
{
    Q_OBJECT
public:
    explicit QnxPushIODevice(QnxAudioOutput *output);
    ~QnxPushIODevice();

    qint64 readData(char *data, qint64 len);
    qint64 writeData(const char *data, qint64 len);

private:
    QnxAudioOutput *m_output;
};

QT_END_NAMESPACE

#endif
