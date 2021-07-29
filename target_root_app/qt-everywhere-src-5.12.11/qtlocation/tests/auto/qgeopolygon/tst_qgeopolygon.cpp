/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/QGeoPolygon>

QT_USE_NAMESPACE

class tst_QGeoPolygon : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructor();
    void listConstructor();
    void assignment();

    void comparison();
    void type();

    void path();
    void size();

    void translate_data();
    void translate();

    void valid_data();
    void valid();

    void contains_data();
    void contains();

    void boundingGeoRectangle_data();
    void boundingGeoRectangle();

    void extendShape();
    void extendShape_data();
};

void tst_QGeoPolygon::defaultConstructor()
{
    QGeoPolygon p;
    QVERIFY(!p.path().size());
    QVERIFY(!p.size());
    QVERIFY(!p.isValid());
    QVERIFY(p.isEmpty());
}

void tst_QGeoPolygon::listConstructor()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    QGeoPolygon p2(coords);
    QCOMPARE(p2.path().size(), 2);
    QCOMPARE(p2.size(), 2);
    QVERIFY(!p2.isValid()); // a polygon can't have only 2 coords
    QVERIFY(!p2.isEmpty());

    coords.append(QGeoCoordinate(3,0));

    QGeoPolygon p(coords);
    QCOMPARE(p.path().size(), 3);
    QCOMPARE(p.size(), 3);
    QVERIFY(p.isValid());
    QVERIFY(!p.isEmpty());


    for (const QGeoCoordinate &c : coords) {
        QCOMPARE(p.path().contains(c), true);
        QCOMPARE(p.containsCoordinate(c), true);
    }
}

void tst_QGeoPolygon::assignment()
{
    QGeoPolygon p1;
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));
    QGeoPolygon p2(coords);

    QVERIFY(p1 != p2);

    p1 = p2;
    QCOMPARE(p1.path(), coords);
    QCOMPARE(p1, p2);

    // Assign c1 to an area
    QGeoShape area = p1;
    QCOMPARE(area.type(), p1.type());
    QVERIFY(area == p1);

    // Assign the area back to a polygon
    QGeoPolygon p3 = area;
    QCOMPARE(p3.path(), coords);
    QVERIFY(p3 == p1);

    // Check that the copy is not modified when modifying the original.
    p1.addCoordinate(QGeoCoordinate(4,0));
    QVERIFY(p3 != p1);
}

void tst_QGeoPolygon::comparison()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));
    QList<QGeoCoordinate> coords2;
    coords2.append(QGeoCoordinate(3,1));
    coords2.append(QGeoCoordinate(4,2));
    coords2.append(QGeoCoordinate(3,0));
    QGeoPolygon c1(coords);
    QGeoPolygon c2(coords);
    QGeoPolygon c3(coords2);

    QVERIFY(c1 == c2);
    QVERIFY(!(c1 != c2));

    QVERIFY(!(c1 == c3));
    QVERIFY(c1 != c3);

    QVERIFY(!(c2 == c3));
    QVERIFY(c2 != c3);

    QGeoRectangle b1(QGeoCoordinate(20,20),QGeoCoordinate(10,30));
    QVERIFY(!(c1 == b1));
    QVERIFY(c1 != b1);

    QGeoShape *c2Ptr = &c2;
    QVERIFY(c1 == *c2Ptr);
    QVERIFY(!(c1 != *c2Ptr));

    QGeoShape *c3Ptr = &c3;
    QVERIFY(!(c1 == *c3Ptr));
    QVERIFY(c1 != *c3Ptr);
}

void tst_QGeoPolygon::type()
{
    QGeoPolygon c;
    QCOMPARE(c.type(), QGeoShape::PolygonType);
}

void tst_QGeoPolygon::path()
{
    QList<QGeoCoordinate> coords;
    coords.append(QGeoCoordinate(1,1));
    coords.append(QGeoCoordinate(2,2));
    coords.append(QGeoCoordinate(3,0));

    QGeoPolygon p;
    p.setPath(coords);
    QCOMPARE(p.path().size(), 3);
    QCOMPARE(p.size(), 3);

    for (const QGeoCoordinate &c : coords) {
        QCOMPARE(p.path().contains(c), true);
        QCOMPARE(p.containsCoordinate(c), true);
    }
}

void tst_QGeoPolygon::size()
{
    QList<QGeoCoordinate> coords;

    QGeoPolygon p1(coords);
    QCOMPARE(p1.size(), coords.size());

    coords.append(QGeoCoordinate(1,1));
    QGeoPolygon p2(coords);
    QCOMPARE(p2.size(), coords.size());

    coords.append(QGeoCoordinate(2,2));
    QGeoPolygon p3(coords);
    QCOMPARE(p3.size(), coords.size());

    coords.append(QGeoCoordinate(3,0));
    QGeoPolygon p4(coords);
    QCOMPARE(p4.size(), coords.size());

    p4.removeCoordinate(2);
    QCOMPARE(p4.size(), coords.size() - 1);

    p4.removeCoordinate(coords.first());
    QCOMPARE(p4.size(), coords.size() - 2);
}

