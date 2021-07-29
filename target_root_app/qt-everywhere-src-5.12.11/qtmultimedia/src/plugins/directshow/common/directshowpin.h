/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef DIRECTSHOWPIN_H
#define DIRECTSHOWPIN_H

#include "directshowobject.h"

#include "directshowmediatype.h"
#include <qstring.h>
#include <qmutex.h>

QT_BEGIN_NAMESPACE

class DirectShowBaseFilter;

class DirectShowPin : public IPin
{
public:
    virtual ~DirectShowPin();

    QString name() const { return m_name; }
    bool isConnected() const { return m_peerPin != nullptr; }

    virtual bool isMediaTypeSupported(const AM_MEDIA_TYPE *type) = 0;
    virtual QList<DirectShowMediaType> supportedMediaTypes();
    virtual bool setMediaType(const AM_MEDIA_TYPE *type);

    virtual HRESULT completeConnection(IPin *pin);
    virtual HRESULT connectionEnded();

    virtual HRESULT setActive(bool active);

    // IPin
    STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt) override;
    STDMETHODIMP ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt) override;
    STDMETHODIMP Disconnect() override;
    STDMETHODIMP ConnectedTo(IPin **ppPin) override;

    STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt) override;

    STDMETHODIMP QueryPinInfo(PIN_INFO *pInfo) override;
    STDMETHODIMP QueryId(LPWSTR *Id) override;

    STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt) override;

    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum) override;

    STDMETHODIMP QueryInternalConnections(IPin **apPin, ULONG *nPin) override;

    STDMETHODIMP EndOfStream() override;

    STDMETHODIMP BeginFlush() override;
    STDMETHODIMP EndFlush() override;

    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate) override;

    STDMETHODIMP QueryDirection(PIN_DIRECTION *pPinDir) override;

protected:
    DirectShowPin(DirectShowBaseFilter *filter, const QString &name, PIN_DIRECTION direction);

    QMutex m_mutex;

    DirectShowBaseFilter *m_filter;
    QString m_name;
    PIN_DIRECTION m_direction;

    IPin *m_peerPin;
    DirectShowMediaType m_mediaType;

private:
    Q_DISABLE_COPY(DirectShowPin)
    HRESULT tryMediaTypes(IPin *pin, const AM_MEDIA_TYPE *type, IEnumMediaTypes *enumMediaTypes);
    HRESULT tryConnect(IPin *pin, const AM_MEDIA_TYPE *type);
};


class DirectShowOutputPin : public DirectShowPin
{
public:
    ~DirectShowOutputPin() override;

    // DirectShowPin
    HRESULT completeConnection(IPin *pin) override;
    HRESULT connectionEnded() override;
    HRESULT setActive(bool active) override;

    // IPin
    STDMETHODIMP EndOfStream() override;

protected:
    DirectShowOutputPin(DirectShowBaseFilter *filter, const QString &name);

    IMemAllocator *m_allocator;
    IMemInputPin *m_inputPin;

private:
    Q_DISABLE_COPY(DirectShowOutputPin)
};


class DirectShowInputPin : public DirectShowPin
                         , public IMemInputPin
{
public:
     ~DirectShowInputPin() override;

    const AM_SAMPLE2_PROPERTIES *currentSampleProperties() const { return &m_sampleProperties; }

    // DirectShowPin
    HRESULT connectionEnded() override;
    HRESULT setActive(bool active) override;

    // IPin
    STDMETHODIMP EndOfStream() override;
    STDMETHODIMP BeginFlush() override;
    STDMETHODIMP EndFlush() override;

    // IMemInputPin
    STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator) override;
    STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly) override;
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps) override;

    STDMETHODIMP Receive(IMediaSample *pSample) override;
    STDMETHODIMP ReceiveMultiple(IMediaSample **pSamples, long nSamples,
                                 long *nSamplesProcessed) override;
    STDMETHODIMP ReceiveCanBlock() override;

protected:
    DirectShowInputPin(DirectShowBaseFilter *filter, const QString &name);

    IMemAllocator *m_allocator;
    bool m_flushing;
    bool m_inErrorState;
    AM_SAMPLE2_PROPERTIES m_sampleProperties;

private:
    Q_DISABLE_COPY(DirectShowInputPin)
};

QT_END_NAMESPACE

#endif // DIRECTSHOWPIN_H
