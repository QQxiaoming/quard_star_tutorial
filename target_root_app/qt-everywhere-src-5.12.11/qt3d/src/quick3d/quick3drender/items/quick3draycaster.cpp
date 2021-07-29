/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include <Qt3DQuickRender/private/quick3draycaster_p.h>
#include <Qt3DQuickRender/private/quick3draycaster_p_p.h>
#include <Qt3DCore/private/qscene_p.h>

#include <Qt3DCore/QEntity>

#include <QQmlEngine>
#include <QJSValue>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {
namespace Quick {

void Quick3DRayCasterPrivate::dispatchHits(const QAbstractRayCaster::Hits &hits)
{
    m_hits = hits;
    updateHitEntites(m_hits, m_scene);

    Q_Q(Quick3DRayCaster);
    if (!m_engine)
        m_engine = qmlEngine(q->parent());

    m_jsHits = convertHits(m_hits, m_engine);

    bool v = q->blockNotifications(true);
    emit q->hitsChanged(m_jsHits);
    q->blockNotifications(v);
}

QJSValue Quick3DRayCasterPrivate::convertHits(const QAbstractRayCaster::Hits &hits, QQmlEngine *engine)
{
    auto jsHits = engine->newArray(hits.length());
    for (int i=0; i<hits.size(); i++) {
        QJSValue v = engine->newObject();
        v.setProperty(QLatin1Literal("type"), hits[i].type());
        v.setProperty(QLatin1Literal("entity"), engine->newQObject(hits[i].entity()));
        v.setProperty(QLatin1Literal("distance"), hits[i].distance());
        {
            QJSValue p = engine->newObject();
            p.setProperty(QLatin1Literal("x"), hits[i].localIntersection().x());
            p.setProperty(QLatin1Literal("y"), hits[i].localIntersection().y());
            p.setProperty(QLatin1Literal("z"), hits[i].localIntersection().z());
            v.setProperty(QLatin1Literal("localIntersection"), p);
        }
        {
            QJSValue p = engine->newObject();
            p.setProperty(QLatin1Literal("x"), hits[i].worldIntersection().x());
            p.setProperty(QLatin1Literal("y"), hits[i].worldIntersection().y());
            p.setProperty(QLatin1Literal("z"), hits[i].worldIntersection().z());
            v.setProperty(QLatin1Literal("worldIntersection"), p);
        }

        switch (hits[i].type()) {
        case Qt3DRender::QRayCasterHit::TriangleHit:
            v.setProperty(QLatin1Literal("primitiveIndex"), hits[i].primitiveIndex());
            v.setProperty(QLatin1Literal("vertex1Index"), hits[i].vertex1Index());
            v.setProperty(QLatin1Literal("vertex2Index"), hits[i].vertex2Index());
            v.setProperty(QLatin1Literal("vertex3Index"), hits[i].vertex3Index());
            break;
        case Qt3DRender::QRayCasterHit::LineHit:
            v.setProperty(QLatin1Literal("primitiveIndex"), hits[i].primitiveIndex());
            v.setProperty(QLatin1Literal("vertex1Index"), hits[i].vertex1Index());
            v.setProperty(QLatin1Literal("vertex2Index"), hits[i].vertex2Index());
            break;
        case Qt3DRender::QRayCasterHit::PointHit:
            v.setProperty(QLatin1Literal("primitiveIndex"), hits[i].primitiveIndex());
            break;
        default: break;
        }

        jsHits.setProperty(i, v);
    }

    return jsHits;
}

void Quick3DRayCasterPrivate::appendLayer(QQmlListProperty<QLayer> *list, QLayer *layer)
{
    QAbstractRayCaster *filter = qobject_cast<QAbstractRayCaster *>(list->object);
    if (filter)
        filter->addLayer(layer);
}

QLayer *Quick3DRayCasterPrivate::layerAt(QQmlListProperty<QLayer> *list, int index)
{
    QAbstractRayCaster *filter = qobject_cast<QAbstractRayCaster *>(list->object);
    if (filter)
        return filter->layers().at(index);
    return nullptr;
}

int Quick3DRayCasterPrivate::layerCount(QQmlListProperty<QLayer> *list)
{
    QAbstractRayCaster *filter = qobject_cast<QAbstractRayCaster *>(list->object);
    if (filter)
        return filter->layers().count();
    return 0;
}

void Quick3DRayCasterPrivate::clearLayers(QQmlListProperty<QLayer> *list)
{
    QAbstractRayCaster *filter = qobject_cast<QAbstractRayCaster *>(list->object);
    if (filter) {
        const auto layers = filter->layers();
        for (QLayer *layer : layers)
            filter->removeLayer(layer);
    }
}

Quick3DRayCaster::Quick3DRayCaster(QObject *parent)
    : QRayCaster(*new Quick3DRayCasterPrivate(), qobject_cast<Qt3DCore::QNode *>(parent))
{
}

QJSValue Quick3DRayCaster::hits() const
{
    Q_D(const Quick3DRayCaster);
    return d->m_jsHits;
}

QQmlListProperty<Qt3DRender::QLayer> Qt3DRender::Render::Quick::Quick3DRayCaster::qmlLayers()
{
    return QQmlListProperty<QLayer>(this, 0,
                                    &Quick3DRayCasterPrivate::appendLayer,
                                    &Quick3DRayCasterPrivate::layerCount,
                                    &Quick3DRayCasterPrivate::layerAt,
                                    &Quick3DRayCasterPrivate::clearLayers);
}

} // namespace Quick
} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
