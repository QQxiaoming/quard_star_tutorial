/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
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

#include "locationvaluetypehelper_p.h"
#include <QVariantMap>
#include <QtQml/QQmlInfo>
#include <private/qqmlengine_p.h>


QGeoCoordinate parseCoordinate(const QJSValue &value, bool *ok)
{
    QGeoCoordinate c;
    if (ok)
        *ok = false;

    if (value.isObject()) {
        if (value.hasProperty(QStringLiteral("latitude")))
            c.setLatitude(value.property(QStringLiteral("latitude")).toNumber());
        if (value.hasProperty(QStringLiteral("longitude")))
            c.setLongitude(value.property(QStringLiteral("longitude")).toNumber());
        if (value.hasProperty(QStringLiteral("altitude")))
            c.setAltitude(value.property(QStringLiteral("altitude")).toNumber());

        if (ok)
            *ok = true;
    }

    return c;
}

QGeoCoordinate parseCoordinate(const QVariant &value, bool *ok)
{
    QGeoCoordinate c;
    if (ok)
        *ok = false;

    if (value.canConvert<QGeoCoordinate>()) {
        c = value.value<QGeoCoordinate>();
        if (ok)
            *ok = true;
    } else if (value.type() == QVariant::Map) {
        const QVariantMap &map = value.toMap();

        if (map.contains(QStringLiteral("latitude")))
            c.setLatitude(map.value(QStringLiteral("latitude")).toDouble());
        if (map.contains(QStringLiteral("longitude")))
            c.setLongitude(map.value(QStringLiteral("longitude")).toDouble());
        if (map.contains(QStringLiteral("altitude")))
            c.setAltitude(map.value(QStringLiteral("altitude")).toDouble());

        if (ok)
            *ok = c.isValid(); // Not considering the case where the map is valid but containing NaNs.
    }

    return c;
}

QGeoRectangle parseRectangle(const QJSValue &value, bool *ok)
{
    QGeoRectangle r;

    *ok = false;

    if (value.isObject()) {
        if (value.hasProperty(QStringLiteral("bottomLeft"))) {
            QGeoCoordinate c = parseCoordinate(value.property(QStringLiteral("bottomLeft")), ok);
            if (*ok)
                r.setBottomLeft(c);
        }
        if (value.hasProperty(QStringLiteral("bottomRight"))) {
            QGeoCoordinate c = parseCoordinate(value.property(QStringLiteral("bottomRight")), ok);
            if (*ok)
                r.setBottomRight(c);
        }
        if (value.hasProperty(QStringLiteral("topLeft"))) {
            QGeoCoordinate c = parseCoordinate(value.property(QStringLiteral("topLeft")), ok);
            if (*ok)
                r.setTopLeft(c);
        }
        if (value.hasProperty(QStringLiteral("topRight"))) {
            QGeoCoordinate c = parseCoordinate(value.property(QStringLiteral("topRight")), ok);
            if (*ok)
                r.setTopRight(c);
        }
        if (value.hasProperty(QStringLiteral("center"))) {
            QGeoCoordinate c = parseCoordinate(value.property(QStringLiteral("center")), ok);
            if (*ok)
                r.setCenter(c);
        }
        if (value.hasProperty(QStringLiteral("height")))
            r.setHeight(value.property(QStringLiteral("height")).toNumber());
        if (value.hasProperty(QStringLiteral("width")))
            r.setWidth(value.property(QStringLiteral("width")).toNumber());
    }

    return r;
}

QGeoCircle parseCircle(const QJSValue &value, bool *ok)
{
    QGeoCircle c;

    *ok = false;

    if (value.isObject()) {
        if (value.hasProperty(QStringLiteral("center"))) {
            QGeoCoordinate coord = parseCoordinate(value.property(QStringLiteral("center")), ok);
            if (*ok)
                c.setCenter(coord);
        }
        if (value.hasProperty(QStringLiteral("radius")))
            c.setRadius(value.property(QStringLiteral("radius")).toNumber());
    }

    return c;
}

QJSValue fromList(const QObject *object, const QList<QGeoCoordinate> &list)
{
    QQmlContext *context = QQmlEngine::contextForObject(object);
    QQmlEngine *engine = context->engine();
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);

    QV4::Scope scope(v4);
    QV4::Scoped<QV4::ArrayObject> pathArray(scope, v4->newArrayObject(list.length()));
    int i = 0;
    for (const auto &val : list) {
        QV4::ScopedValue cv(scope, v4->fromVariant(QVariant::fromValue(val)));
        pathArray->put(i++, cv);
    }

    return QJSValue(v4, pathArray.asReturnedValue());
}

QList<QGeoCoordinate> toList(const QObject *object, const QJSValue &value)
{
    if (!value.isArray())
        return {};

    QList<QGeoCoordinate> pathList;
    quint32 length = value.property(QStringLiteral("length")).toUInt();
    for (quint32 i = 0; i < length; ++i) {
        bool ok;
        QGeoCoordinate c = parseCoordinate(value.property(i), &ok);

        if (!ok || !c.isValid()) {
            qmlWarning(object) << "Unsupported path type";
            return {};
        }

        pathList.append(c);
    }

    return pathList;
}
