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
#include <QtQuick/qquickview.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlexpression.h>
#include <QtQml/qqmlincubator.h>
#include <QtQuickShapes/private/qquickshape_p.h>

#include "../../shared/util.h"
#include "../shared/viewtestutil.h"
#include "../shared/visualtestutil.h"

using namespace QQuickViewTestUtil;
using namespace QQuickVisualTestUtil;

class tst_QQuickShape : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_QQuickShape();

private slots:
    void initValues();
    void vpInitValues();
    void basicShape();
    void changeSignals();
    void render();
    void renderWithMultipleSp();
    void radialGrad();
    void conicalGrad();
};

tst_QQuickShape::tst_QQuickShape()
{
    // Force the software backend to get reliable rendering results regardless of the hw and drivers.
    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);

    const char *uri = "tst_qquickpathitem";
    qmlRegisterType<QQuickShape>(uri, 1, 0, "Shape");
    qmlRegisterType<QQuickShapePath>(uri, 1, 0, "ShapePath");
    qmlRegisterUncreatableType<QQuickShapeGradient>(uri, 1, 0, "ShapeGradient", QQuickShapeGradient::tr("ShapeGradient is an abstract base class"));
    qmlRegisterType<QQuickShapeLinearGradient>(uri, 1, 0, "LinearGradient");
    qmlRegisterType<QQuickShapeRadialGradient>(uri, 1, 0, "RadialGradient");
    qmlRegisterType<QQuickShapeConicalGradient>(uri, 1, 0, "ConicalGradient");
}

void tst_QQuickShape::initValues()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("pathitem1.qml"));
    QQuickShape *obj = qobject_cast<QQuickShape *>(c.create());

    QVERIFY(obj != nullptr);
    QVERIFY(obj->rendererType() == QQuickShape::UnknownRenderer);
    QVERIFY(!obj->asynchronous());
    QVERIFY(!obj->vendorExtensionsEnabled());
    QVERIFY(obj->status() == QQuickShape::Null);
    auto vps = obj->data();
    QVERIFY(vps.count(&vps) == 0);

    delete obj;
}

void tst_QQuickShape::vpInitValues()
{
    QQmlEngine engine;
    QQmlComponent c(&engine, testFileUrl("pathitem2.qml"));
    QQuickShape *obj = qobject_cast<QQuickShape *>(c.create());

    QVERIFY(obj != nullptr);
    QVERIFY(obj->rendererType() == QQuickShape::UnknownRenderer);
    QVERIFY(!obj->asynchronous());
    QVERIFY(!obj->vendorExtensionsEnabled());
    QVERIFY(obj->status() == QQuickShape::Null);
    auto vps = obj->data();
    QVERIFY(vps.count(&vps) == 2);

    QQuickShapePath *vp = qobject_cast<QQuickShapePath *>(vps.at(&vps, 0));
    QVERIFY(vp != nullptr);
    QQmlListReference pathList(vp, "pathElements");
    QCOMPARE(pathList.count(), 0);
    QCOMPARE(vp->strokeColor(), QColor(Qt::white));
    QCOMPARE(vp->strokeWidth(), 1.0f);
    QCOMPARE(vp->fillColor(), QColor(Qt::white));
    QCOMPARE(vp->fillRule(), QQuickShapePath::OddEvenFill);
    QCOMPARE(vp->joinStyle(), QQuickShapePath::BevelJoin);
    QCOMPARE(vp->miterLimit(), 2);
    QCOMPARE(vp->capStyle(), QQuickShapePath::SquareCap);
    QCOMPARE(vp->strokeStyle(), QQuickShapePath::SolidLine);
    QCOMPARE(vp->dashOffset(), 0.0f);
    QCOMPARE(vp->dashPattern(), QVector<qreal>() << 4 << 2);
    QVERIFY(!vp->fillGradient());

    delete obj;
}

void tst_QQuickShape::basicShape()
{
    QScopedPointer<QQuickView> window(createView());

    window->setSource(testFileUrl("pathitem3.qml"));
    qApp->processEvents();

    QQuickShape *obj = findItem<QQuickShape>(window->rootObject(), "pathItem");
    QVERIFY(obj != nullptr);
    QQmlListReference list(obj, "data");
    QCOMPARE(list.count(), 1);
    QQuickShapePath *vp = qobject_cast<QQuickShapePath *>(list.at(0));
    QVERIFY(vp != nullptr);
    QCOMPARE(vp->strokeWidth(), 4.0f);
    QVERIFY(vp->fillGradient() != nullptr);
    QCOMPARE(vp->strokeStyle(), QQuickShapePath::DashLine);

    vp->setStrokeWidth(5.0f);
    QCOMPARE(vp->strokeWidth(), 5.0f);

    QQuickShapeLinearGradient *lgrad = qobject_cast<QQuickShapeLinearGradient *>(vp->fillGradient());
    QVERIFY(lgrad != nullptr);
    QCOMPARE(lgrad->spread(), QQuickShapeGradient::PadSpread);
    QCOMPARE(lgrad->x1(), 20.0f);
    QQmlListReference stopList(lgrad, "stops");
    QCOMPARE(stopList.count(), 5);
    QVERIFY(stopList.at(2) != nullptr);

    QQuickPath *path = vp;
    QCOMPARE(path->startX(), 20.0f);
    QQmlListReference pathList(path, "pathElements");
    QCOMPARE(pathList.count(), 3);
}

