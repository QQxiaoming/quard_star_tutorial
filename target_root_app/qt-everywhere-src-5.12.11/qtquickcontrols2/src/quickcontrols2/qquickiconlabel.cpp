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

#include "qquickiconlabel_p.h"
#include "qquickiconlabel_p_p.h"
#include "qquickiconimage_p.h"
#include "qquickmnemoniclabel_p.h"

#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquicktext_p.h>

QT_BEGIN_NAMESPACE

static void beginClass(QQuickItem *item)
{
    if (QQmlParserStatus *parserStatus = qobject_cast<QQmlParserStatus *>(item))
        parserStatus->classBegin();
}

static void completeComponent(QQuickItem *item)
{
    if (QQmlParserStatus *parserStatus = qobject_cast<QQmlParserStatus *>(item))
        parserStatus->componentComplete();
}

bool QQuickIconLabelPrivate::hasIcon() const
{
    return display != QQuickIconLabel::TextOnly && !icon.isEmpty();
}

bool QQuickIconLabelPrivate::hasText() const
{
    return display != QQuickIconLabel::IconOnly && !text.isEmpty();
}

bool QQuickIconLabelPrivate::createImage()
{
    Q_Q(QQuickIconLabel);
    if (image)
        return false;

    image = new QQuickIconImage(q);
    watchChanges(image);
    beginClass(image);
    image->setObjectName(QStringLiteral("image"));
    image->setName(icon.name());
    image->setSource(icon.source());
    image->setSourceSize(QSize(icon.width(), icon.height()));
    image->setColor(icon.color());
    QQmlEngine::setContextForObject(image, qmlContext(q));
    if (componentComplete)
        completeComponent(image);
    return true;
}

bool QQuickIconLabelPrivate::destroyImage()
{
    if (!image)
        return false;

    unwatchChanges(image);
    delete image;
    image = nullptr;
    return true;
}

bool QQuickIconLabelPrivate::updateImage()
{
    if (!hasIcon())
        return destroyImage();
    return createImage();
}

void QQuickIconLabelPrivate::syncImage()
{
    if (!image || icon.isEmpty())
        return;

    image->setName(icon.name());
    image->setSource(icon.source());
    image->setSourceSize(QSize(icon.width(), icon.height()));
    image->setColor(icon.color());
    const int valign = alignment & Qt::AlignVertical_Mask;
    image->setVerticalAlignment(static_cast<QQuickImage::VAlignment>(valign));
    const int halign = alignment & Qt::AlignHorizontal_Mask;
    image->setHorizontalAlignment(static_cast<QQuickImage::HAlignment>(halign));
}

void QQuickIconLabelPrivate::updateOrSyncImage()
{
    if (updateImage()) {
        if (componentComplete) {
            updateImplicitSize();
            layout();
        }
    } else {
        syncImage();
    }
}

bool QQuickIconLabelPrivate::createLabel()
{
    Q_Q(QQuickIconLabel);
    if (label)
        return false;

    label = new QQuickMnemonicLabel(q);
    watchChanges(label);
    beginClass(label);
    label->setObjectName(QStringLiteral("label"));
    label->setFont(font);
    label->setColor(color);
    label->setElideMode(QQuickText::ElideRight);
    const int valign = alignment & Qt::AlignVertical_Mask;
    label->setVAlign(static_cast<QQuickText::VAlignment>(valign));
    const int halign = alignment & Qt::AlignHorizontal_Mask;
    label->setHAlign(static_cast<QQuickText::HAlignment>(halign));
    label->setText(text);
    if (componentComplete)
        completeComponent(label);
    return true;
}

bool QQuickIconLabelPrivate::destroyLabel()
{
    if (!label)
        return false;

    unwatchChanges(label);
    delete label;
    label = nullptr;
    return true;
}

bool QQuickIconLabelPrivate::updateLabel()
{
    if (!hasText())
        return destroyLabel();
    return createLabel();
}

void QQuickIconLabelPrivate::syncLabel()
{
    if (!label)
        return;

    label->setText(text);
}

