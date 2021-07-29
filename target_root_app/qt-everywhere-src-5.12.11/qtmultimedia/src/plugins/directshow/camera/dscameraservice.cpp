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

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

#include "dscameraservice.h"
#include "dscameracontrol.h"
#include "dscamerasession.h"
#include "dsvideorenderer.h"
#include "dsvideodevicecontrol.h"
#include "dsimagecapturecontrol.h"
#include "dscameraviewfindersettingscontrol.h"
#include "dscameraimageprocessingcontrol.h"
#include "directshowcameraexposurecontrol.h"
#include "directshowcameracapturedestinationcontrol.h"
#include "directshowcameracapturebufferformatcontrol.h"
#include "directshowvideoprobecontrol.h"
#include "directshowcamerazoomcontrol.h"
#include "directshowcameraimageencodercontrol.h"

QT_BEGIN_NAMESPACE

DSCameraService::DSCameraService(QObject *parent):
    QMediaService(parent)
  , m_session(new DSCameraSession(this))
  , m_control(new DSCameraControl(m_session))
  , m_videoDevice(new DSVideoDeviceControl(m_session))
  , m_videoRenderer(0)
  , m_imageCapture(new DSImageCaptureControl(m_session))
  , m_viewfinderSettings(new DSCameraViewfinderSettingsControl(m_session))
  , m_imageProcessingControl(new DSCameraImageProcessingControl(m_session))
  , m_exposureControl(new DirectShowCameraExposureControl(m_session))
  , m_captureDestinationControl(new DirectShowCameraCaptureDestinationControl(m_session))
  , m_captureBufferFormatControl(new DirectShowCameraCaptureBufferFormatControl)
  , m_videoProbeControl(nullptr)
  , m_zoomControl(new DirectShowCameraZoomControl(m_session))
  , m_imageEncoderControl(new DirectShowCameraImageEncoderControl(m_session))
{
}

DSCameraService::~DSCameraService()
{
    delete m_control;
    delete m_viewfinderSettings;
    delete m_imageProcessingControl;
    delete m_videoDevice;
    delete m_videoRenderer;
    delete m_imageCapture;
    delete m_imageEncoderControl;
    delete m_session;
    delete m_exposureControl;
    delete m_captureDestinationControl;
    delete m_captureBufferFormatControl;
    delete m_videoProbeControl;
    delete m_zoomControl;
}

QMediaControl* DSCameraService::requestControl(const char *name)
{
    if(qstrcmp(name,QCameraControl_iid) == 0)
        return m_control;

    if (qstrcmp(name, QCameraImageCaptureControl_iid) == 0)
        return m_imageCapture;

    if (qstrcmp(name,QVideoRendererControl_iid) == 0) {
        if (!m_videoRenderer) {
            m_videoRenderer = new DSVideoRendererControl(m_session, this);
            return m_videoRenderer;
        }
    }

    if (qstrcmp(name,QVideoDeviceSelectorControl_iid) == 0)
        return m_videoDevice;

    if (qstrcmp(name, QCameraViewfinderSettingsControl2_iid) == 0)
        return m_viewfinderSettings;

    if (qstrcmp(name, QCameraImageProcessingControl_iid) == 0)
        return m_imageProcessingControl;

    if (qstrcmp(name, QCameraExposureControl_iid) == 0)
        return m_exposureControl;

    if (qstrcmp(name, QCameraCaptureDestinationControl_iid) == 0)
        return m_captureDestinationControl;

    if (qstrcmp(name, QCameraCaptureBufferFormatControl_iid) == 0)
        return m_captureBufferFormatControl;

    if (qstrcmp(name, QMediaVideoProbeControl_iid) == 0) {
        if (!m_videoProbeControl)
            m_videoProbeControl = new DirectShowVideoProbeControl;

        m_videoProbeControl->ref();
        m_session->addVideoProbe(m_videoProbeControl);
        return m_videoProbeControl;
    }

    if (qstrcmp(name, QCameraZoomControl_iid) == 0)
        return m_zoomControl;

    if (qstrcmp(name, QImageEncoderControl_iid) == 0)
        return m_imageEncoderControl;

    return 0;
}

void DSCameraService::releaseControl(QMediaControl *control)
{
    if (control == m_videoRenderer) {
        delete m_videoRenderer;
        m_videoRenderer = 0;
        return;
    }

    if (control == m_videoProbeControl) {
        m_session->removeVideoProbe(m_videoProbeControl);
        if (!m_videoProbeControl->deref()) {
            delete m_videoProbeControl;
            m_videoProbeControl = nullptr;
        }
    }
}

QT_END_NAMESPACE
