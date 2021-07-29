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
#include <qbackendnodetester.h>
#include <Qt3DRender/private/raycaster_p.h>
#include <Qt3DRender/qpickevent.h>
#include <Qt3DRender/qraycaster.h>
#include <Qt3DCore/private/qbackendnode_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include "testpostmanarbiter.h"
#include "testrenderer.h"

class tst_RayCaster : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT
private Q_SLOTS:

    void checkPeerPropertyMirroring()
    {
        // GIVEN
        Qt3DRender::Render::RayCaster rayCaster;
        Qt3DRender::QRayCaster caster;
        caster.setRunMode(Qt3DRender::QRayCaster::Continuous);
        caster.setOrigin(QVector3D(1., 2., 3.));
        caster.setDirection(QVector3D(1., 0., 0.));
        caster.setLength(42.f);

        // WHEN
        simulateInitialization(&caster, &rayCaster);

        // THEN
        QVERIFY(!rayCaster.peerId().isNull());
        QCOMPARE(rayCaster.runMode(), Qt3DRender::QRayCaster::Continuous);
        QCOMPARE(rayCaster.origin(), QVector3D(1., 2., 3.));
        QCOMPARE(rayCaster.direction(), QVector3D(1., 0., 0.));
        QCOMPARE(rayCaster.length(), 42.f);
    }

    void checkInitialAndCleanedUpState()
    {
        // GIVEN
        Qt3DRender::Render::RayCaster rayCaster;

        // THEN
        QVERIFY(rayCaster.peerId().isNull());
        QCOMPARE(rayCaster.runMode(), Qt3DRender::QRayCaster::SingleShot);

        // GIVEN
        Qt3DRender::QRayCaster caster;
        caster.setRunMode(Qt3DRender::QRayCaster::Continuous);

        // WHEN
        simulateInitialization(&caster, &rayCaster);
        rayCaster.cleanup();

        // THEN
        QCOMPARE(rayCaster.runMode(), Qt3DRender::QRayCaster::SingleShot);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        TestRenderer renderer;
        {
            Qt3DRender::Render::RayCaster rayCaster;
            rayCaster.setRenderer(&renderer);

            // WHEN
            Qt3DCore::QPropertyUpdatedChangePtr updateChange(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
            updateChange->setValue(Qt3DRender::QRayCaster::Continuous);
            updateChange->setPropertyName("runMode");
            rayCaster.sceneChangeEvent(updateChange);

            // THEN
            QCOMPARE(rayCaster.runMode(), Qt3DRender::QRayCaster::Continuous);
            QVERIFY(renderer.dirtyBits() != 0);
        }
    }

    void checkBackendPropertyNotifications()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::Render::RayCaster rayCaster;
        Qt3DCore::QBackendNodePrivate::get(&rayCaster)->setArbiter(&arbiter);
        Qt3DRender::QAbstractRayCaster::Hits hits;

        // WHEN
        rayCaster.dispatchHits(hits);

        // THEN
        QCOMPARE(arbiter.events.count(), 2);
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "hits");
        QVERIFY(!rayCaster.isEnabled());

        arbiter.events.clear();
    }
};


QTEST_APPLESS_MAIN(tst_RayCaster)

#include "tst_raycaster.moc"
