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
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>

#include <Qt3DRender/QBlitFramebuffer>
#include <Qt3DRender/private/qblitframebuffer_p.h>

#include "testpostmanarbiter.h"

class tst_QBlitFrameBuffer: public QObject
{
    Q_OBJECT

public:
    tst_QBlitFrameBuffer() : QObject()
    {
        qRegisterMetaType<Qt3DRender::QRenderTarget*>();
    }

private Q_SLOTS:

    void checkCreationData_data()
    {
        QTest::addColumn<Qt3DRender::QBlitFramebuffer *>("blitFramebuffer");

        Qt3DRender::QBlitFramebuffer *defaultConstructed = new Qt3DRender::QBlitFramebuffer();
        Qt3DRender::QRenderTarget *sourceRenderTarget = new Qt3DRender::QRenderTarget();
        Qt3DRender::QRenderTarget *destinationRenderTarget = new Qt3DRender::QRenderTarget();
        defaultConstructed->setSource(sourceRenderTarget);
        defaultConstructed->setDestination(destinationRenderTarget);
        QTest::newRow("defaultConstructed") << defaultConstructed;
    }

    void checkInitialState()
    {
        // GIVEN
        Qt3DRender::QBlitFramebuffer blitFramebuffer;

        // THEN
        QCOMPARE(blitFramebuffer.source(), nullptr);
        QCOMPARE(blitFramebuffer.destination(), nullptr);
        QCOMPARE(blitFramebuffer.sourceRect(), QRect());
        QCOMPARE(blitFramebuffer.destinationRect(), QRect());
        QCOMPARE(blitFramebuffer.sourceAttachmentPoint(), Qt3DRender::QRenderTargetOutput::AttachmentPoint::Color0);
        QCOMPARE(blitFramebuffer.destinationAttachmentPoint(), Qt3DRender::QRenderTargetOutput::AttachmentPoint::Color0);
        QCOMPARE(blitFramebuffer.interpolationMethod(), Qt3DRender::QBlitFramebuffer::Linear);
    }

    void checkCreationData()
    {
        // GIVEN
        QFETCH(Qt3DRender::QBlitFramebuffer *, blitFramebuffer);

        // WHEN
        Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(blitFramebuffer);
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges = creationChangeGenerator.creationChanges();

        // THEN
        QCOMPARE(creationChanges.size(), 3); // 3 due to automatic parenting

        const Qt3DCore::QNodeCreatedChangePtr<Qt3DRender::QBlitFramebufferData> creationChangeData =
                qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DRender::QBlitFramebufferData>>(creationChanges.first());
        const Qt3DRender::QBlitFramebufferData &cloneData = creationChangeData->data;

        // THEN
        QCOMPARE(blitFramebuffer->id(), creationChangeData->subjectId());
        QCOMPARE(blitFramebuffer->isEnabled(), creationChangeData->isNodeEnabled());
        QCOMPARE(blitFramebuffer->metaObject(), creationChangeData->metaObject());
        QCOMPARE(blitFramebuffer->source()->id(), cloneData.m_sourceRenderTargetId);
        QCOMPARE(blitFramebuffer->destination()->id(), cloneData.m_destinationRenderTargetId);
        QCOMPARE(blitFramebuffer->sourceRect(), cloneData.m_sourceRect);
        QCOMPARE(blitFramebuffer->destinationRect(), cloneData.m_destinationRect);
        QCOMPARE(blitFramebuffer->sourceAttachmentPoint(), cloneData.m_sourceAttachmentPoint);
        QCOMPARE(blitFramebuffer->destinationAttachmentPoint(), cloneData.m_destinationAttachmentPoint);


        delete blitFramebuffer;
    }

    void checkPropertyUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        QScopedPointer<Qt3DRender::QBlitFramebuffer> blitFramebuffer(new Qt3DRender::QBlitFramebuffer());
        arbiter.setArbiterOnNode(blitFramebuffer.data());

        Qt3DRender::QRenderTarget *sourceRenderTarget = new Qt3DRender::QRenderTarget;
        Qt3DRender::QRenderTarget *destinationRenderTarget = new Qt3DRender::QRenderTarget;

