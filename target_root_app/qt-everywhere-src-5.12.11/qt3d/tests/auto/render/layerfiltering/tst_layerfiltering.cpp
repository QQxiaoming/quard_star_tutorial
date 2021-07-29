/****************************************************************************
**
** Copyright (C) 2016 Paul Lemire
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
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/managers_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/filterlayerentityjob_p.h>
#include <Qt3DRender/private/updatetreeenabledjob_p.h>
#include <Qt3DRender/private/updateentityhierarchyjob_p.h>
#include <Qt3DRender/qlayer.h>
#include <Qt3DRender/qlayerfilter.h>
#include "testaspect.h"

class tst_LayerFiltering : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void checkInitialState()
    {
        // GIVEN
        Qt3DRender::Render::FilterLayerEntityJob filterJob;
        Qt3DRender::Render::UpdateEntityLayersJob updateEntityLayerJob;
        Qt3DRender::QLayer frontendLayer;

        // THEN
        QVERIFY(updateEntityLayerJob.manager() == nullptr);
        QCOMPARE(filterJob.hasLayerFilter(), false);
        QCOMPARE(filterJob.filteredEntities().size(), 0);
        QCOMPARE(filterJob.layerFilters().size(), 0);
        QCOMPARE(frontendLayer.recursive(), false);
    }

    void filterEntities_data()
    {
        QTest::addColumn<Qt3DCore::QEntity *>("entitySubtree");
        QTest::addColumn<Qt3DCore::QNodeIdVector>("layerFilterIds");
        QTest::addColumn<Qt3DCore::QNodeIdVector>("expectedSelectedEntities");


        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity2);
            Q_UNUSED(childEntity3);

            QTest::newRow("EntitiesNoLayerNoLayerFilterNoDiscardNoRecursive-ShouldSelectAll") << rootEntity
                                                                                              << Qt3DCore::QNodeIdVector()
                                                                                              << (Qt3DCore::QNodeIdVector()
                                                                                                  << rootEntity->id()
                                                                                                  << childEntity1->id()
                                                                                                  << childEntity2->id()
                                                                                                  << childEntity3->id()
                                                                                                  );
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);

            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity2);
            Q_UNUSED(childEntity3);

            QTest::newRow("EntityNoLayerWithLayerFilterWithNoFilter-ShouldSelectNone") << rootEntity
                                                                                       << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                       << Qt3DCore::QNodeIdVector();
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity2);
            Q_UNUSED(childEntity3);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->addLayer(layer);


            QTest::newRow("AcceptAny-NoLayerWithLayerFilterWithFilter-ShouldSelectNone") << rootEntity
                                                                                         << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                         << Qt3DCore::QNodeIdVector();
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);


            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            childEntity1->addComponent(layer);
            childEntity2->addComponent(layer);
            childEntity3->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->addLayer(layer);

            QTest::newRow("AcceptAny-LayerWithLayerFilterWithFilter-ShouldSelectAllButRoot") << rootEntity
                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                             << (Qt3DCore::QNodeIdVector() << childEntity1->id() << childEntity2->id() << childEntity3->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Q_UNUSED(childEntity1)

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            childEntity2->addComponent(layer2);
            childEntity3->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->addLayer(layer);
            layerFilter->addLayer(layer2);

            QTest::newRow("AcceptAny-LayerWithLayerFilterWithFilter-ShouldSelectChild2And3") << rootEntity
                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                             << (Qt3DCore::QNodeIdVector() << childEntity2->id() << childEntity3->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);


            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            childEntity1->addComponent(layer);
            childEntity2->addComponent(layer);
            childEntity3->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->addLayer(layer2);

            QTest::newRow("AcceptAny-LayerWithLayerFilterWithFilter-ShouldSelectNone") << rootEntity
                                                                                       << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                       << Qt3DCore::QNodeIdVector();
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            childEntity1->setEnabled(false);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            childEntity1->addComponent(layer);
            childEntity2->addComponent(layer);
            childEntity3->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->addLayer(layer);

            QTest::newRow("AcceptAny-LayerWithEntityDisabled-ShouldSelectOnlyEntityEnabled") << rootEntity
                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                             << (Qt3DCore::QNodeIdVector() << childEntity2->id() << childEntity3->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);
            rootEntity->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->addLayer(layer);

            QTest::newRow("AcceptAny-RecursiveLayerOnRoot-ShouldSelectAll") << rootEntity
                                                                            << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                            << (Qt3DCore::QNodeIdVector()
                                                                                << rootEntity->id()
                                                                                << childEntity1->id()
                                                                                << childEntity2->id()
                                                                                << childEntity3->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Q_UNUSED(childEntity1);
            Q_UNUSED(childEntity2);
            Q_UNUSED(childEntity3);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);
            rootEntity->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
            layerFilter->addLayer(layer);

            QTest::newRow("DiscardAny-RecursiveLayerLayerFilterDiscardOnRoot-ShouldSelectNone") << rootEntity
                                                                                                << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                                << (Qt3DCore::QNodeIdVector());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            rootEntity->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
            layerFilter->addLayer(layer);

            QTest::newRow("DiscardAny-LayerLayerFilterDiscardOnRoot-ShouldSelectAllButRoot") << rootEntity
                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                             << (Qt3DCore::QNodeIdVector()
                                                                                                 << childEntity1->id()
                                                                                                 << childEntity2->id()
                                                                                                 << childEntity3->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);
            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer2);

            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer2);
            childEntity3->addComponent(layer3);
            childEntity3->addComponent(layer);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
            layerFilter->addLayer(layer2);
            layerFilter->addLayer(layer3);

            QTest::newRow("DiscardAny-LayerLayerFilterDiscardOnRoot-ShouldSelectRoot") << rootEntity
                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                             << (Qt3DCore::QNodeIdVector()
                                                                                                 << rootEntity->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer3);
            childEntity1->addComponent(layer2);

            childEntity2->addComponent(layer);
            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer);
            childEntity3->addComponent(layer2);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
            layerFilter->addLayer(layer2);
            layerFilter->addLayer(layer3);

            QTest::newRow("AcceptAll-LayerFilterWith2LayersNonRecursive-ShouldSelectChild1") << rootEntity
                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                             << (Qt3DCore::QNodeIdVector()
                                                                                                 << childEntity1->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);

            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer3);
            childEntity1->addComponent(layer2);

            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer2);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
            layerFilter->addLayer(layer);
            layerFilter->addLayer(layer3);

            QTest::newRow("AcceptAll-LayerFilterWith2LayersRecursive-ShouldSelectChild12") << rootEntity
                                                                                           << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                           << (Qt3DCore::QNodeIdVector()
                                                                                               << childEntity1->id()
                                                                                               << childEntity2->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);

            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer3);
            childEntity1->addComponent(layer2);

            childEntity2->addComponent(layer);
            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer2);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
            layerFilter->addLayer(layer);
            layerFilter->addLayer(layer3);

            QTest::newRow("AcceptAll-LayerFilterWith2LayersRecursiveAndDirectReferenceToRecursiveLayer-ShouldSelectChild12") << rootEntity
                                                                                                                             << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                                                             << (Qt3DCore::QNodeIdVector()
                                                                                                                                 << childEntity1->id()
                                                                                                                                 << childEntity2->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer);
            childEntity1->addComponent(layer2);

            childEntity2->addComponent(layer);
            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer2);
            childEntity3->addComponent(layer3);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAllMatchingLayers);
            layerFilter->addLayer(layer2);
            layerFilter->addLayer(layer3);

            QTest::newRow("DiscardAll-LayerFilterWith2Layers-ShouldSelectRootAndChild12") << rootEntity
                                                                                          << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                          << (Qt3DCore::QNodeIdVector()
                                                                                              << rootEntity->id()
                                                                                              << childEntity1->id()
                                                                                              << childEntity2->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);

            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer2);

            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer3);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAllMatchingLayers);
            layerFilter->addLayer(layer);
            layerFilter->addLayer(layer3);

            QTest::newRow("DiscardAll-LayerFilterWith2LayersRecursive-ShouldSelectRootAndChild1") << rootEntity
                                                                                                  << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                                  << (Qt3DCore::QNodeIdVector()
                                                                                                      << rootEntity->id()
                                                                                                      << childEntity1->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);

            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer2);
            childEntity1->addComponent(layer);

            childEntity2->addComponent(layer3);

            childEntity3->addComponent(layer3);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAllMatchingLayers);
            layerFilter->addLayer(layer);
            layerFilter->addLayer(layer3);

            QTest::newRow("DiscardAll-LayerFilterWith2LayersRecursiveAndDirectReference-ShouldSelectRootAndChild1") << rootEntity
                                                                                                                    << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                                                    << (Qt3DCore::QNodeIdVector()
                                                                                                                        << rootEntity->id()
                                                                                                                        << childEntity1->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);

            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer4 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer5 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer2);
            childEntity1->addComponent(layer3);

            childEntity2->addComponent(layer2);
            childEntity2->addComponent(layer3);
            childEntity2->addComponent(layer4);
            childEntity2->addComponent(layer5);

            childEntity3->addComponent(layer2);
            childEntity3->addComponent(layer3);
            childEntity3->addComponent(layer5);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
            layerFilter->addLayer(layer);
            layerFilter->addLayer(layer2);
            layerFilter->addLayer(layer3);

            Qt3DRender::QLayerFilter *layerFilter2 = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter2->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
            layerFilter2->addLayer(layer4);
            layerFilter2->addLayer(layer5);

            QTest::newRow("NestedFiltering-SelectAllOfLayer123AndNoneOf45-ShouldChild1") << rootEntity
                                                                                         << (Qt3DCore::QNodeIdVector() << layerFilter->id() << layerFilter2->id())
                                                                                         << (Qt3DCore::QNodeIdVector()
                                                                                             << childEntity1->id());
        }

        {
            Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();
            Qt3DCore::QEntity *childEntity1 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity2 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity3 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity4 = new Qt3DCore::QEntity(rootEntity);
            Qt3DCore::QEntity *childEntity5 = new Qt3DCore::QEntity(rootEntity);

            Qt3DRender::QLayer *layer = new Qt3DRender::QLayer(rootEntity);
            layer->setRecursive(true);

            Qt3DRender::QLayer *layer2 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer3 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer4 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer5 = new Qt3DRender::QLayer(rootEntity);
            Qt3DRender::QLayer *layer6 = new Qt3DRender::QLayer(rootEntity);

            rootEntity->addComponent(layer);

            childEntity1->addComponent(layer2);
            childEntity1->addComponent(layer3);

            childEntity2->addComponent(layer2);
            childEntity2->addComponent(layer3);
            childEntity2->addComponent(layer4);
            childEntity2->addComponent(layer5);

            childEntity3->addComponent(layer2);
            childEntity3->addComponent(layer5);

            childEntity4->addComponent(layer2);
            childEntity4->addComponent(layer);
            childEntity4->addComponent(layer3);
            childEntity4->addComponent(layer6);

            childEntity5->addComponent(layer3);
            childEntity5->addComponent(layer4);
            childEntity5->addComponent(layer6);

            Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
            layerFilter->addLayer(layer5);
            layerFilter->addLayer(layer4);

            Qt3DRender::QLayerFilter *layerFilter2 = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter2->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
            layerFilter2->addLayer(layer2);
            layerFilter2->addLayer(layer3);

            Qt3DRender::QLayerFilter *layerFilter3 = new Qt3DRender::QLayerFilter(rootEntity);
            layerFilter3->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
            layerFilter3->addLayer(layer);
            layerFilter3->addLayer(layer6);

            QTest::newRow("NestedFiltering-SelectAllNoneOfAnyLayer45AndAnyOf23AndAllOf16-ShouldSelectChild4-Step1") << rootEntity
                                                                                                                    << (Qt3DCore::QNodeIdVector() << layerFilter->id())
                                                                                                                    << (Qt3DCore::QNodeIdVector()
                                                                                                                        << rootEntity->id()
                                                                                                                        << childEntity1->id()
                                                                                                                        << childEntity4->id()
                                                                                                                        );

            QTest::newRow("NestedFiltering-SelectAllNoneOfAnyLayer45AndAnyOf23AndAllOf16-ShouldSelectChild4-Step2") << rootEntity
                                                                                                                    << (Qt3DCore::QNodeIdVector() << layerFilter->id() << layerFilter2->id())
                                                                                                                    << (Qt3DCore::QNodeIdVector()
                                                                                                                        << childEntity1->id()
                                                                                                                        << childEntity4->id());

            QTest::newRow("NestedFiltering-SelectAllNoneOfAnyLayer45AndAnyOf23AndAllOf16-ShouldSelectChild4-Step3") << rootEntity
                                                                                                                    << (Qt3DCore::QNodeIdVector() << layerFilter->id() << layerFilter2->id() << layerFilter3->id())
                                                                                                                    << (Qt3DCore::QNodeIdVector()
                                                                                                                        << childEntity4->id());
        }
    }

    void filterEntities()
    {
        //QSKIP("Skipping until TestAspect can be registered");
        QFETCH(Qt3DCore::QEntity *, entitySubtree);
        QFETCH(Qt3DCore::QNodeIdVector, layerFilterIds);
        QFETCH(Qt3DCore::QNodeIdVector, expectedSelectedEntities);


        // GIVEN
        QScopedPointer<Qt3DRender::TestAspect> aspect(new Qt3DRender::TestAspect(entitySubtree));

        // WHEN
        Qt3DRender::Render::Entity *backendRoot = aspect->nodeManagers()->renderNodesManager()->getOrCreateResource(entitySubtree->id());

        Qt3DRender::Render::UpdateEntityHierarchyJob updateEntitiesJob;
        updateEntitiesJob.setManager(aspect->nodeManagers());
        updateEntitiesJob.run();

        Qt3DRender::Render::UpdateTreeEnabledJob updateTreeEnabledJob;
        updateTreeEnabledJob.setRoot(backendRoot);
        updateTreeEnabledJob.run();

        // WHEN
        Qt3DRender::Render::UpdateEntityLayersJob updateLayerEntityJob;
        updateLayerEntityJob.setManager(aspect->nodeManagers());
        updateLayerEntityJob.run();

        Qt3DRender::Render::FilterLayerEntityJob filterJob;
        filterJob.setLayerFilters(layerFilterIds);
        filterJob.setManager(aspect->nodeManagers());
        filterJob.run();

        // THEN
        const QVector<Qt3DRender::Render::Entity *> filterEntities = filterJob.filteredEntities();
        QCOMPARE(filterEntities.size(), expectedSelectedEntities.size());
        for (auto i = 0, m = expectedSelectedEntities.size(); i < m; ++i)
            QCOMPARE(filterEntities.at(i)->peerId(), expectedSelectedEntities.at(i));
    }
};

QTEST_MAIN(tst_LayerFiltering)

#include "tst_layerfiltering.moc"
