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

#include "qquickiconimage_p.h"
#include "qquickiconimage_p_p.h"

#include <QtCore/qmath.h>
#include <QtQuick/private/qquickimagebase_p_p.h>

QT_BEGIN_NAMESPACE

bool QQuickIconImagePrivate::updateDevicePixelRatio(qreal targetDevicePixelRatio)
{
    if (isThemeIcon) {
        devicePixelRatio = calculateDevicePixelRatio();
        return true;
    }

    return QQuickImagePrivate::updateDevicePixelRatio(targetDevicePixelRatio);
}

void QQuickIconImagePrivate::updateIcon()
{
    Q_Q(QQuickIconImage);
    // Both geometryChanged() and QQuickImageBase::sourceSizeChanged()
    // (which we connect to updateIcon() in the constructor) can be called as a result
    // of updateIcon() changing the various sizes, so we must check that we're not recursing.
    if (updatingIcon)
        return;

    updatingIcon = true;

    QSize size = sourcesize;
    // If no size is specified for theme icons, it will use the smallest available size.
    if (size.width() <= 0)
        size.setWidth(q->width());
    if (size.height() <= 0)
        size.setHeight(q->height());

    const qreal dpr = calculateDevicePixelRatio();
    const QIconLoaderEngineEntry *entry = QIconLoaderEngine::entryForSize(icon, size * dpr, qCeil(dpr));

    if (entry) {
        QQmlContext *context = qmlContext(q);
        const QUrl entryUrl = QUrl::fromLocalFile(entry->filename);
        url = context ? context->resolvedUrl(entryUrl) : entryUrl;
        isThemeIcon = true;
    } else {
        url = source;
        isThemeIcon = false;
    }
    q->load();

    updatingIcon = false;
}

void QQuickIconImagePrivate::updateFillMode()
{
    Q_Q(QQuickIconImage);
    // If we start with a sourceSize of 28x28 and then set sourceSize.width to 24, the fillMode
    // will change to PreserveAspectFit (because pixmapSize.width() > width()), which causes the
    // pixmap to be reloaded at its original size of 28x28, which causes the fillMode to change
    // to Pad (because pixmapSize.width() <= width()), and so on.
    if (updatingFillMode)
        return;

    updatingFillMode = true;

    const QSize pixmapSize = QSize(pix.width(), pix.height()) / calculateDevicePixelRatio();
    if (pixmapSize.width() > q->width() || pixmapSize.height() > q->height())
        q->setFillMode(QQuickImage::PreserveAspectFit);
    else
        q->setFillMode(QQuickImage::Pad);

    updatingFillMode = false;
}

qreal QQuickIconImagePrivate::calculateDevicePixelRatio() const
{
    Q_Q(const QQuickIconImage);
    return q->window() ? q->window()->effectiveDevicePixelRatio() : qApp->devicePixelRatio();
}

QQuickIconImage::QQuickIconImage(QQuickItem *parent)
    : QQuickImage(*(new QQuickIconImagePrivate), parent)
{
    setFillMode(Pad);
}

QString QQuickIconImage::name() const
{
    Q_D(const QQuickIconImage);
    return d->icon.iconName;
}

void QQuickIconImage::setName(const QString &name)
{
    Q_D(QQuickIconImage);
    if (d->icon.iconName == name)
        return;

    d->icon = QIconLoader::instance()->loadIcon(name);
    if (isComponentComplete())
        d->updateIcon();
    emit nameChanged();
}

QColor QQuickIconImage::color() const
{
    Q_D(const QQuickIconImage);
    return d->color;
}

void QQuickIconImage::setColor(const QColor &color)
{
    Q_D(QQuickIconImage);
    if (d->color == color)
        return;

    d->color = color;
    if (isComponentComplete())
        d->updateIcon();
    emit colorChanged();
}

void QQuickIconImage::setSource(const QUrl &source)
{
    Q_D(QQuickIconImage);
    if (d->source == source)
        return;

    d->source = source;
    if (isComponentComplete())
        d->updateIcon();
    emit sourceChanged(source);
}

void QQuickIconImage::componentComplete()
{
    Q_D(QQuickIconImage);
    QQuickImage::componentComplete();
    d->updateIcon();
    QObjectPrivate::connect(this, &QQuickImageBase::sourceSizeChanged, d, &QQuickIconImagePrivate::updateIcon);
}

void QQuickIconImage::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickIconImage);
    QQuickImage::geometryChanged(newGeometry, oldGeometry);
    if (isComponentComplete() && newGeometry.size() != oldGeometry.size())
        d->updateIcon();
}

void QQuickIconImage::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QQuickIconImage);
    if (change == ItemDevicePixelRatioHasChanged)
        d->updateIcon();
    QQuickImage::itemChange(change, value);
}

void QQuickIconImage::pixmapChange()
{
    Q_D(QQuickIconImage);
    QQuickImage::pixmapChange();
    d->updateFillMode();

    // Don't apply the color if we're recursing (updateFillMode() can cause us to recurse).
    if (!d->updatingFillMode && d->color.alpha() > 0) {
        QImage image = d->pix.image();
        if (!image.isNull()) {
            QPainter painter(&image);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(image.rect(), d->color);
            d->pix.setImage(image);
        }
    }
}

QT_END_NAMESPACE
