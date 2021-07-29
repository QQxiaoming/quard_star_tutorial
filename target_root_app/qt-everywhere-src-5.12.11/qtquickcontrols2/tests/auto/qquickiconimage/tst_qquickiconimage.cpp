/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <qtest.h>
#include <QtTest/qsignalspy.h>

#include <QtCore/qmath.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlfileselector.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickimageprovider.h>
#include <QtQuick/qquickitemgrabresult.h>
#include <QtQuick/private/qquickimage_p.h>
#include <QtQuickControls2/private/qquickiconimage_p.h>

#include "../shared/util.h"
#include "../shared/visualtestutil.h"

using namespace QQuickVisualTestUtil;

class tst_qquickiconimage : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_qquickiconimage();

private slots:
    void initTestCase();
    void defaults();
    void nameBindingSourceSize();
    void nameBindingSourceSizeWidthHeight();
    void nameBindingNoSizes();
    void sourceBindingNoSizes();
    void sourceBindingSourceSize();
    void sourceBindingSourceSizeWidthHeight();
    void sourceBindingSourceTooLarge();
    void changeSourceSize();
    void alignment_data();
    void alignment();
    void svgNoSizes();
    void svgSourceBindingSourceSize();
    void color();
    void fileSelectors();
    void imageProvider();
    void translucentColors();

private:
    void setTheme();

    qreal dpr;
    int integerDpr;
};

static QImage grabItemToImage(QQuickItem *item)
{
    QSharedPointer<QQuickItemGrabResult> result = item->grabToImage();
    QSignalSpy spy(result.data(), SIGNAL(ready()));
    spy.wait();
    return result->image();
}

#define SKIP_IF_DPR_TOO_HIGH() \
    if (dpr > 2) \
        QSKIP("Test does not support device pixel ratio greater than 2")

tst_qquickiconimage::tst_qquickiconimage() :
    dpr(qGuiApp->devicePixelRatio()),
    integerDpr(qCeil(dpr))
{
}

void tst_qquickiconimage::initTestCase()
{
    QQmlDataTest::initTestCase();
    QIcon::setThemeName(QStringLiteral("testtheme"));
}

void tst_qquickiconimage::defaults()
{
    QQuickIconImage iconImage;
    QCOMPARE(iconImage.fillMode(), QQuickImage::Pad);
    QCOMPARE(iconImage.name(), QString());
    QCOMPARE(iconImage.source(), QUrl());
    QCOMPARE(iconImage.color(), QColor(Qt::transparent));
}

void tst_qquickiconimage::nameBindingSourceSize()
{
    // We can't have images for every DPR.
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("nameBindingSourceSize.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickItem *image = view.rootObject()->childItems().at(1);
    QVERIFY(image);

    QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));
    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 22.0);
    QCOMPARE(iconImage->height(), 22.0);

    // The requested width of 16 is less than the pixmap's size on disk which
    // is 22x22. Our default fillMode, Pad, would result in the image being clipped,
    // so instead we change the fillMode to PreserveAspectFit. Doing so causes
    // QQuickImage::updatePaintedGeometry() to set our implicit size to 22x16 to
    // ensure that the aspect ratio is respected. Since we have no explicit height,
    // the height (previously 22) becomes the implicit height (16).
    iconImage->setWidth(16.0);
    QCOMPARE(iconImage->fillMode(), QQuickImage::PreserveAspectFit);
    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 16.0);
    QCOMPARE(iconImage->width(), 16.0);
    QCOMPARE(iconImage->height(), 16.0);
}

void tst_qquickiconimage::nameBindingSourceSizeWidthHeight()
{
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("nameBindingSourceSizeWidthHeight.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject());
    QVERIFY(iconImage);
    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 16.0);
    QCOMPARE(iconImage->height(), 16.0);
}

