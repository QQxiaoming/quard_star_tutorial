/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "directshowcamerazoomcontrol.h"
#include "dscamerasession.h"
#include "directshowutils.h"

QT_BEGIN_NAMESPACE

inline static qreal defaultZoomValue() { return qreal(1.0); }

DirectShowCameraZoomControl::DirectShowCameraZoomControl(DSCameraSession *session)
    : m_session(session)
    , m_opticalZoom({0, 0, 0, 0, 0})
    , m_currentOpticalZoom(qreal(0.0))
    , m_requestedOpticalZoom(qreal(0.0))
    , m_maxOpticalZoom(qreal(1.0))
{
    Q_ASSERT(m_session);
    connect(m_session, &DSCameraSession::statusChanged,
            this, &DirectShowCameraZoomControl::onStatusChanged);
}

qreal DirectShowCameraZoomControl::maximumOpticalZoom() const
{
    return m_maxOpticalZoom;
}

qreal DirectShowCameraZoomControl::maximumDigitalZoom() const
{
    return defaultZoomValue();
}

qreal DirectShowCameraZoomControl::requestedOpticalZoom() const
{
    return qMax(defaultZoomValue(), m_requestedOpticalZoom);
}

qreal DirectShowCameraZoomControl::requestedDigitalZoom() const
{
    return defaultZoomValue();
}

qreal DirectShowCameraZoomControl::currentOpticalZoom() const
{
    return qMax(defaultZoomValue(), m_currentOpticalZoom);
}

qreal DirectShowCameraZoomControl::currentDigitalZoom() const
{
    return defaultZoomValue();
}

void DirectShowCameraZoomControl::zoomTo(qreal optical, qreal digital)
{
    Q_UNUSED(digital);
    if (!(m_opticalZoom.caps & CameraControl_Flags_Manual))
        return;

    if (qFuzzyCompare(optical, m_requestedOpticalZoom))
        return;

    m_requestedOpticalZoom = optical;
    Q_EMIT requestedOpticalZoomChanged(m_requestedOpticalZoom);

    if (qFuzzyCompare(m_requestedOpticalZoom, m_currentOpticalZoom))
        return;

    if (m_session->status() != QCamera::LoadedStatus && m_session->status() != QCamera::ActiveStatus)
        return; // We'll wait until the camera is loaded, see: statusChanged connection

    opticalZoomToPrivate(optical);
}

void DirectShowCameraZoomControl::onStatusChanged(QCamera::Status status)
{
    if (status == QCamera::LoadedStatus) {
        updateZoomValues();
    } else if (status == QCamera::UnloadedStatus) {
        SecureZeroMemory(&m_opticalZoom, sizeof(ZoomValues));
        m_currentOpticalZoom = qreal(0.0);
        m_requestedOpticalZoom = qreal(0.0);
    }

}

void DirectShowCameraZoomControl::updateZoomValues()
{
    IAMCameraControl *cameraControl = nullptr;
    const DirectShowUtils::ScopedSafeRelease<IAMCameraControl> rControl { &cameraControl };
    if (!m_session->getCameraControlInterface(&cameraControl))
        return;

    ZoomValues values { 0, 0, 0, 0, 0 };
    // Zoom levels in DS are in the range [10, 600]
    // The default zoom is device specific.
    HRESULT hr = cameraControl->GetRange(CameraControl_Zoom,
                                         &values.minZoom,
                                         &values.maxZoom,
                                         &values.stepping,
                                         &values.defaultZoom,
                                         &values.caps);

    if (FAILED(hr)) {
        qCDebug(qtDirectShowPlugin, "Getting the camera's zoom properties failed");
        SecureZeroMemory(&m_opticalZoom, sizeof(ZoomValues));
        return;
    }

    if (!(values.caps & CameraControl_Flags_Manual)) {
        qCDebug(qtDirectShowPlugin, "Camera does not support manual zoom");
        SecureZeroMemory(&m_opticalZoom, sizeof(ZoomValues));
        return;
    }

    if (values.maxZoom != m_opticalZoom.maxZoom) {
        const qreal newMaxZoomScale = (values.minZoom == 0) ? defaultZoomValue()
                                                            : (qreal(values.maxZoom) / qreal(values.minZoom));
        if (!qFuzzyCompare(newMaxZoomScale, m_maxOpticalZoom)) {
            m_maxOpticalZoom = newMaxZoomScale;
            Q_EMIT maximumOpticalZoomChanged(m_maxOpticalZoom);
        }
    }

    m_opticalZoom = values;

    long currentZoom = 0;
    long flags = 0;
    if (FAILED(cameraControl->Get(CameraControl_Zoom, &currentZoom, &flags))) {
        qCDebug(qtDirectShowPlugin, "Getting the camera's current zoom value failed!");
        return;
    }

    qreal currentOpticalZoom = (m_opticalZoom.minZoom == 0) ? defaultZoomValue()
                                                            : (qreal(currentZoom) / qreal(m_opticalZoom.minZoom));
    currentOpticalZoom = qMax(defaultZoomValue(), currentOpticalZoom);
    if (!qFuzzyCompare(m_currentOpticalZoom, currentOpticalZoom)) {
        m_currentOpticalZoom = currentOpticalZoom;
        Q_EMIT currentOpticalZoomChanged(m_currentOpticalZoom);
    }

    // Check if there is a pending zoom value.
    if (!qFuzzyCompare(m_currentOpticalZoom, m_requestedOpticalZoom) && !qFuzzyIsNull(m_requestedOpticalZoom))
        opticalZoomToPrivate(m_requestedOpticalZoom);
}

bool DirectShowCameraZoomControl::opticalZoomToPrivate(qreal scaleFactor)
{
    IAMCameraControl *cameraControl = nullptr;
    const DirectShowUtils::ScopedSafeRelease<IAMCameraControl> rControl { &cameraControl };
    if (!m_session->getCameraControlInterface(&cameraControl))
        return false;

    // Convert to DS zoom value
    const int newDSOpticalZoom = qRound(m_opticalZoom.minZoom * scaleFactor);
    long newDSOpticalZoomAdjusted = newDSOpticalZoom - (newDSOpticalZoom % m_opticalZoom.stepping);
    newDSOpticalZoomAdjusted = qBound(m_opticalZoom.minZoom, newDSOpticalZoomAdjusted, m_opticalZoom.maxZoom);

    if (FAILED(cameraControl->Set(CameraControl_Zoom, newDSOpticalZoomAdjusted, CameraControl_Flags_Manual))) {
        qCDebug(qtDirectShowPlugin, "Setting the camera's zoom value failed");
        return false;
    }

    const qreal newScaleFactor = (m_opticalZoom.minZoom == 0) ? defaultZoomValue()
                                                              : (qreal(newDSOpticalZoomAdjusted) / qreal(m_opticalZoom.minZoom));
    // convert back to Qt scale value
    m_currentOpticalZoom = qMax(defaultZoomValue(), newScaleFactor);
    Q_EMIT currentOpticalZoomChanged(m_currentOpticalZoom);

    return true;
}

QT_END_NAMESPACE
