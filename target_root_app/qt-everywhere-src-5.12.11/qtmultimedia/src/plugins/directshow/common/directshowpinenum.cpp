/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
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

#include "directshowpinenum.h"
#include "directshowbasefilter.h"

QT_BEGIN_NAMESPACE

DirectShowPinEnum::DirectShowPinEnum(DirectShowBaseFilter *filter)
    : m_filter(filter)
    , m_index(0)
{
    m_filter->AddRef();
    const QList<DirectShowPin *> pinList = filter->pins();
    for (DirectShowPin *pin : pinList) {
        pin->AddRef();
        m_pins.append(pin);
    }
}

DirectShowPinEnum::DirectShowPinEnum(const QList<IPin *> &pins)
    : m_filter(nullptr)
    , m_pins(pins)
    , m_index(0)
{
    for (IPin *pin : qAsConst(m_pins))
        pin->AddRef();
}

DirectShowPinEnum::~DirectShowPinEnum()
{
    for (IPin *pin : qAsConst(m_pins))
        pin->Release();
    if (m_filter)
        m_filter->Release();
}

HRESULT DirectShowPinEnum::QueryInterface(REFIID riid, void **ppv)
{
    if (ppv == nullptr)
        return E_POINTER;
    if (riid == IID_IUnknown)
        *ppv = static_cast<IUnknown *>(this);
    else if (riid == IID_IEnumPins)
        *ppv = static_cast<IEnumPins *>(this);
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

HRESULT DirectShowPinEnum::Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched)
{
    if (!ppPins || (!pcFetched && cPins != 1))
        return E_POINTER;

    ULONG count = qBound<ULONG>(0, cPins, m_pins.count() - m_index);

    for (ULONG i = 0; i < count; ++i, ++m_index) {
        ppPins[i] = m_pins.at(m_index);
        ppPins[i]->AddRef();
    }

    if (pcFetched)
        *pcFetched = count;

    return count == cPins ? S_OK : S_FALSE;
}

HRESULT DirectShowPinEnum::Skip(ULONG cPins)
{
    m_index = qMin(int(m_index + cPins), m_pins.count());

    return m_index < m_pins.count() ? S_OK : S_FALSE;
}

HRESULT DirectShowPinEnum::Reset()
{
    m_index = 0;

    return S_OK;
}

HRESULT DirectShowPinEnum::Clone(IEnumPins **ppEnum)
{
    if (!ppEnum)
        return E_POINTER;
    *ppEnum = m_filter ? new DirectShowPinEnum(m_filter) : new DirectShowPinEnum(m_pins);
    return S_OK;
}

QT_END_NAMESPACE
