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

#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QAudioInput>
#include <QByteArray>
#include <QComboBox>
#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QWidget>
#include <QScopedPointer>

class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &format);

    void start();
    void stop();

    qreal level() const { return m_level; }

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    const QAudioFormat m_format;
    quint32 m_maxAmplitude = 0;
    qreal m_level = 0.0; // 0.0 <= m_level <= 1.0

signals:
    void update();
};


class RenderArea : public QWidget
{
    Q_OBJECT

public:
    explicit RenderArea(QWidget *parent = nullptr);

    void setLevel(qreal value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_level = 0;
    QPixmap m_pixmap;
};


class InputTest : public QMainWindow
{
    Q_OBJECT

public:
    InputTest();

private:
    void initializeWindow();
    void initializeAudio(const QAudioDeviceInfo &deviceInfo);

private slots:
    void toggleMode();
    void toggleSuspend();
    void deviceChanged(int index);
    void sliderChanged(int value);

private:
    // Owned by layout
    RenderArea *m_canvas = nullptr;
    QPushButton *m_modeButton = nullptr;
    QPushButton *m_suspendResumeButton = nullptr;
    QComboBox *m_deviceBox = nullptr;
    QSlider *m_volumeSlider = nullptr;

    QScopedPointer<AudioInfo> m_audioInfo;
    QScopedPointer<QAudioInput> m_audioInput;
    bool m_pullMode = true;
};

#endif // AUDIOINPUT_H
