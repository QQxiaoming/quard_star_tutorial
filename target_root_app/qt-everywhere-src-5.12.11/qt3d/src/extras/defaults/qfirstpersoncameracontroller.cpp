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

#include "qfirstpersoncameracontroller.h"

#include <Qt3DRender/QCamera>

QT_BEGIN_NAMESPACE

namespace Qt3DExtras {

/*!
    \class Qt3DExtras::QFirstPersonCameraController
    \ingroup qt3d-extras-cameracontrollers
    \brief The QFirstPersonCameraController class allows controlling the scene camera
    from the first person perspective.
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
        \li While the left mouse button is pressed, mouse movement along x-axis pans the camera and
        movement along y-axis tilts it.
    \row
        \li Mouse scroll wheel
        \li Zooms the camera in and out without changing the view center.
    \row
        \li Shift key
        \li Turns the fine motion control active while pressed. Makes mouse pan and tilt less
        sensitive.
    \row
        \li Arrow keys
        \li Move the camera horizontally relative to camera viewport.
    \row
        \li Page up and page down keys
        \li Move the camera vertically relative to camera viewport.
    \row
        \li Escape
        \li Moves the camera so that entire scene is visible in the camera viewport.
    \endtable
*/

QFirstPersonCameraController::QFirstPersonCameraController(Qt3DCore::QNode *parent)
    : QAbstractCameraController(parent)
{
}

QFirstPersonCameraController::~QFirstPersonCameraController()
{
}


void QFirstPersonCameraController::moveCamera(const QAbstractCameraController::InputState &state, float dt)
{
    Qt3DRender::QCamera *theCamera = camera();

    if (theCamera == nullptr)
        return;

    theCamera->translate(QVector3D(state.txAxisValue * linearSpeed(),
                                  state.tyAxisValue * linearSpeed(),
                                  state.tzAxisValue * linearSpeed()) * dt);
    if (state.leftMouseButtonActive) {
        float theLookSpeed = lookSpeed();
        if (state.shiftKeyActive) {
            theLookSpeed *= 0.2f;
        }

        const QVector3D upVector(0.0f, 1.0f, 0.0f);

        theCamera->pan(state.rxAxisValue * theLookSpeed * dt, upVector);
        theCamera->tilt(state.ryAxisValue * theLookSpeed * dt);
    }
}

} // Qt3DExtras

QT_END_NAMESPACE

#include "moc_qfirstpersoncameracontroller.cpp"
