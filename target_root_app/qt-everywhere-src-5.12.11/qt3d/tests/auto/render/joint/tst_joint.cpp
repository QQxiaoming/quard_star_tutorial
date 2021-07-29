/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include <QtTest/QTest>
#include <Qt3DRender/private/joint_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DCore/qjoint.h>
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qpropertyupdatedchangebase_p.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qvector3d.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>
#include <testrenderer.h>

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DRender::Render;

class tst_Joint : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:
    void checkPeerPropertyMirroring()
    {
        // GIVEN
        TestRenderer renderer;
        NodeManagers nodeManagers;
        renderer.setNodeManagers(&nodeManagers);
        Joint backendJoint;
        backendJoint.setRenderer(&renderer);
        backendJoint.setJointManager(nodeManagers.jointManager());
        QJoint joint;

        joint.setTranslation(QVector3D(1.0f, 2.0f, 3.0f));
        joint.setScale(QVector3D(1.5f, 2.5f, 3.5));
        joint.setRotation(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 45.0f));
        QMatrix4x4 inverseBind;
        inverseBind.rotate(-45.0f, 1.0f, 0.0, 0.0f);
        joint.setInverseBindMatrix(inverseBind);

        QVector<QJoint *> childJoints;
        for (int i = 0; i < 10; ++i) {
            const auto childJoint = new QJoint();
            joint.addChildJoint(childJoint);
            childJoints.push_back(childJoint);
        }

        // WHEN
        simulateInitialization(&joint, &backendJoint);

        // THEN
        QCOMPARE(backendJoint.peerId(), joint.id());
        QCOMPARE(backendJoint.isEnabled(), joint.isEnabled());
        QCOMPARE(backendJoint.translation(), joint.translation());
        QCOMPARE(backendJoint.rotation(), joint.rotation());
        QCOMPARE(backendJoint.scale(), joint.scale());
        QCOMPARE(backendJoint.inverseBindMatrix(), joint.inverseBindMatrix());
        for (int i = 0; i < childJoints.size(); ++i) {
            QCOMPARE(backendJoint.childJointIds()[i], childJoints[i]->id());
        }
    }

    void checkInitialAndCleanedUpState()
    {
        // GIVEN
        TestRenderer renderer;
        NodeManagers nodeManagers;
        renderer.setNodeManagers(&nodeManagers);
        Joint backendJoint;
        backendJoint.setRenderer(&renderer);
        backendJoint.setJointManager(nodeManagers.jointManager());

        // THEN
        QVERIFY(backendJoint.peerId().isNull());
        QCOMPARE(backendJoint.isEnabled(), false);
        QCOMPARE(backendJoint.translation(), QVector3D());
        QCOMPARE(backendJoint.rotation(), QQuaternion());
        QCOMPARE(backendJoint.scale(), QVector3D(1.0f, 1.0f, 1.0f));
        QCOMPARE(backendJoint.inverseBindMatrix(), QMatrix4x4());
        QCOMPARE(backendJoint.childJointIds(), QNodeIdVector());
        QCOMPARE(backendJoint.owningSkeleton(), HSkeleton());

        // GIVEN
        QJoint joint;
        joint.setTranslation(QVector3D(1.0f, 2.0f, 3.0f));
        joint.setScale(QVector3D(1.5f, 2.5f, 3.5));
        joint.setRotation(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 45.0f));
        QMatrix4x4 inverseBind;
        inverseBind.rotate(-45.0f, 1.0f, 0.0, 0.0f);
        joint.setInverseBindMatrix(inverseBind);

        QVector<QJoint *> childJoints;
        for (int i = 0; i < 10; ++i) {
            const auto childJoint = new QJoint();
            joint.addChildJoint(childJoint);
            childJoints.push_back(childJoint);
        }

        // WHEN
        simulateInitialization(&joint, &backendJoint);
        backendJoint.cleanup();

        // THEN
        QCOMPARE(backendJoint.isEnabled(), false);
        QCOMPARE(backendJoint.translation(), QVector3D());
        QCOMPARE(backendJoint.rotation(), QQuaternion());
        QCOMPARE(backendJoint.scale(), QVector3D(1.0f, 1.0f, 1.0f));
        QCOMPARE(backendJoint.inverseBindMatrix(), QMatrix4x4());
        QCOMPARE(backendJoint.childJointIds(), QNodeIdVector());
        QCOMPARE(backendJoint.owningSkeleton(), HSkeleton());
    }

    void checkPropertyChanges()
    {
        // GIVEN
        TestRenderer renderer;
        NodeManagers nodeManagers;
        renderer.setNodeManagers(&nodeManagers);
        Joint backendJoint;
        backendJoint.setRenderer(&renderer);
        backendJoint.setJointManager(nodeManagers.jointManager());
        backendJoint.setSkeletonManager(nodeManagers.skeletonManager());
        Qt3DCore::QPropertyUpdatedChangePtr updateChange;

        // WHEN
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("enabled");
        updateChange->setValue(true);
        backendJoint.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendJoint.isEnabled(), true);

        // WHEN
        const QVector3D newTranslation = QVector3D(1.0f, 2.0f, 3.0f);
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("translation");
        updateChange->setValue(newTranslation);
        backendJoint.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendJoint.translation(), newTranslation);

        // WHEN
        const QQuaternion newRotation = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, 45.0f);
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("rotation");
        updateChange->setValue(newRotation);
        backendJoint.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendJoint.rotation(), newRotation);

        // WHEN
        const QVector3D newScale = QVector3D(1.5f, 2.5f, 3.5f);
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("scale");
        updateChange->setValue(newScale);
        backendJoint.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendJoint.scale(), newScale);

        // WHEN
        QMatrix4x4 newInverseBind;
        newInverseBind.scale(5.4f);
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("inverseBindMatrix");
        updateChange->setValue(newInverseBind);
        backendJoint.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendJoint.inverseBindMatrix(), newInverseBind);

        // WHEN
        QVector<QJoint *> childJoints;
        QPropertyNodeAddedChangePtr nodeAddedChange;
        for (int i = 0; i < 10; ++i) {
            const auto childJoint = new QJoint();
            childJoints.push_back(childJoint);

            nodeAddedChange.reset(new QPropertyNodeAddedChange(QNodeId(), childJoint));
            nodeAddedChange->setPropertyName("childJoint");
            backendJoint.sceneChangeEvent(nodeAddedChange);
        }

        // THEN
        for (int i = 0; i < childJoints.size(); ++i) {
            QCOMPARE(backendJoint.childJointIds()[i], childJoints[i]->id());
        }

        QPropertyNodeRemovedChangePtr nodeRemovedChange;
        for (int i = 0; i < 10; ++i) {
            // WHEN
            const auto childJoint = childJoints.takeLast();

            nodeRemovedChange.reset(new QPropertyNodeRemovedChange(QNodeId(), childJoint));
            nodeRemovedChange->setPropertyName("childJoint");
            backendJoint.sceneChangeEvent(nodeAddedChange);

            // THEN
            for (int i = 0; i < childJoints.size(); ++i) {
                QCOMPARE(backendJoint.childJointIds()[i], childJoints[i]->id());
            }
        }
    }
};

QTEST_APPLESS_MAIN(tst_Joint)

#include "tst_joint.moc"
