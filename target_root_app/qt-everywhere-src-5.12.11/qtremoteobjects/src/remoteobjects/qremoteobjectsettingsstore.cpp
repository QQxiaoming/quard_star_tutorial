/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
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

#include "qremoteobjectsettingsstore.h"

#include "qremoteobjectnode_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qsettings.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype SettingsStore
    \inqmlmodule QtRemoteObjects
    \brief A basic store for persisted properties.

    This type provides simple QSettings-based storage for properties marked as PERSISTED. It is used in
    conjunction with Node::persistedStore:

    \code
    Node {
        persistedStore: SettingsStore {}
    }
    \endcode
*/

class QRemoteObjectSettingsStorePrivate : public QRemoteObjectAbstractPersistedStorePrivate
{
public:
    QRemoteObjectSettingsStorePrivate();
    virtual ~QRemoteObjectSettingsStorePrivate();

    QSettings settings;
    Q_DECLARE_PUBLIC(QRemoteObjectSettingsStore)
};

QRemoteObjectSettingsStorePrivate::QRemoteObjectSettingsStorePrivate()
{
}

QRemoteObjectSettingsStorePrivate::~QRemoteObjectSettingsStorePrivate()
{
}

QRemoteObjectSettingsStore::QRemoteObjectSettingsStore(QObject *parent)
    : QRemoteObjectAbstractPersistedStore(*new QRemoteObjectSettingsStorePrivate, parent)
{
}

QRemoteObjectSettingsStore::~QRemoteObjectSettingsStore()
{
}

QVariantList QRemoteObjectSettingsStore::restoreProperties(const QString &repName, const QByteArray &repSig)
{
    Q_D(QRemoteObjectSettingsStore);
    d->settings.beginGroup(repName + QLatin1Char('/') + QString::fromLatin1(repSig));
    const QVariantList values = d->settings.value(QStringLiteral("values")).toList();
    d->settings.endGroup();
    return values;
}

void QRemoteObjectSettingsStore::saveProperties(const QString &repName, const QByteArray &repSig, const QVariantList &values)
{
    Q_D(QRemoteObjectSettingsStore);
    d->settings.beginGroup(repName + QLatin1Char('/') + QString::fromLatin1(repSig));
    d->settings.setValue(QStringLiteral("values"), values);
    d->settings.endGroup();
    d->settings.sync();
}

QT_END_NAMESPACE
