/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#include "qquickicon_p.h"

QT_BEGIN_NAMESPACE

class QQuickIconPrivate : public QSharedData
{
public:
    QString name;
    QUrl source;
    int width = 0;
    int height = 0;
    QColor color = Qt::transparent;

    enum ResolveProperties {
        NameResolved = 0x0001,
        SourceResolved = 0x0002,
        WidthResolved = 0x0004,
        HeightResolved = 0x0008,
        ColorResolved = 0x0010,
        AllPropertiesResolved = 0x1ffff
    };

    // This is based on QFont's resolve_mask.
    int resolveMask = 0;
};

QQuickIcon::QQuickIcon()
    : d(new QQuickIconPrivate)
{
}

QQuickIcon::QQuickIcon(const QQuickIcon &other)
    : d(other.d)
{
}

QQuickIcon::~QQuickIcon()
{
}

QQuickIcon &QQuickIcon::operator=(const QQuickIcon &other)
{
    d = other.d;
    return *this;
}

bool QQuickIcon::operator==(const QQuickIcon &other) const
{
    return d == other.d || (d->name == other.d->name
                            && d->source == other.d->source
                            && d->width == other.d->width
                            && d->height == other.d->height
                            && d->color == other.d->color);
}

bool QQuickIcon::operator!=(const QQuickIcon &other) const
{
    return !(*this == other);
}

bool QQuickIcon::isEmpty() const
{
    return d->name.isEmpty() && d->source.isEmpty();
}

QString QQuickIcon::name() const
{
    return d->name;
}

void QQuickIcon::setName(const QString &name)
{
    if ((d->resolveMask & QQuickIconPrivate::NameResolved) && d->name == name)
        return;

    d->name = name;
    d->resolveMask |= QQuickIconPrivate::NameResolved;
}

void QQuickIcon::resetName()
{
    d->name = QString();
    d->resolveMask &= ~QQuickIconPrivate::NameResolved;
}

QUrl QQuickIcon::source() const
{
    return d->source;
}

void QQuickIcon::setSource(const QUrl &source)
{
    if ((d->resolveMask & QQuickIconPrivate::SourceResolved) && d->source == source)
        return;

    d->source = source;
    d->resolveMask |= QQuickIconPrivate::SourceResolved;
}

void QQuickIcon::resetSource()
{
    d->source = QString();
    d->resolveMask &= ~QQuickIconPrivate::SourceResolved;
}

int QQuickIcon::width() const
{
    return d->width;
}

void QQuickIcon::setWidth(int width)
{
    if ((d->resolveMask & QQuickIconPrivate::WidthResolved) && d->width == width)
        return;

    d->width = width;
    d->resolveMask |= QQuickIconPrivate::WidthResolved;
}

void QQuickIcon::resetWidth()
{
    d->width = 0;
    d->resolveMask &= ~QQuickIconPrivate::WidthResolved;
}

int QQuickIcon::height() const
{
    return d->height;
}

void QQuickIcon::setHeight(int height)
{
    if ((d->resolveMask & QQuickIconPrivate::HeightResolved) && d->height == height)
        return;

    d->height = height;
    d->resolveMask |= QQuickIconPrivate::HeightResolved;
}

void QQuickIcon::resetHeight()
{
    d->height = 0;
    d->resolveMask &= ~QQuickIconPrivate::HeightResolved;
}

QColor QQuickIcon::color() const
{
    return d->color;
}

void QQuickIcon::setColor(const QColor &color)
{
    if ((d->resolveMask & QQuickIconPrivate::ColorResolved) && d->color == color)
        return;

    d->color = color;
    d->resolveMask |= QQuickIconPrivate::ColorResolved;
}

void QQuickIcon::resetColor()
{
    d->color = Qt::transparent;
    d->resolveMask &= ~QQuickIconPrivate::ColorResolved;
}

QQuickIcon QQuickIcon::resolve(const QQuickIcon &other) const
{
    QQuickIcon resolved = *this;

    if (!(d->resolveMask & QQuickIconPrivate::NameResolved))
        resolved.d->name = other.name();

    if (!(d->resolveMask & QQuickIconPrivate::SourceResolved))
        resolved.d->source = other.source();

    if (!(d->resolveMask & QQuickIconPrivate::WidthResolved))
        resolved.d->width = other.width();

    if (!(d->resolveMask & QQuickIconPrivate::HeightResolved))
        resolved.d->height = other.height();

    if (!(d->resolveMask & QQuickIconPrivate::ColorResolved))
        resolved.d->color = other.color();

    return resolved;
}

QT_END_NAMESPACE
