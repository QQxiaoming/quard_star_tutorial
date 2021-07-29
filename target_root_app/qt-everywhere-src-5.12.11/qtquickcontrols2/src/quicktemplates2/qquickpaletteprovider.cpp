/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
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

#include "qquickpaletteprovider_p.h"
#include "qquickpalette_p.h"

#include <QtQml/private/qqmlvaluetype_p.h>

QT_BEGIN_NAMESPACE

static QQmlValueTypeProvider *instance()
{
    static QQuickPaletteProvider provider;
    return &provider;
}

void QQuickPaletteProvider::init()
{
    QQml_addValueTypeProvider(instance());
}

void QQuickPaletteProvider::cleanup()
{
    QQml_removeValueTypeProvider(instance());
}

#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
    #define ASSERT_VALID_SIZE(size, min) Q_UNUSED(size)
#else
    #define ASSERT_VALID_SIZE(size, min) Q_ASSERT(size >= min)
#endif

const QMetaObject *QQuickPaletteProvider::getMetaObjectForMetaType(int type)
{
    switch (type) {
    case QMetaType::QPalette:
        return &QQuickPalette::staticMetaObject;
    default:
        break;
    }

    return nullptr;
}

bool QQuickPaletteProvider::init(int type, QVariant& dst)
{
    switch (type) {
    case QMetaType::QPalette:
        dst.setValue<QPalette>(QPalette());
        return true;
    default: break;
    }

    return false;
}

template<typename T>
bool typedEqual(const void *lhs, const QVariant& rhs)
{
    return (*(reinterpret_cast<const T *>(lhs)) == rhs.value<T>());
}

bool QQuickPaletteProvider::equal(int type, const void *lhs, const QVariant &rhs)
{
    switch (type) {
    case QMetaType::QPalette:
        return typedEqual<QPalette>(lhs, rhs);
    default: break;
    }

    return false;
}

template<typename T>
bool typedStore(const void *src, void *dst, size_t dstSize)
{
    ASSERT_VALID_SIZE(dstSize, sizeof(T));
    const T *srcT = reinterpret_cast<const T *>(src);
    T *dstT = reinterpret_cast<T *>(dst);
    new (dstT) T(*srcT);
    return true;
}

bool QQuickPaletteProvider::store(int type, const void *src, void *dst, size_t dstSize)
{
    switch (type) {
    case QMetaType::QPalette:
        return typedStore<QPalette>(src, dst, dstSize);
    default: break;
    }

    return false;
}

template<typename T>
bool typedRead(const QVariant& src, int dstType, void *dst)
{
    T *dstT = reinterpret_cast<T *>(dst);
    if (src.type() == static_cast<uint>(dstType)) {
        *dstT = src.value<T>();
    } else {
        *dstT = T();
    }
    return true;
}

bool QQuickPaletteProvider::read(const QVariant &src, void *dst, int dstType)
{
    switch (dstType) {
    case QMetaType::QPalette:
        return typedRead<QPalette>(src, dstType, dst);
    default: break;
    }

    return false;
}

template<typename T>
bool typedWrite(const void *src, QVariant& dst)
{
    const T *srcT = reinterpret_cast<const T *>(src);
    if (dst.value<T>() != *srcT) {
        dst = *srcT;
        return true;
    }
    return false;
}

bool QQuickPaletteProvider::write(int type, const void *src, QVariant& dst)
{
    switch (type) {
    case QMetaType::QPalette:
        return typedWrite<QPalette>(src, dst);
    default: break;
    }

    return false;
}

#undef ASSERT_VALID_SIZE

QT_END_NAMESPACE
