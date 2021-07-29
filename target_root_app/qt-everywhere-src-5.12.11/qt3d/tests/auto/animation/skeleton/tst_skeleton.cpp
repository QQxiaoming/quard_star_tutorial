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
#include <Qt3DCore/qjoint.h>
#include <Qt3DCore/qskeleton.h>
#include <Qt3DAnimation/private/skeleton_p.h>
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qbackendnode_p.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

using namespace Qt3DCore;
using namespace Qt3DAnimation;
using namespace Qt3DAnimation::Animation;

Q_DECLARE_METATYPE(Skeleton*)

class tst_Skeleton : public QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:
    void checkPeerPropertyMirroring()
    {
        // GIVEN
        Handler handler;
        Skeleton backendSkeleton;
        backendSkeleton.setHandler(&handler);
        QSkeleton skeleton;
        auto rootJoint = new QJoint();
        rootJoint->setName(QLatin1String("rootJoint"));
        auto childJoint = new QJoint();
        childJoint->setName(QLatin1String("childJoint"));
        rootJoint->addChildJoint(childJoint);
        skeleton.setRootJoint(rootJoint);

        // WHEN
        simulateInitialization(&skeleton, &backendSkeleton);

        // THEN - nothing mirrored from frontend
        QCOMPARE(backendSkeleton.peerId(), skeleton.id());
        QCOMPARE(backendSkeleton.isEnabled(), skeleton.isEnabled());
        QCOMPARE(backendSkeleton.jointNames().size(), 0);
        QCOMPARE(backendSkeleton.jointLocalPoses().size(), 0);
    }

    void checkJointName()
    {
        // GIVEN
        Skeleton backendSkeleton;
        const QVector<QString> jointNames = { QLatin1String("rootJoint"),
                                              QLatin1String("child1Joint"),
                                              QLatin1String("child2Joint") };

        // WHEN
        backendSkeleton.setJointNames(jointNames);

        // THEN
        const int jointNameCount = jointNames.size();
        for (int i = 0; i < jointNameCount; ++i) {
            QCOMPARE(jointNames[i], backendSkeleton.jointName(i));
        }
    }

    void checkInitialAndCleanedUpState()
    {
        // GIVEN
        Handler handler;
        Skeleton backendSkeleton;
        backendSkeleton.setHandler(&handler);

        // THEN
        QVERIFY(backendSkeleton.peerId().isNull());
        QCOMPARE(backendSkeleton.isEnabled(), false);
        QCOMPARE(backendSkeleton.jointNames().size(), 0);
        QCOMPARE(backendSkeleton.jointLocalPoses().size(), 0);

        // GIVEN
        const QVector<QString> names = (QVector<QString>()
                                        << QLatin1String("root")
                                        << QLatin1String("child1")
                                        << QLatin1String("child2"));
        const QVector<Sqt> localPoses = (QVector<Sqt>() << Sqt() << Sqt() << Sqt());

        // WHEN
        backendSkeleton.setJointNames(names);
        backendSkeleton.setJointLocalPoses(localPoses);
        backendSkeleton.cleanup();

        // THEN
        QCOMPARE(backendSkeleton.isEnabled(), false);
        QCOMPARE(backendSkeleton.jointNames().size(), 0);
        QCOMPARE(backendSkeleton.jointLocalPoses().size(), 0);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Handler handler;
        Skeleton backendSkeleton;
        backendSkeleton.setHandler(&handler);
        QPropertyUpdatedChangePtr updateChange;

        // WHEN
        updateChange = QPropertyUpdatedChangePtr::create(QNodeId());
        updateChange->setPropertyName("enabled");
        updateChange->setValue(true);
        backendSkeleton.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendSkeleton.isEnabled(), true);

        // GIVEN
        const QVector<QString> names = (QVector<QString>()
                                        << QLatin1String("root")
                                        << QLatin1String("child1")
                                        << QLatin1String("child2"));
        const QVector<Sqt> localPoses = (QVector<Sqt>() << Sqt() << Sqt() << Sqt());

        // WHEN
        JointNamesAndLocalPoses namesAndPoses;
        namesAndPoses.names = names;
        namesAndPoses.localPoses = localPoses;

        updateChange = QPropertyUpdatedChangePtr::create(QNodeId());
        updateChange->setPropertyName("jointNamesAndLocalPoses");
        updateChange->setValue(QVariant::fromValue(namesAndPoses));
        backendSkeleton.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendSkeleton.jointNames(), names);
        QCOMPARE(backendSkeleton.jointLocalPoses(), localPoses);
    }

    void checkJointTransforms_data()
    {
        QTest::addColumn<Skeleton*>("skeleton");
        QTest::addColumn<QVector<Sqt>>("jointTransforms");

        const int count = 5;
        auto skeleton = new Skeleton;
        skeleton->setJointCount(count);
        QVector<Sqt> jointTransforms;
        jointTransforms.reserve(count);
        for (int i = 0; i < count; ++i) {
            const float f = float(i);
            Sqt transform;
            transform.scale = QVector3D(f, f, f);
            transform.rotation = QQuaternion(f, 1.0f, 0.0f, 0.0f).normalized();
            transform.translation = QVector3D(1.0 * f, 2.0 * f, 3.0 * f);
            skeleton->setJointScale(i, transform.scale);
            skeleton->setJointRotation(i, transform.rotation);
            skeleton->setJointTranslation(i, transform.translation);
            jointTransforms.push_back(transform);
        }

        QTest::newRow("5 joints") << skeleton << jointTransforms;
    }

    void checkJointTransforms()
    {
        // GIVEN
        QFETCH(Skeleton*, skeleton);
        QFETCH(QVector<Sqt>, jointTransforms);

        const int count = skeleton->jointCount();
        for (int i = 0; i < count; ++i) {
            // WHEN
            const QVector3D s = skeleton->jointScale(i);

            // THEN
            QCOMPARE(s, jointTransforms[i].scale);

            // WHEN
            const QQuaternion q = skeleton->jointRotation(i);

            // THEN
            QCOMPARE(q, jointTransforms[i].rotation);

            // WHEN
            const QVector3D t = skeleton->jointTranslation(i);

            // THEN
            QCOMPARE(t, jointTransforms[i].translation);
        }

        // Cleanup
        delete skeleton;
    }
};

QTEST_APPLESS_MAIN(tst_Skeleton)

#include "tst_skeleton.moc"
