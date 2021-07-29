/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the autotests in the Qt ImageFormats module.
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
#include <QtGui/QtGui>

class tst_qheif: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void readImage_data();
    void readImage();
    void readProperties_data();
    void readProperties();
    void writeImage();
};

void tst_qheif::initTestCase()
{
    if (!QImageReader::supportedImageFormats().contains("heic"))
        QSKIP("The image format handler is not installed.");
}

void tst_qheif::readImage_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QSize>("size");
    QTest::addColumn<int>("transform");

    QTest::newRow("col") << QString("col320x480.heic") << QSize(320, 480) << int(QImageIOHandler::TransformationNone);
    QTest::newRow("rot") << QString("newlogoCCW.heic") << QSize(110, 78) << int(QImageIOHandler::TransformationRotate90);
}

void tst_qheif::readImage()
{
    QFETCH(QString, fileName);
    QFETCH(QSize, size);

    QString path = QStringLiteral(":/heif/") + fileName;
    QImageReader reader(path);
    QVERIFY(reader.canRead());
    QImage image = reader.read();
    QVERIFY(!image.isNull());
    QCOMPARE(image.size(), size);
}

void tst_qheif::readProperties_data()
{
    readImage_data();
}

void tst_qheif::readProperties()
{
    QFETCH(QString, fileName);
    QFETCH(QSize, size);
    QFETCH(int, transform);

    QSize rawSize = (transform & QImageIOHandler::TransformationRotate90) ? size.transposed() : size;

    QString path = QStringLiteral(":/heif/") + fileName;
    QImageReader reader(path);
    QCOMPARE(reader.size(), rawSize);
    QCOMPARE(int(reader.transformation()), transform);

    QImage image = reader.read();
    QCOMPARE(image.size(), size);

    QCOMPARE(reader.size(), rawSize);
    QCOMPARE(int(reader.transformation()), transform);
}

void tst_qheif::writeImage()
{
    QImage img(20, 10, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::green);

    QBuffer buf1, buf2;
    QImage rimg1;

    {
        buf1.open(QIODevice::WriteOnly);
        QImageWriter writer(&buf1, "heic");
        QVERIFY(writer.write(img));
        buf1.close();
        QVERIFY(buf1.size() > 0);

        buf1.open(QIODevice::ReadOnly);
        QImageReader reader(&buf1);
        QVERIFY(reader.read(&rimg1));
        buf1.close();
        QVERIFY(rimg1.size() == img.size());
    }

    {
        buf2.open(QIODevice::WriteOnly);
        QImageWriter writer(&buf2, "heic");
        writer.setQuality(20);
        QVERIFY(writer.write(img));
        buf2.close();
        QVERIFY(buf2.size() > 0);
        QVERIFY(buf2.size() < buf1.size());
    }

    {
        buf2.open(QIODevice::WriteOnly);
        QImageWriter writer(&buf2, "heic");
        writer.setTransformation(QImageIOHandler::TransformationRotate270);
        QVERIFY(writer.write(img));
        buf2.close();

        QImage rimg2;
        buf2.open(QIODevice::ReadOnly);
        QImageReader reader(&buf2);
        QVERIFY(reader.read(&rimg2));
        buf2.close();
        QVERIFY(rimg2.size() == img.size().transposed());
    }
}

QTEST_MAIN(tst_qheif)
#include "tst_qheif.moc"