        // sourceRenderTarget
        // WHEN
        blitFramebuffer->setSource(sourceRenderTarget);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "source");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), sourceRenderTarget->id());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        blitFramebuffer->setSource(sourceRenderTarget);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 0);

        // WHEN
        blitFramebuffer->setSource(nullptr);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "source");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), Qt3DCore::QNodeId());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // destinationRenderTarget
        // WHEN
        blitFramebuffer->setDestination(destinationRenderTarget);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "destination");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), destinationRenderTarget->id());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        blitFramebuffer->setDestination(destinationRenderTarget);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 0);

        // WHEN
        blitFramebuffer->setDestination(nullptr);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "destination");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), Qt3DCore::QNodeId());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // sourceRect
        // WHEN
        blitFramebuffer->setSourceRect(QRect(0,0,1,1));
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "sourceRect");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<QRect>(), QRect(0,0,1,1)) ;
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        blitFramebuffer->setSourceRect(QRect(0,0,1,1));
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 0);

        // WHEN
        blitFramebuffer->setSourceRect(QRect());
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "sourceRect");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<QRect>(), QRect());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // destinationRect
        blitFramebuffer->setDestinationRect(QRect(0,0,1,1));
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "destinationRect");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<QRect>(), QRect(0,0,1,1)) ;
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        blitFramebuffer->setDestinationRect(QRect(0,0,1,1));
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 0);

        // WHEN
        blitFramebuffer->setDestinationRect(QRect());
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "destinationRect");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<QRect>(), QRect());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // sourceAttachmentPoint
        // WHEN
        blitFramebuffer->setSourceAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color1);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "sourceAttachmentPoint");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DRender::QRenderTargetOutput::AttachmentPoint>(), Qt3DRender::QRenderTargetOutput::Color1);
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        blitFramebuffer->setSourceAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color1);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 0);

        // WHEN
        blitFramebuffer->setSourceAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color0);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "sourceAttachmentPoint");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DRender::QRenderTargetOutput::AttachmentPoint>(), Qt3DRender::QRenderTargetOutput::Color0);
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // destinationAttachmentPoint
        // WHEN
        blitFramebuffer->setDestinationAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color1);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "destinationAttachmentPoint");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DRender::QRenderTargetOutput::AttachmentPoint>(), Qt3DRender::QRenderTargetOutput::Color1);
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();

        // WHEN
        blitFramebuffer->setDestinationAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color1);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 0);

        // WHEN
        blitFramebuffer->setDestinationAttachmentPoint(Qt3DRender::QRenderTargetOutput::Color0);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "destinationAttachmentPoint");
        QCOMPARE(change->subjectId(), blitFramebuffer->id());
        QCOMPARE(change->value().value<Qt3DRender::QRenderTargetOutput::AttachmentPoint>(), Qt3DRender::QRenderTargetOutput::Color0);
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();
    }

    void checkSourceDestReset()
    {
        // GIVEN
        TestArbiter arbiter;
        QScopedPointer<Qt3DRender::QBlitFramebuffer> blitFramebuffer(new Qt3DRender::QBlitFramebuffer());
        arbiter.setArbiterOnNode(blitFramebuffer.data());

        Qt3DRender::QRenderTarget *sourceRenderTarget = new Qt3DRender::QRenderTarget; // no parent
        Qt3DRender::QRenderTarget *destinationRenderTarget = new Qt3DRender::QRenderTarget(sourceRenderTarget); // have a parent

        // WHEN
        blitFramebuffer->setSource(sourceRenderTarget);
        blitFramebuffer->setDestination(destinationRenderTarget);
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(sourceRenderTarget->parent(), blitFramebuffer.data());
        QCOMPARE(destinationRenderTarget->parent(), sourceRenderTarget); // already set parent must not change

        // WHEN
        delete sourceRenderTarget;

        // THEN
        QVERIFY(!blitFramebuffer->source());
        QVERIFY(!blitFramebuffer->destination()); // gone too since destinationRenderTarget was parented to sourceRenderTarget
    }
};

QTEST_MAIN(tst_QBlitFrameBuffer)

#include "tst_qblitframebuffer.moc"
