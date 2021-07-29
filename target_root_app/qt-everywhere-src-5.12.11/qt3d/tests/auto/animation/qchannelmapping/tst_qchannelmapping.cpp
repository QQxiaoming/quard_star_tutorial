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
#include <Qt3DAnimation/qchannelmapping.h>
#include <Qt3DAnimation/private/qabstractchannelmapping_p.h>
#include <Qt3DAnimation/private/qchannelmapping_p.h>
#include <Qt3DAnimation/private/qchannelmappingcreatedchange_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <QObject>
#include <QSignalSpy>
#include <QQuaternion>
#include <testpostmanarbiter.h>

class tst_QTargetEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
    Q_PROPERTY(QQuaternion rotation MEMBER m_rotation NOTIFY rotationChanged)
    Q_PROPERTY(QVector3D translation MEMBER m_translation NOTIFY translationChanged)
    Q_PROPERTY(QVector3D scale MEMBER m_scale NOTIFY scaleChanged)
    Q_PROPERTY(float floatProperty MEMBER m_floatProperty NOTIFY floatPropertyChanged)
    Q_PROPERTY(QVector2D vec2Property MEMBER m_vec2Property NOTIFY vec2PropertyChanged)
    Q_PROPERTY(QVector3D vec3Property MEMBER m_vec3Property NOTIFY vec3PropertyChanged)
    Q_PROPERTY(QVector4D vec4Property MEMBER m_vec4Property NOTIFY vec4PropertyChanged)
    Q_PROPERTY(QQuaternion quaternionProperty MEMBER m_quaternionProperty NOTIFY quaternionPropertyChanged)
    Q_PROPERTY(QVariantList listProperty MEMBER m_listProperty NOTIFY listPropertyChanged)
    Q_PROPERTY(QVector<float> vecProperty MEMBER m_vecProperty NOTIFY vecPropertyChanged)

signals:
    void rotationChanged();
    void translationChanged();
    void scaleChanged();
    void floatPropertyChanged();
    void vec2PropertyChanged();
    void vec3PropertyChanged();
    void vec4PropertyChanged();
    void quaternionPropertyChanged();
    void listPropertyChanged();
    void vecPropertyChanged();

private:
    QQuaternion m_rotation;
    QVector3D m_translation;
    QVector3D m_scale;
    float m_floatProperty;
    QVector2D m_vec2Property;
    QVector3D m_vec3Property;
    QVector4D m_vec4Property;
    QQuaternion m_quaternionProperty;
    QVariantList m_listProperty;
    QVector<float> m_vecProperty;
};


