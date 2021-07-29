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

#include <QtTest/QtTest>
#include <Qt3DCore/qcomponent.h>
#include <Qt3DRender/qgeometryfactory.h>
#include <Qt3DRender/qgeometry.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/private/qmesh_p.h>
#include <Qt3DCore/qaspectengine.h>

class MeshFunctorA : public Qt3DRender::QGeometryFactory
{
public:
    MeshFunctorA()
    {}

    ~MeshFunctorA()
    {}

    Qt3DRender::QGeometry *operator ()() override
    {
        return nullptr;
    }

    bool operator ==(const Qt3DRender::QGeometryFactory &other) const override
    {
        return Qt3DRender::functor_cast<MeshFunctorA>(&other);
    }

    QT3D_FUNCTOR(MeshFunctorA)
};

class MeshFunctorB : public Qt3DRender::QGeometryFactory
{
public:
    MeshFunctorB()
    {}

    ~MeshFunctorB()
    {}

    Qt3DRender::QGeometry *operator ()() override
    {
        return nullptr;
    }

    bool operator ==(const Qt3DRender::QGeometryFactory &other) const override
    {
        return Qt3DRender::functor_cast<MeshFunctorB>(&other);
    }

    QT3D_FUNCTOR(MeshFunctorB)
};

class MeshFunctorASub : public MeshFunctorA
{
public:
    MeshFunctorASub()
    {}

    ~MeshFunctorASub()
    {}

    bool operator ==(const Qt3DRender::QGeometryFactory &other) const override
    {
        return Qt3DRender::functor_cast<MeshFunctorASub>(&other);
    }

    QT3D_FUNCTOR(MeshFunctorASub)
};


class tst_MeshFunctors : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void checkInitialState()
    {
        // GIVEN
        Qt3DRender::QMesh mesh;
        mesh.setSource(QUrl(QStringLiteral("./some_path.obj")));

        // WHEN
        const Qt3DRender::MeshLoaderFunctor functor(&mesh);

        // THEN
        QVERIFY(functor.nodeManagers() == nullptr);
        QVERIFY(functor.downloaderService() == nullptr);
        QVERIFY(functor.sourceData().isEmpty());
        QCOMPARE(functor.mesh(), mesh.id());
        QCOMPARE(functor.sourcePath(), mesh.source());
        QCOMPARE(functor.status(), Qt3DRender::QMesh::None);
    }

    void functorComparison()
    {
        // GIVEN
        QScopedPointer<MeshFunctorA> functorA(new MeshFunctorA);
        QScopedPointer<MeshFunctorB> functorB(new MeshFunctorB);
        QScopedPointer<MeshFunctorASub> functorASub(new MeshFunctorASub);

        // THEN
        QVERIFY(!(*functorA == *functorB));
        QVERIFY(!(*functorA == *functorASub));

        QVERIFY(!(*functorB == *functorA));
        QVERIFY(!(*functorB == *functorASub));

        QVERIFY(!(*functorASub == *functorA));
        QVERIFY(!(*functorASub == *functorB));

        QVERIFY(*functorA == *functorA);
        QVERIFY(*functorB == *functorB);
        QVERIFY(*functorASub == *functorASub);
    }

    void checkMeshFunctorEquality()
    {
        // GIVEN
        auto meshA = new Qt3DRender::QMesh();
        meshA->setSource(QUrl::fromLocalFile(QLatin1String("/foo")));
        meshA->setMeshName(QLatin1String("bar"));

        auto meshB = new Qt3DRender::QMesh();
        meshB->setSource(QUrl::fromLocalFile(QLatin1String("/foo")));
        meshB->setMeshName(QLatin1String("baz"));

        auto meshC = new Qt3DRender::QMesh();
        meshC->setSource(QUrl::fromLocalFile(QLatin1String("/baz")));
        meshC->setMeshName(QLatin1String("bar"));

        auto meshD = new Qt3DRender::QMesh();
        meshD->setSource(QUrl::fromLocalFile(QLatin1String("/foo")));
        meshD->setMeshName(QLatin1String("bar"));

        const Qt3DRender::MeshLoaderFunctor functorA(meshA);
        const Qt3DRender::MeshLoaderFunctor functorB(meshB);
        const Qt3DRender::MeshLoaderFunctor functorC(meshC);
        const Qt3DRender::MeshLoaderFunctor functorD(meshD);

        // WHEN
        const bool selfEquality = (functorA == functorA);
        const bool sameSource = (functorA == functorB);
        const bool sameMeshName = (functorA == functorC);
        const bool perfectMatch = (functorA == functorD);

        // THEN
        QCOMPARE(selfEquality, true);
        QCOMPARE(sameSource, false);
        QCOMPARE(sameMeshName, false);
        QCOMPARE(perfectMatch, true);
    }

    void checkExecution()
    {
        {
            // GIVEN
            Qt3DRender::QMesh mesh;
            Qt3DRender::MeshLoaderFunctor functor(&mesh);

            // WHEN
            const Qt3DRender::QGeometry *g = functor();

            // THEN
            QVERIFY(g == nullptr);
            QCOMPARE(functor.status(), Qt3DRender::QMesh::Error);
        }

        {
            // GIVEN
            Qt3DRender::QMesh mesh;
            mesh.setSource(QUrl(QStringLiteral("./non_existing.obj")));
            Qt3DRender::MeshLoaderFunctor functor(&mesh);

            // WHEN
            const Qt3DRender::QGeometry *g = functor();

            // THEN
            QVERIFY(g == nullptr);
            QCOMPARE(functor.status(), Qt3DRender::QMesh::Error);
        }

        {
            // GIVEN
            Qt3DRender::QMesh mesh;
            mesh.setSource(QUrl(QStringLiteral("http://www.somedomain.org/non_exisiting.obj")));
            Qt3DRender::MeshLoaderFunctor functor(&mesh);

            // WHEN
            const Qt3DRender::QGeometry *g = functor();

            // THEN
            QVERIFY(g == nullptr);
            QCOMPARE(functor.status(), Qt3DRender::QMesh::Error);
        }
    }
};

QTEST_MAIN(tst_MeshFunctors)

#include "tst_meshfunctors.moc"
