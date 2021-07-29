/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include <QtTest/QtTest>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QAspectEngine>

#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QInputSettings>

#include <Qt3DLogic/QLogicAspect>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QRenderSettings>

#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>

#include <QPropertyAnimation>

namespace {

Qt3DCore::QEntity *createScene(QWindow *w)
{
    // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity;

    // Camera
    Qt3DRender::QCamera *camera = new Qt3DRender::QCamera(rootEntity);
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 40.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));

    // FrameGraph
    Qt3DRender::QRenderSettings *renderSettings = new Qt3DRender::QRenderSettings();
    Qt3DExtras::QForwardRenderer *forwardRenderer = new Qt3DExtras::QForwardRenderer();
    forwardRenderer->setSurface(w);
    forwardRenderer->setCamera(camera);
    forwardRenderer->setClearColor(QColor(Qt::blue));
    renderSettings->setActiveFrameGraph(forwardRenderer);
    rootEntity->addComponent(renderSettings);

    // InputSettings
    Qt3DInput::QInputSettings *inputSettigns = new Qt3DInput::QInputSettings();
    rootEntity->addComponent(inputSettigns);

    // Material
    Qt3DRender::QMaterial *material = new Qt3DExtras::QPhongMaterial(rootEntity);

    // Torus
    Qt3DCore::QEntity *torusEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DExtras::QTorusMesh *torusMesh = new Qt3DExtras::QTorusMesh;
    torusMesh->setRadius(5);
    torusMesh->setMinorRadius(1);
    torusMesh->setRings(100);
    torusMesh->setSlices(20);

    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform;
    torusTransform->setScale3D(QVector3D(1.5, 1, 0.5));
    torusTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 45.0f));

    torusEntity->addComponent(torusMesh);
    torusEntity->addComponent(torusTransform);
    torusEntity->addComponent(material);

    // Sphere
    Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DExtras::QSphereMesh *sphereMesh = new Qt3DExtras::QSphereMesh;
    sphereMesh->setRadius(3);

    Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform;

    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(sphereTransform);
    sphereEntity->addComponent(material);

    return rootEntity;
}

} // anonymous

class tst_AspectsStartupShutdown : public QObject
{
    Q_OBJECT

public:

private slots:

    void checkStartupAndShutdownImmediately()
    {
        // GIVEN
        QWindow *win = new QWindow();
        win->setSurfaceType(QSurface::OpenGLSurface);
        win->resize(1024, 768);
        win->show();

        // WHEN
        Qt3DCore::QAspectEngine *engine = new Qt3DCore::QAspectEngine();
        engine->registerAspect(new Qt3DRender::QRenderAspect());
        engine->registerAspect(new Qt3DInput::QInputAspect());
        engine->registerAspect(new Qt3DLogic::QLogicAspect());
        QPointer<Qt3DCore::QEntity> scene = createScene(win);
        engine->setRootEntity(Qt3DCore::QEntityPtr(scene.data()));

        // THEN
        QCOMPARE(engine->rootEntity().data(), scene.data());

        // WHEN
        win->close();
        delete engine;

        // THEN -> shouldn't crash or deadlock
        delete win;
    }

    void checkStartupAndShutdownAfterAFewFrames()
    {
        // GIVEN
        QWindow *win = new QWindow();
        win->setSurfaceType(QSurface::OpenGLSurface);
        win->resize(1024, 768);
        win->show();

        // WHEN
        Qt3DCore::QAspectEngine *engine = new Qt3DCore::QAspectEngine();
        engine->registerAspect(new Qt3DRender::QRenderAspect());
        engine->registerAspect(new Qt3DInput::QInputAspect());
        engine->registerAspect(new Qt3DLogic::QLogicAspect());
        QPointer<Qt3DCore::QEntity> scene = createScene(win);
        engine->setRootEntity(Qt3DCore::QEntityPtr(scene.data()));

        // THEN
        QCOMPARE(engine->rootEntity().data(), scene.data());

        // Allow a few frames
        for (int i = 0; i < 10; ++i) {
            QCoreApplication::processEvents();
            QThread::msleep(16);
        }

        // WHEN
        // Right now we need to close the window before destroying
        // the engine to prevent the render thread from trying to render
        // while things are being destroyed
        win->close();
        delete engine;

        // THEN -> shouldn't crash or deadlock
        delete win;
    }
};

QTEST_MAIN(tst_AspectsStartupShutdown)

#include "tst_aspects_startup_shutdown.moc"