void tst_QQuickShape::changeSignals()
{
    QScopedPointer<QQuickView> window(createView());

    window->setSource(testFileUrl("pathitem3.qml"));
    qApp->processEvents();

    QQuickShape *obj = findItem<QQuickShape>(window->rootObject(), "pathItem");
    QVERIFY(obj != nullptr);

    QSignalSpy asyncPropSpy(obj, SIGNAL(asynchronousChanged()));
    obj->setAsynchronous(true);
    obj->setAsynchronous(false);
    QCOMPARE(asyncPropSpy.count(), 2);

    QQmlListReference list(obj, "data");
    QQuickShapePath *vp = qobject_cast<QQuickShapePath *>(list.at(0));
    QVERIFY(vp != nullptr);

    // Verify that VisualPath property changes emit shapePathChanged().
    QSignalSpy vpChangeSpy(vp, SIGNAL(shapePathChanged()));
    QSignalSpy strokeColorPropSpy(vp, SIGNAL(strokeColorChanged()));
    vp->setStrokeColor(Qt::blue);
    vp->setStrokeWidth(1.0f);
    QQuickShapeGradient *g = vp->fillGradient();
    vp->setFillGradient(nullptr);
    vp->setFillColor(Qt::yellow);
    vp->setFillRule(QQuickShapePath::WindingFill);
    vp->setJoinStyle(QQuickShapePath::MiterJoin);
    vp->setMiterLimit(5);
    vp->setCapStyle(QQuickShapePath::RoundCap);
    vp->setDashOffset(10);
    vp->setDashPattern(QVector<qreal>() << 1 << 2 << 3 << 4);
    QCOMPARE(strokeColorPropSpy.count(), 1);
    QCOMPARE(vpChangeSpy.count(), 10);

    // Verify that property changes from Path and its elements bubble up and result in shapePathChanged().
    QQuickPath *path = vp;
    path->setStartX(30);
    QCOMPARE(vpChangeSpy.count(), 11);
    QQmlListReference pathList(path, "pathElements");
    qobject_cast<QQuickPathLine *>(pathList.at(1))->setY(200);
    QCOMPARE(vpChangeSpy.count(), 12);

    // Verify that property changes from the gradient bubble up and result in shapePathChanged().
    vp->setFillGradient(g);
    QCOMPARE(vpChangeSpy.count(), 13);
    QQuickShapeLinearGradient *lgrad = qobject_cast<QQuickShapeLinearGradient *>(g);
    lgrad->setX2(200);
    QCOMPARE(vpChangeSpy.count(), 14);
    QQmlListReference stopList(lgrad, "stops");
    QCOMPARE(stopList.count(), 5);
    qobject_cast<QQuickGradientStop *>(stopList.at(1))->setPosition(0.3);
    QCOMPARE(vpChangeSpy.count(), 15);
    qobject_cast<QQuickGradientStop *>(stopList.at(1))->setColor(Qt::black);
    QCOMPARE(vpChangeSpy.count(), 16);
}

void tst_QQuickShape::render()
{
    QScopedPointer<QQuickView> window(createView());

    window->setSource(testFileUrl("pathitem3.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.data()));

    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QEXPECT_FAIL("", "Failure due to grabWindow not functional on offscreen/minimimal platforms", Abort);

    QImage img = window->grabWindow();
    QVERIFY(!img.isNull());

    QImage refImg(testFileUrl("pathitem3.png").toLocalFile());
    QVERIFY(!refImg.isNull());

    QString errorMessage;
    const QImage actualImg = img.convertToFormat(refImg.format());
    QVERIFY2(QQuickVisualTestUtil::compareImages(actualImg, refImg, &errorMessage),
             qPrintable(errorMessage));
}

void tst_QQuickShape::renderWithMultipleSp()
{
    QScopedPointer<QQuickView> window(createView());

    window->setSource(testFileUrl("pathitem4.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.data()));

    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QEXPECT_FAIL("", "Failure due to grabWindow not functional on offscreen/minimimal platforms", Abort);

    QImage img = window->grabWindow();
    QVERIFY(!img.isNull());

    QImage refImg(testFileUrl("pathitem4.png").toLocalFile());
    QVERIFY(!refImg.isNull());

    QString errorMessage;
    const QImage actualImg = img.convertToFormat(refImg.format());
    QVERIFY2(QQuickVisualTestUtil::compareImages(actualImg, refImg, &errorMessage),
             qPrintable(errorMessage));
}

void tst_QQuickShape::radialGrad()
{
    QScopedPointer<QQuickView> window(createView());

    window->setSource(testFileUrl("pathitem5.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.data()));

    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QEXPECT_FAIL("", "Failure due to grabWindow not functional on offscreen/minimimal platforms", Abort);

    QImage img = window->grabWindow();
    QVERIFY(!img.isNull());

    QImage refImg(testFileUrl("pathitem5.png").toLocalFile());
    QVERIFY(!refImg.isNull());

    QString errorMessage;
    const QImage actualImg = img.convertToFormat(refImg.format());
    QVERIFY2(QQuickVisualTestUtil::compareImages(actualImg, refImg, &errorMessage),
             qPrintable(errorMessage));
}

void tst_QQuickShape::conicalGrad()
{
    QScopedPointer<QQuickView> window(createView());

    window->setSource(testFileUrl("pathitem6.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window.data()));

    if ((QGuiApplication::platformName() == QLatin1String("offscreen"))
        || (QGuiApplication::platformName() == QLatin1String("minimal")))
        QEXPECT_FAIL("", "Failure due to grabWindow not functional on offscreen/minimimal platforms", Abort);

    QImage img = window->grabWindow();
    QVERIFY(!img.isNull());

    QImage refImg(testFileUrl("pathitem6.png").toLocalFile());
    QVERIFY(!refImg.isNull());

    QString errorMessage;
    const QImage actualImg = img.convertToFormat(refImg.format());
    QVERIFY2(QQuickVisualTestUtil::compareImages(actualImg, refImg, &errorMessage),
             qPrintable(errorMessage));
}

QTEST_MAIN(tst_QQuickShape)

#include "tst_qquickshape.moc"
