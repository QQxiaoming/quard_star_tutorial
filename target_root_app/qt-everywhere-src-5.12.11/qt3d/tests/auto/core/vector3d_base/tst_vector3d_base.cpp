/****************************************************************************
**
** Copyright (C) 2016 Paul Lemire <paul.lemire350@gmail.com>
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
#include <Qt3DCore/private/vector3d_p.h>

class tst_Vector3D_Base: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void defaultConstruction()
    {
        // GIVEN
        QVector3D vec3;

        // THEN
        QCOMPARE(vec3.x(), 0.0f);
        QCOMPARE(vec3.y(), 0.0f);
        QCOMPARE(vec3.z(), 0.0f);
    }

    void checkExplicitConstruction()
    {
        // GIVEN
        QVector3D vec3(427.0f, 454.0f, 383.0f);

        // THEN
        QCOMPARE(vec3.x(), 427.0f);
        QCOMPARE(vec3.y(), 454.0f);
        QCOMPARE(vec3.z(), 383.0f);
    }

    void checkSelfAddition_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << 1.0f << 5.0f << 8.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << 0.0f << 0.0f << 0.0f;
    }

    void checkSelfAddition()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D vo(xO, yO, zO);

        // WHEN
        vo += QVector3D(xA, yA, zA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
    }

    void checkSelfSubstraction_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << -1.0f << -5.0f << -8.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << -10.0f << 16.0f << -8.0f;
    }

    void checkSelfSubstraction()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D vo(xO, yO, zO);

        // WHEN
        vo -= QVector3D(xA, yA, zA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
    }

    void checkSelfMultiplication_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << 0.0f << 0.0f << 16.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << -25.0f << -64.0f << 16.0f;
    }

    void checkSelfMultiplication()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D vo(xO, yO, zO);

        // WHEN
        vo *= QVector3D(xA, yA, zA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
    }

    void checkSelfDivision_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << 0.0f << 0.0f << 0.25f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << -1.0f << -1.0f << 1.0f;
    }

    void checkSelfDivision()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D vo(xO, yO, zO);

        // WHEN
        vo /= QVector3D(xA, yA, zA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
    }

    void checkSelfDivisionFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f
                                  << 1.0f
                                  << 0.0f << 0.0f << 2.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f
                                  << 5.0f
                                  << -1.0f << 4.0f << -5.0f;
    }

    void checkSelfDivisionFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D vo(xO, yO, zO);

        // WHEN
        vo /= factor;

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
    }

    void checkSelfMultiplicationFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << -3.0f << 2.0f
                                  << 1.0f
                                  << 0.0f << -3.0f << 2.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f
                                  << 5.0f
                                  << -25.0f << 100.0f << -125.0f;
    }

    void checkSelfMultiplicationFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D vo(xO, yO, zO);

        // WHEN
        vo *= factor;

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
    }

    void checkAddition_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << 1.0f << 5.0f << 8.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << 0.0f << 0.0f << 0.0f;
    }

    void checkAddition()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D v0(xO, yO, zO);
        QVector3D v1(xA, yA, zA);

        // WHEN
        QVector3D v2 = v0 + v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
    }

    void checkSubstraction_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << -1.0f << -5.0f << -8.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << -10.0f << 16.0f << -8.0f;
    }

    void checkSubstraction()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D v0(xO, yO, zO);
        QVector3D v1(xA, yA, zA);

        // WHEN
        QVector3D v2 = v0 - v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
    }

    void checkMultiplication_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << 0.0f << 0.0f << 16.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << -25.0f << -64.0f << 16.0f;
    }

    void checkMultiplication()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D v0(xO, yO, zO);
        QVector3D v1(xA, yA, zA);

        // WHEN
        QVector3D v2 = v0 * v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
    }

    void checkDivision_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f
                                  << 1.0f << 5.0f << 8.0f
                                  << 0.0f << 0.0f << 0.25f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f
                                  << 5.0f << -8.0f << 4.0f
                                  << -1.0f << -1.0f << 1.0f;
    }

    void checkDivision()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D v0(xO, yO, zO);
        QVector3D v1(xA, yA, zA);

        // WHEN
        QVector3D v2 = v0 / v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
    }

    void checkDivisionFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f
                                  << 1.0f
                                  << 0.0f << 0.0f << 2.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f
                                  << 5.0f
                                  << -1.0f << 4.0f << -5.0f;
    }

    void checkDivisionFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D v0(xO, yO, zO);

        // WHEN
        QVector3D v2 = v0 / factor;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
    }

    void checkMultiplicationFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");

        QTest::newRow("sample_1") << 0.0f << -3.0f << 2.0f
                                  << 1.0f
                                  << 0.0f << -3.0f << 2.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f
                                  << 5.0f
                                  << -25.0f << 100.0f << -125.0f;
    }

    void checkMultiplicationFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);

        // GIVEN
        QVector3D v0(xO, yO, zO);

        // WHEN
        QVector3D v2 = v0 * factor;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
    }

    void checkEquality()
    {
        {
            // GIVEN
            QVector3D v0;
            QVector3D v1;

            // THEN
            QVERIFY(v0 == v1);
        }
        {
            // GIVEN
            QVector3D v0(1.0f, 2.0f, -5.0f);
            QVector3D v1(1.0f, 2.0f, -5.0f);

            // THEN
            QVERIFY(v0 == v1);
        }
    }

    void checkInequality()
    {
        {
            // GIVEN
            QVector3D v0;
            QVector3D v1;

            // THEN
            QVERIFY(!(v0 != v1));
        }
        {
            // GIVEN
            QVector3D v0(1.0f, 2.0f, -5.0f);
            QVector3D v1(1.0f, 5.0f, -5.0f);

            // THEN
            QVERIFY(v0 != v1);
        }
    }

    void checkToQQVector3D()
    {
        {
            // GIVEN
            QVector3D v0;

            // WHEN
            QVector3D v1 = v0;

            // THEN
            QCOMPARE(v0.x(), v1.x());
            QCOMPARE(v0.y(), v1.y());
            QCOMPARE(v0.z(), v1.z());
        }
        {
            // GIVEN
            QVector3D v0(1.0f, 2.0f, -5.0f);

            // WHEN
            QVector3D v1 = v0;

            // THEN
            QCOMPARE(v0.x(), v1.x());
            QCOMPARE(v0.y(), v1.y());
            QCOMPARE(v0.z(), v1.z());
        }
    }
};

QTEST_MAIN(tst_Vector3D_Base)

#include "tst_vector3d_base.moc"
