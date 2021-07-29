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
#include <qbackendnodetester.h>
#include <testrenderer.h>

#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DRender/private/renderstatenode_p.h>
#include <Qt3DRender/private/managers_p.h>

#include <Qt3DRender/QRenderState>
#include <Qt3DRender/QAlphaCoverage>
#include <Qt3DRender/QAlphaTest>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QDithering>
#include <Qt3DRender/QFrontFace>
#include <Qt3DRender/QPointSize>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/QScissorTest>
#include <Qt3DRender/QStencilTest>
#include <Qt3DRender/QStencilTestArguments>
#include <Qt3DRender/QStencilMask>
#include <Qt3DRender/QStencilOperation>
#include <Qt3DRender/QStencilOperationArguments>
#include <Qt3DRender/QClipPlane>

#include "testpostmanarbiter.h"

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DRender::Render;

class tst_QRenderState : public QBackendNodeTester
{
    Q_OBJECT
public:
    tst_QRenderState() {}
    ~tst_QRenderState() {}

private:
    RenderStateManager m_renderStateManager;
    TestRenderer m_renderer;

    RenderStateNode* createBackendNode(QRenderState *frontend)
    {
        RenderStateNode *backend = m_renderStateManager.getOrCreateResource(frontend->id());
        simulateInitialization(frontend, backend);
        backend->setRenderer(&m_renderer);
        return backend;
    }

    // Create two frontend objects of class T and set given property to v1 / v2 respectively
    template <class T, class V, class Setter>
    void addTestCase(Qt3DRender::Render::StateMask mask, const char *property, Setter setter, V v1, V v2)
    {
        T *obj1 = new T();
        T *obj2 = new T();

        (obj1->*(setter))(v1);
        (obj2->*(setter))(v2);

        QTest::addRow("%s::%s", obj1->metaObject()->className(), property)
                << (QRenderState*) obj1
                << (QRenderState*) obj2
                << (quint64) mask
                << QString(property)
                << QVariant(v2);
    }

    // Create two frontend objects of class T and set given property to v1 / v2 respectively, for the specified arguments sub-object
    template <class T, class Args, class V, class Setter>
    void addStencilTestCase(Qt3DRender::Render::StateMask mask, const char *argsName, const char *property, Setter setter, V v1, V v2)
    {
        T *obj1 = new T();
        T *obj2 = new T();

        const QMetaObject *metaObj = obj1->metaObject();
        const int pIndex = metaObj->indexOfProperty(argsName);
        const QMetaProperty prop = metaObj->property(pIndex);
        Args *args1 = qvariant_cast<Args*>(prop.read(obj1));
        Args *args2 = qvariant_cast<Args*>(prop.read(obj2));

        (args1->*(setter))(v1);
        (args2->*(setter))(v2);

        QTest::addRow("%s::%s::%s", metaObj->className(), argsName, property)
                << (QRenderState*) obj1
                << (QObject*) args1
                << (QRenderState*) obj2
                << (quint64) mask
                << QString(property)
                << QVariant(v2);
    }

private Q_SLOTS:

