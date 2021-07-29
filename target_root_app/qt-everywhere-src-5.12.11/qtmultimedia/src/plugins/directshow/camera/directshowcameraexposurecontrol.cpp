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

#include "directshowcameraexposurecontrol.h"
#include "dscamerasession.h"
#include "directshowglobal.h"
#include "directshowutils.h"

#include <functional>
#include <cmath>

QT_BEGIN_NAMESPACE

static qreal convertToSec(long v) { return (v < 0) ? (1 / std::pow(2., qreal(v))) : std::pow(2., qreal(v)); }
static Q_DECL_CONSTEXPR qreal convertToFvalue(long v) { return qreal(v) / 10.; }

DirectShowCameraExposureControl::DirectShowCameraExposureControl(DSCameraSession *session)
    : m_session(session)
    , m_shutterSpeedValues({ 0, 0, 0, 0, 0 })
    , m_apertureValues({ 0, 0, 0, 0, 0 })
    , m_requestedShutterSpeed(qreal(0.0))
    , m_currentShutterSpeed(qreal(-1.0))
    , m_requestedAperture(qreal(0.0))
    , m_currentAperture(qreal(-1.0))
    , m_requestedExposureMode(QCameraExposure::ExposureAuto)
    , m_currentExposureMode(QCameraExposure::ExposureAuto)
{
    Q_ASSERT(m_session);
    connect(m_session, &DSCameraSession::statusChanged,
            this, &DirectShowCameraExposureControl::onStatusChanged);
}

bool DirectShowCameraExposureControl::isParameterSupported(QCameraExposureControl::ExposureParameter parameter) const
{
    switch (parameter) {
    case QCameraExposureControl::Aperture:
        return (m_apertureValues.caps & CameraControl_Flags_Manual);
    case QCameraExposureControl::ShutterSpeed:
        return (m_shutterSpeedValues.caps & CameraControl_Flags_Manual);
    case QCameraExposureControl::ExposureMode:
        return true;
    default:
        break;
    }
    return false;
}

QVariantList DirectShowCameraExposureControl::supportedParameterRange(QCameraExposureControl::ExposureParameter parameter,
                                                                      bool *continuous) const
{
    if (continuous)
        *continuous = false;

    if (parameter == ShutterSpeed)
        return m_supportedShutterSpeeds;

    if (parameter == Aperture)
        return m_supportedApertureValues;

    if (parameter == ExposureMode)
        return m_supportedExposureModes;

    return QVariantList();
}

QVariant DirectShowCameraExposureControl::requestedValue(QCameraExposureControl::ExposureParameter parameter) const
{
    if (parameter == ShutterSpeed)
        return QVariant::fromValue(m_requestedShutterSpeed);

    if (parameter == Aperture)
        return QVariant::fromValue(m_requestedAperture);

    if (parameter == ExposureMode)
        return QVariant::fromValue(m_requestedExposureMode);

    return QVariant();
}

QVariant DirectShowCameraExposureControl::actualValue(QCameraExposureControl::ExposureParameter parameter) const
{
    if (parameter == ExposureMode)
        return QVariant::fromValue(m_currentExposureMode);

    if (parameter == ShutterSpeed) {
        return qFuzzyCompare(m_currentShutterSpeed, qreal(-1.0))
                ? QVariant()
                : QVariant::fromValue(m_currentShutterSpeed);
    }

    if (parameter == Aperture) {
        return qFuzzyCompare(m_currentAperture, qreal(-1.0))
                ? QVariant()
                : QVariant::fromValue(m_currentAperture);
    }

    return QVariant();
}

bool DirectShowCameraExposureControl::setValue(QCameraExposureControl::ExposureParameter parameter,
                                               const QVariant &value)
{
    IAMCameraControl *cameraControl = nullptr;
    const DirectShowUtils::ScopedSafeRelease<IAMCameraControl> rControl { &cameraControl };
    if (!m_session->getCameraControlInterface(&cameraControl))
        return false;

    // Reset exposure mode if the value is invalid.
    if (!value.isValid()) {
        m_requestedExposureMode = QCameraExposure::ExposureAuto;
        return setExposureMode(cameraControl, m_requestedExposureMode);
    }

    if (parameter == ShutterSpeed || parameter == Aperture) {
        bool ok = false;
        const qreal newValue = value.toReal(&ok);
        if (!ok)
            return false;

        // Change the exposure mode first
        setExposureMode(cameraControl, QCameraExposure::ExposureManual);

        if (parameter == ShutterSpeed) {
            m_requestedShutterSpeed = newValue;
            return setShutterSpeed(cameraControl, m_requestedShutterSpeed);
        }
        m_requestedAperture = newValue;
        return setAperture(cameraControl, m_requestedAperture);
    }

    if (parameter == ExposureMode) {
        m_requestedExposureMode = value.value<QCameraExposure::ExposureMode>();
        return setExposureMode(cameraControl, m_requestedExposureMode);
    }

    return false;
}

