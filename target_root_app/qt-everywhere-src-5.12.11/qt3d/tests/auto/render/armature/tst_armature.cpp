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
#include <Qt3DRender/private/armature_p.h>
#include <Qt3DCore/qarmature.h>
#include <Qt3DCore/qskeleton.h>
#include <Qt3DCore/private/qnode_p.h>
#include <Qt3DCore/private/qscene_p.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qbackendnode_p.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DRender::Render;

class tst_Armature: public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private Q_SLOTS:
    void checkPeerPropertyMirroring()
    {
        // GIVEN
        Armature backendArmature;
        QArmature armature;
        auto skeleton = new QSkeleton;

        armature.setSkeleton(skeleton);

        // WHEN
        simulateInitialization(&armature, &backendArmature);

        // THEN
        QCOMPARE(backendArmature.peerId(), armature.id());
        QCOMPARE(backendArmature.isEnabled(), armature.isEnabled());
        QCOMPARE(backendArmature.skeletonId(), skeleton->id());
    }

    void checkInitialAndCleanedUpState()
    {
        // GIVEN
        Armature backendArmature;

        // THEN
        QVERIFY(backendArmature.peerId().isNull());
        QCOMPARE(backendArmature.isEnabled(), false);
        QCOMPARE(backendArmature.skeletonId(), Qt3DCore::QNodeId());

        // GIVEN
        QArmature armature;
        auto skeleton = new QSkeleton();
        armature.setSkeleton(skeleton);

        // WHEN
        simulateInitialization(&armature, &backendArmature);
        backendArmature.cleanup();

        // THEN
        QCOMPARE(backendArmature.skeletonId(), Qt3DCore::QNodeId());
        QCOMPARE(backendArmature.isEnabled(), false);
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Armature backendArmature;
        Qt3DCore::QPropertyUpdatedChangePtr updateChange;

        // WHEN
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("enabled");
        updateChange->setValue(true);
        backendArmature.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendArmature.isEnabled(), true);

        // WHEN
        auto newSkeleton = new QSkeleton();
        updateChange.reset(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
        updateChange->setPropertyName("skeleton");
        updateChange->setValue(QVariant::fromValue(newSkeleton->id()));
        backendArmature.sceneChangeEvent(updateChange);

        // THEN
        QCOMPARE(backendArmature.skeletonId(), newSkeleton->id());
    }
};

QTEST_APPLESS_MAIN(tst_Armature)

#include "tst_armature.moc"
