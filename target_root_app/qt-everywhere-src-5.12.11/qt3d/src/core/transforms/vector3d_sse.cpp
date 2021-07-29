/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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

#include <private/qsimd_p.h>

#ifdef __AVX2__
#include "matrix4x4_avx2_p.h"
#else
#include "matrix4x4_sse_p.h"
#endif

#include "vector3d_sse_p.h"
#include "vector4d_sse_p.h"
#include <QDebug>

#ifdef QT_COMPILER_SUPPORTS_SSE2

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QDebug operator<<(QDebug dbg, const Vector3D_SSE &v)
{
    dbg.nospace() << "Vector3D_SSE(" << v.x() << ", " << v.y() << ", " << v.z() << ") ";
    return dbg;
}

Vector3D_SSE::Vector3D_SSE(const Vector4D_SSE &v)
{
    m_xyzw = _mm_mul_ps(v.m_xyzw, _mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f));
}

#ifdef __AVX2__

Vector3D_SSE Vector3D_SSE::unproject(const Matrix4x4_AVX2 &modelView, const Matrix4x4_AVX2 &projection, const QRect &viewport) const
{
    const Matrix4x4_AVX2 inverse = (projection * modelView).inverted();

    Vector4D_SSE tmp(*this, 1.0f);
    tmp.setX((tmp.x() - float(viewport.x())) / float(viewport.width()));
    tmp.setY((tmp.y() - float(viewport.y())) / float(viewport.height()));
    tmp = tmp * 2.0f - Vector4D_SSE(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4D_SSE obj = inverse * tmp;
    if (qFuzzyIsNull(obj.w()))
        obj.setW(1.0f);
    obj /= obj.w();
    return Vector3D_SSE(obj);
}

Vector3D_SSE Vector3D_SSE::project(const Matrix4x4_AVX2 &modelView, const Matrix4x4_AVX2 &projection, const QRect &viewport) const
{
    Vector4D_SSE tmp(*this, 1.0f);
    tmp = projection * modelView * tmp;
    if (qFuzzyIsNull(tmp.w()))
        tmp.setW(1.0f);
    tmp /= tmp.w();

    tmp = tmp * 0.5f + Vector4D_SSE(0.5f, 0.5f, 0.5f, 0.5f);
    tmp.setX(tmp.x() * viewport.width() + viewport.x());
    tmp.setY(tmp.y() * viewport.height() + viewport.y());

    return Vector3D_SSE(tmp);
}

#else

Vector3D_SSE Vector3D_SSE::unproject(const Matrix4x4_SSE &modelView, const Matrix4x4_SSE &projection, const QRect &viewport) const
{
    const Matrix4x4_SSE inverse = (projection * modelView).inverted();

    Vector4D_SSE tmp(*this, 1.0f);
    tmp.setX((tmp.x() - float(viewport.x())) / float(viewport.width()));
    tmp.setY((tmp.y() - float(viewport.y())) / float(viewport.height()));
    tmp = tmp * 2.0f - Vector4D_SSE(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4D_SSE obj = inverse * tmp;
    if (qFuzzyIsNull(obj.w()))
        obj.setW(1.0f);
    obj /= obj.w();
    return Vector3D_SSE(obj);
}

Vector3D_SSE Vector3D_SSE::project(const Matrix4x4_SSE &modelView, const Matrix4x4_SSE &projection, const QRect &viewport) const
{
    Vector4D_SSE tmp(*this, 1.0f);
    tmp = projection * modelView * tmp;
    if (qFuzzyIsNull(tmp.w()))
        tmp.setW(1.0f);
    tmp /= tmp.w();

    tmp = tmp * 0.5f + Vector4D_SSE(0.5f, 0.5f, 0.5f, 0.5f);
    tmp.setX(tmp.x() * viewport.width() + viewport.x());
    tmp.setY(tmp.y() * viewport.height() + viewport.y());

    return Vector3D_SSE(tmp);
}

#endif

} // Qt3DCore

QT_END_NAMESPACE

#endif // QT_COMPILER_SUPPORTS_SSE2