void tst_qquickiconimage::nameBindingNoSizes()
{
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("nameBindingNoSizes.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject());
    QVERIFY(iconImage);
    // The smallest available size will be chosen.
    QCOMPARE(iconImage->sourceSize().width(), 16);
    QCOMPARE(iconImage->sourceSize().height(), 16);
    QCOMPARE(iconImage->implicitWidth(), 16.0);
    QCOMPARE(iconImage->implicitHeight(), 16.0);
    QCOMPARE(iconImage->width(), 16.0);
    QCOMPARE(iconImage->height(), 16.0);
}

void tst_qquickiconimage::sourceBindingNoSizes()
{
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("sourceBindingNoSizes.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickItem *image = view.rootObject()->childItems().at(1);
    QVERIFY(image);

    QCOMPARE(iconImage->sourceSize().width(), 22 * integerDpr);
    QCOMPARE(iconImage->sourceSize().height(), 22 * integerDpr);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 22.0);
    QCOMPARE(iconImage->height(), 22.0);
    QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));
}

void tst_qquickiconimage::sourceBindingSourceSize()
{
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("sourceBindingSourceSize.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickItem *image = view.rootObject()->childItems().at(1);
    QVERIFY(image);

    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 22.0);
    QCOMPARE(iconImage->height(), 22.0);
    QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));

    // Changing width and height should not affect sourceSize.
    iconImage->setWidth(50);
    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    iconImage->setHeight(50);
    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
}

void tst_qquickiconimage::sourceBindingSourceSizeWidthHeight()
{
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("sourceBindingSourceSizeWidthHeight.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject());
    QVERIFY(iconImage);
    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 16.0);
    QCOMPARE(iconImage->height(), 16.0);
}

void tst_qquickiconimage::sourceBindingSourceTooLarge()
{
    SKIP_IF_DPR_TOO_HIGH();

    QQuickView view(testFileUrl("sourceBindingSourceTooLarge.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject());
    QVERIFY(iconImage);
    QCOMPARE(iconImage->sourceSize().width(), 32);
    QCOMPARE(iconImage->sourceSize().height(), 32);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 22.0);
    QCOMPARE(iconImage->height(), 22.0);
}

void tst_qquickiconimage::alignment_data()
{
    QTest::addColumn<QQuickImage::HAlignment>("horizontalAlignment");
    QTest::addColumn<QQuickImage::VAlignment>("verticalAlignment");

    QTest::newRow("AlignLeft,AlignTop") << QQuickImage::AlignLeft << QQuickImage::AlignTop;
    QTest::newRow("AlignLeft,AlignVCenter") << QQuickImage::AlignLeft << QQuickImage::AlignVCenter;
    QTest::newRow("AlignLeft,AlignBottom") << QQuickImage::AlignLeft << QQuickImage::AlignBottom;
    QTest::newRow("AlignHCenter,AlignTop") << QQuickImage::AlignHCenter << QQuickImage::AlignTop;
    QTest::newRow("AlignHCenter,AlignVCenter") << QQuickImage::AlignHCenter << QQuickImage::AlignVCenter;
    QTest::newRow("AlignHCenter,AlignBottom") << QQuickImage::AlignHCenter << QQuickImage::AlignBottom;
    QTest::newRow("AlignRight,AlignTop") << QQuickImage::AlignRight << QQuickImage::AlignTop;
    QTest::newRow("AlignRight,AlignVCenter") << QQuickImage::AlignRight << QQuickImage::AlignVCenter;
    QTest::newRow("AlignRight,AlignBottom") << QQuickImage::AlignRight << QQuickImage::AlignBottom;
}

void tst_qquickiconimage::alignment()
{
    SKIP_IF_DPR_TOO_HIGH();

    QFETCH(QQuickImage::HAlignment, horizontalAlignment);
    QFETCH(QQuickImage::VAlignment, verticalAlignment);

    QQuickView view(testFileUrl("alignment.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickImage *image = qobject_cast<QQuickImage*>(view.rootObject()->childItems().at(1));
    QVERIFY(image);

    // The default fillMode for IconImage is Image::Pad, so these two grabs
    // should only be equal when the device pixel ratio is 1 or 2, as there is no
    // @3x version of the image, and hence the Image will be upscaled
    // and therefore blurry when the ratio is higher than 2.
    if (qGuiApp->devicePixelRatio() <= 2)
        QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));
    else
        QVERIFY(grabItemToImage(iconImage) != grabItemToImage(image));

    // Check that the images are what we expect in different alignment configurations.
    iconImage->setWidth(200);
    iconImage->setHeight(100);
    iconImage->setHorizontalAlignment(horizontalAlignment);
    iconImage->setVerticalAlignment(verticalAlignment);
    iconImage->setFillMode(QQuickImage::Pad);
    image->setWidth(200);
    image->setHeight(100);
    image->setHorizontalAlignment(horizontalAlignment);
    image->setVerticalAlignment(verticalAlignment);
    image->setFillMode(QQuickImage::Pad);

    if (qGuiApp->devicePixelRatio() <= 2)
        QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));
    else
        QVERIFY(grabItemToImage(iconImage) != grabItemToImage(image));
}

