/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickninepatchimage_p.h"

#include <QtCore/qfileinfo.h>
#include <QtQuick/qsggeometry.h>
#include <QtQuick/qsgtexturematerial.h>
#include <QtQuick/private/qsgnode_p.h>
#include <QtQuick/private/qquickimage_p_p.h>

QT_BEGIN_NAMESPACE

struct QQuickNinePatchData
{
    QVector<qreal> coordsForSize(qreal count) const;

    inline bool isNull() const { return data.isEmpty(); }
    inline int count() const { return data.size(); }
    inline qreal at(int index) const { return data.at(index); }
    inline qreal size() const { return data.last(); }

    void fill(const QVector<qreal> &coords, qreal count);
    void clear();

private:
    bool inverted = false;
    QVector<qreal> data;
};

QVector<qreal> QQuickNinePatchData::coordsForSize(qreal size) const
{
    // n = number of stretchable sections
    // We have to compensate when adding 0 and/or
    // the source image width to the divs vector.
    const int l = data.size();
    const int n = (inverted ? l - 1 : l) / 2;
    const qreal stretch = (size - data.last()) / n;

    QVector<qreal> coords;
    coords.reserve(l);
    coords.append(0);

    bool stretched = !inverted;
    for (int i = 1; i < l; ++i) {
        qreal advance = data[i] - data[i - 1];
        if (stretched)
            advance += stretch;
        coords.append(coords.last() + advance);

        stretched = !stretched;
    }

    return coords;
}

void QQuickNinePatchData::fill(const QVector<qreal> &coords, qreal size)
{
    data.clear();
    inverted = coords.isEmpty() || coords.first() != 0;

    // Reserve an extra item in case we need to add the image width/height
    if (inverted) {
        data.reserve(coords.size() + 2);
        data.append(0);
    } else {
        data.reserve(coords.size() + 1);
    }

    data += coords;
    data.append(size);
}

void QQuickNinePatchData::clear()
{
    data.clear();
}

class QQuickNinePatchNode : public QSGGeometryNode
{
public:
    QQuickNinePatchNode();
    ~QQuickNinePatchNode();

    void initialize(QSGTexture *texture, const QSizeF &targetSize, const QSize &sourceSize,
                    const QQuickNinePatchData &xDivs, const QQuickNinePatchData &yDivs, qreal dpr);

private:
    QSGGeometry m_geometry;
    QSGTextureMaterial m_material;
};

QQuickNinePatchNode::QQuickNinePatchNode()
    : m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
{
    m_geometry.setDrawingMode(QSGGeometry::DrawTriangles);
    setGeometry(&m_geometry);
    setMaterial(&m_material);
}

QQuickNinePatchNode::~QQuickNinePatchNode()
{
    delete m_material.texture();
}

void QQuickNinePatchNode::initialize(QSGTexture *texture, const QSizeF &targetSize, const QSize &sourceSize,
                                     const QQuickNinePatchData &xDivs, const QQuickNinePatchData &yDivs, qreal dpr)
{
    delete m_material.texture();
    m_material.setTexture(texture);

    const int xlen = xDivs.count();
    const int ylen = yDivs.count();

    if (xlen > 0 && ylen > 0) {
        const int quads = (xlen - 1) * (ylen - 1);
        static const int verticesPerQuad = 6;
        m_geometry.allocate(xlen * ylen, verticesPerQuad * quads);

        QSGGeometry::TexturedPoint2D *vertices = m_geometry.vertexDataAsTexturedPoint2D();
        QVector<qreal> xCoords = xDivs.coordsForSize(targetSize.width());
        QVector<qreal> yCoords = yDivs.coordsForSize(targetSize.height());

        for (int y = 0; y < ylen; ++y) {
            for (int x = 0; x < xlen; ++x, ++vertices)
                vertices->set(xCoords[x] / dpr, yCoords[y] / dpr,
                              xDivs.at(x) / sourceSize.width(),
                              yDivs.at(y) / sourceSize.height());
        }

        quint16 *indices = m_geometry.indexDataAsUShort();
        int n = quads;
        for (int q = 0; n--; ++q) {
            if ((q + 1) % xlen == 0) // next row
                ++q;
            // Bottom-left half quad triangle
            indices[0] = q;
            indices[1] = q + xlen;
            indices[2] = q + xlen + 1;

            // Top-right half quad triangle
            indices[3] = q;
            indices[4] = q + xlen + 1;
            indices[5] = q + 1;

            indices += verticesPerQuad;
        }
    }

    markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
}

