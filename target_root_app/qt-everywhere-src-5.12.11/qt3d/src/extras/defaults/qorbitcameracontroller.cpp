/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qorbitcameracontroller.h"
#include "qorbitcameracontroller_p.h"

#include <Qt3DRender/QCamera>

QT_BEGIN_NAMESPACE

namespace Qt3DExtras {

QOrbitCameraControllerPrivate::QOrbitCameraControllerPrivate()
    : m_zoomInLimit(2.0f)
{}

/*!
    \class Qt3DExtras::QOrbitCameraController
    \ingroup qt3d-extras-cameracontrollers
    \brief The QOrbitCameraController class allows controlling the scene camera along orbital path.
    \inmodule Qt3DExtras
    \since 5.7
    \inherits Qt3DCore::QEntity

    The controls are:
    \table
    \header
        \li Input
        \li Action
    \row
        \li Left mouse button
        \li While the left mouse button is pressed, mouse movement along x-axis moves the camera
        left and right and movement along y-axis moves it up and down.
    \row
        \li Right mouse button
        \li While the right mouse button is pressed, mouse movement along x-axis pans the camera
        around the camera view center and movement along y-axis tilts it around the camera
        view center.
    \row
        \li Both left and right mouse button
        \li While both the left and the right mouse button are pressed, mouse movement along y-axis
        zooms the camera in and out without changing the view center.
    \row
        \li Mouse scroll wheel
        \li Zooms the camera in and out without changing the view center.
    \row
        \li Arrow keys
        \li Move the camera vertically and horizontally relative to camera viewport.
    \row
        \li Page up and page down keys
        \li Move the camera forwards and backwards.
    \row
        \li Shift key
        \li Changes the behavior of the up and down arrow keys to zoom the camera in and out
        without changing the view center. The other movement keys are disabled.
    \row
        \li Alt key
        \li Changes the behovior of the arrow keys to pan and tilt the camera around the view
        center. Disables the page up and page down keys.
    \row
        \li Escape
        \li Moves the camera so that entire scene is visible in the camera viewport.
    \endtable
*/

QOrbitCameraController::QOrbitCameraController(Qt3DCore::QNode *parent)
    : QOrbitCameraController(*new QOrbitCameraControllerPrivate, parent)
{
}

/*! \internal
 */
QOrbitCameraController::QOrbitCameraController(QOrbitCameraControllerPrivate &dd, Qt3DCore::QNode *parent)
    : QAbstractCameraController(dd, parent)
{
}

QOrbitCameraController::~QOrbitCameraController()
{
}

/*!
    \property QOrbitCameraController::zoomInLimit

    Holds the current zoom-in limit. The zoom-in limit determines how close to the view center
    the camera can be zoomed.
*/
float QOrbitCameraController::zoomInLimit() const
{
    Q_D(const QOrbitCameraController);
    return d->m_zoomInLimit;
}

void QOrbitCameraController::setZoomInLimit(float zoomInLimit)
{
    Q_D(QOrbitCameraController);
    if (d->m_zoomInLimit != zoomInLimit) {
        d->m_zoomInLimit = zoomInLimit;
        emit zoomInLimitChanged();
    }
}

inline float clampInputs(float input1, float input2)
{
    float axisValue = input1 + input2;
    return (axisValue < -1) ? -1 : (axisValue > 1) ? 1 : axisValue;
}

inline float zoomDistance(QVector3D firstPoint, QVector3D secondPoint)
{
    return (secondPoint - firstPoint).lengthSquared();
}

void QOrbitCameraController::moveCamera(const QAbstractCameraController::InputState &state, float dt)
{
    Q_D(QOrbitCameraController);

    Qt3DRender::QCamera *theCamera = camera();

    if (theCamera == nullptr)
        return;

    const QVector3D upVector(0.0f, 1.0f, 0.0f);

    // Mouse input
    if (state.leftMouseButtonActive) {
        if (state.rightMouseButtonActive) {
            if ( zoomDistance(camera()->position(), theCamera->viewCenter()) > d->m_zoomInLimit * d->m_zoomInLimit) {
                // Dolly up to limit
                theCamera->translate(QVector3D(0, 0, state.ryAxisValue), theCamera->DontTranslateViewCenter);
            } else {
                theCamera->translate(QVector3D(0, 0, -0.5), theCamera->DontTranslateViewCenter);
            }
        } else {
            // Translate
            theCamera->translate(QVector3D(clampInputs(state.rxAxisValue, state.txAxisValue) * linearSpeed(),
                                          clampInputs(state.ryAxisValue, state.tyAxisValue) * linearSpeed(),
                                          0) * dt);
        }
        return;
    }
    else if (state.rightMouseButtonActive) {
        // Orbit
        theCamera->panAboutViewCenter((state.rxAxisValue * lookSpeed()) * dt, upVector);
        theCamera->tiltAboutViewCenter((state.ryAxisValue * lookSpeed()) * dt);
    }

    // Keyboard Input
    if (state.altKeyActive) {
        // Orbit
        theCamera->panAboutViewCenter((state.txAxisValue * lookSpeed()) * dt, upVector);
        theCamera->tiltAboutViewCenter((state.tyAxisValue * lookSpeed()) * dt);
    } else if (state.shiftKeyActive) {
        if (zoomDistance(camera()->position(), theCamera->viewCenter()) > d->m_zoomInLimit * d->m_zoomInLimit) {
            // Dolly
            theCamera->translate(QVector3D(0, 0, state.tzAxisValue * linearSpeed() * dt), theCamera->DontTranslateViewCenter);
        } else {
            theCamera->translate(QVector3D(0, 0, -0.5), theCamera->DontTranslateViewCenter);
        }
    } else {
        // Translate
        theCamera->translate(QVector3D(clampInputs(state.leftMouseButtonActive ? state.rxAxisValue : 0, state.txAxisValue) * linearSpeed(),
                                      clampInputs(state.leftMouseButtonActive ? state.ryAxisValue : 0, state.tyAxisValue) * linearSpeed(),
                                      state.tzAxisValue * linearSpeed()) * dt);
    }
}

} // Qt3DExtras

QT_END_NAMESPACE

#include "moc_qorbitcameracontroller.cpp"
