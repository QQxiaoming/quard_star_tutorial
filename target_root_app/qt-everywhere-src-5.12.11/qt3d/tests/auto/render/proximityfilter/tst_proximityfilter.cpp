/****************************************************************************
**
** Copyright (C) 2017 Paul Lemire <paul.lemire350@gmail.com>
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
#include <Qt3DRender/qproximityfilter.h>
#include <Qt3DRender/private/qproximityfilter_p.h>
#include <Qt3DRender/private/proximityfilter_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include "qbackendnodetester.h"
#include "testrenderer.h"

class tst_ProximityFilter : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:

    void checkInitialState()
    {
        // GIVEN
        Qt3DRender::Render::ProximityFilter backendProximityFilter;

        // THEN
        QCOMPARE(backendProximityFilter.isEnabled(), false);
        QVERIFY(backendProximityFilter.peerId().isNull());
        QCOMPARE(backendProximityFilter.distanceThreshold(), 0.0f);
        QCOMPARE(backendProximityFilter.entityId(), Qt3DCore::QNodeId());
    }

    void checkInitializeFromPeer()
    {
        // GIVEN
        Qt3DRender::QProximityFilter proximityFilter;
        Qt3DCore::QEntity entity;
        proximityFilter.setDistanceThreshold(1340.0f);
        proximityFilter.setEntity(&entity);

        {
            // WHEN
            Qt3DRender::Render::ProximityFilter backendProximityFilter;
            simulateInitialization(&proximityFilter, &backendProximityFilter);

            // THEN
            QCOMPARE(backendProximityFilter.isEnabled(), true);
            QCOMPARE(backendProximityFilter.peerId(), proximityFilter.id());
            QCOMPARE(backendProximityFilter.distanceThreshold(), 1340.f);
            QCOMPARE(backendProximityFilter.entityId(), entity.id());
        }
        {
            // WHEN
            Qt3DRender::Render::ProximityFilter backendProximityFilter;
            proximityFilter.setEnabled(false);
            simulateInitialization(&proximityFilter, &backendProximityFilter);

            // THEN
            QCOMPARE(backendProximityFilter.peerId(), proximityFilter.id());
            QCOMPARE(backendProximityFilter.isEnabled(), false);
        }
    }

    void checkSceneChangeEvents()
    {
        // GIVEN
        Qt3DRender::Render::ProximityFilter backendProximityFilter;
        TestRenderer renderer;
        backendProximityFilter.setRenderer(&renderer);

        {
             // WHEN
             const bool newValue = false;
             const auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(Qt3DCore::QNodeId());
             change->setPropertyName("enabled");
             change->setValue(newValue);
             backendProximityFilter.sceneChangeEvent(change);

             // THEN
            QCOMPARE(backendProximityFilter.isEnabled(), newValue);
        }
        {
             // WHEN
             const float newValue = 383.0f;
             const auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(Qt3DCore::QNodeId());
             change->setPropertyName("distanceThreshold");
             change->setValue(QVariant::fromValue(newValue));
             backendProximityFilter.sceneChangeEvent(change);

             // THEN
            QCOMPARE(backendProximityFilter.distanceThreshold(), newValue);
        }
        {
             // WHEN
             const Qt3DCore::QNodeId newValue = Qt3DCore::QNodeId::createId();
             const auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(Qt3DCore::QNodeId());
             change->setPropertyName("entity");
             change->setValue(QVariant::fromValue(newValue));
             backendProximityFilter.sceneChangeEvent(change);

             // THEN
            QCOMPARE(backendProximityFilter.entityId(), newValue);
        }
    }

};

QTEST_MAIN(tst_ProximityFilter)

#include "tst_proximityfilter.moc"