    void checkPropertyUpdates_data()
    {
        QTest::addColumn<QRenderState *>("frontend1");
        QTest::addColumn<QRenderState *>("frontend2");
        QTest::addColumn<quint64>("mask");
        QTest::addColumn<QString>("propertyName");
        QTest::addColumn<QVariant>("value");

        addTestCase<QAlphaTest>(AlphaTestMask, "alphaFunction", &QAlphaTest::setAlphaFunction, QAlphaTest::Always, QAlphaTest::LessOrEqual);
        addTestCase<QAlphaTest>(AlphaTestMask, "referenceValue", &QAlphaTest::setReferenceValue, 0.f, 1.f);

        addTestCase<QBlendEquation>(BlendStateMask, "blendFunction", &QBlendEquation::setBlendFunction, QBlendEquation::Add, QBlendEquation::Subtract);

        addTestCase<QBlendEquationArguments>(BlendEquationArgumentsMask, "sourceRgb", &QBlendEquationArguments::setSourceRgb, QBlendEquationArguments::Zero, QBlendEquationArguments::One);
        addTestCase<QBlendEquationArguments>(BlendEquationArgumentsMask, "sourceAlpha", &QBlendEquationArguments::setSourceAlpha, QBlendEquationArguments::SourceAlpha, QBlendEquationArguments::OneMinusSource1Color);
        addTestCase<QBlendEquationArguments>(BlendEquationArgumentsMask, "destinationRgb", &QBlendEquationArguments::setDestinationRgb, QBlendEquationArguments::DestinationColor, QBlendEquationArguments::OneMinusSourceAlpha);
        addTestCase<QBlendEquationArguments>(BlendEquationArgumentsMask, "destinationAlpha", &QBlendEquationArguments::setDestinationAlpha, QBlendEquationArguments::DestinationAlpha, QBlendEquationArguments::DestinationColor);
        addTestCase<QBlendEquationArguments>(BlendEquationArgumentsMask, "bufferIndex", &QBlendEquationArguments::setBufferIndex, 0, 1);

        addTestCase<QClipPlane>(ClipPlaneMask, "planeIndex", &QClipPlane::setPlaneIndex, 0, 1);
        addTestCase<QClipPlane>(ClipPlaneMask, "normal", &QClipPlane::setNormal, QVector3D(0, 1, 0), QVector3D(0, 0, 1));
        addTestCase<QClipPlane>(ClipPlaneMask, "distance", &QClipPlane::setDistance, 1.f, 2.f);

        addTestCase<QColorMask>(ColorStateMask, "redMasked", &QColorMask::setRedMasked, false, true);
        addTestCase<QColorMask>(ColorStateMask, "blueMasked", &QColorMask::setBlueMasked, false, true);
        addTestCase<QColorMask>(ColorStateMask, "greenMasked", &QColorMask::setGreenMasked, false, true);
        addTestCase<QColorMask>(ColorStateMask, "alphaMasked", &QColorMask::setAlphaMasked, false, true);

        addTestCase<QCullFace>(CullFaceStateMask, "mode", &QCullFace::setMode, QCullFace::Back, QCullFace::FrontAndBack);

        addTestCase<QFrontFace>(FrontFaceStateMask, "direction", &QFrontFace::setDirection, QFrontFace::ClockWise, QFrontFace::CounterClockWise);

        addTestCase<QPointSize>(PointSizeMask, "sizeMode", &QPointSize::setSizeMode, QPointSize::Programmable, QPointSize::Fixed);
        addTestCase<QPointSize>(PointSizeMask, "value", &QPointSize::setValue, 2.f, 4.f);

        addTestCase<QPolygonOffset>(PolygonOffsetStateMask, "scaleFactor", &QPolygonOffset::setScaleFactor, 1.f, 2.f);
        addTestCase<QPolygonOffset>(PolygonOffsetStateMask, "depthSteps", &QPolygonOffset::setDepthSteps, 1.f, 2.f);

        addTestCase<QScissorTest>(ScissorStateMask, "left", &QScissorTest::setLeft, 10, 20);
        addTestCase<QScissorTest>(ScissorStateMask, "bottom", &QScissorTest::setBottom, 10, 20);
        addTestCase<QScissorTest>(ScissorStateMask, "width", &QScissorTest::setWidth, 10, 20);
        addTestCase<QScissorTest>(ScissorStateMask, "height", &QScissorTest::setHeight, 10, 20);

        addTestCase<QStencilMask>(StencilWriteStateMask, "frontOutputMask", &QStencilMask::setFrontOutputMask, 0x12, 0x34);
        addTestCase<QStencilMask>(StencilWriteStateMask, "backOutputMask", &QStencilMask::setBackOutputMask, 0x12, 0x34);
    }

    void checkPropertyUpdates()
    {
        // GIVEN
        QFETCH(QRenderState*, frontend1);
        QFETCH(QRenderState*, frontend2);
        QFETCH(quint64, mask);
        QFETCH(QString, propertyName);
        QFETCH(QVariant, value);

        // THEN
        RenderStateNode *backend1 = createBackendNode(frontend1);
        RenderStateNode *backend2 = createBackendNode(frontend2);
        QVERIFY(backend1->type() == mask);
        QVERIFY(backend2->type() == mask);
        QVERIFY(backend1->impl() != backend2->impl());

        // WHEN
        TestArbiter arbiter;
        arbiter.setArbiterOnNode(frontend1);
        const QMetaObject *metaObj = frontend1->metaObject();
        const int pIndex = metaObj->indexOfProperty(propertyName.toStdString().c_str());
        metaObj->property(pIndex).write(frontend1, value);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<QPropertyUpdatedChange>();
        QVERIFY(change->propertyName() == propertyName);
        QCOMPARE(change->subjectId(), frontend1->id());

        // WHEN
        backend1->sceneChangeEvent(change.staticCast<QSceneChange>());

        // THEN
        QVERIFY(backend1->impl() == backend2->impl());

        arbiter.events.clear();
    }

