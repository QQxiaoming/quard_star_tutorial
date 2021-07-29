/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qsgareaallocator_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qrect.h>
#include <QtCore/qpoint.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qstack.h>
#include <QtCore/qendian.h>

QT_BEGIN_NAMESPACE

namespace
{
    enum SplitType
    {
        VerticalSplit,
        HorizontalSplit
    };

    static const int maxMargin = 2;
}

struct QSGAreaAllocatorNode
{
    QSGAreaAllocatorNode(QSGAreaAllocatorNode *parent);
    ~QSGAreaAllocatorNode();
    inline bool isLeaf();

    QSGAreaAllocatorNode *parent;
    QSGAreaAllocatorNode *left;
    QSGAreaAllocatorNode *right;
    int split; // only valid for inner nodes.
    SplitType splitType;
    bool isOccupied; // only valid for leaf nodes.
};

QSGAreaAllocatorNode::QSGAreaAllocatorNode(QSGAreaAllocatorNode *parent)
    : parent(parent)
    , left(nullptr)
    , right(nullptr)
    , isOccupied(false)
{
}

QSGAreaAllocatorNode::~QSGAreaAllocatorNode()
{
    delete left;
    delete right;
}

bool QSGAreaAllocatorNode::isLeaf()
{
    Q_ASSERT((left != nullptr) == (right != nullptr));
    return !left;
}


QSGAreaAllocator::QSGAreaAllocator(const QSize &size) : m_size(size)
{
    m_root = new QSGAreaAllocatorNode(nullptr);
}

QSGAreaAllocator::~QSGAreaAllocator()
{
    delete m_root;
}

QRect QSGAreaAllocator::allocate(const QSize &size)
{
    QPoint point;
    bool result = allocateInNode(size, point, QRect(QPoint(0, 0), m_size), m_root);
    return result ? QRect(point, size) : QRect();
}

bool QSGAreaAllocator::deallocate(const QRect &rect)
{
    return deallocateInNode(rect.topLeft(), m_root);
}

bool QSGAreaAllocator::allocateInNode(const QSize &size, QPoint &result, const QRect &currentRect, QSGAreaAllocatorNode *node)
{
    if (size.width() > currentRect.width() || size.height() > currentRect.height())
        return false;

    if (node->isLeaf()) {
        if (node->isOccupied)
            return false;
        if (size.width() + maxMargin >= currentRect.width() && size.height() + maxMargin >= currentRect.height()) {
            //Snug fit, occupy entire rectangle.
            node->isOccupied = true;
            result = currentRect.topLeft();
            return true;
        }
        // TODO: Reuse nodes.
        // Split node.
        node->left = new QSGAreaAllocatorNode(node);
        node->right = new QSGAreaAllocatorNode(node);
        QRect splitRect = currentRect;
        if ((currentRect.width() - size.width()) * currentRect.height() < (currentRect.height() - size.height()) * currentRect.width()) {
            node->splitType = HorizontalSplit;
            node->split = currentRect.top() + size.height();
            splitRect.setHeight(size.height());
        } else {
            node->splitType = VerticalSplit;
            node->split = currentRect.left() + size.width();
            splitRect.setWidth(size.width());
        }
        return allocateInNode(size, result, splitRect, node->left);
    } else {
        // TODO: avoid unnecessary recursion.
        //  has been split.
        QRect leftRect = currentRect;
        QRect rightRect = currentRect;
        if (node->splitType == HorizontalSplit) {
            leftRect.setHeight(node->split - leftRect.top());
            rightRect.setTop(node->split);
        } else {
            leftRect.setWidth(node->split - leftRect.left());
            rightRect.setLeft(node->split);
        }
        if (allocateInNode(size, result, leftRect, node->left))
            return true;
        if (allocateInNode(size, result, rightRect, node->right))
            return true;
        return false;
    }
}

bool QSGAreaAllocator::deallocateInNode(const QPoint &pos, QSGAreaAllocatorNode *node)
{
    while (!node->isLeaf()) {
        //  has been split.
        int cmp = node->splitType == HorizontalSplit ? pos.y() : pos.x();
        node = cmp < node->split ? node->left : node->right;
    }
    if (!node->isOccupied)
        return false;
    node->isOccupied = false;
    mergeNodeWithNeighbors(node);
    return true;
}

