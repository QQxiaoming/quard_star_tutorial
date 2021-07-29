/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/private/qrendercapture_p.h>

#include <QPointer>

#include "testpostmanarbiter.h"

class MyRenderCapture : public Qt3DRender::QRenderCapture
{
    Q_OBJECT
public:
    MyRenderCapture(Qt3DCore::QNode *parent = nullptr)
        : Qt3DRender::QRenderCapture(parent)
    {}

    void sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change) final
    {
        Qt3DRender::QRenderCapture::sceneChangeEvent(change);
    }

private:
    friend class tst_QRenderCapture;
};

class tst_QRenderCapture : public Qt3DCore::QNode
{
    Q_OBJECT

private Q_SLOTS:

    void checkCaptureRequest()
    {
        // GIVEN
        TestArbiter arbiter;
        QScopedPointer<Qt3DRender::QRenderCapture> renderCapture(new Qt3DRender::QRenderCapture());
        arbiter.setArbiterOnNode(renderCapture.data());

        // WHEN
        QScopedPointer<Qt3DRender::QRenderCaptureReply> reply(renderCapture->requestCapture(QRect(10, 15, 20, 50)));

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "renderCaptureRequest");
        QCOMPARE(change->subjectId(),renderCapture->id());
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);
        QVERIFY(change->value().canConvert<Qt3DRender::QRenderCaptureRequest>());
        const Qt3DRender::QRenderCaptureRequest request = change->value().value<Qt3DRender::QRenderCaptureRequest>();
        QCOMPARE(request.captureId, 1);
        QCOMPARE(request.rect, QRect(10, 15, 20, 50));

        arbiter.events.clear();
    }

    void checkRenderCaptureReply()
    {
        // GIVEN
        QScopedPointer<MyRenderCapture> renderCapture(new MyRenderCapture());
        QScopedPointer<Qt3DRender::QRenderCaptureReply> reply(renderCapture->requestCapture());
        QImage img = QImage(20, 20, QImage::Format_ARGB32);

        // WHEN
        Qt3DRender::RenderCaptureDataPtr data = Qt3DRender::RenderCaptureDataPtr::create();
        data.data()->captureId = 2;
        data.data()->image = img;

        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(renderCapture->id());
        e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
        e->setPropertyName("renderCaptureData");
        e->setValue(QVariant::fromValue(data));

        renderCapture->sceneChangeEvent(e);

        // THEN
        QCOMPARE(reply->isComplete(), true);
        QCOMPARE(reply->image().width(), 20);
        QCOMPARE(reply->image().height(), 20);
        QCOMPARE(reply->image().format(), QImage::Format_ARGB32);
    }

    void checkRenderCaptureDestroy()
    {
        // GIVEN
        QScopedPointer<MyRenderCapture> renderCapture(new MyRenderCapture());
        QScopedPointer<Qt3DRender::QRenderCaptureReply> reply(renderCapture->requestCapture());
        QImage img = QImage(20, 20, QImage::Format_ARGB32);
        Qt3DRender::RenderCaptureDataPtr data = Qt3DRender::RenderCaptureDataPtr::create();
        data.data()->captureId = 2;
        data.data()->image = img;
        auto e = Qt3DCore::QPropertyUpdatedChangePtr::create(renderCapture->id());
        e->setDeliveryFlags(Qt3DCore::QSceneChange::DeliverToAll);
        e->setPropertyName("renderCaptureData");
        e->setValue(QVariant::fromValue(data));

        // WHEN
        reply.reset();

        // THEN
        renderCapture->sceneChangeEvent(e); // Should not reset
    }

    void crashOnRenderCaptureDeletion()
    {
        // GIVEN
        QScopedPointer<Qt3DRender::QRenderCapture> renderCapture(new Qt3DRender::QRenderCapture());
        QPointer<Qt3DRender::QRenderCaptureReply> renderCaptureReply(renderCapture->requestCapture());

        // THEN
        QVERIFY(renderCaptureReply);

        // WHEN
        renderCapture.reset();

        // THEN (Should not crash and delete reply)
        QVERIFY(!renderCaptureReply);
    }

};

QTEST_MAIN(tst_QRenderCapture)

#include "tst_qrendercapture.moc"