void QQuickIconLabelPrivate::updateOrSyncLabel()
{
    if (updateLabel()) {
        if (componentComplete) {
            updateImplicitSize();
            layout();
        }
    } else {
        syncLabel();
    }
}

void QQuickIconLabelPrivate::updateImplicitSize()
{
    Q_Q(QQuickIconLabel);
    const bool showIcon = image && hasIcon();
    const bool showText = label && hasText();
    const qreal horizontalPadding = leftPadding + rightPadding;
    const qreal verticalPadding = topPadding + bottomPadding;
    const qreal iconImplicitWidth = showIcon ? image->implicitWidth() : 0;
    const qreal iconImplicitHeight = showIcon ? image->implicitHeight() : 0;
    const qreal textImplicitWidth = showText ? label->implicitWidth() : 0;
    const qreal textImplicitHeight = showText ? label->implicitHeight() : 0;
    const qreal effectiveSpacing = showText && showIcon && image->implicitWidth() > 0 ? spacing : 0;
    const qreal implicitWidth = display == QQuickIconLabel::TextBesideIcon ? iconImplicitWidth + textImplicitWidth + effectiveSpacing
                                                                           : qMax(iconImplicitWidth, textImplicitWidth);
    const qreal implicitHeight = display == QQuickIconLabel::TextUnderIcon ? iconImplicitHeight + textImplicitHeight + effectiveSpacing
                                                                           : qMax(iconImplicitHeight, textImplicitHeight);
    q->setImplicitSize(implicitWidth + horizontalPadding, implicitHeight + verticalPadding);
}

// adapted from QStyle::alignedRect()
static QRectF alignedRect(bool mirrored, Qt::Alignment alignment, const QSizeF &size, const QRectF &rectangle)
{
    alignment = QGuiApplicationPrivate::visualAlignment(mirrored ? Qt::RightToLeft : Qt::LeftToRight, alignment);
    qreal x = rectangle.x();
    qreal y = rectangle.y();
    const qreal w = size.width();
    const qreal h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rectangle.height() / 2 - h / 2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rectangle.height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rectangle.width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rectangle.width() / 2 - w / 2;
    return QRectF(x, y, w, h);
}

