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

#include "qquickclippedtext_p.h"

#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

QQuickClippedText::QQuickClippedText(QQuickItem *parent)
    : QQuickText(parent)
{
}

qreal QQuickClippedText::clipX() const
{
    return m_clipX;
}

void QQuickClippedText::setClipX(qreal x)
{
    if (qFuzzyCompare(x, m_clipX))
        return;

    m_clipX = x;
    markClipDirty();
}

qreal QQuickClippedText::clipY() const
{
    return m_clipY;
}

void QQuickClippedText::setClipY(qreal y)
{
    if (qFuzzyCompare(y, m_clipY))
        return;

    m_clipY = y;
    markClipDirty();
}

qreal QQuickClippedText::clipWidth() const
{
    return m_clipWidth ? m_clipWidth : width();
}

void QQuickClippedText::setClipWidth(qreal width)
{
    m_hasClipWidth = true;
    if (qFuzzyCompare(width, m_clipWidth))
        return;

    m_clipWidth = width;
    markClipDirty();
}

qreal QQuickClippedText::clipHeight() const
{
    return m_clipHeight ? m_clipHeight : height();
}

void QQuickClippedText::setClipHeight(qreal height)
{
    m_hasClipHeight = true;
    if (qFuzzyCompare(height, m_clipHeight))
        return;

    m_clipHeight = height;
    markClipDirty();
}

QRectF QQuickClippedText::clipRect() const
{
    return QRectF(clipX(), clipY(), clipWidth(), clipHeight());
}

void QQuickClippedText::markClipDirty()
{
    QQuickItemPrivate::get(this)->dirty(QQuickItemPrivate::Size);
}

QT_END_NAMESPACE