void DirectShowCameraExposureControl::onStatusChanged(QCamera::Status status)
{
    const bool shouldUpdate = (qFuzzyCompare(m_currentAperture, qreal(-1.0)) && qFuzzyCompare(m_currentShutterSpeed, qreal(-1.0)));

    if (status == QCamera::LoadedStatus && shouldUpdate)
        updateExposureSettings();

    if (status == QCamera::UnloadedStatus) {
        m_supportedApertureValues.clear();
        m_supportedExposureModes.clear();
        m_supportedShutterSpeeds.clear();
        m_currentAperture = qreal(-1.0);
        m_currentShutterSpeed = qreal(-1.0);
        m_currentExposureMode = QCameraExposure::ExposureAuto;
    }
}

void DirectShowCameraExposureControl::updateExposureSettings()
{
    IAMCameraControl *cameraControl = nullptr;
    const DirectShowUtils::ScopedSafeRelease<IAMCameraControl> rControl { &cameraControl };
    if (!m_session->getCameraControlInterface(&cameraControl))
        return;

    const auto updateValues = [cameraControl](long property,
                                              ExposureValues &currentValues,
                                              QVariantList &parameterRange,
                                              const std::function<qreal(long)> &converter,
                                              bool *changed) -> bool {
        ExposureValues values { 0, 0, 0, 0, 0 };
        if (FAILED(cameraControl->GetRange(property,
                                           &values.minValue,
                                           &values.maxValue,
                                           &values.stepping,
                                           &values.defaultValue,
                                           &values.caps))) {
            return false;
        }

        const bool minValueChanged = values.minValue != currentValues.minValue;
        const bool maxValueChanged = values.maxValue != currentValues.maxValue;
        const bool steppingChanged = values.stepping != currentValues.stepping;

        if (minValueChanged || maxValueChanged || steppingChanged) {
            parameterRange.clear();
            long nextValue = values.minValue;
            while (nextValue != values.maxValue && values.stepping != 0) {
                parameterRange << converter(nextValue);
                nextValue += values.stepping;
            }

            if (changed)
                *changed = true;
        }

        currentValues = values;
        return true;
    };

    const auto getCurrentValue = [cameraControl](long property, const std::function<qreal(long)> &converter, qreal *value) -> bool {
        long currentValue;
        long currentFlags;
        if (FAILED(cameraControl->Get(property, &currentValue, &currentFlags)))
            return false;

        *value = converter(currentValue);
        return true;
    };

    // Shutter speed
    bool changed = false;
    if (!updateValues(CameraControl_Exposure, m_shutterSpeedValues, m_supportedShutterSpeeds, convertToSec, &changed))
        qCDebug(qtDirectShowPlugin, "Unable to update the shutter speed values");

    if (changed)
        Q_EMIT parameterRangeChanged(int(ShutterSpeed));

    if ((m_shutterSpeedValues.caps & CameraControl_Flags_Manual)) {
        if (getCurrentValue(CameraControl_Exposure, convertToSec, &m_currentShutterSpeed)) {
            if (m_currentExposureMode == QCameraExposure::ExposureManual)
                setShutterSpeed(cameraControl, m_requestedShutterSpeed);
        } else {
            m_currentShutterSpeed = qreal(-1.0);
            qCDebug(qtDirectShowPlugin, "Unable to get the current shutter speed!");
        }
    }

    // Aperture
    changed = false;
    if (!updateValues(CameraControl_Iris, m_apertureValues, m_supportedApertureValues, convertToFvalue, &changed))
        qCDebug(qtDirectShowPlugin, "Unable to update the aperture values");

    if (changed)
        Q_EMIT parameterRangeChanged(int(Aperture));

    if (getCurrentValue(CameraControl_Iris, convertToFvalue, &m_currentAperture)) {
        if (m_currentExposureMode == QCameraExposure::ExposureManual)
            setAperture(cameraControl, m_requestedAperture);
    } else {
        m_currentAperture = qreal(-1.0);
        qCDebug(qtDirectShowPlugin, "Unable to get the current aperture value!");
    }

    // Update exposure modes
    const bool hasAutoExposure = (m_apertureValues.caps & CameraControl_Flags_Auto)
                                 || (m_shutterSpeedValues.caps & CameraControl_Flags_Auto);
    const bool hasManualExposure = (m_apertureValues.caps & CameraControl_Flags_Manual)
                                   || (m_shutterSpeedValues.caps & CameraControl_Flags_Manual);

    QVariantList exposureModes;
    if (hasAutoExposure && !m_supportedExposureModes.contains(QVariant::fromValue(QCameraExposure::ExposureAuto)))
        exposureModes << QVariant::fromValue(QCameraExposure::ExposureAuto);

    if (hasManualExposure && !m_supportedExposureModes.contains(QVariant::fromValue(QCameraExposure::ExposureManual)))
        exposureModes << QVariant::fromValue(QCameraExposure::ExposureManual);

    if (!exposureModes.isEmpty() || !m_supportedExposureModes.isEmpty()) {
        m_supportedExposureModes = exposureModes;
        Q_EMIT parameterRangeChanged(int(ExposureMode));
    }
}

