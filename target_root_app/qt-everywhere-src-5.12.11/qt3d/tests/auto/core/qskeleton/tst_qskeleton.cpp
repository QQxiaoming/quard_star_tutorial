/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
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
#include <Qt3DCore/qskeleton.h>
#include <Qt3DCore/private/qskeleton_p.h>
#include <Qt3DCore/qjoint.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>

#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>

#include <QSignalSpy>
#include <testpostmanarbiter.h>

using namespace Qt3DCore;

class tst_QSkeleton: public QSkeleton
{
    Q_OBJECT
public:
    tst_QSkeleton()
    {
        qRegisterMetaType<Qt3DCore::QJoint*>();
    }

private Q_SLOTS:
    void checkDefaultConstruction()
    {
        // GIVEN
        QSkeleton skeleton;

        // THEN
        QCOMPARE(skeleton.jointCount(), 0);
    }

    void checkCreationChange_data()
    {
        QTest::addColumn<QSkeleton *>("skeleton");

        QSkeleton *defaultConstructed = new QSkeleton();
        QTest::newRow("defaultConstructed") << defaultConstructed;

        QSkeleton *skeletonWithOneJoint = new QSkeleton();
        skeletonWithOneJoint->setRootJoint(new QJoint());
        QTest::newRow("skeletonWithOneJoint") << skeletonWithOneJoint;
    }

    void checkCreationChange()
    {
        // GIVEN
        QFETCH(QSkeleton *, skeleton);

        // WHEN
        QNodeCreatedChangeGenerator creationChangeGenerator(skeleton);
        QVector<QNodeCreatedChangeBasePtr> creationChanges = creationChangeGenerator.creationChanges();

        const int jointCount = skeleton->rootJoint() ? 1 : 0;

        // THEN
        QCOMPARE(creationChanges.size(), 1 + jointCount);

        const auto creationChangeData = qSharedPointerCast<QNodeCreatedChange<QSkeletonData>>(creationChanges.first());
        const QSkeletonData &data = creationChangeData->data;

        // THEN
        QCOMPARE(skeleton->id(), creationChangeData->subjectId());
        QCOMPARE(skeleton->isEnabled(), creationChangeData->isNodeEnabled());
        QCOMPARE(skeleton->metaObject(), creationChangeData->metaObject());
        if (skeleton->rootJoint()) {
            QCOMPARE(skeleton->rootJoint()->id(), data.rootJointId);
        }
    }

    void checkPropertyUpdates()
    {
        // GIVEN
        TestArbiter arbiter;
        QScopedPointer<QSkeleton> skeleton(new QSkeleton());
        arbiter.setArbiterOnNode(skeleton.data());

        // WHEN
        QJoint *joint = new QJoint(skeleton.data());
        QCoreApplication::processEvents();
        arbiter.events.clear();

        skeleton->setRootJoint(joint);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "rootJoint");
        QCOMPARE(change->value().value<QNodeId>(), joint->id());
        QCOMPARE(change->type(), PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        skeleton->setRootJoint(nullptr);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "rootJoint");
        QCOMPARE(change->value().value<QNodeId>(), QNodeId());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();
    }

    void checkRootJointBookkeeping()
    {
        // GIVEN
        QScopedPointer<QSkeleton> skeleton(new QSkeleton);
        {
            // WHEN
            QJoint joint;
            skeleton->setRootJoint(&joint);

            // THEN
            QCOMPARE(joint.parent(), skeleton.data());
            QCOMPARE(skeleton->rootJoint(), &joint);
        }
        // THEN (Should not crash and parameter be unset)
        QVERIFY(skeleton->rootJoint() == nullptr);

        {
            // WHEN
            QSkeleton someOtherSkeleton;
            QScopedPointer<QJoint> joint(new QJoint(&someOtherSkeleton));
            skeleton->setRootJoint(joint.data());

            // THEN
            QCOMPARE(joint->parent(), &someOtherSkeleton);
            QCOMPARE(skeleton->rootJoint(), joint.data());

            // WHEN
            skeleton.reset();
            joint.reset();

            // THEN Should not crash when the joint is destroyed (tests for failed removal of destruction helper)
        }
    }

    void checkJointCountPropertyUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        arbiter.setArbiterOnNode(this);
        QSignalSpy spy(this, SIGNAL(jointCountChanged(int)));
        const int newJointCount = 99;

        // THEN
        QVERIFY(spy.isValid());

        // WHEN
        auto valueChange = QPropertyUpdatedChangePtr::create(Qt3DCore::QNodeId());
        valueChange->setPropertyName("jointCount");
        valueChange->setValue(QVariant(newJointCount));
        sceneChangeEvent(valueChange);

        // THEN
        QCOMPARE(spy.count(), 1);
        QCOMPARE(arbiter.events.size(), 0);
        QCOMPARE(jointCount(), newJointCount);

        // WHEN
        spy.clear();
        sceneChangeEvent(valueChange);

        // THEN
        QCOMPARE(spy.count(), 0);
        QCOMPARE(arbiter.events.size(), 0);
        QCOMPARE(jointCount(), newJointCount);
    }
};

QTEST_MAIN(tst_QSkeleton)

#include "tst_qskeleton.moc"
