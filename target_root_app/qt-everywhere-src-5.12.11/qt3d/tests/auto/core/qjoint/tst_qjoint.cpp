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
#include <Qt3DCore/private/qjoint_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <QObject>
#include <QSignalSpy>
#include <testpostmanarbiter.h>

using namespace Qt3DCore;

class tst_QJoint : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkDefaultConstruction()
    {
        // GIVEN
        QJoint joint;

        // THEN
        QCOMPARE(joint.scale(), QVector3D(1.0f, 1.0f, 1.0f));
        QCOMPARE(joint.rotation(), QQuaternion());
        QCOMPARE(joint.translation(), QVector3D(0.0f, 0.0f, 0.0f));
        QCOMPARE(joint.inverseBindMatrix(), QMatrix4x4());
        QCOMPARE(joint.rotationX(), 0.0f);
        QCOMPARE(joint.rotationY(), 0.0f);
        QCOMPARE(joint.rotationZ(), 0.0f);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        QJoint joint;

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(scaleChanged(QVector3D)));
            const QVector3D newValue(2.5f, 2.0f, 1.3f);
            joint.setScale(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(joint.scale(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            joint.setScale(newValue);

            // THEN
            QCOMPARE(joint.scale(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(rotationChanged(QQuaternion)));
            const auto newValue = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 45.0f);
            joint.setRotation(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(joint.rotation(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            joint.setRotation(newValue);

            // THEN
            QCOMPARE(joint.rotation(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(translationChanged(QVector3D)));
            const QVector3D newValue(1.0f, 2.0f, 3.0f);
            joint.setTranslation(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(joint.translation(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            joint.setTranslation(newValue);

            // THEN
            QCOMPARE(joint.translation(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(inverseBindMatrixChanged(QMatrix4x4)));
            QMatrix4x4 newValue;
            newValue.scale(3.5f);
            joint.setInverseBindMatrix(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(joint.inverseBindMatrix(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            joint.setInverseBindMatrix(newValue);

            // THEN
            QCOMPARE(joint.inverseBindMatrix(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(rotationChanged(QQuaternion)));
            QSignalSpy spyEuler(&joint, SIGNAL(rotationXChanged(float)));
            const auto newValue = 45.0f;
            const auto newValueAsQuaternion = QQuaternion::fromEulerAngles(newValue, 0.0f, 0.0f);
            joint.setRotationX(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QVERIFY(spyEuler.isValid());
            QCOMPARE(joint.rotationX(), newValue);
            QCOMPARE(joint.rotation(), newValueAsQuaternion);
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spyEuler.count(), 1);

            // WHEN
            spy.clear();
            spyEuler.clear();
            joint.setRotationX(newValue);

            // THEN
            QCOMPARE(joint.rotationX(), newValue);
            QCOMPARE(joint.rotation(), newValueAsQuaternion);
            QCOMPARE(spy.count(), 0);
            QCOMPARE(spyEuler.count(), 0);

            joint.setRotationX(0.0f);
        }

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(rotationChanged(QQuaternion)));
            QSignalSpy spyEuler(&joint, SIGNAL(rotationYChanged(float)));
            const auto newValue = 45.0f;
            const auto newValueAsQuaternion = QQuaternion::fromEulerAngles(0.0f, newValue, 0.0f);
            joint.setRotationY(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QVERIFY(spyEuler.isValid());
            QCOMPARE(joint.rotationY(), newValue);
            QCOMPARE(joint.rotation(), newValueAsQuaternion);
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spyEuler.count(), 1);

            // WHEN
            spy.clear();
            spyEuler.clear();
            joint.setRotationY(newValue);

            // THEN
            QCOMPARE(joint.rotationY(), newValue);
            QCOMPARE(joint.rotation(), newValueAsQuaternion);
            QCOMPARE(spy.count(), 0);
            QCOMPARE(spyEuler.count(), 0);

            joint.setRotationY(0.0f);
        }

        {
            // WHEN
            joint.setToIdentity();
            QSignalSpy spy(&joint, SIGNAL(rotationChanged(QQuaternion)));
            QSignalSpy spyEuler(&joint, SIGNAL(rotationZChanged(float)));
            const auto newValue = 45.0f;
            const auto newValueAsQuaternion = QQuaternion::fromEulerAngles(0.0f, 0.0f, newValue);
            joint.setRotationZ(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QVERIFY(spyEuler.isValid());
            QCOMPARE(joint.rotationZ(), newValue);
            QCOMPARE(joint.rotation(), newValueAsQuaternion);
            QCOMPARE(spy.count(), 1);
            QCOMPARE(spyEuler.count(), 1);

            // WHEN
            spy.clear();
            spyEuler.clear();
            joint.setRotationZ(newValue);

            // THEN
            QCOMPARE(joint.rotationZ(), newValue);
            QCOMPARE(joint.rotation(), newValueAsQuaternion);
            QCOMPARE(spy.count(), 0);
            QCOMPARE(spyEuler.count(), 0);

            joint.setRotationZ(0.0f);
        }
    }

    void checkCreationData()
    {
        // GIVEN
        QJoint joint;

        joint.setScale(QVector3D(3.5f, 2.0f, 1.3f));
        joint.setRotation(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, 30.0f));
        joint.setTranslation(QVector3D(3.0f, 2.0f, 1.0f));
        QMatrix4x4 ibm;
        ibm.scale(5.2f);
        joint.setInverseBindMatrix(ibm);

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&joint);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData
                = qSharedPointerCast<QNodeCreatedChange<QJointData>>(creationChanges.first());
            const QJointData data = creationChangeData->data;

            QCOMPARE(joint.id(), creationChangeData->subjectId());
            QCOMPARE(joint.isEnabled(), true);
            QCOMPARE(joint.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(joint.metaObject(), creationChangeData->metaObject());
            QCOMPARE(joint.scale(), data.scale);
            QCOMPARE(joint.rotation(), data.rotation);
            QCOMPARE(joint.translation(), data.translation);
            QCOMPARE(joint.inverseBindMatrix(), data.inverseBindMatrix);
        }

        // WHEN
        joint.setEnabled(false);

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&joint);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData
                = qSharedPointerCast<QNodeCreatedChange<QJointData>>(creationChanges.first());
            const QJointData data = creationChangeData->data;

            QCOMPARE(joint.id(), creationChangeData->subjectId());
            QCOMPARE(joint.isEnabled(), false);
            QCOMPARE(joint.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(joint.metaObject(), creationChangeData->metaObject());
            QCOMPARE(joint.scale(), data.scale);
            QCOMPARE(joint.rotation(), data.rotation);
            QCOMPARE(joint.translation(), data.translation);
            QCOMPARE(joint.inverseBindMatrix(), data.inverseBindMatrix);
        }
    }

    void checkPropertyUpdateChanges()
    {
        // GIVEN
        TestArbiter arbiter;
        QJoint joint;
        arbiter.setArbiterOnNode(&joint);

        {
            // WHEN
            joint.setScale(QVector3D(2.0f, 1.0f, 3.0f));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "scale");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<QVector3D>(), joint.scale());

            arbiter.events.clear();

            // WHEN
            joint.setScale(QVector3D(2.0f, 1.0f, 3.0f));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

        {
            // WHEN
            const auto newValue = QQuaternion::fromAxisAndAngle(1.0f, 1.0f, 1.0f, 45.0f);
            joint.setRotation(newValue);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "rotation");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<QQuaternion>(), newValue);

            arbiter.events.clear();

            // WHEN
            joint.setRotation(newValue);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

        {
            // WHEN
            const QVector3D newValue(1.0f, 2.0f, 3.0f);
            joint.setTranslation(newValue);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "translation");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<QVector3D>(), newValue);

            arbiter.events.clear();

            // WHEN
            joint.setTranslation(newValue);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

        {
            // WHEN
            QMatrix4x4 newValue;
            newValue.rotate(90.0f, 1.0f, 0.0f, 0.0f);
            joint.setInverseBindMatrix(newValue);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "inverseBindMatrix");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<QMatrix4x4>(), newValue);

            arbiter.events.clear();

            // WHEN
            joint.setInverseBindMatrix(newValue);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }
};

QTEST_MAIN(tst_QJoint)

#include "tst_qjoint.moc"