class QQuickNinePatchImagePrivate : public QQuickImagePrivate
{
    Q_DECLARE_PUBLIC(QQuickNinePatchImage)

public:
    void updatePatches();
    void updatePaddings(const QSizeF &size, const QVector<qreal> &horizontal, const QVector<qreal> &vertical);
    void updateInsets(const QVector<qreal> &horizontal, const QVector<qreal> &vertical);

    bool resetNode = false;
    qreal topPadding = 0;
    qreal leftPadding = 0;
    qreal rightPadding = 0;
    qreal bottomPadding = 0;
    qreal topInset = 0;
    qreal leftInset = 0;
    qreal rightInset = 0;
    qreal bottomInset = 0;

    QImage ninePatch;
    QQuickNinePatchData xDivs;
    QQuickNinePatchData yDivs;
};

static QVector<qreal> readCoords(const QRgb *data, int from, int count, int offset, QRgb color)
{
    int p1 = -1;
    QVector<qreal> coords;
    for (int i = 0; i < count; ++i) {
        int p2 = from + i * offset;
        if (data[p2] == color) {
            // colored pixel
            if (p1 == -1)
                p1 = i;
        } else {
            // empty pixel
            if (p1 != -1) {
                coords << p1 << i;
                p1 = -1;
            }
        }
    }
    return coords;
}

void QQuickNinePatchImagePrivate::updatePatches()
{
    if (ninePatch.isNull())
        return;

    int w = ninePatch.width();
    int h = ninePatch.height();
    const QRgb *data = reinterpret_cast<const QRgb *>(ninePatch.constBits());

    const QRgb black = qRgb(0,0,0);
    const QRgb red = qRgb(255,0,0);

    xDivs.fill(readCoords(data, 1, w - 1, 1, black), w - 2); // top left -> top right
    yDivs.fill(readCoords(data, w, h - 1, w, black), h - 2); // top left -> bottom left

    QVector<qreal> hInsets = readCoords(data, (h - 1) * w + 1, w - 1, 1, red); // bottom left -> bottom right
    QVector<qreal> vInsets = readCoords(data, 2 * w - 1, h - 1, w, red); // top right -> bottom right
    updateInsets(hInsets, vInsets);

    const QSizeF sz(w - leftInset - rightInset, h - topInset - bottomInset);
    QVector<qreal> hPaddings = readCoords(data, (h - 1) * w + leftInset + 1, sz.width() - 2, 1, black); // bottom left -> bottom right
    QVector<qreal> vPaddings = readCoords(data, (2 + topInset) * w - 1, sz.height() - 2, w, black); // top right -> bottom right
    updatePaddings(sz, hPaddings, vPaddings);
}

void QQuickNinePatchImagePrivate::updatePaddings(const QSizeF &size, const QVector<qreal> &horizontal, const QVector<qreal> &vertical)
{
    Q_Q(QQuickNinePatchImage);
    qreal oldTopPadding = topPadding;
    qreal oldLeftPadding = leftPadding;
    qreal oldRightPadding = rightPadding;
    qreal oldBottomPadding = bottomPadding;

    if (horizontal.count() >= 2) {
        leftPadding = horizontal.first();
        rightPadding = size.width() - horizontal.last() - 2;
    } else {
        leftPadding = 0;
        rightPadding = 0;
    }

    if (vertical.count() >= 2) {
        topPadding = vertical.first();
        bottomPadding = size.height() - vertical.last() - 2;
    } else {
        topPadding = 0;
        bottomPadding = 0;
    }

    if (!qFuzzyCompare(oldTopPadding, topPadding))
        emit q->topPaddingChanged();
    if (!qFuzzyCompare(oldBottomPadding, bottomPadding))
        emit q->bottomPaddingChanged();
    if (!qFuzzyCompare(oldLeftPadding, leftPadding))
        emit q->leftPaddingChanged();
    if (!qFuzzyCompare(oldRightPadding, rightPadding))
        emit q->rightPaddingChanged();
}

