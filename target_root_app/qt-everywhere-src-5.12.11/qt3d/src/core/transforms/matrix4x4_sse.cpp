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

#include "matrix4x4_sse_p.h"

#ifdef QT_COMPILER_SUPPORTS_SSE2

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

QDebug operator<<(QDebug dbg, const Matrix4x4_SSE &m)
{
    dbg.nospace() << "Matrix4x4_SSE(" << endl
                  << qSetFieldWidth(10)
                  << m.m11() << m.m12() << m.m13() << m.m14() << endl
                  << m.m21() << m.m22() << m.m23() << m.m24() << endl
                  << m.m31() << m.m32() << m.m33() << m.m34() << endl
                  << m.m41() << m.m42() << m.m43() << m.m44() << endl
                  << qSetFieldWidth(0) << ')';
    return dbg;
}

} // Qt3DCore

QT_END_NAMESPACE

#endif // QT_COMPILER_SUPPORTS_SSE2