void QQuickIconLabelPrivate::layout()
{
    Q_Q(QQuickIconLabel);
    if (!componentComplete)
        return;

    const qreal availableWidth = width - leftPadding - rightPadding;
    const qreal availableHeight = height - topPadding - bottomPadding;

    switch (display) {
    case QQuickIconLabel::IconOnly:
        if (image) {
            const QRectF iconRect = alignedRect(mirrored, alignment,
                                                QSizeF(qMin(image->implicitWidth(), availableWidth),
                                                       qMin(image->implicitHeight(), availableHeight)),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        break;
    case QQuickIconLabel::TextOnly:
        if (label) {
            const QRectF textRect = alignedRect(mirrored, alignment,
                                                QSizeF(qMin(label->implicitWidth(), availableWidth),
                                                       qMin(label->implicitHeight(), availableHeight)),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;

    case QQuickIconLabel::TextUnderIcon: {
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize;
        QSizeF textSize;
        if (image) {
            iconSize.setWidth(qMin(image->implicitWidth(), availableWidth));
            iconSize.setHeight(qMin(image->implicitHeight(), availableHeight));
        }
        qreal effectiveSpacing = 0;
        if (label) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(label->implicitWidth(), availableWidth));
            textSize.setHeight(qMin(label->implicitHeight(), availableHeight - iconSize.height() - effectiveSpacing));
        }

        QRectF combinedRect = alignedRect(mirrored, alignment,
                                          QSizeF(qMax(iconSize.width(), textSize.width()),
                                                 iconSize.height() + effectiveSpacing + textSize.height()),
                                          QRectF(leftPadding, topPadding, availableWidth, availableHeight));
        if (image) {
            QRectF iconRect = alignedRect(mirrored, Qt::AlignHCenter | Qt::AlignTop, iconSize, combinedRect);
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        if (label) {
            QRectF textRect = alignedRect(mirrored, Qt::AlignHCenter | Qt::AlignBottom, textSize, combinedRect);
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;
    }

    case QQuickIconLabel::TextBesideIcon:
    default:
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize(0, 0);
        QSizeF textSize(0, 0);
        if (image) {
            iconSize.setWidth(qMin(image->implicitWidth(), availableWidth));
            iconSize.setHeight(qMin(image->implicitHeight(), availableHeight));
        }
        qreal effectiveSpacing = 0;
        if (label) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(label->implicitWidth(), availableWidth - iconSize.width() - effectiveSpacing));
            textSize.setHeight(qMin(label->implicitHeight(), availableHeight));
        }

        const QRectF combinedRect = alignedRect(mirrored, alignment,
                                                QSizeF(iconSize.width() + effectiveSpacing + textSize.width(),
                                                       qMax(iconSize.height(), textSize.height())),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
        if (image) {
            const QRectF iconRect = alignedRect(mirrored, Qt::AlignLeft | Qt::AlignVCenter, iconSize, combinedRect);
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        if (label) {
            const QRectF textRect = alignedRect(mirrored, Qt::AlignRight | Qt::AlignVCenter, textSize, combinedRect);
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;
    }

    q->setBaselineOffset(label ? label->y() + label->baselineOffset() : 0);
}

static const QQuickItemPrivate::ChangeTypes itemChangeTypes =
    QQuickItemPrivate::ImplicitWidth
    | QQuickItemPrivate::ImplicitHeight
    | QQuickItemPrivate::Destroyed;

void QQuickIconLabelPrivate::watchChanges(QQuickItem *item)
{
    QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(item);
    itemPrivate->addItemChangeListener(this, itemChangeTypes);
}

void QQuickIconLabelPrivate::unwatchChanges(QQuickItem* item)
{
    QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(item);
    itemPrivate->removeItemChangeListener(this, itemChangeTypes);
}

void QQuickIconLabelPrivate::itemImplicitWidthChanged(QQuickItem *)
{
    updateImplicitSize();
    layout();
}

void QQuickIconLabelPrivate::itemImplicitHeightChanged(QQuickItem *)
{
    updateImplicitSize();
    layout();
}

void QQuickIconLabelPrivate::itemDestroyed(QQuickItem *item)
{
    unwatchChanges(item);
    if (item == image)
        image = nullptr;
    else if (item == label)
        label = nullptr;
}

QQuickIconLabel::QQuickIconLabel(QQuickItem *parent)
    : QQuickItem(*(new QQuickIconLabelPrivate), parent)
{
}

QQuickIconLabel::~QQuickIconLabel()
{
    Q_D(QQuickIconLabel);
    if (d->image)
        d->unwatchChanges(d->image);
    if (d->label)
        d->unwatchChanges(d->label);
}

QQuickIcon QQuickIconLabel::icon() const
{
    Q_D(const QQuickIconLabel);
    return d->icon;
}

void QQuickIconLabel::setIcon(const QQuickIcon &icon)
{
    Q_D(QQuickIconLabel);
    if (d->icon == icon)
        return;

    d->icon = icon;
    d->updateOrSyncImage();
}

QString QQuickIconLabel::text() const
{
    Q_D(const QQuickIconLabel);
    return d->text;
}

void QQuickIconLabel::setText(const QString text)
{
    Q_D(QQuickIconLabel);
    if (d->text == text)
        return;

    d->text = text;
    d->updateOrSyncLabel();
}

QFont QQuickIconLabel::font() const
{
    Q_D(const QQuickIconLabel);
    return d->font;
}

void QQuickIconLabel::setFont(const QFont &font)
{
    Q_D(QQuickIconLabel);
    if (d->font == font)
        return;

    d->font = font;
    if (d->label)
        d->label->setFont(font);
}

QColor QQuickIconLabel::color() const
{
    Q_D(const QQuickIconLabel);
    return d->color;
}

void QQuickIconLabel::setColor(const QColor &color)
{
    Q_D(QQuickIconLabel);
    if (d->color == color)
        return;

    d->color = color;
    if (d->label)
        d->label->setColor(color);
}

QQuickIconLabel::Display QQuickIconLabel::display() const
{
    Q_D(const QQuickIconLabel);
    return d->display;
}

void QQuickIconLabel::setDisplay(Display display)
{
    Q_D(QQuickIconLabel);
    if (d->display == display)
        return;

    d->display = display;
    d->updateImage();
    d->updateLabel();
    d->updateImplicitSize();
    d->layout();
}

qreal QQuickIconLabel::spacing() const
{
    Q_D(const QQuickIconLabel);
    return d->spacing;
}

void QQuickIconLabel::setSpacing(qreal spacing)
{
    Q_D(QQuickIconLabel);
    if (qFuzzyCompare(d->spacing, spacing))
        return;

    d->spacing = spacing;
    if (d->image && d->label) {
        d->updateImplicitSize();
        d->layout();
    }
}

bool QQuickIconLabel::isMirrored() const
{
    Q_D(const QQuickIconLabel);
    return d->mirrored;
}

void QQuickIconLabel::setMirrored(bool mirrored)
{
    Q_D(QQuickIconLabel);
    if (d->mirrored == mirrored)
        return;

    d->mirrored = mirrored;
    d->layout();
}

Qt::Alignment QQuickIconLabel::alignment() const
{
    Q_D(const QQuickIconLabel);
    return d->alignment;
}

void QQuickIconLabel::setAlignment(Qt::Alignment alignment)
{
    Q_D(QQuickIconLabel);
    const int valign = alignment & Qt::AlignVertical_Mask;
    const int halign = alignment & Qt::AlignHorizontal_Mask;
    const uint align = (valign ? valign : Qt::AlignVCenter) | (halign ? halign : Qt::AlignHCenter);
    if (d->alignment == align)
        return;

    d->alignment = static_cast<Qt::Alignment>(align);
    if (d->label) {
        d->label->setVAlign(static_cast<QQuickText::VAlignment>(valign));
        d->label->setHAlign(static_cast<QQuickText::HAlignment>(halign));
    }
    if (d->image) {
        d->image->setVerticalAlignment(static_cast<QQuickImage::VAlignment>(valign));
        d->image->setHorizontalAlignment(static_cast<QQuickImage::HAlignment>(halign));
    }
    d->layout();
}

qreal QQuickIconLabel::topPadding() const
{
    Q_D(const QQuickIconLabel);
    return d->topPadding;
}

void QQuickIconLabel::setTopPadding(qreal padding)
{
    Q_D(QQuickIconLabel);
    if (qFuzzyCompare(d->topPadding, padding))
        return;

    d->topPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void QQuickIconLabel::resetTopPadding()
{
    setTopPadding(0);
}

qreal QQuickIconLabel::leftPadding() const
{
    Q_D(const QQuickIconLabel);
    return d->leftPadding;
}

void QQuickIconLabel::setLeftPadding(qreal padding)
{
    Q_D(QQuickIconLabel);
    if (qFuzzyCompare(d->leftPadding, padding))
        return;

    d->leftPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void QQuickIconLabel::resetLeftPadding()
{
    setLeftPadding(0);
}

qreal QQuickIconLabel::rightPadding() const
{
    Q_D(const QQuickIconLabel);
    return d->rightPadding;
}

void QQuickIconLabel::setRightPadding(qreal padding)
{
    Q_D(QQuickIconLabel);
    if (qFuzzyCompare(d->rightPadding, padding))
        return;

    d->rightPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void QQuickIconLabel::resetRightPadding()
{
    setRightPadding(0);
}

qreal QQuickIconLabel::bottomPadding() const
{
    Q_D(const QQuickIconLabel);
    return d->bottomPadding;
}

void QQuickIconLabel::setBottomPadding(qreal padding)
{
    Q_D(QQuickIconLabel);
    if (qFuzzyCompare(d->bottomPadding, padding))
        return;

    d->bottomPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void QQuickIconLabel::resetBottomPadding()
{
    setBottomPadding(0);
}

void QQuickIconLabel::componentComplete()
{
    Q_D(QQuickIconLabel);
    if (d->image)
        completeComponent(d->image);
    if (d->label)
        completeComponent(d->label);
    QQuickItem::componentComplete();
    d->layout();
}

void QQuickIconLabel::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickIconLabel);
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->layout();
}

QT_END_NAMESPACE
