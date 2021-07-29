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

#include "qlinewidth.h"
#include "qlinewidth_p.h"

#include <Qt3DRender/private/qrenderstatecreatedchange_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {

/*!
    \class Qt3DRender::QLineWidth
    \inmodule Qt3DRender
    \since 5.10
    \brief Specifies the width of rasterized lines.
 */

/*!
    \qmltype LineWidth
    \since 5.10
    \inherits RenderState
    \instantiates Qt3DRender::QLineWidth
    \inqmlmodule Qt3D.Render

    \brief Specifies the width of rasterized lines.
 */

/*!
    \qmlproperty real LineWidth::value
    Specifies the width value to be used.
*/

/*!
    \property  QLineWidth::value
    Specifies the width value to be used.
*/

QLineWidth::QLineWidth(Qt3DCore::QNode *parent)
    : QRenderState(*new QLineWidthPrivate(1.0f), parent)
{
}

QLineWidth::~QLineWidth()
{
}

float QLineWidth::value() const
{
    Q_D(const QLineWidth);
    return d->m_value;
}

void QLineWidth::setValue(float width)
{
    Q_D(QLineWidth);
    d->m_value = width;
    emit valueChanged(width);
}

bool QLineWidth::smooth() const
{
    Q_D(const QLineWidth);
    return d->m_smooth;
}

void QLineWidth::setSmooth(bool enabled)
{
    Q_D(QLineWidth);
    if (d->m_smooth != enabled) {
        d->m_smooth = enabled;
        emit smoothChanged(enabled);
    }
}

Qt3DCore::QNodeCreatedChangeBasePtr QLineWidth::createNodeCreationChange() const
{
    auto creationChange = QRenderStateCreatedChangePtr<QLineWidthData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QLineWidth);
    data.value = d->m_value;
    data.smooth = d->m_smooth;
    return creationChange;
}

} // namespace Qt3DRender

QT_END_NAMESPACE
