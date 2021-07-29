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

#include <QtTest/QtTest>
#include <Qt3DCore/qnodeid.h>
#include <Qt3DCore/private/matrix4x4_avx2_p.h>
#include <Qt3DCore/private/qresourcemanager_p.h>
#include <Qt3DRender/private/cameralens_p.h>

using namespace Qt3DCore;

using HMatrix = Qt3DCore::QHandle<Matrix4x4_AVX2>;
using HCameraLens = Qt3DCore::QHandle<Qt3DRender::Render::CameraLens>;

class MatrixManager : public Qt3DCore::QResourceManager<
        Matrix4x4_AVX2,
        Qt3DCore::QNodeId,
        Qt3DCore::NonLockingPolicy>
{
public:
    MatrixManager() {}
};

class CameraLensManager : public Qt3DCore::QResourceManager<
        Qt3DRender::Render::CameraLens,
        Qt3DCore::QNodeId,
        Qt3DCore::NonLockingPolicy>
{
public:
    CameraLensManager() {}
};


class tst_AlignedResourcesManagersAVX: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void checkAllocationAndAlignmentMatrix4x4()
    {
        // GIVEN
        MatrixManager manager;

        // WHEN
        for (uint i = 0; i < std::numeric_limits<ushort>::max(); ++i)
            manager.getOrCreateResource(Qt3DCore::QNodeId::createId());

        // THEN
        // Shouldn't crash

        const QVector<HMatrix> activeHandles = manager.activeHandles();
        for (const HMatrix handle : activeHandles) {
            // WHEN
            Matrix4x4_AVX2 *mat = manager.data(handle);
            // THEN
            QCOMPARE(int((uintptr_t)mat % 32), 0);
        }

        // WHEN
        for (uint i = 2; i < std::numeric_limits<ushort>::max(); ++i) {
            Matrix4x4_AVX2 *mat1 = manager.data(activeHandles.at(i - 2));
            Matrix4x4_AVX2 *mat2 = manager.data(activeHandles.at(i - 1));
            Matrix4x4_AVX2 *mat3 = manager.data(activeHandles.at(i));

            // WHEN
            *mat3 = (*mat2 * *mat1);

            // THEN
            // Shouldn't crash
        }
    }

    void checkAllocationAndAlignmentCameraLens()
    {
        // GIVEN
        CameraLensManager manager;

        // WHEN
        for (uint i = 0; i < std::numeric_limits<ushort>::max(); ++i)
            manager.getOrCreateResource(Qt3DCore::QNodeId::createId());

        // THEN
        // Shouldn't crash

        const QVector<HCameraLens> activeHandles = manager.activeHandles();
        for (const HCameraLens handle : activeHandles) {
            // WHEN
            Qt3DRender::Render::CameraLens *lens = manager.data(handle);
            // THEN
            QCOMPARE(int((uintptr_t)lens % 32), 0);
        }
    }
};

QTEST_MAIN(tst_AlignedResourcesManagersAVX)

#include "tst_alignedresourcesmanagers-avx.moc"