void QQuickNinePatchImagePrivate::updateInsets(const QVector<qreal> &horizontal, const QVector<qreal> &vertical)
{
    Q_Q(QQuickNinePatchImage);
    qreal oldTopInset = topInset;
    qreal oldLeftInset = leftInset;
    qreal oldRightInset = rightInset;
    qreal oldBottomInset = bottomInset;

    if (horizontal.count() >= 2 && horizontal.first() == 0)
        leftInset = horizontal.at(1);
    else
        leftInset = 0;

    if (horizontal.count() == 2 && horizontal.first() > 0)
        rightInset = horizontal.last() - horizontal.first();
    else if (horizontal.count() == 4)
        rightInset = horizontal.last() - horizontal.at(2);
    else
        rightInset = 0;

    if (vertical.count() >= 2 && vertical.first() == 0)
        topInset = vertical.at(1);
    else
        topInset = 0;

    if (vertical.count() == 2 && vertical.first() > 0)
        bottomInset = vertical.last() - vertical.first();
    else if (vertical.count() == 4)
        bottomInset = vertical.last() - vertical.at(2);
    else
        bottomInset = 0;

    if (!qFuzzyCompare(oldTopInset, topInset))
        emit q->topInsetChanged();
    if (!qFuzzyCompare(oldBottomInset, bottomInset))
        emit q->bottomInsetChanged();
    if (!qFuzzyCompare(oldLeftInset, leftInset))
        emit q->leftInsetChanged();
    if (!qFuzzyCompare(oldRightInset, rightInset))
        emit q->rightInsetChanged();
}

QQuickNinePatchImage::QQuickNinePatchImage(QQuickItem *parent)
    : QQuickImage(*(new QQuickNinePatchImagePrivate), parent)
{
}

qreal QQuickNinePatchImage::topPadding() const
{
    Q_D(const QQuickNinePatchImage);
    return d->topPadding / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::leftPadding() const
{
    Q_D(const QQuickNinePatchImage);
    return d->leftPadding / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::rightPadding() const
{
    Q_D(const QQuickNinePatchImage);
    return d->rightPadding / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::bottomPadding() const
{
    Q_D(const QQuickNinePatchImage);
    return d->bottomPadding / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::topInset() const
{
    Q_D(const QQuickNinePatchImage);
    return d->topInset / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::leftInset() const
{
    Q_D(const QQuickNinePatchImage);
    return d->leftInset / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::rightInset() const
{
    Q_D(const QQuickNinePatchImage);
    return d->rightInset / d->devicePixelRatio;
}

qreal QQuickNinePatchImage::bottomInset() const
{
    Q_D(const QQuickNinePatchImage);
    return d->bottomInset / d->devicePixelRatio;
}

void QQuickNinePatchImage::pixmapChange()
{
    Q_D(QQuickNinePatchImage);
    if (QFileInfo(d->url.fileName()).completeSuffix().toLower() == QLatin1String("9.png")) {
        d->resetNode = d->ninePatch.isNull();
        d->ninePatch = d->pix.image();
        if (d->ninePatch.depth() != 32)
            d->ninePatch = d->ninePatch.convertToFormat(QImage::Format_ARGB32);

        int w = d->ninePatch.width();
        int h = d->ninePatch.height();
        d->pix.setImage(QImage(d->ninePatch.constBits() + 4 * (w + 1), w - 2, h - 2, d->ninePatch.bytesPerLine(), d->ninePatch.format()));

        d->updatePatches();
    } else {
        d->resetNode = !d->ninePatch.isNull();
        d->ninePatch = QImage();
    }
    QQuickImage::pixmapChange();
}

QSGNode *QQuickNinePatchImage::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_D(QQuickNinePatchImage);
    Q_UNUSED(data);

    if (d->resetNode) {
        delete oldNode;
        oldNode = nullptr;
        d->resetNode = false;
    }

    QSizeF sz = size();
    QImage image = d->pix.image();
    if (!sz.isValid() || image.isNull()) {
        delete oldNode;
        return nullptr;
    }

    if (d->ninePatch.isNull())
        return QQuickImage::updatePaintNode(oldNode, data);

    QQuickNinePatchNode *patchNode = static_cast<QQuickNinePatchNode *>(oldNode);
    if (!patchNode)
        patchNode = new QQuickNinePatchNode;

#ifdef QSG_RUNTIME_DESCRIPTION
    qsgnode_set_description(patchNode, QString::fromLatin1("QQuickNinePatchImage: '%1'").arg(d->url.toString()));
#endif

    QSGTexture *texture = window()->createTextureFromImage(image);
    patchNode->initialize(texture, sz * d->devicePixelRatio, image.size(), d->xDivs, d->yDivs, d->devicePixelRatio);
    return patchNode;
}

QT_END_NAMESPACE
