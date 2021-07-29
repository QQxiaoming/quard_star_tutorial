/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the MacJp2 plugin in the Qt ImageFormats module.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QGuiApplication>
#include <QBuffer>
#include <QImageIOHandler>
#include <QImage>

#include "qiiofhelpers_p.h"


QT_BEGIN_NAMESPACE

// Callbacks for sequential data provider & consumer:

static size_t cbGetBytes(void *info, void *buffer, size_t count)
{
    QIODevice *dev = static_cast<QIODevice *>(info);
    if (!dev || !buffer)
        return 0;
    qint64 res = dev->read(static_cast<char *>(buffer), qint64(count));
    return size_t(qMax(qint64(0), res));
}

static off_t cbSkipForward(void *info, off_t count)
{
    QIODevice *dev = static_cast<QIODevice *>(info);
    if (!dev || count <= 0)
        return 0;
    qint64 res = 0;
    if (!dev->isSequential()) {
        qint64 prevPos = dev->pos();
        dev->seek(prevPos + count);
        res = dev->pos() - prevPos;
    } else {
        char *buf = new char[quint64(count)];
        res = dev->read(buf, count);
        delete[] buf;
    }
    return qMax(qint64(0), res);
}

static void cbRewind(void *)
{
    // Ignore this; we do not want the Qt device to be rewound after reading the image
}

static size_t cbPutBytes(void *info, const void *buffer, size_t count)
{
    QIODevice *dev = static_cast<QIODevice *>(info);
    if (!dev || !buffer)
        return 0;
    qint64 res = dev->write(static_cast<const char *>(buffer), qint64(count));
    return size_t(qMax(qint64(0), res));
}


// QImage <-> CGImage conversion functions from QtGui on darwin
CGImageRef qt_mac_toCGImage(const QImage &qImage);
QImage qt_mac_toQImage(CGImageRef image);

QImageIOPlugin::Capabilities QIIOFHelpers::systemCapabilities(const QString &uti)
{
    QImageIOPlugin::Capabilities res;
    QCFString cfUti(uti);

    QCFType<CFArrayRef> cfSourceTypes = CGImageSourceCopyTypeIdentifiers();
    CFIndex len = CFArrayGetCount(cfSourceTypes);
    if (CFArrayContainsValue(cfSourceTypes, CFRangeMake(0, len), cfUti))
        res |= QImageIOPlugin::CanRead;

    QCFType<CFArrayRef> cfDestTypes = CGImageDestinationCopyTypeIdentifiers();
    len = CFArrayGetCount(cfDestTypes);
    if (CFArrayContainsValue(cfDestTypes, CFRangeMake(0, len), cfUti))
        res |= QImageIOPlugin::CanWrite;

    return res;
}

bool QIIOFHelpers::readImage(QImageIOHandler *q_ptr, QImage *out)
{
    QIIOFHelper h(q_ptr);
    return h.readImage(out);
}

bool QIIOFHelpers::writeImage(QImageIOHandler *q_ptr, const QImage &in, const QString &uti)
{
    QIIOFHelper h(q_ptr);
    return h.writeImage(in, uti);
}

QIIOFHelper::QIIOFHelper(QImageIOHandler *q)
    : q_ptr(q)
{
}

bool QIIOFHelper::initRead()
{
    static const CGDataProviderSequentialCallbacks cgCallbacks = { 0, &cbGetBytes, &cbSkipForward, &cbRewind, nullptr };

    if (cgImageSource)
        return true;
    if (!q_ptr || !q_ptr->device())
        return false;

    if (QBuffer *b = qobject_cast<QBuffer *>(q_ptr->device())) {
        // do direct access to avoid data copy
        const void *rawData = b->data().constData() + b->pos();
        cgDataProvider = CGDataProviderCreateWithData(nullptr, rawData, size_t(b->data().size() - b->pos()), nullptr);
    } else {
        cgDataProvider = CGDataProviderCreateSequential(q_ptr->device(), &cgCallbacks);
    }

    cgImageSource = CGImageSourceCreateWithDataProvider(cgDataProvider, nullptr);

    if (cgImageSource)
        cfImageDict = CGImageSourceCopyPropertiesAtIndex(cgImageSource, 0, nullptr);

    return (cgImageSource);
}

bool QIIOFHelper::readImage(QImage *out)
{
    if (!out || !initRead())
        return false;

    QCFType<CGImageRef> cgImage = CGImageSourceCreateImageAtIndex(cgImageSource, 0, nullptr);
    if (!cgImage)
        return false;

    *out = qt_mac_toQImage(cgImage);
    if (out->isNull())
        return false;

    int dpi = 0;
    if (getIntProperty(kCGImagePropertyDPIWidth, &dpi))
        out->setDotsPerMeterX(qRound(dpi / 0.0254f));
    if (getIntProperty(kCGImagePropertyDPIHeight, &dpi))
        out->setDotsPerMeterY(qRound(dpi / 0.0254f));

    return true;
}

bool QIIOFHelper::getIntProperty(CFStringRef property, int *value)
{
    if (!cfImageDict)
        return false;

    CFNumberRef cfNumber = static_cast<CFNumberRef>(CFDictionaryGetValue(cfImageDict, property));
    if (cfNumber) {
        int intVal;
        if (CFNumberGetValue(cfNumber, kCFNumberIntType, &intVal)) {
            if (value)
                *value = intVal;
            return true;
        }
    }
    return false;
}

