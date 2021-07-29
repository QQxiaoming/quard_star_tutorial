/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <math.h>

#include <QAudioOutput>
#include <QByteArray>
#include <QComboBox>
#include <QIODevice>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QScopedPointer>

class Generator : public QIODevice
{
    Q_OBJECT

public:
    Generator(const QAudioFormat &format, qint64 durationUs, int sampleRate);

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 bytesAvailable() const override;

private:
    void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);

private:
    qint64 m_pos = 0;
    QByteArray m_buffer;
};

class AudioTest : public QMainWindow
{
    Q_OBJECT

public:
    AudioTest();
    ~AudioTest();

private:
    void initializeWindow();
    void initializeAudio(const QAudioDeviceInfo &deviceInfo);

private:
    QTimer *m_pushTimer = nullptr;

    // Owned by layout
    QPushButton *m_modeButton = nullptr;
    QPushButton *m_suspendResumeButton = nullptr;
    QComboBox *m_deviceBox = nullptr;
    QLabel *m_volumeLabel = nullptr;
    QSlider *m_volumeSlider = nullptr;

    QScopedPointer<Generator> m_generator;
    QScopedPointer<QAudioOutput> m_audioOutput;

    bool m_pullMode = true;

private slots:
    void toggleMode();
    void toggleSuspendResume();
    void deviceChanged(int index);
    void volumeChanged(int);
};

#endif // AUDIOOUTPUT_H
