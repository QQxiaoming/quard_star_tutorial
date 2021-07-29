/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
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
#include <QtTest/QSignalSpy>
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>
#include <Qt3DRender/QRayCaster>
#include <Qt3DRender/QLayer>

#include "testpostmanarbiter.h"

class MyRayCaster : public Qt3DRender::QRayCaster
{
    Q_OBJECT
public:
    MyRayCaster(Qt3DCore::QNode *parent = nullptr)
        : Qt3DRender::QRayCaster(parent)
    {
        qRegisterMetaType<Qt3DRender::QAbstractRayCaster::Hits>("Hits");
    }

    void sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change) final
    {
        Qt3DRender::QRayCaster::sceneChangeEvent(change);
    }

private:
    friend class tst_RayCaster;
};

// We need to call QNode::clone which is protected
// So we sublcass QNode instead of QObject
class tst_QRayCaster : public Qt3DCore::QNode
{
    Q_OBJECT
public:
    tst_QRayCaster()
    {
    }

    ~tst_QRayCaster()
    {
        QMetaObject::invokeMethod(this, "_q_cleanup", Qt::DirectConnection);
    }

private Q_SLOTS:

    void checkState()
    {
        // GIVEN
        QScopedPointer<Qt3DRender::QRayCaster> rayCaster(new Qt3DRender::QRayCaster());

        QVERIFY(!rayCaster->isEnabled());
        QVERIFY(rayCaster->direction().length() > 0.);
        QCOMPARE(rayCaster->runMode(), Qt3DRender::QAbstractRayCaster::SingleShot);

        // WHEN
        rayCaster->trigger();

        // THEN
        QVERIFY(rayCaster->isEnabled());

        // WHEN
        rayCaster->setEnabled(false);
        rayCaster->trigger(QVector3D(1., 2., 3.), QVector3D(1., 0., 0.), 10.f);

        // THEN
        QVERIFY(rayCaster->isEnabled());
        QCOMPARE(rayCaster->origin(), QVector3D(1., 2., 3.));
        QCOMPARE(rayCaster->direction(), QVector3D(1., 0., 0.));
        QCOMPARE(rayCaster->length(), 10.f);
    }

    void checkPropertyUpdates()
    {
        // GIVEN
        TestArbiter arbiter;
        QScopedPointer<Qt3DRender::QRayCaster> rayCaster(new Qt3DRender::QRayCaster());
        arbiter.setArbiterOnNode(rayCaster.data());

        // WHEN
        rayCaster->setOrigin({1., 1., 1.});
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.last().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "origin");
        QCOMPARE(change->value().value<QVector3D>(), QVector3D(1., 1., 1.));
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        auto layer = new Qt3DRender::QLayer(rayCaster.data());
        QCoreApplication::processEvents();
        rayCaster->addLayer(layer);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        auto addChange = arbiter.events.first().staticCast<Qt3DCore::QPropertyNodeAddedChange>();
        QCOMPARE(addChange->propertyName(), "layer");
        QCOMPARE(addChange->subjectId(), rayCaster->id());
        QCOMPARE(addChange->addedNodeId(), rayCaster->layers().at(0)->id());
        QCOMPARE(addChange->type(), Qt3DCore::PropertyValueAdded);

        arbiter.events.clear();

        // WHEN
        layer = new Qt3DRender::QLayer(rayCaster.data());
        QCoreApplication::processEvents();
        rayCaster->addLayer(layer);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        addChange = arbiter.events.first().staticCast<Qt3DCore::QPropertyNodeAddedChange>();
        QCOMPARE(addChange->propertyName(), "layer");
        QCOMPARE(addChange->subjectId(), rayCaster->id());
        QCOMPARE(addChange->addedNodeId(), rayCaster->layers().at(1)->id());
        QCOMPARE(addChange->type(), Qt3DCore::PropertyValueAdded);

        arbiter.events.clear();

        // WHEN
        layer = rayCaster->layers().at(0);
        rayCaster->removeLayer(layer);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        auto removeChange = arbiter.events.first().staticCast<Qt3DCore::QPropertyNodeRemovedChange>();
        QCOMPARE(removeChange->propertyName(), "layer");
        QCOMPARE(removeChange->subjectId(), rayCaster->id());
        QCOMPARE(removeChange->removedNodeId(), layer->id());
        QCOMPARE(removeChange->type(), Qt3DCore::PropertyValueRemoved);

        arbiter.events.clear();
    }

    void checkLayerBookkeeping()
    {
        // GIVEN
        QScopedPointer<Qt3DRender::QRayCaster> rayCaster(new Qt3DRender::QRayCaster);
        {
            // WHEN
            Qt3DRender::QLayer layer;
            rayCaster->addLayer(&layer);

            // THEN
            QCOMPARE(layer.parent(), rayCaster.data());
            QCOMPARE(rayCaster->layers().size(), 1);
        }
        // THEN (Should not crash and parameter be unset)
        QVERIFY(rayCaster->layers().empty());

        {
            // WHEN
            Qt3DRender::QRayCaster someOtherRayCaster;
            QScopedPointer<Qt3DRender::QLayer> layer(new Qt3DRender::QLayer(&someOtherRayCaster));
            rayCaster->addLayer(layer.data());

            // THEN
            QCOMPARE(layer->parent(), &someOtherRayCaster);
            QCOMPARE(rayCaster->layers().size(), 1);

            // WHEN
            rayCaster.reset();
            layer.reset();

            // THEN Should not crash when the layer is destroyed (tests for failed removal of destruction helper)
        }
    }

    void checkBackendUpdates_data()
    {
        QTest::addColumn<QByteArray>("signalPrototype");
        QTest::addColumn<QByteArray>("propertyName");

        QTest::newRow("hits")
                << QByteArray(SIGNAL(hitsChanged(const Hits &)))
                << QByteArrayLiteral("hits");
    }

    void checkBackendUpdates()
    {
        // GIVEN
        QFETCH(QByteArray, signalPrototype);
        QFETCH(QByteArray, propertyName);
        QScopedPointer<MyRayCaster> rayCaster(new MyRayCaster());
        QSignalSpy spy(rayCaster.data(), signalPrototype.constData());
        Qt3DRender::QRayCaster::Hits hits;

        // WHEN
        // Create Backend Change and distribute it to frontend node
        Qt3DCore::QPropertyUpdatedChangePtr e(new Qt3DCore::QPropertyUpdatedChange(rayCaster->id()));
        e->setPropertyName(propertyName.constData());
        QVariant v;
        v.setValue<Qt3DRender::QRayCaster::Hits>(hits);
        e->setValue(v);
        rayCaster->sceneChangeEvent(e);

        // THEN
        // Check that the QRayCaster triggers the expected signal
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(tst_QRayCaster)

#include "tst_qraycaster.moc"