class tst_QChannelMapping : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void checkDefaultConstruction()
    {
        // GIVEN
        Qt3DAnimation::QChannelMapping mapping;

        // THEN
        QCOMPARE(mapping.channelName(), QString());
        QCOMPARE(mapping.target(), static_cast<Qt3DCore::QNode *>(nullptr));
        QCOMPARE(mapping.property(), QString());

        const Qt3DAnimation::QChannelMappingPrivate *d =
            static_cast<const Qt3DAnimation::QChannelMappingPrivate *>(
                Qt3DAnimation::QChannelMappingPrivate::get(&mapping));

        QCOMPARE(d->m_type, static_cast<int>(QVariant::Invalid));
        QCOMPARE(d->m_componentCount, 0);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DAnimation::QChannelMapping mapping;

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
            QSignalSpy spy(&mapping, SIGNAL(targetChanged(Qt3DCore::QNode*)));
            auto newValue = new Qt3DCore::QEntity();
            mapping.setTarget(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(mapping.target(), newValue);
            QCOMPARE(newValue->parent(), &mapping);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            mapping.setTarget(newValue);

            // THEN
            QCOMPARE(mapping.target(), newValue);
            QCOMPARE(spy.count(), 0);
        }

        {
            // WHEN
            QSignalSpy spy(&mapping, SIGNAL(propertyChanged(QString)));
            const QString newValue(QStringLiteral("rotation"));
            mapping.setProperty(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(mapping.property(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            mapping.setProperty(newValue);

            // THEN
            QCOMPARE(mapping.property(), newValue);
            QCOMPARE(spy.count(), 0);
        }
    }

    void checkCreationData()
    {
        // GIVEN
        Qt3DAnimation::QChannelMapping mapping;
        auto target = new tst_QTargetEntity;

        mapping.setChannelName(QStringLiteral("Location"));
        mapping.setTarget(target);
        mapping.setProperty(QStringLiteral("translation"));

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&mapping);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // 1 for mapping, 1 for target

            const auto creationChangeData = qSharedPointerCast<Qt3DAnimation::QChannelMappingCreatedChange<Qt3DAnimation::QChannelMappingData>>(creationChanges.first());
            const Qt3DAnimation::QChannelMappingData data = creationChangeData->data;

            QCOMPARE(mapping.id(), creationChangeData->subjectId());
            QCOMPARE(mapping.isEnabled(), true);
            QCOMPARE(mapping.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(mapping.metaObject(), creationChangeData->metaObject());
            QCOMPARE(creationChangeData->type(), Qt3DAnimation::QChannelMappingCreatedChangeBase::ChannelMapping);
            QCOMPARE(mapping.channelName(), data.channelName);
            QCOMPARE(mapping.target()->id(), data.targetId);
            QVERIFY(qstrcmp(mapping.property().toLatin1().constData(), data.propertyName) == 0);
            QCOMPARE(data.type, static_cast<int>(QVariant::Vector3D));
            QCOMPARE(data.componentCount, 3);
        }

        // WHEN
        mapping.setEnabled(false);

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&mapping);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // 1 for mapping, 1 for target

            const auto creationChangeData = qSharedPointerCast<Qt3DAnimation::QChannelMappingCreatedChange<Qt3DAnimation::QChannelMappingData>>(creationChanges.first());
            const Qt3DAnimation::QChannelMappingData data = creationChangeData->data;

            QCOMPARE(mapping.id(), creationChangeData->subjectId());
            QCOMPARE(mapping.isEnabled(), false);
            QCOMPARE(mapping.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(mapping.metaObject(), creationChangeData->metaObject());
            QCOMPARE(creationChangeData->type(), Qt3DAnimation::QChannelMappingCreatedChangeBase::ChannelMapping);
            QCOMPARE(mapping.channelName(), data.channelName);
            QCOMPARE(mapping.target()->id(), data.targetId);
            QVERIFY(qstrcmp(mapping.property().toLatin1().constData(), data.propertyName) == 0);
            QCOMPARE(data.type, static_cast<int>(QVariant::Vector3D));
            QCOMPARE(data.componentCount, 3);
        }
    }

    void checkPropertyUpdateChanges()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DAnimation::QChannelMapping mapping;
        QScopedPointer<Qt3DCore::QEntity> target(new tst_QTargetEntity());
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
            mapping.setTarget(target.data());
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "target");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), mapping.target()->id());

            arbiter.events.clear();

            // WHEN
            mapping.setTarget(target.data());
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

        {
            // WHEN
            target->setProperty("scale", QVector3D(1.0f, 0.0f, 0.0f));
            mapping.setProperty(QStringLiteral("scale"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 3);

            auto change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "type");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toInt(), static_cast<int>(QVariant::Vector3D));

            change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "componentCount");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toInt(), 3);

            change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "propertyName");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QVERIFY(qstrcmp(reinterpret_cast<const char *>(change->value().value<void *>()), "scale") == 0);

            arbiter.events.clear();

            // WHEN
            mapping.setProperty(QStringLiteral("scale"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }

    void checkPropertyUpdateNameTypeAndComponentCount_data()
    {
        QTest::addColumn<QByteArray>("propertyName");
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<int>("expectedType");
        QTest::addColumn<int>("expectedComponentCount");

        QTest::newRow("float") << QByteArrayLiteral("floatProperty") << QVariant(1.0f) << static_cast<int>(QMetaType::Float) << 1;
        QTest::newRow("vec2") << QByteArrayLiteral("vec2Property") << QVariant(QVector2D(1.0f, 1.0f)) << static_cast<int>(QVariant::Vector2D) << 2;
        QTest::newRow("vec3") << QByteArrayLiteral("vec3Property") << QVariant(QVector3D(1.0f, 1.0f, 1.0f)) << static_cast<int>(QVariant::Vector3D) << 3;
        QTest::newRow("vec4") << QByteArrayLiteral("vec4Property") << QVariant(QVector4D(1.0f, 1.0f, 1.0f, 1.0f)) << static_cast<int>(QVariant::Vector4D) << 4;
        QTest::newRow("quaternion") << QByteArrayLiteral("quaternionProperty") << QVariant(QQuaternion(1.0f, 1.0f, 1.0f, 1.0f)) << static_cast<int>(QVariant::Quaternion) << 4;

        QVariantList list = QVariantList() << QVariant(1.0f) << QVariant(1.0) << QVariant(1.0f) << QVariant(1.0f) << QVariant(1.0f);
        QTest::newRow("variantlist") << QByteArrayLiteral("listProperty") << QVariant::fromValue(list) << static_cast<int>(QVariant::List) << 5;

        QVector<float> vec(8);
        QTest::newRow("vector") << QByteArrayLiteral("vecProperty") << QVariant::fromValue(vec) << qMetaTypeId<decltype(vec)>() << 8;
    }

    void checkPropertyUpdateNameTypeAndComponentCount()
    {
        // GIVEN
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, value);
        QFETCH(int, expectedType);
        QFETCH(int, expectedComponentCount);

        TestArbiter arbiter;
        Qt3DAnimation::QChannelMapping mapping;
        QScopedPointer<Qt3DCore::QEntity> target(new tst_QTargetEntity());
        mapping.setTarget(target.data());
        arbiter.setArbiterOnNode(&mapping);

        {
            // WHEN
            target->setProperty(propertyName.constData(), value);
            mapping.setProperty(QString::fromLatin1(propertyName));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 4);

            // Automatic notification change when property is updated
            auto change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), propertyName.constData());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value(), value);

            change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "type");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toInt(), expectedType);

            change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "componentCount");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QCOMPARE(change->value().toInt(), expectedComponentCount);

            change = arbiter.events.takeFirst().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "propertyName");
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
            QVERIFY(qstrcmp(reinterpret_cast<const char *>(change->value().value<void *>()), propertyName.constData()) == 0);

            arbiter.events.clear();

            // WHEN
            mapping.setProperty(QString::fromLatin1(propertyName));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }

};

QTEST_MAIN(tst_QChannelMapping)

#include "tst_qchannelmapping.moc"