void QSGAreaAllocator::mergeNodeWithNeighbors(QSGAreaAllocatorNode *node)
{
    bool done = false;
    QSGAreaAllocatorNode *parent = nullptr;
    QSGAreaAllocatorNode *current = nullptr;
    QSGAreaAllocatorNode *sibling;
    while (!done) {
        Q_ASSERT(node->isLeaf());
        Q_ASSERT(!node->isOccupied);
        if (node->parent == nullptr)
            return; // No neighbours.

        SplitType splitType = SplitType(node->parent->splitType);
        done = true;

        /* Special case. Might be faster than going through the general code path.
        // Merge with sibling.
        parent = node->parent;
        sibling = (node == parent->left ? parent->right : parent->left);
        if (sibling->isLeaf() && !sibling->isOccupied) {
            Q_ASSERT(!sibling->right);
            node = parent;
            parent->isOccupied = false;
            delete parent->left;
            delete parent->right;
            parent->left = parent->right = 0;
            done = false;
            continue;
        }
        */

        // Merge with left neighbour.
        current = node;
        parent = current->parent;
        while (parent && current == parent->left && parent->splitType == splitType) {
            current = parent;
            parent = parent->parent;
        }

        if (parent && parent->splitType == splitType) {
            Q_ASSERT(current == parent->right);
            Q_ASSERT(parent->left);

            QSGAreaAllocatorNode *neighbor = parent->left;
            while (neighbor->right && neighbor->splitType == splitType)
                neighbor = neighbor->right;

            if (neighbor->isLeaf() && neighbor->parent->splitType == splitType && !neighbor->isOccupied) {
                // Left neighbour can be merged.
                parent->split = neighbor->parent->split;

                parent = neighbor->parent;
                sibling = neighbor == parent->left ? parent->right : parent->left;
                QSGAreaAllocatorNode **nodeRef = &m_root;
                if (parent->parent) {
                    if (parent == parent->parent->left)
                        nodeRef = &parent->parent->left;
                    else
                        nodeRef = &parent->parent->right;
                }
                sibling->parent = parent->parent;
                *nodeRef = sibling;
                parent->left = parent->right = nullptr;
                delete parent;
                delete neighbor;
                done = false;
            }
        }

        // Merge with right neighbour.
        current = node;
        parent = current->parent;
        while (parent && current == parent->right && parent->splitType == splitType) {
            current = parent;
            parent = parent->parent;
        }

        if (parent && parent->splitType == splitType) {
            Q_ASSERT(current == parent->left);
            Q_ASSERT(parent->right);

            QSGAreaAllocatorNode *neighbor = parent->right;
            while (neighbor->left && neighbor->splitType == splitType)
                neighbor = neighbor->left;

            if (neighbor->isLeaf() && neighbor->parent->splitType == splitType && !neighbor->isOccupied) {
                // Right neighbour can be merged.
                parent->split = neighbor->parent->split;

                parent = neighbor->parent;
                sibling = neighbor == parent->left ? parent->right : parent->left;
                QSGAreaAllocatorNode **nodeRef = &m_root;
                if (parent->parent) {
                    if (parent == parent->parent->left)
                        nodeRef = &parent->parent->left;
                    else
                        nodeRef = &parent->parent->right;
                }
                sibling->parent = parent->parent;
                *nodeRef = sibling;
                parent->left = parent->right = nullptr;
                delete parent;
                delete neighbor;
                done = false;
            }
        }
    } // end while(!done)
}

namespace {
    struct AreaAllocatorTable
    {
        enum TableSize {
            HeaderSize = 10,
            NodeSize   = 9
        };

        enum Offset {
            // Header
            majorVersion    = 0,
            minorVersion    = 1,
            width           = 2,
            height          = 6,

            // Node
            split           = 0,
            splitType       = 4,
            flags           = 8
        };

        enum Flags {
            IsOccupied = 1,
            HasLeft = 2,
            HasRight = 4
        };

        template <typename T>
        static inline T fetch(const char *data, Offset offset)
        {
            return qFromBigEndian<T>(data + int(offset));
        }

        template <typename T>
        static inline void put(char *data, Offset offset, T value)
        {
            qToBigEndian(value, data + int(offset));
        }
    };
}