void tst_qquickiconimage::svgNoSizes()
{
#ifndef QT_SVG_LIB
    QSKIP("This test requires qtsvg");
#else
    QQuickView view(testFileUrl("svgNoSizes.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickImage *image = qobject_cast<QQuickImage*>(view.rootObject()->childItems().at(1));
    QVERIFY(image);

    QCOMPARE(iconImage->sourceSize().width(), 48);
    QCOMPARE(iconImage->sourceSize().height(), 48);
    QCOMPARE(iconImage->implicitWidth(), 48.0);
    QCOMPARE(iconImage->implicitHeight(), 48.0);
    QCOMPARE(iconImage->width(), 48.0);
    QCOMPARE(iconImage->height(), 48.0);
    QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));
#endif
}

void tst_qquickiconimage::svgSourceBindingSourceSize()
{
#ifndef QT_SVG_LIB
    QSKIP("This test requires qtsvg");
#else
    QQuickView view(testFileUrl("alignment.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickImage *image = qobject_cast<QQuickImage*>(view.rootObject()->childItems().at(1));
    QVERIFY(image);

    QCOMPARE(iconImage->sourceSize().width(), 22);
    QCOMPARE(iconImage->sourceSize().height(), 22);
    QCOMPARE(iconImage->implicitWidth(), 22.0);
    QCOMPARE(iconImage->implicitHeight(), 22.0);
    QCOMPARE(iconImage->width(), 22.0);
    QCOMPARE(iconImage->height(), 22.0);
    QCOMPARE(grabItemToImage(iconImage), grabItemToImage(image));
#endif
}

void tst_qquickiconimage::color()
{
    SKIP_IF_DPR_TOO_HIGH();

    if (QGuiApplication::platformName() == QLatin1String("offscreen"))
        QSKIP("grabToImage() doesn't work on the \"offscreen\" platform plugin (QTBUG-63185)");

    QQuickView view(testFileUrl("color.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickImage *image = qobject_cast<QQuickImage*>(view.rootObject()->childItems().at(1));
    QVERIFY(image);

    QImage iconImageWindowGrab = grabItemToImage(iconImage);
    QCOMPARE(iconImageWindowGrab, grabItemToImage(image));

    // Transparent pixels should remain transparent.
    QCOMPARE(iconImageWindowGrab.pixelColor(0, 0), QColor(0, 0, 0, 0));

    // Set a color after component completion.
    iconImage->setColor(QColor(Qt::green));
    iconImageWindowGrab = grabItemToImage(iconImage);
    const QPoint centerPixelPos(11, 11);
    QCOMPARE(iconImageWindowGrab.pixelColor(centerPixelPos), QColor(Qt::green));

    // Set a semi-transparent color after component completion.
    iconImage->setColor(QColor(0, 0, 255, 127));
    iconImageWindowGrab = grabItemToImage(iconImage);
    QCOMPARE(iconImageWindowGrab.pixelColor(centerPixelPos).red(), 0);
    QCOMPARE(iconImageWindowGrab.pixelColor(centerPixelPos).green(), 0);
    QCOMPARE(iconImageWindowGrab.pixelColor(centerPixelPos).blue(), 255);
    QCOMPARE(iconImageWindowGrab.pixelColor(centerPixelPos).alpha(), 127);
}

void tst_qquickiconimage::changeSourceSize()
{
    QQuickView view(testFileUrl("sourceBindingSourceSize.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    // Ensure that there isn't any infinite recursion when trying to change the sourceSize.
    QSize sourceSize = iconImage->sourceSize();
    sourceSize.setWidth(sourceSize.width() - 1);
    iconImage->setSourceSize(sourceSize);
}


void tst_qquickiconimage::fileSelectors()
{
    SKIP_IF_DPR_TOO_HIGH();

    if (QGuiApplication::platformName() == QLatin1String("offscreen"))
        QSKIP("grabToImage() doesn't work on the \"offscreen\" platform plugin (QTBUG-63185)");

    QQuickView view;
    QQmlFileSelector* fileSelector = new QQmlFileSelector(view.engine());
    fileSelector->setExtraSelectors(QStringList() << "testselector");
    view.setSource(testFileUrl("fileSelectors.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->childItems().at(0));
    QVERIFY(iconImage);

    QQuickItem *image = view.rootObject()->childItems().at(1);
    QVERIFY(image);

    QImage iconImageWindowGrab = grabItemToImage(iconImage);
    QCOMPARE(iconImageWindowGrab, grabItemToImage(image));

    QCOMPARE(iconImageWindowGrab.pixelColor(iconImageWindowGrab.width() / 2, iconImageWindowGrab.height() / 2), QColor(Qt::blue));
}

class TestImageProvider : public QQuickImageProvider
{
public:
    TestImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) { }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        QSize defaultSize(32, 32);
        if (size)
            *size = defaultSize;

        QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : defaultSize.width(),
                       requestedSize.height() > 0 ? requestedSize.height() : defaultSize.height());
        pixmap.fill(QColor(id).rgba());
        return pixmap;
    }
};

// don't crash (QTBUG-63959)
void tst_qquickiconimage::imageProvider()
{
    if (QGuiApplication::platformName() == QLatin1String("offscreen"))
        QSKIP("grabToImage() doesn't work on the \"offscreen\" platform plugin (QTBUG-63185)");

    QQuickView view;
    view.engine()->addImageProvider("provider", new TestImageProvider);
    view.setSource(testFileUrl("imageProvider.qml"));
    QCOMPARE(view.status(), QQuickView::Ready);
    view.show();
    view.requestActivate();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(view.rootObject()->findChild<QQuickIconImage *>());
    QVERIFY(iconImage);

    QImage image = grabItemToImage(iconImage);
    QVERIFY(!image.isNull());
    QCOMPARE(image.pixelColor(image.width() / 2, image.height() / 2), QColor(Qt::red));
}

/*
    QQuickIconImage::componentComplete() calls QQuickIconImagePrivate::updateIcon(),
    which loads the icon's image via QQuickImageBase::load(). That eventually calls
    QQuickImageBase::requestFinished(), which calls QQuickIconImage::pixmapChange().
    That then calls QQuickIconImagePrivate::updateFillMode(), which can in turn
    cause QQuickIconImage::pixmapChange() to be called again, causing recursion.

    This was a problem because it resulted in icon.color being applied twice.

    This test checks that that doesn't happen.
*/
void tst_qquickiconimage::translucentColors()
{
    if (QGuiApplication::platformName() == QLatin1String("offscreen"))
        QSKIP("grabToImage() doesn't work on the \"offscreen\" platform plugin (QTBUG-63185)");

    // Doesn't reproduce with QQuickView.
    QQmlApplicationEngine engine;
    engine.load(testFileUrl("translucentColors.qml"));
    QQuickWindow *window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());

    QQuickIconImage *iconImage = qobject_cast<QQuickIconImage*>(window->findChild<QQuickIconImage*>());
    QVERIFY(iconImage);

    const QImage image = grabItemToImage(iconImage);
    QVERIFY(!image.isNull());
    QCOMPARE(image.pixelColor(image.width() / 2, image.height() / 2), QColor::fromRgba(0x80000000));
}

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    tst_qquickiconimage test;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&test, argc, argv);
}

#include "tst_qquickiconimage.moc"
