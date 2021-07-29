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

#include "qspritegrid.h"
#include "qspritegrid_p.h"

#include <Qt3DRender/qabstracttexture.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DExtras {

QSpriteGridPrivate::QSpriteGridPrivate()
    : QAbstractSpriteSheetPrivate()
    , m_numColumns(1)
    , m_numRows(1)
{
}

int QSpriteGridPrivate::maxIndex() const
{
    return m_numColumns * m_numRows - 1;
}

void QSpriteGridPrivate::updateSizes()
{
    Q_Q(QSpriteGrid);
    if (m_texture && m_numColumns && m_numRows) {
        m_textureSize = QSize(m_texture->width(), m_texture->height());
        m_cellSize = QSizeF((float) m_texture->width() / m_numColumns, (float) m_texture->height() / m_numRows);
    } else {
        m_textureSize = QSize();
        m_cellSize = QSizeF();
    }

    if (m_cellSize.isEmpty() || m_numColumns == 0 || m_numRows == 0) {
        if (m_currentIndex != -1) {
            m_currentIndex = -1;
            emit q->currentIndexChanged(m_currentIndex);
        }
        m_textureTransform.setToIdentity();
        emit q->textureTransformChanged(m_textureTransform);
        return;
    }

    if (m_currentIndex == -1) {
        m_currentIndex = 0;
        emit q->currentIndexChanged(m_currentIndex);
    }
    updateTransform();
}

void QSpriteGridPrivate::updateTransform()
{
    Q_Q(QSpriteGrid);
    const float xScale = (float) m_cellSize.width() / (float) m_textureSize.width();
    const float yScale = (float) m_cellSize.height() / (float) m_textureSize.height();

    const int currentRow = m_currentIndex / m_numColumns;
    const int currentColumn = m_currentIndex % m_numColumns;
    const float xTranslate = currentColumn * xScale;
    const float yTranslate = currentRow * yScale;

    m_textureTransform.setToIdentity();
    m_textureTransform(0, 0) = xScale;
    m_textureTransform(1, 1) = yScale;
    m_textureTransform(0, 2) = xTranslate;
    m_textureTransform(1, 2) = yTranslate;
    emit q->textureTransformChanged(m_textureTransform);
}

/*!
    Constructs a new QSpriteGrid instance with parent object \a parent.
 */
QSpriteGrid::QSpriteGrid(QNode *parent)
    : QAbstractSpriteSheet(*new QSpriteGridPrivate, parent)
{
}

QSpriteGrid::~QSpriteGrid()
{
}

int QSpriteGrid::rows() const
{
    Q_D(const QSpriteGrid);
    return d->m_numRows;
}

void QSpriteGrid::setRows(int rows)
{
    Q_D(QSpriteGrid);
    if (d->m_numRows != rows) {
        d->m_numRows = rows;
        d->updateSizes();
        emit rowsChanged(rows);
    }
}

int QSpriteGrid::columns() const
{
    Q_D(const QSpriteGrid);
    return d->m_numColumns;
}

void QSpriteGrid::setColumns(int columns)
{
    Q_D(QSpriteGrid);
    if (d->m_numColumns != columns) {
        d->m_numColumns = columns;
        d->updateSizes();
        emit columnsChanged(columns);
    }
}

} // namespace Qt3DExtras

QT_END_NAMESPACE
