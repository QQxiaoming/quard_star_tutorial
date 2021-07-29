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
#include <Qt3DCore/qskeletonloader.h>
#include <Qt3DCore/qjoint.h>
#include <Qt3DCore/private/qskeletonloader_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <QObject>
#include <QSignalSpy>
#include <testpostmanarbiter.h>

using namespace Qt3DCore;

class tst_QSkeletonLoader : public QSkeletonLoader
{
    Q_OBJECT

private Q_SLOTS:
    void checkDefaultConstruction()
    {
        // GIVEN
        QSkeletonLoader skeleton;

        // THEN
        QCOMPARE(skeleton.source(), QUrl());
        QCOMPARE(skeleton.status(), QSkeletonLoader::NotReady);
        QCOMPARE(skeleton.isCreateJointsEnabled(), false);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        QSkeletonLoader skeleton;

        {
            // WHEN
            QSignalSpy spy(&skeleton, SIGNAL(sourceChanged(QUrl)));
            const QUrl newValue(QStringLiteral("qrc:/zergling.skel"));
            skeleton.setSource(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(skeleton.source(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            skeleton.setSource(newValue);

            // THEN
            QCOMPARE(skeleton.source(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            QSignalSpy spy(&skeleton, SIGNAL(createJointsEnabledChanged(bool)));
            const bool newValue(true);
            skeleton.setCreateJointsEnabled(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(skeleton.isCreateJointsEnabled(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            skeleton.setCreateJointsEnabled(newValue);

            // THEN
            QCOMPARE(skeleton.isCreateJointsEnabled(), newValue);
            QCOMPARE(spy.count(), 0);
        }
    }

    void checkCreationData()
    {
        // GIVEN
        QSkeletonLoader skeleton;

        skeleton.setSource(QUrl(QStringLiteral("http://someRemoteURL.com/dem-bones.skel")));

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            QNodeCreatedChangeGenerator creationChangeGenerator(&skeleton);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData = qSharedPointerCast<QNodeCreatedChange<QSkeletonLoaderData>>(creationChanges.first());
            const QSkeletonLoaderData data = creationChangeData->data;

            QCOMPARE(skeleton.id(), creationChangeData->subjectId());
            QCOMPARE(skeleton.isEnabled(), true);
            QCOMPARE(skeleton.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(skeleton.metaObject(), creationChangeData->metaObject());
            QCOMPARE(skeleton.source(), data.source);
            QCOMPARE(skeleton.isCreateJointsEnabled(), data.createJoints);
        }

        // WHEN
        skeleton.setEnabled(false);

        {
            QNodeCreatedChangeGenerator creationChangeGenerator(&skeleton);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData = qSharedPointerCast<QNodeCreatedChange<QSkeletonLoaderData>>(creationChanges.first());
            const QSkeletonLoaderData data = creationChangeData->data;

            QCOMPARE(skeleton.id(), creationChangeData->subjectId());
            QCOMPARE(skeleton.isEnabled(), false);
            QCOMPARE(skeleton.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(skeleton.metaObject(), creationChangeData->metaObject());
            QCOMPARE(skeleton.source(), data.source);
            QCOMPARE(skeleton.isCreateJointsEnabled(), data.createJoints);
        }
    }

    void checkPropertyUpdates()
    {
        // GIVEN
        TestArbiter arbiter;
        QSkeletonLoader skeleton;
        arbiter.setArbiterOnNode(&skeleton);

        {
            // WHEN
            skeleton.setSource(QUrl(QStringLiteral("qrc:/hydralisk.skel")));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "source");
            QCOMPARE(change->type(), PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            skeleton.setSource(QStringLiteral("qrc:/hydralisk.skel"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }


        {
            // WHEN
            skeleton.setCreateJointsEnabled(true);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "createJointsEnabled");
            QCOMPARE(change->type(), PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            skeleton.setCreateJointsEnabled(true);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }

    void checkStatusPropertyUpdate()
    {
        // GIVEN
        qRegisterMetaType<Qt3DCore::QSkeletonLoader::Status>("Status");
        TestArbiter arbiter;
        arbiter.setArbiterOnNode(this);
        QSignalSpy spy(this, SIGNAL(statusChanged(Status)));
        const QSkeletonLoader::Status newStatus = QSkeletonLoader::Error;

        // THEN
        QVERIFY(spy.isValid());

        // WHEN
        QPropertyUpdatedChangePtr valueChange(new QPropertyUpdatedChange(QNodeId()));
        valueChange->setPropertyName("status");
        valueChange->setValue(QVariant::fromValue(newStatus));
        sceneChangeEvent(valueChange);

        // THEN
        QCOMPARE(spy.count(), 1);
        QCOMPARE(arbiter.events.size(), 0);
        QCOMPARE(status(), newStatus);

        // WHEN
        spy.clear();
        sceneChangeEvent(valueChange);

        // THEN
        QCOMPARE(spy.count(), 0);
        QCOMPARE(arbiter.events.size(), 0);
        QCOMPARE(status(), newStatus);
    }

    void checkRootJointPropertyUpdate()
    {
        // GIVEN
        qRegisterMetaType<Qt3DCore::QJoint*>();
        TestArbiter arbiter;
        arbiter.setArbiterOnNode(this);
        QSignalSpy spy(this, SIGNAL(rootJointChanged(Qt3DCore::QJoint*)));
        std::unique_ptr<QJoint> root(new QJoint());

        // THEN
        QVERIFY(spy.isValid());
        QVERIFY(rootJoint() == nullptr);

        // WHEN
        auto valueChange = QJointChangePtr::create(id());
        valueChange->setDeliveryFlags(Qt3DCore::QSceneChange::Nodes);
        valueChange->setPropertyName("rootJoint");
        valueChange->data = std::move(root);
        sceneChangeEvent(valueChange);

        // THEN
        QCOMPARE(spy.count(), 1);
        QCOMPARE(arbiter.events.size(), 1);
        QVERIFY(rootJoint() != nullptr);
    }
};

QTEST_MAIN(tst_QSkeletonLoader)

#include "tst_qskeletonloader.moc"