    void checkStencilUpdates_data()
    {
        QTest::addColumn<QRenderState *>("frontend1");
        QTest::addColumn<QObject *>("args1");
        QTest::addColumn<QRenderState *>("frontend2");
        QTest::addColumn<quint64>("mask");
        QTest::addColumn<QString>("propertyName");
        QTest::addColumn<QVariant>("value");

        qRegisterMetaType<QStencilOperationArguments*>("QStencilOperationArguments*");
        qRegisterMetaType<QStencilTestArguments*>("QStencilTestArguments*");

        for (bool front : QVector<bool>{false, true}) {
            const char *argsProperty = front ? "front" : "back";

            addStencilTestCase<QStencilOperation, QStencilOperationArguments>(
                        StencilOpMask, argsProperty, "allTestsPassOperation",
                        &QStencilOperationArguments::setAllTestsPassOperation,
                        QStencilOperationArguments::Zero, QStencilOperationArguments::Keep);

            addStencilTestCase<QStencilOperation, QStencilOperationArguments>(
                        StencilOpMask, argsProperty, "depthTestFailureOperation",
                        &QStencilOperationArguments::setDepthTestFailureOperation,
                        QStencilOperationArguments::Replace, QStencilOperationArguments::Zero);

            addStencilTestCase<QStencilOperation, QStencilOperationArguments>(
                        StencilOpMask, argsProperty, "stencilTestFailureOperation",
                        &QStencilOperationArguments::setStencilTestFailureOperation,
                        QStencilOperationArguments::Increment, QStencilOperationArguments::Decrement);

            addStencilTestCase<QStencilTest, QStencilTestArguments>(
                        StencilTestStateMask, argsProperty, "comparisonMask",
                        &QStencilTestArguments::setComparisonMask, 0x12, 0x34);

            addStencilTestCase<QStencilTest, QStencilTestArguments>(
                        StencilTestStateMask, argsProperty, "referenceValue",
                        &QStencilTestArguments::setReferenceValue, 1, 2);

            addStencilTestCase<QStencilTest, QStencilTestArguments>(
                        StencilTestStateMask, argsProperty, "stencilFunction",
                        &QStencilTestArguments::setStencilFunction,
                        QStencilTestArguments::Always, QStencilTestArguments::Equal);
        }
    }

    void checkStencilUpdates()
    {
        // GIVEN
        QFETCH(QRenderState*, frontend1);
        QFETCH(QObject*, args1);
        QFETCH(QRenderState*, frontend2);
        QFETCH(quint64, mask);
        QFETCH(QString, propertyName);
        QFETCH(QVariant, value);

        // THEN
        RenderStateNode *backend1 = createBackendNode(frontend1);
        RenderStateNode *backend2 = createBackendNode(frontend2);
        QVERIFY(backend1->type() == mask);
        QVERIFY(backend2->type() == mask);
        QVERIFY(backend1->impl() != backend2->impl());

        // WHEN
        TestArbiter arbiter;
        arbiter.setArbiterOnNode(frontend1);
        const QMetaObject *metaObj = args1->metaObject();
        const int pIndex = metaObj->indexOfProperty(propertyName.toStdString().c_str());
        metaObj->property(pIndex).write(args1, value);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<QPropertyUpdatedChange>();
        QCOMPARE(change->subjectId(), frontend1->id());

        // WHEN
        backend1->sceneChangeEvent(change.staticCast<QSceneChange>());

        // THEN
        QVERIFY(backend1->impl() == backend2->impl());

        arbiter.events.clear();
    }
};

QTEST_MAIN(tst_QRenderState)

#include "tst_qrenderstate.moc"
