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

#include "qabstractcameracontroller.h"
#include "qabstractcameracontroller_p.h"

#include <Qt3DRender/QCamera>
#include <Qt3DInput/QAxis>
#include <Qt3DInput/QAnalogAxisInput>
#include <Qt3DInput/QButtonAxisInput>
#include <Qt3DInput/QAction>
#include <Qt3DInput/QActionInput>
#include <Qt3DInput/QLogicalDevice>
#include <Qt3DInput/QKeyboardDevice>
#include <Qt3DInput/QMouseDevice>
#include <Qt3DInput/QMouseEvent>
#include <Qt3DLogic/QFrameAction>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

namespace Qt3DExtras {

QAbstractCameraControllerPrivate::QAbstractCameraControllerPrivate()
    : Qt3DCore::QEntityPrivate()
    , m_camera(nullptr)
    , m_leftMouseButtonAction(new Qt3DInput::QAction())
    , m_middleMouseButtonAction(new Qt3DInput::QAction())
    , m_rightMouseButtonAction(new Qt3DInput::QAction())
    , m_altButtonAction(new Qt3DInput::QAction())
    , m_shiftButtonAction(new Qt3DInput::QAction())
    , m_escapeButtonAction(new Qt3DInput::QAction())
    , m_rxAxis(new Qt3DInput::QAxis())
    , m_ryAxis(new Qt3DInput::QAxis())
    , m_txAxis(new Qt3DInput::QAxis())
    , m_tyAxis(new Qt3DInput::QAxis())
    , m_tzAxis(new Qt3DInput::QAxis())
    , m_leftMouseButtonInput(new Qt3DInput::QActionInput())
    , m_middleMouseButtonInput(new Qt3DInput::QActionInput())
    , m_rightMouseButtonInput(new Qt3DInput::QActionInput())
    , m_altButtonInput(new Qt3DInput::QActionInput())
    , m_shiftButtonInput(new Qt3DInput::QActionInput())
    , m_escapeButtonInput(new Qt3DInput::QActionInput())
    , m_mouseRxInput(new Qt3DInput::QAnalogAxisInput())
    , m_mouseRyInput(new Qt3DInput::QAnalogAxisInput())
    , m_mouseTzXInput(new Qt3DInput::QAnalogAxisInput())
    , m_mouseTzYInput(new Qt3DInput::QAnalogAxisInput())
    , m_keyboardTxPosInput(new Qt3DInput::QButtonAxisInput())
    , m_keyboardTyPosInput(new Qt3DInput::QButtonAxisInput())
    , m_keyboardTzPosInput(new Qt3DInput::QButtonAxisInput())
    , m_keyboardTxNegInput(new Qt3DInput::QButtonAxisInput())
    , m_keyboardTyNegInput(new Qt3DInput::QButtonAxisInput())
    , m_keyboardTzNegInput(new Qt3DInput::QButtonAxisInput())
    , m_keyboardDevice(new Qt3DInput::QKeyboardDevice())
    , m_mouseDevice(new Qt3DInput::QMouseDevice())
    , m_logicalDevice(new Qt3DInput::QLogicalDevice())
    , m_frameAction(new Qt3DLogic::QFrameAction())
    , m_linearSpeed(10.0f)
    , m_lookSpeed(180.0f)
    , m_acceleration(-1.0f)
    , m_deceleration(-1.0f)
    , m_sceneUp(0.0f, 1.0f, 0.0f)
{}

void QAbstractCameraControllerPrivate::init()
{
    //// Actions

    // Left Mouse Button Action
    m_leftMouseButtonInput->setButtons(QVector<int>() << Qt::LeftButton);
    m_leftMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_leftMouseButtonAction->addInput(m_leftMouseButtonInput);

    // Middle Mouse Button Action
    m_middleMouseButtonInput->setButtons(QVector<int>() << Qt::MiddleButton);
    m_middleMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_middleMouseButtonAction->addInput(m_middleMouseButtonInput);

    // Right Mouse Button Action
    m_rightMouseButtonInput->setButtons(QVector<int>() << Qt::RightButton);
    m_rightMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_rightMouseButtonAction->addInput(m_rightMouseButtonInput);

    // Alt Button Action
    m_altButtonInput->setButtons(QVector<int>() << Qt::Key_Alt);
    m_altButtonInput->setSourceDevice(m_keyboardDevice);
    m_altButtonAction->addInput(m_altButtonInput);

    // Shift Button Action
    m_shiftButtonInput->setButtons(QVector<int>() << Qt::Key_Shift);
    m_shiftButtonInput->setSourceDevice(m_keyboardDevice);
    m_shiftButtonAction->addInput(m_shiftButtonInput);

    // Escape Button Action
    m_escapeButtonInput->setButtons(QVector<int>() << Qt::Key_Escape);
    m_escapeButtonInput->setSourceDevice(m_keyboardDevice);
    m_escapeButtonAction->addInput(m_escapeButtonInput);

    //// Axes

    // Mouse X
    m_mouseRxInput->setAxis(Qt3DInput::QMouseDevice::X);
    m_mouseRxInput->setSourceDevice(m_mouseDevice);
    m_rxAxis->addInput(m_mouseRxInput);

    // Mouse Y
    m_mouseRyInput->setAxis(Qt3DInput::QMouseDevice::Y);
    m_mouseRyInput->setSourceDevice(m_mouseDevice);
    m_ryAxis->addInput(m_mouseRyInput);

    // Mouse Wheel X
    m_mouseTzXInput->setAxis(Qt3DInput::QMouseDevice::WheelX);
    m_mouseTzXInput->setSourceDevice(m_mouseDevice);
    m_tzAxis->addInput(m_mouseTzXInput);

    // Mouse Wheel Y
    m_mouseTzYInput->setAxis(Qt3DInput::QMouseDevice::WheelY);
    m_mouseTzYInput->setSourceDevice(m_mouseDevice);
    m_tzAxis->addInput(m_mouseTzYInput);

    // Keyboard Pos Tx
    m_keyboardTxPosInput->setButtons(QVector<int>() << Qt::Key_Right);
    m_keyboardTxPosInput->setScale(1.0f);
    m_keyboardTxPosInput->setSourceDevice(m_keyboardDevice);
    m_txAxis->addInput(m_keyboardTxPosInput);

    // Keyboard Pos Tz
    m_keyboardTzPosInput->setButtons(QVector<int>() << Qt::Key_PageUp);
    m_keyboardTzPosInput->setScale(1.0f);
    m_keyboardTzPosInput->setSourceDevice(m_keyboardDevice);
    m_tzAxis->addInput(m_keyboardTzPosInput);

    // Keyboard Pos Ty
    m_keyboardTyPosInput->setButtons(QVector<int>() << Qt::Key_Up);
    m_keyboardTyPosInput->setScale(1.0f);
    m_keyboardTyPosInput->setSourceDevice(m_keyboardDevice);
    m_tyAxis->addInput(m_keyboardTyPosInput);

    // Keyboard Neg Tx
    m_keyboardTxNegInput->setButtons(QVector<int>() << Qt::Key_Left);
    m_keyboardTxNegInput->setScale(-1.0f);
    m_keyboardTxNegInput->setSourceDevice(m_keyboardDevice);
    m_txAxis->addInput(m_keyboardTxNegInput);

    // Keyboard Neg Tz
    m_keyboardTzNegInput->setButtons(QVector<int>() << Qt::Key_PageDown);
    m_keyboardTzNegInput->setScale(-1.0f);
    m_keyboardTzNegInput->setSourceDevice(m_keyboardDevice);
    m_tzAxis->addInput(m_keyboardTzNegInput);

    // Keyboard Neg Ty
    m_keyboardTyNegInput->setButtons(QVector<int>() << Qt::Key_Down);
    m_keyboardTyNegInput->setScale(-1.0f);
    m_keyboardTyNegInput->setSourceDevice(m_keyboardDevice);
    m_tyAxis->addInput(m_keyboardTyNegInput);

    //// Logical Device

    m_logicalDevice->addAction(m_leftMouseButtonAction);
    m_logicalDevice->addAction(m_middleMouseButtonAction);
    m_logicalDevice->addAction(m_rightMouseButtonAction);
    m_logicalDevice->addAction(m_altButtonAction);
    m_logicalDevice->addAction(m_shiftButtonAction);
    m_logicalDevice->addAction(m_escapeButtonAction);
    m_logicalDevice->addAxis(m_rxAxis);
    m_logicalDevice->addAxis(m_ryAxis);
    m_logicalDevice->addAxis(m_txAxis);
    m_logicalDevice->addAxis(m_tyAxis);
    m_logicalDevice->addAxis(m_tzAxis);

    applyInputAccelerations();

    Q_Q(QAbstractCameraController);
    //// FrameAction

    // Disable the logical device when the entity is disabled
    QObject::connect(q, &Qt3DCore::QEntity::enabledChanged,
                     m_logicalDevice, &Qt3DInput::QLogicalDevice::setEnabled);


    QObject::connect(m_escapeButtonAction, &Qt3DInput::QAction::activeChanged,
                     q, [this](bool isActive) {
                         if (isActive && m_camera)
                             m_camera->viewAll();
                     });

    q->addComponent(m_frameAction);
    q->addComponent(m_logicalDevice);
}

void QAbstractCameraControllerPrivate::applyInputAccelerations()
{
    const auto inputs = {
        m_keyboardTxPosInput,
        m_keyboardTyPosInput,
        m_keyboardTzPosInput,
        m_keyboardTxNegInput,
        m_keyboardTyNegInput,
        m_keyboardTzNegInput
    };

    for (auto input : inputs) {
        input->setAcceleration(m_acceleration);
        input->setDeceleration(m_deceleration);
    }
}

/*!
    \class Qt3DExtras::QAbstractCameraController

    \brief The QAbstractCameraController class provides basic
    functionality for camera controllers.

    \inmodule Qt3DExtras
    \since 5.10

    QAbstractCameraController sets up and handles input from keyboard,
    mouse, and other devices. QAbstractCameraController is an abstract
    class and cannot itself be instantiated. It provides a standard
    interface for camera controllers.

    Derived classes need only implement the frameActionTriggered()
    method to move the camera.
*/

/*!
    \fn void Qt3DExtras::QAbstractCameraController::moveCamera(const InputState &state, float dt) = 0

    This method is called whenever a frame action is triggered. Derived
    classes must override this method to implement the camera movement
    specific to the controller.

    In the base class this is a pure virtual function.
*/

QAbstractCameraController::QAbstractCameraController(Qt3DCore::QNode *parent)
    : QAbstractCameraController(*new QAbstractCameraControllerPrivate, parent)
{
}

/*! \internal
 */
QAbstractCameraController::QAbstractCameraController(QAbstractCameraControllerPrivate &dd, Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(dd, parent)
{
    Q_D(QAbstractCameraController);
    d->init();

    QObject::connect(d->m_frameAction, &Qt3DLogic::QFrameAction::triggered,
                     this, [=] (float dt) {
        InputState  state;

        state.rxAxisValue = d->m_rxAxis->value();
        state.ryAxisValue = d->m_ryAxis->value();
        state.txAxisValue = d->m_txAxis->value();
        state.tyAxisValue = d->m_tyAxis->value();
        state.tzAxisValue = d->m_tzAxis->value();

        state.leftMouseButtonActive = d->m_leftMouseButtonAction->isActive();
        state.middleMouseButtonActive = d->m_middleMouseButtonAction->isActive();
        state.rightMouseButtonActive = d->m_rightMouseButtonAction->isActive();

        state.altKeyActive = d->m_altButtonAction->isActive();
        state.shiftKeyActive = d->m_shiftButtonAction->isActive();

        moveCamera(state, dt);
    });
}

QAbstractCameraController::~QAbstractCameraController()
{
}

/*!
    \property Qt3DExtras::QAbstractCameraController::camera

    Holds the currently controlled camera.
*/
Qt3DRender::QCamera *QAbstractCameraController::camera() const
{
    Q_D(const QAbstractCameraController);
    return d->m_camera;
}

/*!
    \property Qt3DExtras::QAbstractCameraController::linearSpeed

    Holds the current linear speed of the camera controller. Linear speed determines the
    movement speed of the camera.

    The default is \c {10.0}.
*/
float QAbstractCameraController::linearSpeed() const
{
    Q_D(const QAbstractCameraController);
    return d->m_linearSpeed;
}

/*!
    \property Qt3DExtras::QAbstractCameraController::lookSpeed

    Holds the current look speed of the camera controller. The look speed determines the turn rate
    of the camera pan and tilt.

    The default is \c {180.0}.
*/
float QAbstractCameraController::lookSpeed() const
{
    Q_D(const QAbstractCameraController);
    return d->m_lookSpeed;
}

/*!
    \property Qt3DExtras::QAbstractCameraController::acceleration

    Holds the current acceleration of the camera controller.
*/
float QAbstractCameraController::acceleration() const
{
    Q_D(const QAbstractCameraController);
    return d->m_acceleration;
}

/*!
    \property Qt3DExtras::QAbstractCameraController::deceleration

    Holds the current deceleration of the camera controller.
*/
float QAbstractCameraController::deceleration() const
{
    Q_D(const QAbstractCameraController);
    return d->m_deceleration;
}

void QAbstractCameraController::setCamera(Qt3DRender::QCamera *camera)
{
    Q_D(QAbstractCameraController);
    if (d->m_camera != camera) {

        if (d->m_camera)
            d->unregisterDestructionHelper(d->m_camera);

        if (camera && !camera->parent())
            camera->setParent(this);

        d->m_camera = camera;

        // Ensures proper bookkeeping
        if (d->m_camera)
            d->registerDestructionHelper(d->m_camera, &QAbstractCameraController::setCamera, d->m_camera);

        emit cameraChanged();
    }
}

void QAbstractCameraController::setLinearSpeed(float linearSpeed)
{
    Q_D(QAbstractCameraController);
    if (d->m_linearSpeed != linearSpeed) {
        d->m_linearSpeed = linearSpeed;
        emit linearSpeedChanged();
    }
}

void QAbstractCameraController::setLookSpeed(float lookSpeed)
{
    Q_D(QAbstractCameraController);
    if (d->m_lookSpeed != lookSpeed) {
        d->m_lookSpeed = lookSpeed;
        emit lookSpeedChanged();
    }
}

void QAbstractCameraController::setAcceleration(float acceleration)
{
    Q_D(QAbstractCameraController);
    if (d->m_acceleration != acceleration) {
        d->m_acceleration = acceleration;
        d->applyInputAccelerations();
        emit accelerationChanged(acceleration);
    }
}

void QAbstractCameraController::setDeceleration(float deceleration)
{
    Q_D(QAbstractCameraController);
    if (d->m_deceleration != deceleration) {
        d->m_deceleration = deceleration;
        d->applyInputAccelerations();
        emit decelerationChanged(deceleration);
    }
}

/*!
    Provides access to the keyboard device.
*/

Qt3DInput::QKeyboardDevice *QAbstractCameraController::keyboardDevice() const
{
    Q_D(const QAbstractCameraController);
    return d->m_keyboardDevice;
}

/*!
    Provides access to the mouse device.
*/

Qt3DInput::QMouseDevice *QAbstractCameraController::mouseDevice() const
{
    Q_D(const QAbstractCameraController);
    return d->m_mouseDevice;
}

} // Qt3DExtras

QT_END_NAMESPACE

#include "moc_qabstractcameracontroller.cpp"
