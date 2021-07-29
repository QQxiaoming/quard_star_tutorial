/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
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

#include "triangleboundingvolume_p.h"
#include <Qt3DRender/private/qray3d_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

namespace Render {

// Note: a, b, c in clockwise order
// RealTime Collision Detection page 192
bool intersectsSegmentTriangle(const RayCasting::QRay3D &ray,
                               const Vector3D &a,
                               const Vector3D &b,
                               const Vector3D &c,
                               Vector3D &uvw,
                               float &t)
{
    const Vector3D ab = b - a;
    const Vector3D ac = c - a;
    const Vector3D qp = (ray.origin() - ray.point(ray.distance()));

    const Vector3D n = Vector3D::crossProduct(ab, ac);
    const float d = Vector3D::dotProduct(qp, n);

    if (d <= 0.0f)
        return false;

    const Vector3D ap = ray.origin() - a;
    t = Vector3D::dotProduct(ap, n);

    if (t < 0.0f || t > d)
        return false;

    const Vector3D e = Vector3D::crossProduct(qp, ap);
    uvw.setY(Vector3D::dotProduct(ac, e));

    if (uvw.y() < 0.0f || uvw.y() > d)
        return false;

    uvw.setZ(-Vector3D::dotProduct(ab, e));

    if (uvw.z() < 0.0f || uvw.y() + uvw.z() > d)
        return false;

    const float ood = 1.0f / d;
    t *= ood;
    uvw.setY(uvw.y() * ood);
    uvw.setZ(uvw.z() * ood);
    uvw.setX(1.0f - uvw.y() - uvw.z());

    return true;
}

TriangleBoundingVolume::TriangleBoundingVolume()
    : QBoundingVolume()
{
}

/*
    The vertices a, b, c are assumed to be in counter clockwise order.
 */
TriangleBoundingVolume::TriangleBoundingVolume(Qt3DCore::QNodeId id, const Vector3D &a, const Vector3D &b, const Vector3D &c)
    : QBoundingVolume()
    , m_id(id)
    , m_a(a)
    , m_b(b)
    , m_c(c)
{}

Qt3DCore::QNodeId TriangleBoundingVolume::id() const
{
    return m_id;
}

bool TriangleBoundingVolume::intersects(const RayCasting::QRay3D &ray, Vector3D *q, Vector3D *uvw) const
{
    float t = 0.0f;
    Vector3D uvwr;
    const float intersected = intersectsSegmentTriangle(ray, m_c, m_b, m_a, uvwr, t);

    if (intersected) {
        if (q != nullptr)
            *q = ray.point(t);
        if (uvw != nullptr)
            *uvw = uvwr;
    }
    return intersected;
}

TriangleBoundingVolume::Type TriangleBoundingVolume::type() const
{
    return RayCasting::QBoundingVolume::Triangle;
}

Vector3D TriangleBoundingVolume::a() const
{
    return m_a;
}

Vector3D TriangleBoundingVolume::b() const
{
    return m_b;
}

Vector3D TriangleBoundingVolume::c() const
{
    return m_c;
}

void TriangleBoundingVolume::setA(const Vector3D &a)
{
    m_a = a;
}

void TriangleBoundingVolume::setB(const Vector3D &b)
{
    m_b = b;
}

void TriangleBoundingVolume::setC(const Vector3D &c)
{
    m_c = c;
}

TriangleBoundingVolume TriangleBoundingVolume::transformed(const Matrix4x4 &mat) const
{
    const Vector3D tA = mat * m_a;
    const Vector3D tB = mat * m_b;
    const Vector3D tC = mat * m_c;
    return TriangleBoundingVolume(id(), tA, tB, tC);
}

} // namespace Render

} // namespace Qt3DRender

QT_END_NAMESPACE
