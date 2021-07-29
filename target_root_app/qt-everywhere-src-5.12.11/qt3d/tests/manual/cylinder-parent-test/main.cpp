/****************************************************************************
**
** Copyright (C) 2019 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QGuiApplication>
#include <QTimer>

#include <Qt3DInput/QInputAspect>

#include <Qt3DRender/qcamera.h>
#include <Qt3DRender/qcameralens.h>
#include <Qt3DExtras/qcylindermesh.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DExtras/qphongmaterial.h>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DExtras/qforwardrenderer.h>

#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DCore/qaspectengine.h>

#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qorbitcameracontroller.h>
#include <QThread>
#include <QLoggingCategory>

#include <type_traits>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Qt3DExtras::Qt3DWindow view;

    QLoggingCategory::setFilterRules("Qt3D.Renderer.RenderNodes=true");

    // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
    view.setRootEntity(rootEntity);
    rootEntity->setObjectName("Root Entity");

    // Set root object of the scene
    view.show();

    // Camera
    Qt3DRender::QCamera *camera = view.camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 20.0f));
    camera->setUpVector(QVector3D(0, 1, 0));
    camera->setViewCenter(QVector3D(0, 0, 0));

    // For camera controls
    Qt3DExtras::QOrbitCameraController *cameraController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    cameraController->setCamera(camera);

    // Cylinder shape data
    Qt3DExtras::QCylinderMesh *mesh = new Qt3DExtras::QCylinderMesh();

    qDebug() << "Setup complete.  Creating cylinders\n";

    // simple setParent from nullptr (OK for QTBUG-73905)
    // green cylinder, bottom left
    {
        Qt3DCore::QTransform *leftTransform = new Qt3DCore::QTransform;
        leftTransform->setTranslation(QVector3D(-5, -2, 0));
        leftTransform->setObjectName("Green transform");

        Qt3DExtras::QPhongMaterial *greenMaterial = new Qt3DExtras::QPhongMaterial(rootEntity);
        greenMaterial->setObjectName("Green Material");
        greenMaterial->setDiffuse(Qt::green);

        Qt3DCore::QEntity *grandParentNode = new Qt3DCore::QEntity();
        Qt3DCore::QEntity *parentNode = new Qt3DCore::QEntity();
        Qt3DCore::QEntity *leafNode = new Qt3DCore::QEntity();
        grandParentNode->setObjectName("Green Grandparent");
        parentNode->setObjectName("Green Parent");
        leafNode->setObjectName("Green Leaf");

        leafNode->addComponent(mesh);
        leafNode->addComponent(greenMaterial);
        parentNode->addComponent(leftTransform);

        grandParentNode->setParent(rootEntity);
        parentNode->setParent(grandParentNode);
        leafNode->setParent(parentNode);
    }

    // simple setParent from rootEntity (doesn't work QTBUG-73905)
    // yellow cylinder, top left
    {
        Qt3DCore::QTransform *leftTransform = new Qt3DCore::QTransform;
        leftTransform->setTranslation(QVector3D(-5, 2, 0));
        leftTransform->setObjectName("Yellow Transform");

        Qt3DExtras::QPhongMaterial *yellowMaterial = new Qt3DExtras::QPhongMaterial(rootEntity);
        yellowMaterial->setObjectName("Yellow Material");
        yellowMaterial->setDiffuse(Qt::yellow);

        Qt3DCore::QEntity *grandParentNode = new Qt3DCore::QEntity(rootEntity);
        Qt3DCore::QEntity *parentNode = new Qt3DCore::QEntity(rootEntity);
        Qt3DCore::QEntity *leafNode = new Qt3DCore::QEntity(rootEntity);
        leafNode->setObjectName("Yellow Leaf");
        grandParentNode->setObjectName("Yellow Grandparent");
        parentNode->setObjectName("Yellow Parent");

        leafNode->addComponent(mesh);
        leafNode->addComponent(yellowMaterial);
        parentNode->addComponent(leftTransform);

        // sometimes this can change things
        //QCoreApplication::processEvents();

        grandParentNode->setParent(rootEntity);
        parentNode->setParent(grandParentNode);
        leafNode->setParent(parentNode);
    }

    // complex setParent from nullptr (OK QTBUG-73905?)
    // red cylinder, Bottom-right
    {
        Qt3DCore::QNode *tree1node1 = new Qt3DCore::QNode();
        Qt3DCore::QEntity *tree1node2 = new Qt3DCore::QEntity();
        Qt3DCore::QNode *tree1node3 = new Qt3DCore::QNode();
        tree1node1->setObjectName("Red Tree1-Node1");
        tree1node2->setObjectName("Red Tree1-Node2");
        tree1node3->setObjectName("Red Tree1-Node3");

        Qt3DCore::QNode *tree2node1 = new Qt3DCore::QNode();
        Qt3DCore::QEntity *tree2node2 = new Qt3DCore::QEntity();
        Qt3DCore::QNode *tree2node3 = new Qt3DCore::QNode();
        tree2node1->setObjectName("Red Tree2-Node1");
        tree2node2->setObjectName("Red Tree2-Node2");
        tree2node3->setObjectName("Red Tree2-Node3");

        Qt3DCore::QTransform *wrongRedTransform = new Qt3DCore::QTransform;
        wrongRedTransform->setTranslation(QVector3D(1, -1, 0));
        Qt3DCore::QTransform *bottomRightTransform = new Qt3DCore::QTransform;
        bottomRightTransform->setTranslation(QVector3D(5, -2, 0));
        bottomRightTransform->setObjectName("Red BR Transform");
        wrongRedTransform->setObjectName("Red Wrong Transform");

        Qt3DExtras::QPhongMaterial *redMaterial = new Qt3DExtras::QPhongMaterial(rootEntity);
        redMaterial->setDiffuse(Qt::red);
        redMaterial->setObjectName("Red Material");
        Qt3DCore::QEntity *leafNode = new Qt3DCore::QEntity();
        leafNode->setObjectName("Red Leaf");
        leafNode->addComponent(mesh);
        leafNode->addComponent(redMaterial);

        tree1node2->addComponent(wrongRedTransform);
        tree2node2->addComponent(bottomRightTransform);

        tree1node1->setParent(rootEntity);
        tree1node2->setParent(tree1node1);
        tree1node3->setParent(tree1node2);

        tree2node1->setParent(rootEntity);
        tree2node2->setParent(tree2node1);
        tree2node3->setParent(tree2node2);

        leafNode->setParent(tree1node3);
        leafNode->setParent(tree2node3);
    }

    // complex setParent from rootEntity (doesn't work QTBUG-73905)
    // blue cylinder, top right
    {
        Qt3DCore::QNode *tree1node1 = new Qt3DCore::QNode(rootEntity);
        Qt3DCore::QEntity *tree1node2 = new Qt3DCore::QEntity(rootEntity);
        Qt3DCore::QNode *tree1node3 = new Qt3DCore::QNode(rootEntity);
        tree1node1->setObjectName("Blue Tree1-Node1");
        tree1node2->setObjectName("Blue Tree1-Node2");
        tree1node3->setObjectName("Blue Tree1-Node3");

        Qt3DCore::QNode *tree2node1 = new Qt3DCore::QNode(rootEntity);
        Qt3DCore::QEntity *tree2node2 = new Qt3DCore::QEntity(rootEntity);
        Qt3DCore::QNode *tree2node3 = new Qt3DCore::QNode(rootEntity);
        tree2node1->setObjectName("Blue Tree2-Node1");
        tree2node2->setObjectName("Blue Tree2-Node2");
        tree2node3->setObjectName("Blue Tree2-Node3");

        Qt3DCore::QTransform *wrongBlueTransform = new Qt3DCore::QTransform;
        wrongBlueTransform->setTranslation(QVector3D(1, 1, 0));
        Qt3DCore::QTransform *topRightTransform = new Qt3DCore::QTransform;
        topRightTransform->setTranslation(QVector3D(5, 2, 0));
        wrongBlueTransform->setObjectName("Blue Wrong Transform");
        topRightTransform->setObjectName("Blue TR Transform");

        Qt3DExtras::QPhongMaterial *blueMaterial = new Qt3DExtras::QPhongMaterial(rootEntity);
        blueMaterial->setObjectName("Blue Material");
        blueMaterial->setDiffuse(Qt::blue);
        Qt3DCore::QEntity *leafNode = new Qt3DCore::QEntity(rootEntity);
        leafNode->addComponent(mesh);
        leafNode->addComponent(blueMaterial);
        leafNode->setObjectName("Blue Leaf");

        // sometimes this can change things
        //QCoreApplication::processEvents();

        tree1node2->addComponent(wrongBlueTransform);
        tree2node2->addComponent(topRightTransform);

        tree1node1->setParent(rootEntity);
        tree1node2->setParent(tree1node1);
        tree1node3->setParent(tree1node2);

        tree2node1->setParent(rootEntity);
        tree2node2->setParent(tree2node1);
        tree2node3->setParent(tree2node2);

        leafNode->setParent(tree1node3);
        leafNode->setParent(tree2node3);
    }

    return app.exec();
}