static QImageIOHandler::Transformations exif2Qt(int exifOrientation)
{
    switch (exifOrientation) {
    case 1: // normal
        return QImageIOHandler::TransformationNone;
    case 2: // mirror horizontal
        return QImageIOHandler::TransformationMirror;
    case 3: // rotate 180
        return QImageIOHandler::TransformationRotate180;
    case 4: // mirror vertical
        return QImageIOHandler::TransformationFlip;
    case 5: // mirror horizontal and rotate 270 CW
        return QImageIOHandler::TransformationFlipAndRotate90;
    case 6: // rotate 90 CW
        return QImageIOHandler::TransformationRotate90;
    case 7: // mirror horizontal and rotate 90 CW
        return QImageIOHandler::TransformationMirrorAndRotate90;
    case 8: // rotate 270 CW
        return QImageIOHandler::TransformationRotate270;
    }
    return QImageIOHandler::TransformationNone;
}

static int qt2Exif(QImageIOHandler::Transformations transformation)
{
    switch (transformation) {
    case QImageIOHandler::TransformationNone:
        return 1;
    case QImageIOHandler::TransformationMirror:
        return 2;
    case QImageIOHandler::TransformationRotate180:
        return 3;
    case QImageIOHandler::TransformationFlip:
        return 4;
    case QImageIOHandler::TransformationFlipAndRotate90:
        return 5;
    case QImageIOHandler::TransformationRotate90:
        return 6;
    case QImageIOHandler::TransformationMirrorAndRotate90:
        return 7;
    case QImageIOHandler::TransformationRotate270:
        return 8;
    }
    qWarning("Invalid Qt image transformation");
    return 1;
}

QVariant QIIOFHelper::imageProperty(QImageIOHandler::ImageOption option)
{
    if (!initRead())
        return QVariant();

    switch (option) {
    case QImageIOHandler::Size: {
        QSize sz;
        if (getIntProperty(kCGImagePropertyPixelWidth, &sz.rwidth())
                && getIntProperty(kCGImagePropertyPixelHeight, &sz.rheight())) {
            return sz;
        }
        break;
    }
    case QImageIOHandler::ImageTransformation: {
        int orient;
        if (getIntProperty(kCGImagePropertyOrientation, &orient))
            return int(exif2Qt(orient));
        break;
    }
    default:
        break;
    }

    return QVariant();
}

void QIIOFHelper::setOption(QImageIOHandler::ImageOption option, const QVariant &value)
{
    if (writeOptions.size() < option + 1)
        writeOptions.resize(option + 1);
    writeOptions[option] = value;
}

bool QIIOFHelper::writeImage(const QImage &in, const QString &uti)
{
    static const CGDataConsumerCallbacks cgCallbacks = { &cbPutBytes, nullptr };

    if (!q_ptr || !q_ptr->device() || in.isNull())
        return false;

    QCFType<CGImageRef> cgImage = qt_mac_toCGImage(in);
    QCFType<CGDataConsumerRef> cgDataConsumer = CGDataConsumerCreate(q_ptr->device(), &cgCallbacks);
    QCFType<CFStringRef> cfUti = uti.toCFString();
    QCFType<CGImageDestinationRef> cgImageDest = CGImageDestinationCreateWithDataConsumer(cgDataConsumer, cfUti, 1, nullptr);
    if (!cgImageDest || !cgImage)
        return false;

    QCFType<CFNumberRef> cfQuality = nullptr;
    QCFType<CFNumberRef> cfOrientation = nullptr;
    const void *dictKeys[2];
    const void *dictVals[2];
    int dictSize = 0;

    if (q_ptr->supportsOption(QImageIOHandler::Quality)) {
        bool ok = false;
        int writeQuality = writeOptions.value(QImageIOHandler::Quality).toInt(&ok);
        // If quality is unset, default to 75%
        float quality = (ok && writeQuality >= 0 ? (qMin(writeQuality, 100)) : 75) / 100.0f;
        cfQuality = CFNumberCreate(nullptr, kCFNumberFloatType, &quality);
        dictKeys[dictSize] = static_cast<const void *>(kCGImageDestinationLossyCompressionQuality);
        dictVals[dictSize] = static_cast<const void *>(cfQuality);
        dictSize++;
    }
    if (q_ptr->supportsOption(QImageIOHandler::ImageTransformation)) {
        int orient = qt2Exif(static_cast<QImageIOHandler::Transformation>(writeOptions.value(QImageIOHandler::ImageTransformation).toInt()));
        cfOrientation = CFNumberCreate(nullptr, kCFNumberIntType, &orient);
        dictKeys[dictSize] = static_cast<const void *>(kCGImagePropertyOrientation);
        dictVals[dictSize] = static_cast<const void *>(cfOrientation);
        dictSize++;
    }

    QCFType<CFDictionaryRef> cfProps = nullptr;
    if (dictSize)
        cfProps = CFDictionaryCreate(nullptr, dictKeys, dictVals, dictSize,
                                     &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CGImageDestinationAddImage(cgImageDest, cgImage, cfProps);
    return CGImageDestinationFinalize(cgImageDest);
}

QT_END_NAMESPACE
