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
#include <Qt3DRender/QScreenRayCaster>

#include "testpostmanarbiter.h"

class MyRayCaster : public Qt3DRender::QScreenRayCaster
{
    Q_OBJECT
public:
    MyRayCaster(Qt3DCore::QNode *parent = nullptr)
        : Qt3DRender::QScreenRayCaster(parent)
    {
        qRegisterMetaType<Qt3DRender::QAbstractRayCaster::Hits>("Hits");
    }

    void sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change) final
    {
        Qt3DRender::QScreenRayCaster::sceneChangeEvent(change);
    }

private:
    friend class tst_RayCaster;
};

class tst_QScreenRayCaster : public QObject
{
    Q_OBJECT
public:
    tst_QScreenRayCaster()
    {
    }

    ~tst_QScreenRayCaster()
    {
        QMetaObject::invokeMethod(this, "_q_cleanup", Qt::DirectConnection);
    }

private Q_SLOTS:

    void checkState()
    {
        // GIVEN
        QScopedPointer<Qt3DRender::QScreenRayCaster> rayCaster(new Qt3DRender::QScreenRayCaster());

        QVERIFY(!rayCaster->isEnabled());
        QCOMPARE(rayCaster->runMode(), Qt3DRender::QAbstractRayCaster::SingleShot);

        // WHEN
        rayCaster->trigger();

        // THEN
        QVERIFY(rayCaster->isEnabled());

        // WHEN
        rayCaster->setEnabled(false);
        rayCaster->trigger(QPoint(200, 200));

        // THEN
        QVERIFY(rayCaster->isEnabled());
        QCOMPARE(rayCaster->position(), QPoint(200, 200));
    }

    void checkPropertyUpdates()
    {
        // GIVEN
        TestArbiter arbiter;
        QScopedPointer<Qt3DRender::QScreenRayCaster> rayCaster(new Qt3DRender::QScreenRayCaster());
        arbiter.setArbiterOnNode(rayCaster.data());

        // WHEN
        rayCaster->setPosition({200, 200});
        QCoreApplication::processEvents();

        // THEN
        QCOMPARE(arbiter.events.size(), 1);
        Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.last().staticCast<Qt3DCore::QPropertyUpdatedChange>();
        QCOMPARE(change->propertyName(), "position");
        QCOMPARE(change->value().value<QPoint>(), QPoint(200, 200));
        QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

        arbiter.events.clear();
    }

    void checkBackendUpdates_data()
    {
        QTest::addColumn<QByteArray>("signalPrototype");
        QTest::addColumn<QByteArray>("propertyName");

        QTest::newRow("hits")
                << QByteArray(SIGNAL(hitsChanged(const Hits &)))
                << QByteArrayLiteral("hits");
    }
};

QTEST_MAIN(tst_QScreenRayCaster)

#include "tst_qscreenraycaster.moc"
