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
#include <Qt3DAnimation/qcallbackmapping.h>
#include <Qt3DAnimation/private/qcallbackmapping_p.h>
#include <Qt3DAnimation/private/qchannelmappingcreatedchange_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <QObject>
#include <QSignalSpy>
#include <testpostmanarbiter.h>

class DummyCallback : public Qt3DAnimation::QAnimationCallback
{
public:
    void valueChanged(const QVariant &) override { }
};

class tst_QCallbackMapping : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkDefaultConstruction()
    {
        // GIVEN
        Qt3DAnimation::QCallbackMapping mapping;

        // THEN
        QCOMPARE(mapping.channelName(), QString());
        QCOMPARE(mapping.callback(), static_cast<Qt3DAnimation::QAnimationCallback *>(nullptr));
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DAnimation::QCallbackMapping mapping;

        {
            // WHEN
            QSignalSpy spy(&mapping, SIGNAL(channelNameChanged(QString)));
            const QString newValue(QStringLiteral("Rotation"));
            mapping.setChannelName(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(mapping.channelName(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            mapping.setChannelName(newValue);

            // THEN
            QCOMPARE(mapping.channelName(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            auto newValue = new DummyCallback();
            mapping.setCallback(QVariant::Quaternion, newValue);

            // THEN - no signals for callback
            QCOMPARE(mapping.callback(), newValue);
        }
    }

    void checkCreationData()
    {
        // GIVEN
        Qt3DAnimation::QCallbackMapping mapping;
        auto callback = new DummyCallback();

        mapping.setChannelName(QStringLiteral("Location"));
        mapping.setCallback(QVariant::Vector3D, callback);

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&mapping);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData = qSharedPointerCast<Qt3DAnimation::QChannelMappingCreatedChange<Qt3DAnimation::QCallbackMappingData>>(creationChanges.first());
            const Qt3DAnimation::QCallbackMappingData data = creationChangeData->data;

            QCOMPARE(mapping.id(), creationChangeData->subjectId());
            QCOMPARE(mapping.isEnabled(), true);
            QCOMPARE(mapping.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(mapping.metaObject(), creationChangeData->metaObject());
            QCOMPARE(creationChangeData->type(), Qt3DAnimation::QChannelMappingCreatedChangeBase::CallbackMapping);
            QCOMPARE(mapping.channelName(), data.channelName);
            QCOMPARE(mapping.callback(), data.callback);
            QCOMPARE(int(QVariant::Vector3D), data.type);
        }

        // WHEN
        mapping.setEnabled(false);

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&mapping);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 1);

            const auto creationChangeData = qSharedPointerCast<Qt3DAnimation::QChannelMappingCreatedChange<Qt3DAnimation::QCallbackMappingData>>(creationChanges.first());
            const Qt3DAnimation::QCallbackMappingData data = creationChangeData->data;

            QCOMPARE(mapping.id(), creationChangeData->subjectId());
            QCOMPARE(mapping.isEnabled(), false);
            QCOMPARE(mapping.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(mapping.metaObject(), creationChangeData->metaObject());
            QCOMPARE(creationChangeData->type(), Qt3DAnimation::QChannelMappingCreatedChangeBase::CallbackMapping);
            QCOMPARE(mapping.channelName(), data.channelName);
            QCOMPARE(mapping.callback(), data.callback);
            QCOMPARE(QVariant::Vector3D, data.type);
        }
    }

    void checkPropertyUpdateChanges()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DAnimation::QCallbackMapping mapping;
        arbiter.setArbiterOnNode(&mapping);

        {
            // WHEN
            mapping.setChannelName(QStringLiteral("Scale"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "channelName");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toString(), mapping.channelName());

            arbiter.events.clear();

            // WHEN
            mapping.setChannelName(QStringLiteral("Scale"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

        {
            // WHEN
            auto callback = new DummyCallback();
            mapping.setCallback(QVariant::Vector3D, callback, Qt3DAnimation::QAnimationCallback::OnThreadPool);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 3);
            auto change = arbiter.events.at(0).staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "type");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toInt(), int(QVariant::Vector3D));

            change = arbiter.events.at(1).staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "callback");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(reinterpret_cast<DummyCallback *>(change->value().value<void *>()), callback);

            change = arbiter.events.at(2).staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "callbackFlags");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toInt(), static_cast<int>(Qt3DAnimation::QAnimationCallback::OnThreadPool));

            arbiter.events.clear();

            // WHEN
            mapping.setCallback(QVariant::Vector3D, callback, Qt3DAnimation::QAnimationCallback::OnThreadPool);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }
};

QTEST_MAIN(tst_QCallbackMapping)

#include "tst_qcallbackmapping.moc"
