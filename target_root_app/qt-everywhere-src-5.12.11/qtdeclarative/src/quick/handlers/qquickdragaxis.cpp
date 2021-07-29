/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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
#include "qquickdragaxis_p.h"
#include <limits>

QQuickDragAxis::QQuickDragAxis()
  : m_minimum(-std::numeric_limits<qreal>::max())
  , m_maximum(std::numeric_limits<qreal>::max())
  , m_enabled(true)
{
}

void QQuickDragAxis::setMinimum(qreal minimum)
{
    if (m_minimum == minimum)
        return;

    m_minimum = minimum;
    emit minimumChanged();
}

void QQuickDragAxis::setMaximum(qreal maximum)
{
    if (m_maximum == maximum)
        return;

    m_maximum = maximum;
    emit maximumChanged();
}

void QQuickDragAxis::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    emit enabledChanged();
}