QByteArray QSGAreaAllocator::serialize()
{
    QVarLengthArray<QSGAreaAllocatorNode *> nodesToProcess;

    QStack<QSGAreaAllocatorNode *> nodes;
    nodes.push(m_root);
    while (!nodes.isEmpty()) {
        QSGAreaAllocatorNode *node = nodes.pop();

        nodesToProcess.append(node);
        if (node->left != nullptr)
            nodes.push(node->left);
        if (node->right != nullptr)
            nodes.push(node->right);
    }

    QByteArray ret;
    ret.resize(AreaAllocatorTable::HeaderSize + AreaAllocatorTable::NodeSize * nodesToProcess.size());

    char *data = ret.data();
    AreaAllocatorTable::put(data, AreaAllocatorTable::majorVersion, quint8(5));
    AreaAllocatorTable::put(data, AreaAllocatorTable::minorVersion, quint8(12));
    AreaAllocatorTable::put(data, AreaAllocatorTable::width, quint32(m_size.width()));
    AreaAllocatorTable::put(data, AreaAllocatorTable::height, quint32(m_size.height()));

    data += AreaAllocatorTable::HeaderSize;
    for (QSGAreaAllocatorNode *node : nodesToProcess) {
        AreaAllocatorTable::put(data, AreaAllocatorTable::split, qint32(node->split));
        AreaAllocatorTable::put(data, AreaAllocatorTable::splitType, quint32(node->splitType));

        quint8 flags =
                (node->isOccupied ? AreaAllocatorTable::IsOccupied : 0)
              | (node->left != nullptr ? AreaAllocatorTable::HasLeft : 0)
              | (node->right != nullptr ? AreaAllocatorTable::HasRight : 0);
        AreaAllocatorTable::put(data, AreaAllocatorTable::flags, flags);
        data += AreaAllocatorTable::NodeSize;
    }

    return ret;
}

const char *QSGAreaAllocator::deserialize(const char *data, int size)
{
    if (uint(size) < AreaAllocatorTable::HeaderSize) {
        qWarning("QSGAreaAllocator::deserialize: Data not long enough to fit header");
        return nullptr;
    }

    const char *end = data + size;

    quint8 majorVersion = AreaAllocatorTable::fetch<quint8>(data, AreaAllocatorTable::majorVersion);
    quint8 minorVersion = AreaAllocatorTable::fetch<quint8>(data, AreaAllocatorTable::minorVersion);
    if (majorVersion != 5 || minorVersion != 12) {
        qWarning("Unrecognized version %d.%d of QSGAreaAllocator",
                 majorVersion,
                 minorVersion);
        return nullptr;
    }

    m_size = QSize(AreaAllocatorTable::fetch<quint32>(data, AreaAllocatorTable::width),
                   AreaAllocatorTable::fetch<quint32>(data, AreaAllocatorTable::height));

    Q_ASSERT(m_root != nullptr);
    Q_ASSERT(m_root->left == nullptr);
    Q_ASSERT(m_root->right == nullptr);

    QStack<QSGAreaAllocatorNode *> nodes;
    nodes.push(m_root);

    data += AreaAllocatorTable::HeaderSize;
    while (!nodes.isEmpty()) {
        if (data + AreaAllocatorTable::NodeSize > end) {
            qWarning("QSGAreaAllocator::deseriable: Data not long enough for nodes");
            return nullptr;
        }

        QSGAreaAllocatorNode *node = nodes.pop();

        node->split = AreaAllocatorTable::fetch<qint32>(data, AreaAllocatorTable::split);
        node->splitType = SplitType(AreaAllocatorTable::fetch<quint32>(data, AreaAllocatorTable::splitType));

        quint8 flags = AreaAllocatorTable::fetch<quint8>(data, AreaAllocatorTable::flags);
        node->isOccupied = flags & AreaAllocatorTable::IsOccupied;

        if (flags & AreaAllocatorTable::HasLeft) {
            node->left = new QSGAreaAllocatorNode(node);
            nodes.push(node->left);
        }

        if (flags & AreaAllocatorTable::HasRight) {
            node->right = new QSGAreaAllocatorNode(node);
            nodes.push(node->right);
        }

        data += AreaAllocatorTable::NodeSize;
    }

    return data;
}

QT_END_NAMESPACE
