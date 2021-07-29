/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "qqmlpropertycachecreator_p.h"

#include <private/qqmlengine_p.h>

QT_BEGIN_NAMESPACE

QAtomicInt QQmlPropertyCacheCreatorBase::classIndexCounter(0);

QQmlBindingInstantiationContext::QQmlBindingInstantiationContext(int referencingObjectIndex, const QV4::CompiledData::Binding *instantiatingBinding,
                                                                 const QString &instantiatingPropertyName, QQmlPropertyCache *referencingObjectPropertyCache)
    : referencingObjectIndex(referencingObjectIndex)
    , instantiatingBinding(instantiatingBinding)
    , instantiatingPropertyName(instantiatingPropertyName)
    , referencingObjectPropertyCache(referencingObjectPropertyCache)
{
}

bool QQmlBindingInstantiationContext::resolveInstantiatingProperty()
{
    if (!instantiatingBinding || instantiatingBinding->type != QV4::CompiledData::Binding::Type_GroupProperty)
        return true;

    Q_ASSERT(referencingObjectIndex >= 0);
    Q_ASSERT(referencingObjectPropertyCache);
    Q_ASSERT(instantiatingBinding->propertyNameIndex != 0);

    bool notInRevision = false;
    instantiatingProperty = QmlIR::PropertyResolver(referencingObjectPropertyCache).property(instantiatingPropertyName, &notInRevision, QmlIR::PropertyResolver::IgnoreRevision);
    return instantiatingProperty != nullptr;
}

QQmlRefPointer<QQmlPropertyCache> QQmlBindingInstantiationContext::instantiatingPropertyCache(QQmlEnginePrivate *enginePrivate) const
{
    if (instantiatingProperty) {
        if (instantiatingProperty->isQObject()) {
            return enginePrivate->rawPropertyCacheForType(instantiatingProperty->propType(), instantiatingProperty->typeMinorVersion());
        } else if (const QMetaObject *vtmo = QQmlValueTypeFactory::metaObjectForMetaType(instantiatingProperty->propType())) {
            return enginePrivate->cache(vtmo, instantiatingProperty->typeMinorVersion());
        }
    }
    return QQmlRefPointer<QQmlPropertyCache>();
}

void QQmlPendingGroupPropertyBindings::resolveMissingPropertyCaches(QQmlEnginePrivate *enginePrivate, QQmlPropertyCacheVector *propertyCaches) const
{
    for (QQmlBindingInstantiationContext pendingBinding: *this) {
        const int groupPropertyObjectIndex = pendingBinding.instantiatingBinding->value.objectIndex;

        if (propertyCaches->at(groupPropertyObjectIndex))
            continue;

        if (!pendingBinding.resolveInstantiatingProperty())
            continue;

        auto cache = pendingBinding.instantiatingPropertyCache(enginePrivate);
        propertyCaches->set(groupPropertyObjectIndex, cache);
    }
}

QT_END_NAMESPACE
