/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
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
#include <QMatrix4x4>
#include <Qt3DCore/private/qresourcemanager_p.h>

class tst_ArrayPolicy : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void benchmarkDynamicAllocateSmallResources();
    void benchmarkDynamicReleaseSmallResources();
    void benchmarkDynamicAllocateBigResources();
    void benchmarkDynamicReleaseBigResources();
};

struct SmallType
{
    quint32 data;
};

struct BigType
{
    QMatrix4x4 a;
};

template<typename T>
void benchmarkAllocateResources()
{
    Qt3DCore::ArrayAllocatingPolicy<T> allocator;

    const int max = (1 << 16) - 1;
    QBENCHMARK_ONCE {
        for (int i = 0; i < max; i++) {
            (void) allocator.allocateResource();
        }
    }
}

template<typename T>
void benchmarkReleaseResources()
{
    Qt3DCore::ArrayAllocatingPolicy<T> allocator;

    const int max = (1 << 16) - 1;
    QVector<Qt3DCore::QHandle<T>> resources(max);
    for (int i = 0; i < max; i++) {
        resources[i] = allocator.allocateResource();
    }

    QBENCHMARK_ONCE {
        foreach (auto ptr, resources) {
            allocator.releaseResource(ptr);
        }
    }
}

void tst_ArrayPolicy::benchmarkDynamicAllocateSmallResources()
{
    benchmarkAllocateResources<SmallType>();
}

void tst_ArrayPolicy::benchmarkDynamicReleaseSmallResources()
{
    benchmarkReleaseResources<SmallType>();
}

void tst_ArrayPolicy::benchmarkDynamicAllocateBigResources()
{
    benchmarkAllocateResources<BigType>();
}

void tst_ArrayPolicy::benchmarkDynamicReleaseBigResources()
{
    benchmarkReleaseResources<BigType>();
}

QTEST_APPLESS_MAIN(tst_ArrayPolicy)

#include "tst_bench_arraypolicy.moc"