void tst_QGeoPolygon::translate_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<double>("lat");
    QTest::addColumn<double>("lon");

    QTest::newRow("Simple") << QGeoCoordinate(1,1) << QGeoCoordinate(2,2) <<
                                 QGeoCoordinate(3,0) << 5.0 << 4.0;
    QTest::newRow("Backward") << QGeoCoordinate(1,1) << QGeoCoordinate(2,2) <<
                                 QGeoCoordinate(3,0) << -5.0 << -4.0;
}

void tst_QGeoPolygon::translate()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(double, lat);
    QFETCH(double, lon);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p(coords);

    p.translate(lat, lon);

    for (int i = 0; i < p.path().size(); i++) {
        QCOMPARE(coords[i].latitude(), p.path()[i].latitude() - lat );
        QCOMPARE(coords[i].longitude(), p.path()[i].longitude() - lon );
    }
}

void tst_QGeoPolygon::valid_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<bool>("valid");

    QTest::newRow("empty coords") << QGeoCoordinate() << QGeoCoordinate() << QGeoCoordinate() << false;
    QTest::newRow("invalid coord") << QGeoCoordinate(50, 50) << QGeoCoordinate(60, 60) << QGeoCoordinate(700, 700) << false;
    QTest::newRow("good") << QGeoCoordinate(10, 10) << QGeoCoordinate(11, 11) << QGeoCoordinate(10, 12)  << true;
}

void tst_QGeoPolygon::valid()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(bool, valid);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p(coords);

    QCOMPARE(p.isValid(), valid);

    QGeoShape area = p;
    QCOMPARE(area.isValid(), valid);
}

void tst_QGeoPolygon::contains_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("result");

    QList<QGeoCoordinate> c;
    c.append(QGeoCoordinate(1,1));
    c.append(QGeoCoordinate(2,2));
    c.append(QGeoCoordinate(3,0));

    QTest::newRow("One of the points") << c[0] << c[1] << c[2] <<  QGeoCoordinate(2, 2) << true;
    QTest::newRow("Not so far away") << c[0] << c[1] << c[2] << QGeoCoordinate(0.8, 0.8) << false;
    QTest::newRow("Not so far away and large line") << c[0] << c[1] << c[2] << QGeoCoordinate(0.8, 0.8) << false;
    QTest::newRow("Inside") << c[0] << c[1] << c[2] << QGeoCoordinate(2.0, 1.0) << true;
}

void tst_QGeoPolygon::contains()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(QGeoCoordinate, probe);
    QFETCH(bool, result);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p(coords);

    QCOMPARE(p.contains(probe), result);

    QGeoShape area = p;
    QCOMPARE(area.contains(probe), result);
}

void tst_QGeoPolygon::boundingGeoRectangle_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("result");

    QList<QGeoCoordinate> c;
    c.append(QGeoCoordinate(1,1));
    c.append(QGeoCoordinate(2,2));
    c.append(QGeoCoordinate(3,0));

    QTest::newRow("One of the points") << c[0] << c[1] << c[2] << QGeoCoordinate(2, 2) << true;
    QTest::newRow("Not so far away") << c[0] << c[1] << c[2] <<  QGeoCoordinate(0, 0) << false;
    QTest::newRow("Inside the bounds") << c[0] << c[1] << c[2] <<  QGeoCoordinate(1, 0) << true;
    QTest::newRow("Inside the bounds") << c[0] << c[1] << c[2] <<  QGeoCoordinate(1.1, 0.1) << true;
}

void tst_QGeoPolygon::boundingGeoRectangle()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(QGeoCoordinate, probe);
    QFETCH(bool, result);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p(coords);

    QGeoRectangle box = p.boundingGeoRectangle();
    QCOMPARE(box.contains(probe), result);
}

void tst_QGeoPolygon::extendShape()
{
    QFETCH(QGeoCoordinate, c1);
    QFETCH(QGeoCoordinate, c2);
    QFETCH(QGeoCoordinate, c3);
    QFETCH(QGeoCoordinate, probe);
    QFETCH(bool, before);
    QFETCH(bool, after);

    QList<QGeoCoordinate> coords;
    coords.append(c1);
    coords.append(c2);
    coords.append(c3);
    QGeoPolygon p(coords);


    QCOMPARE(p.contains(probe), before);
    p.extendShape(probe);
    QCOMPARE(p.contains(probe), after);
}

void tst_QGeoPolygon::extendShape_data()
{
    QTest::addColumn<QGeoCoordinate>("c1");
    QTest::addColumn<QGeoCoordinate>("c2");
    QTest::addColumn<QGeoCoordinate>("c3");
    QTest::addColumn<QGeoCoordinate>("probe");
    QTest::addColumn<bool>("before");
    QTest::addColumn<bool>("after");

    QList<QGeoCoordinate> c;
    c.append(QGeoCoordinate(1,1));
    c.append(QGeoCoordinate(2,2));
    c.append(QGeoCoordinate(3,0));

    QTest::newRow("One of the points") << c[0] << c[1] << c[2] <<  QGeoCoordinate(2, 2) << true << true;
    QTest::newRow("Not so far away") << c[0] << c[1] << c[2] <<  QGeoCoordinate(0, 0) << false << true;
    QTest::newRow("Contained point") << c[0] << c[1] << c[2] <<  QGeoCoordinate(2.0, 1.0) << true << true;
}

QTEST_MAIN(tst_QGeoPolygon)
#include "tst_qgeopolygon.moc"
