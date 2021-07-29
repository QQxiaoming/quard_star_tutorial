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
#include <qbackendnodetester.h>
#include <private/renderview_p.h>
#include <private/qframeallocator_p.h>
#include <private/qframeallocator_p_p.h>
#include <private/memorybarrier_p.h>
#include <private/renderviewjobutils_p.h>
#include <private/rendercommand_p.h>
#include <testpostmanarbiter.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

namespace {

void compareShaderParameterPacks(const ShaderParameterPack &t1,
                                 const ShaderParameterPack &t2)
{
    const PackUniformHash hash1 = t1.uniforms();
    const PackUniformHash hash2 = t2.uniforms();

    QCOMPARE(hash1.size(), hash2.size());

    auto it = hash1.constBegin();
    const auto end = hash1.constEnd();

    while (it != end) {
        const auto h2It = hash2.find(it.key());
        QVERIFY(h2It != hash2.cend());
        QCOMPARE(it.value(), h2It.value());
        ++it;
    }
}

} // anonymous

class tst_RenderViews : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT
private Q_SLOTS:

    void checkRenderViewSizeFitsWithAllocator()
    {
        QSKIP("Allocated Disabled");
        QVERIFY(sizeof(RenderView) <= 192);
        QVERIFY(sizeof(RenderView::InnerData) <= 192);
    }

    void checkRenderViewInitialState()
    {
        // GIVEN
        RenderView renderView;

        // THEN
        QCOMPARE(renderView.memoryBarrier(), QMemoryBarrier::None);
    }

    void checkMemoryBarrierInitialization()
    {
        // GIVEN
        RenderView renderView;

        // THEN
        QCOMPARE(renderView.memoryBarrier(), QMemoryBarrier::None);

        // WHEN
        const QMemoryBarrier::Operations barriers(QMemoryBarrier::BufferUpdate|QMemoryBarrier::ShaderImageAccess);
        renderView.setMemoryBarrier(barriers);

        // THEN
        QCOMPARE(renderView.memoryBarrier(), barriers);
    }

    void checkSetRenderViewConfig()
    {
        {
            // GIVEN
            const QMemoryBarrier::Operations barriers(QMemoryBarrier::AtomicCounter|QMemoryBarrier::ShaderStorage);
            Qt3DRender::QMemoryBarrier frontendBarrier;
            FrameGraphManager frameGraphManager;
            MemoryBarrier backendBarrier;
            RenderView renderView;
            // setRenderViewConfigFromFrameGraphLeafNode assumes node has a manager
            backendBarrier.setFrameGraphManager(&frameGraphManager);

            // WHEN
            frontendBarrier.setWaitOperations(barriers);
            simulateInitialization(&frontendBarrier, &backendBarrier);

            // THEN
            QCOMPARE(renderView.memoryBarrier(), QMemoryBarrier::None);
            QCOMPARE(backendBarrier.waitOperations(), barriers);

            // WHEN
            Qt3DRender::Render::setRenderViewConfigFromFrameGraphLeafNode(&renderView, &backendBarrier);

            // THEN
            QCOMPARE(backendBarrier.waitOperations(), renderView.memoryBarrier());
        }
        // TO DO: Complete tests for other framegraph node types
    }

    void checkRenderCommandBackToFrontSorting()
    {
        // GIVEN
        RenderView renderView;
        QVector<RenderCommand *> rawCommands;
        QVector<QSortPolicy::SortType> sortTypes;

        sortTypes.push_back(QSortPolicy::BackToFront);

        for (int i = 0; i < 200; ++i) {
            RenderCommand *c = new RenderCommand();
            c->m_depth = float(i);
            rawCommands.push_back(c);
        }

        // WHEN
        renderView.addSortType(sortTypes);
        renderView.setCommands(rawCommands);
        renderView.sort();

        // THEN
        const QVector<RenderCommand *> sortedCommands = renderView.commands();
        QCOMPARE(rawCommands.size(), sortedCommands.size());
        for (int j = 1; j < sortedCommands.size(); ++j)
            QVERIFY(sortedCommands.at(j - 1)->m_depth > sortedCommands.at(j)->m_depth);

        // RenderCommands are deleted by RenderView dtor
    }

    void checkRenderCommandMaterialSorting()
    {
        // GIVEN
        RenderView renderView;
        QVector<RenderCommand *> rawCommands;
        QVector<QSortPolicy::SortType> sortTypes;

        sortTypes.push_back(QSortPolicy::Material);

        ProgramDNA dnas[5] = {
            ProgramDNA(250),
            ProgramDNA(500),
            ProgramDNA(1000),
            ProgramDNA(1500),
            ProgramDNA(2000),
        };

        for (int i = 0; i < 20; ++i) {
            RenderCommand *c = new RenderCommand();
            c->m_shaderDna = dnas[i % 5];
            rawCommands.push_back(c);
        }

        // WHEN
        renderView.addSortType(sortTypes);
        renderView.setCommands(rawCommands);
        renderView.sort();

        // THEN
        const QVector<RenderCommand *> sortedCommands = renderView.commands();
        QCOMPARE(rawCommands.size(), sortedCommands.size());
        ProgramDNA targetDNA;

        for (int j = 0; j < sortedCommands.size(); ++j) {

            if (j % 4 == 0) {
                targetDNA = sortedCommands.at(j)->m_shaderDna;
                if (j > 0)
                    QVERIFY(targetDNA != sortedCommands.at(j - 1)->m_shaderDna);
            }
            QCOMPARE(targetDNA, sortedCommands.at(j)->m_shaderDna);
        }

        // RenderCommands are deleted by RenderView dtor
    }

    void checkRenderViewUniformMinification_data()
    {
        QTest::addColumn<QVector<ProgramDNA>>("programDNAs");
        QTest::addColumn<QVector<ShaderParameterPack>>("rawParameters");
        QTest::addColumn<QVector<ShaderParameterPack>>("expectedMinimizedParameters");

        Qt3DCore::QNodeId fakeTextureNodeId = Qt3DCore::QNodeId::createId();

        ShaderParameterPack pack1;
        pack1.setUniform(1, UniformValue(883));
        pack1.setUniform(2, UniformValue(1584.0f));
        pack1.setTexture(3, 0, fakeTextureNodeId);

        ShaderParameterPack minifiedPack1;

        QTest::newRow("NoMinification")
                << (QVector<ProgramDNA>() << ProgramDNA(883) << ProgramDNA(1584))
                << (QVector<ShaderParameterPack>() << pack1 << pack1)
                << (QVector<ShaderParameterPack>() << pack1 << pack1);

        QTest::newRow("SingleShaderMinified")
                << (QVector<ProgramDNA>() << ProgramDNA(883) << ProgramDNA(883) << ProgramDNA(883))
                << (QVector<ShaderParameterPack>() << pack1 << pack1 << pack1)
                << (QVector<ShaderParameterPack>() << pack1 << minifiedPack1 << minifiedPack1);

        QTest::newRow("MultipleShadersMinified")
                << (QVector<ProgramDNA>() << ProgramDNA(883) << ProgramDNA(883) << ProgramDNA(883) << ProgramDNA(1584) << ProgramDNA(1584) << ProgramDNA(1584))
                << (QVector<ShaderParameterPack>() << pack1 << pack1 << pack1 << pack1 << pack1 << pack1)
                << (QVector<ShaderParameterPack>() << pack1 << minifiedPack1 << minifiedPack1 << pack1 << minifiedPack1 << minifiedPack1);
    }

    void checkRenderViewUniformMinification()
    {
        QFETCH(QVector<ProgramDNA>, programDNAs);
        QFETCH(QVector<ShaderParameterPack>, rawParameters);
        QFETCH(QVector<ShaderParameterPack>, expectedMinimizedParameters);

        RenderView renderView;
        QVector<RenderCommand *> rawCommands;

        for (int i = 0, m = programDNAs.size(); i < m; ++i) {
            RenderCommand *c = new RenderCommand();
            c->m_shaderDna = programDNAs.at(i);
            c->m_parameterPack = rawParameters.at(i);
            rawCommands.push_back(c);
        }

        // WHEN
        renderView.setCommands(rawCommands);
        renderView.sort();

        // THEN
        const QVector<RenderCommand *> sortedCommands = renderView.commands();
        QCOMPARE(rawCommands, sortedCommands);

        for (int i = 0, m = programDNAs.size(); i < m; ++i) {
            const RenderCommand *c = sortedCommands.at(i);
            QCOMPARE(c->m_shaderDna, programDNAs.at(i));
            compareShaderParameterPacks(c->m_parameterPack, expectedMinimizedParameters.at(i));
        }
    }


    void checkRenderCommandFrontToBackSorting()
    {
        // GIVEN
        RenderView renderView;
        QVector<RenderCommand *> rawCommands;
        QVector<QSortPolicy::SortType> sortTypes;

        sortTypes.push_back(QSortPolicy::FrontToBack);

        for (int i = 0; i < 200; ++i) {
            RenderCommand *c = new RenderCommand();
            c->m_depth = float(i);
            rawCommands.push_back(c);
        }

        // WHEN
        renderView.addSortType(sortTypes);
        renderView.setCommands(rawCommands);
        renderView.sort();

        // THEN
        const QVector<RenderCommand *> sortedCommands = renderView.commands();
        QCOMPARE(rawCommands.size(), sortedCommands.size());
        for (int j = 1; j < sortedCommands.size(); ++j)
            QVERIFY(sortedCommands.at(j - 1)->m_depth < sortedCommands.at(j)->m_depth);

        // RenderCommands are deleted by RenderView dtor
    }

    void checkRenderCommandStateCostSorting()
    {
        // GIVEN
        RenderView renderView;
        QVector<RenderCommand *> rawCommands;
        QVector<QSortPolicy::SortType> sortTypes;

        sortTypes.push_back(QSortPolicy::StateChangeCost);

        for (int i = 0; i < 200; ++i) {
            RenderCommand *c = new RenderCommand();
            c->m_changeCost = i;
            rawCommands.push_back(c);
        }

        // WHEN
        renderView.addSortType(sortTypes);
        renderView.setCommands(rawCommands);
        renderView.sort();

        // THEN
        const QVector<RenderCommand *> sortedCommands = renderView.commands();
        QCOMPARE(rawCommands.size(), sortedCommands.size());
        for (int j = 1; j < sortedCommands.size(); ++j)
            QVERIFY(sortedCommands.at(j - 1)->m_changeCost > sortedCommands.at(j)->m_changeCost);

        // RenderCommands are deleted by RenderView dtor
    }

    void checkRenderCommandCombinedStateMaterialDepthSorting()
    {
        // GIVEN
        RenderView renderView;
        QVector<RenderCommand *> rawCommands;
        QVector<QSortPolicy::SortType> sortTypes;

        sortTypes.push_back(QSortPolicy::StateChangeCost);
        sortTypes.push_back(QSortPolicy::Material);
        sortTypes.push_back(QSortPolicy::BackToFront);

        ProgramDNA dna[4] = {
            ProgramDNA(250),
            ProgramDNA(500),
            ProgramDNA(1000),
            ProgramDNA(1500)
        };

        float depth[3] = {
            10.0f,
            25.0f,
            30.0f
        };

        int stateChangeCost[2] = {
            100,
            200
        };

        auto buildRC = [] (ProgramDNA dna, float depth, int changeCost) {
            RenderCommand *c = new RenderCommand();
            c->m_shaderDna = dna;
            c->m_depth = depth;
            c->m_changeCost = changeCost;
            return c;
        };

        RenderCommand *c5 = buildRC(dna[3], depth[1], stateChangeCost[1]);
        RenderCommand *c3 = buildRC(dna[3], depth[0], stateChangeCost[1]);
        RenderCommand *c4 = buildRC(dna[2], depth[1], stateChangeCost[1]);
        RenderCommand *c8 = buildRC(dna[1], depth[1], stateChangeCost[1]);
        RenderCommand *c0 = buildRC(dna[0], depth[2], stateChangeCost[1]);

        RenderCommand *c2 = buildRC(dna[2], depth[2], stateChangeCost[0]);
        RenderCommand *c9 = buildRC(dna[2], depth[0], stateChangeCost[0]);
        RenderCommand *c1 = buildRC(dna[1], depth[0], stateChangeCost[0]);
        RenderCommand *c7 = buildRC(dna[0], depth[2], stateChangeCost[0]);
        RenderCommand *c6 = buildRC(dna[0], depth[1], stateChangeCost[0]);

        rawCommands << c0 << c1 << c2 << c3 << c4 << c5 << c6 << c7 << c8 << c9;

        // WHEN
        renderView.addSortType(sortTypes);
        renderView.setCommands(rawCommands);
        renderView.sort();

        // THEN
        const QVector<RenderCommand *> sortedCommands = renderView.commands();
        QCOMPARE(rawCommands.size(), sortedCommands.size());

        for (RenderCommand *rc : sortedCommands)
            qDebug() << rc->m_changeCost << rc->m_shaderDna << rc->m_depth;

        // Ordered by higher state, higher shaderDNA and higher depth
        QCOMPARE(c0, sortedCommands.at(4));
        QCOMPARE(c3, sortedCommands.at(1));
        QCOMPARE(c4, sortedCommands.at(2));
        QCOMPARE(c5, sortedCommands.at(0));
        QCOMPARE(c8, sortedCommands.at(3));

        QCOMPARE(c1, sortedCommands.at(7));
        QCOMPARE(c2, sortedCommands.at(5));
        QCOMPARE(c6, sortedCommands.at(9));
        QCOMPARE(c7, sortedCommands.at(8));
        QCOMPARE(c9, sortedCommands.at(6));

        // RenderCommands are deleted by RenderView dtor
    }

private:
};

} // Render

} // Qt3DRender

QT_END_NAMESPACE

//APPLESS_
QTEST_MAIN(Qt3DRender::Render::tst_RenderViews)

#include "tst_renderviews.moc"
