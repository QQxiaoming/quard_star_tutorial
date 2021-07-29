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

#ifndef CAMERA_H
#define CAMERA_H

#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QScopedPointer>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Camera; }
QT_END_NAMESPACE

class Camera : public QMainWindow
{
    Q_OBJECT

public:
    Camera();

private slots:
    void setCamera(const QCameraInfo &cameraInfo);

    void startCamera();
    void stopCamera();

    void record();
    void pause();
    void stop();
    void setMuted(bool);

    void toggleLock();
    void takeImage();
    void displayCaptureError(int, QCameraImageCapture::Error, const QString &errorString);

    void configureCaptureSettings();
    void configureVideoSettings();
    void configureImageSettings();

    void displayRecorderError();
    void displayCameraError();

    void updateCameraDevice(QAction *action);

    void updateCameraState(QCamera::State);
    void updateCaptureMode();
    void updateRecorderState(QMediaRecorder::State state);
    void setExposureCompensation(int index);

    void updateRecordTime();

    void processCapturedImage(int requestId, const QImage &img);
    void updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason);

    void displayViewfinder();
    void displayCapturedImage();

    void readyForCapture(bool ready);
    void imageSaved(int id, const QString &fileName);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Camera *ui;

    QScopedPointer<QCamera> m_camera;
    QScopedPointer<QCameraImageCapture> m_imageCapture;
    QScopedPointer<QMediaRecorder> m_mediaRecorder;

    QImageEncoderSettings m_imageSettings;
    QAudioEncoderSettings m_audioSettings;
    QVideoEncoderSettings m_videoSettings;
    QString m_videoContainerFormat;
    bool m_isCapturingImage = false;
    bool m_applicationExiting = false;
};

#endif