bool DirectShowCameraExposureControl::setShutterSpeed(IAMCameraControl *cameraControl, qreal shutterSpeed)
{
    if (m_currentExposureMode != QCameraExposure::ExposureManual) {
        qCDebug(qtDirectShowPlugin, "Trying to set shutter speed value while in auto exposure mode!");
        return false;
    }

    if (qFuzzyCompare(m_currentShutterSpeed, shutterSpeed))
        return true;

    if ((m_shutterSpeedValues.caps & CameraControl_Flags_Manual) == 0)
        return false;

    if (!m_supportedShutterSpeeds.contains(QVariant::fromValue(shutterSpeed)))
        return false;

    if (qFuzzyIsNull(shutterSpeed) || (shutterSpeed < qreal(0.0)))
        return false;

    const long newValue = long(log2(shutterSpeed));
    if (FAILED(cameraControl->Set(CameraControl_Exposure, newValue, CameraControl_Flags_Manual))) {
        qCDebug(qtDirectShowPlugin, "Unable to set shutter speed value to: %d", int(shutterSpeed));
        return false;
    }

    m_currentShutterSpeed = shutterSpeed;
    Q_EMIT actualValueChanged(int(ShutterSpeed));
    return true;
}

bool DirectShowCameraExposureControl::setAperture(IAMCameraControl *cameraControl, qreal aperture)
{
    if (m_currentExposureMode != QCameraExposure::ExposureManual) {
        qCDebug(qtDirectShowPlugin, "Trying to set aperture value while in auto exposure mode!");
        return false;
    }

    if (qFuzzyCompare(m_currentAperture, aperture))
        return true;

    if ((m_apertureValues.caps & CameraControl_Flags_Manual) == 0)
        return false;

    if (!m_supportedApertureValues.contains(QVariant::fromValue(aperture)))
        return false;

    if (aperture < qreal(0.0))
        return false;

    const long newValue = long(10 * aperture);
    if (FAILED(cameraControl->Set(CameraControl_Iris, newValue, CameraControl_Flags_Manual))) {
        qCDebug(qtDirectShowPlugin, "Unable to set aperture value to: %d", int(aperture));
        return false;
    }

    m_currentAperture = aperture;
    Q_EMIT actualValueChanged(int(Aperture));

    return true;
}

bool DirectShowCameraExposureControl::setExposureMode(IAMCameraControl *cameraControl, QCameraExposure::ExposureMode mode)
{
    if (m_currentExposureMode == mode)
        return true;

    bool exposureModeChanged = true;

    // Set auto exposure mode
    if (mode == QCameraExposure::ExposureAuto) {
        if ((m_apertureValues.caps & CameraControl_Flags_Auto)
            && FAILED(cameraControl->Set(CameraControl_Iris, 0, CameraControl_Flags_Auto))) {
            qCDebug(qtDirectShowPlugin, "Setting auto exposure mode failed!");
            exposureModeChanged = false;
        }

        if ((m_shutterSpeedValues.caps & CameraControl_Flags_Auto)
            && FAILED(cameraControl->Set(CameraControl_Exposure, 0, CameraControl_Flags_Auto))) {
            qCDebug(qtDirectShowPlugin, "Setting auto exposure mode failed");
            exposureModeChanged = false;
        }

        if (exposureModeChanged) {
            m_currentExposureMode = mode;
            Q_EMIT actualValueChanged(int(ExposureMode));
        }

        return exposureModeChanged;
    }

    // Change the current exposure mode to manual first.
    m_currentExposureMode = QCameraExposure::ExposureManual;

    const qreal newShutterSpeed = qFuzzyCompare(m_requestedShutterSpeed, -1.0)
                                  ? convertToSec(m_shutterSpeedValues.defaultValue)
                                  : m_requestedShutterSpeed;
    if ((m_shutterSpeedValues.caps & CameraControl_Flags_Manual))
        setShutterSpeed(cameraControl, newShutterSpeed);

    const qreal newAperture = qFuzzyCompare(m_requestedAperture, -1.0)
                              ? convertToFvalue(m_apertureValues.defaultValue)
                              : m_requestedAperture;
    if ((m_apertureValues.caps & CameraControl_Flags_Manual))
        setAperture(cameraControl, newAperture);


    // Check if any of the values changed.
    exposureModeChanged = (qFuzzyCompare(m_currentShutterSpeed, newShutterSpeed)
                           || qFuzzyCompare(m_currentAperture, newAperture));

    if (exposureModeChanged)
        Q_EMIT actualValueChanged(int(ExposureMode));

    return exposureModeChanged;
}

QT_END_NAMESPACE
