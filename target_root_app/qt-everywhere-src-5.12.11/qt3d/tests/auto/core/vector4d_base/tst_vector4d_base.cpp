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
#include <Qt3DCore/private/vector4d_p.h>

class tst_Vector4D_Base: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void defaultConstruction()
    {
        // GIVEN
        QVector4D vec4;

        // THEN
        QCOMPARE(vec4.x(), 0.0f);
        QCOMPARE(vec4.y(), 0.0f);
        QCOMPARE(vec4.z(), 0.0f);
        QCOMPARE(vec4.w(), 0.0f);
    }

    void checkExplicitConstruction()
    {
        // GIVEN
        QVector4D vec4(427.0f, 454.0f, 383.0f, 350.0f);

        // THEN
        QCOMPARE(vec4.x(), 427.0f);
        QCOMPARE(vec4.y(), 454.0f);
        QCOMPARE(vec4.z(), 383.0f);
        QCOMPARE(vec4.w(), 350.0f);
    }

    void checkSelfAddition_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << -6.0f
                                  << 0.0f << 0.0f << 0.0f << 0.0f;
    }

    void checkSelfAddition()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D vo(xO, yO, zO, wO);

        // WHEN
        vo += QVector4D(xA, yA, zA, wA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
        QCOMPARE(vo.w(), wR);
    }

    void checkSelfSubstraction_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << -1.0f << -5.0f << -8.0f << 5.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << -6.0f
                                  << -10.0f << 16.0f << -8.0f << 12.0f;
    }

    void checkSelfSubstraction()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D vo(xO, yO, zO, wO);

        // WHEN
        vo -= QVector4D(xA, yA, zA, wA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
        QCOMPARE(vo.w(), wR);
    }

    void checkSelfMultiplication_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f << 1.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << 0.0f << 0.0f << 16.0f << -5.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << 6.0f
                                  << -25.0f << -64.0f << 16.0f << 36.0f;
    }

    void checkSelfMultiplication()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D vo(xO, yO, zO, wO);

        // WHEN
        vo *= QVector4D(xA, yA, zA, wA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
        QCOMPARE(vo.w(), wR);
    }

    void checkSelfDivision_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f << 1.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << 0.0f << 0.0f << 0.25f << -0.20f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << 6.0f
                                  << -1.0f << -1.0f << 1.0f << 1.0f;
    }

    void checkSelfDivision()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D vo(xO, yO, zO, wO);

        // WHEN
        vo /= QVector4D(xA, yA, zA, wA);

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
        QCOMPARE(vo.w(), wR);
    }

    void checkSelfDivisionFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f << 1.0f
                                  << 1.0f
                                  << 0.0f << 0.0f << 2.0f << 1.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f << 10.0f
                                  << 5.0f
                                  << -1.0f << 4.0f << -5.0f << 2.0f;
    }

    void checkSelfDivisionFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D vo(xO, yO, zO, wO);

        // WHEN
        vo /= factor;

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
        QCOMPARE(vo.w(), wR);
    }

    void checkSelfMultiplicationFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << -3.0f << 2.0f << 1.0f
                                  << 1.0f
                                  << 0.0f << -3.0f << 2.0f << 1.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f << 10.0f
                                  << 5.0f
                                  << -25.0f << 100.0f << -125.0f << 50.0f;
    }

    void checkSelfMultiplicationFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D vo(xO, yO, zO, wO);

        // WHEN
        vo *= factor;

        // THEN
        QCOMPARE(vo.x(), xR);
        QCOMPARE(vo.y(), yR);
        QCOMPARE(vo.z(), zR);
        QCOMPARE(vo.w(), wR);
    }

    void checkAddition_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << -6.0f
                                  << 0.0f << 0.0f << 0.0f << 0.0f;
    }

    void checkAddition()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D v0(xO, yO, zO, wO);
        QVector4D v1(xA, yA, zA, wA);

        // WHEN
        QVector4D v2 = v0 + v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
        QCOMPARE(v2.w(), wR);
    }

    void checkSubstraction_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 0.0f << 0.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << -1.0f << -5.0f << -8.0f << 5.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << -4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << -6.0f
                                  << -10.0f << 16.0f << -8.0f << 12.0f;
    }

    void checkSubstraction()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D v0(xO, yO, zO, wO);
        QVector4D v1(xA, yA, zA, wA);

        // WHEN
        QVector4D v2 = v0 - v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
        QCOMPARE(v2.w(), wR);
    }

    void checkMultiplication_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f << 1.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << 0.0f << 0.0f << 16.0f << -5.0f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << 6.0f
                                  << -25.0f << -64.0f << 16.0f << 36.0f;
    }

    void checkMultiplication()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D v0(xO, yO, zO, wO);
        QVector4D v1(xA, yA, zA, wA);

        // WHEN
        QVector4D v2 = v0 * v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
        QCOMPARE(v2.w(), wR);
    }

    void checkDivision_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("xA");
        QTest::addColumn<float>("yA");
        QTest::addColumn<float>("zA");
        QTest::addColumn<float>("wA");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f << 1.0f
                                  << 1.0f << 5.0f << 8.0f << -5.0f
                                  << 0.0f << 0.0f << 0.25f << -0.20f;

        QTest::newRow("sample_2") << -5.0f << 8.0f << 4.0f << 6.0f
                                  << 5.0f << -8.0f << 4.0f << 6.0f
                                  << -1.0f << -1.0f << 1.0f << 1.0f;
    }

    void checkDivision()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, xA);
        QFETCH(float, yA);
        QFETCH(float, zA);
        QFETCH(float, wA);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D v0(xO, yO, zO, wO);
        QVector4D v1(xA, yA, zA, wA);

        // WHEN
        QVector4D v2 = v0 / v1;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
        QCOMPARE(v2.w(), wR);
    }

    void checkDivisionFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << 0.0f << 2.0f << 1.0f
                                  << 1.0f
                                  << 0.0f << 0.0f << 2.0f << 1.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f << 10.0f
                                  << 5.0f
                                  << -1.0f << 4.0f << -5.0f << 2.0f;
    }

    void checkDivisionFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D v0(xO, yO, zO, wO);

        // WHEN
        QVector4D v2 = v0 / factor;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
        QCOMPARE(v2.w(), wR);
    }

    void checkMultiplicationFactor_data()
    {
        QTest::addColumn<float>("xO");
        QTest::addColumn<float>("yO");
        QTest::addColumn<float>("zO");
        QTest::addColumn<float>("wO");

        QTest::addColumn<float>("factor");

        QTest::addColumn<float>("xR");
        QTest::addColumn<float>("yR");
        QTest::addColumn<float>("zR");
        QTest::addColumn<float>("wR");

        QTest::newRow("sample_1") << 0.0f << -3.0f << 2.0f << 1.0f
                                  << 1.0f
                                  << 0.0f << -3.0f << 2.0f << 1.0f;

        QTest::newRow("sample_2") << -5.0f << 20.0f << -25.0f << 10.0f
                                  << 5.0f
                                  << -25.0f << 100.0f << -125.0f << 50.0f;
    }

    void checkMultiplicationFactor()
    {
        QFETCH(float, xO);
        QFETCH(float, yO);
        QFETCH(float, zO);
        QFETCH(float, wO);

        QFETCH(float, factor);

        QFETCH(float, xR);
        QFETCH(float, yR);
        QFETCH(float, zR);
        QFETCH(float, wR);

        // GIVEN
        QVector4D v0(xO, yO, zO, wO);

        // WHEN
        QVector4D v2 = v0 * factor;

        // THEN
        QCOMPARE(v2.x(), xR);
        QCOMPARE(v2.y(), yR);
        QCOMPARE(v2.z(), zR);
        QCOMPARE(v2.w(), wR);
    }

    void checkEquality()
    {
        {
            // GIVEN
            QVector4D v0;
            QVector4D v1;

            // THEN
            QVERIFY(v0 == v1);
        }
        {
            // GIVEN
            QVector4D v0(1.0f, 2.0f, -5.0f, 10.0f);
            QVector4D v1(1.0f, 2.0f, -5.0f, 10.0f);

            // THEN
            QVERIFY(v0 == v1);
        }
    }

    void checkInequality()
    {
        {
            // GIVEN
            QVector4D v0;
            QVector4D v1;

            // THEN
            QVERIFY(!(v0 != v1));
        }
        {
            // GIVEN
            QVector4D v0(1.0f, 2.0f, -5.0f, 10.0f);
            QVector4D v1(1.0f, 5.0f, -5.0f, 10.0f);

            // THEN
            QVERIFY(v0 != v1);
        }
    }

    void checkToQQVector4D()
    {
        {
            // GIVEN
            QVector4D v0;

            // WHEN
            QVector4D v1 = v0;

            // THEN
            QCOMPARE(v0.x(), v1.x());
            QCOMPARE(v0.y(), v1.y());
            QCOMPARE(v0.z(), v1.z());
            QCOMPARE(v0.w(), v1.w());
        }
        {
            // GIVEN
            QVector4D v0(1.0f, 2.0f, -5.0f, 10.0f);

            // WHEN
            QVector4D v1 = v0;

            // THEN
            QCOMPARE(v0.x(), v1.x());
            QCOMPARE(v0.y(), v1.y());
            QCOMPARE(v0.z(), v1.z());
            QCOMPARE(v0.w(), v1.w());
        }
    }
};

QTEST_APPLESS_MAIN(tst_Vector4D_Base)

#include "tst_vector4d_base.moc"
