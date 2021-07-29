/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmllistaccessor_p.h"

#include <private/qqmlmetatype_p.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qdebug.h>

// ### Remove me
#include <private/qqmlengine_p.h>

QT_BEGIN_NAMESPACE

QQmlListAccessor::QQmlListAccessor()
: m_type(Invalid)
{
}

QQmlListAccessor::~QQmlListAccessor()
{
}

QVariant QQmlListAccessor::list() const
{
    return d;
}

void QQmlListAccessor::setList(const QVariant &v, QQmlEngine *engine)
{
    d = v;

    // An incoming JS array as model is treated as a variant list, so we need to
    // convert it first with toVariant().
    if (d.userType() == qMetaTypeId<QJSValue>())
        d = d.value<QJSValue>().toVariant();

    QQmlEnginePrivate *enginePrivate = engine?QQmlEnginePrivate::get(engine):nullptr;

    if (!d.isValid()) {
        m_type = Invalid;
    } else if (d.userType() == QVariant::StringList) {
        m_type = StringList;
    } else if (d.userType() == QMetaType::QVariantList) {
        m_type = VariantList;
    } else if (d.canConvert(QVariant::Int)) {
        // Here we have to check for an upper limit, because down the line code might (well, will)
        // allocate memory depending on the number of elements. The upper limit cannot be INT_MAX:
        //      QVector<QPointer<QQuickItem>> something;
        //      something.resize(count());
        // (See e.g. QQuickRepeater::regenerate())
        // This will allocate data along the lines of:
        //      sizeof(QPointer<QQuickItem>) * count() + QVector::headerSize
        // So, doing an approximate round-down-to-nice-number, we get:
        const int upperLimit = 100 * 1000 * 1000;

        int i = v.toInt();
        if (i < 0) {
            qWarning("Model size of %d is less than 0", i);
            m_type = Invalid;
        } else if (i > upperLimit) {
            qWarning("Model size of %d is bigger than the upper limit %d", i, upperLimit);
            m_type = Invalid;
        } else {
            m_type = Integer;
        }
    } else if ((!enginePrivate && QQmlMetaType::isQObject(d.userType())) ||
               (enginePrivate && enginePrivate->isQObject(d.userType()))) {
        QObject *data = enginePrivate?enginePrivate->toQObject(d):QQmlMetaType::toQObject(d);
        d = QVariant::fromValue(data);
        m_type = Instance;
    } else if (d.userType() == qMetaTypeId<QQmlListReference>()) {
        m_type = ListProperty;
    } else {
        m_type = Instance;
    }
}

int QQmlListAccessor::count() const
{
    switch(m_type) {
    case StringList:
        return qvariant_cast<QStringList>(d).count();
    case VariantList:
        return qvariant_cast<QVariantList>(d).count();
    case ListProperty:
        return ((const QQmlListReference *)d.constData())->count();
    case Instance:
        return 1;
    case Integer:
        return d.toInt();
    default:
    case Invalid:
        return 0;
    }
}

QVariant QQmlListAccessor::at(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < count());
    switch(m_type) {
    case StringList:
        return QVariant::fromValue(qvariant_cast<QStringList>(d).at(idx));
    case VariantList:
        return qvariant_cast<QVariantList>(d).at(idx);
    case ListProperty:
        return QVariant::fromValue(((const QQmlListReference *)d.constData())->at(idx));
    case Instance:
        return d;
    case Integer:
        return QVariant(idx);
    default:
    case Invalid:
        return QVariant();
    }
}

bool QQmlListAccessor::isValid() const
{
    return m_type != Invalid;
}

QT_END_NAMESPACE
