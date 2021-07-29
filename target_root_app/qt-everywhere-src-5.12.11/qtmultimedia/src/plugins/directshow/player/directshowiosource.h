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

#ifndef DIRECTSHOWIOSOURCE_H
#define DIRECTSHOWIOSOURCE_H

#include "directshowglobal.h"
#include "directshowioreader.h"
#include "directshowmediatype.h"

#include <QtCore/qfile.h>

QT_BEGIN_NAMESPACE

class DirectShowIOSource
    : public IBaseFilter
    , public IAMFilterMiscFlags
    , public IPin
{
    Q_DISABLE_COPY(DirectShowIOSource)
public:
    DirectShowIOSource(DirectShowEventLoop *loop);
    virtual ~DirectShowIOSource();

    void setDevice(QIODevice *device);
    void setAllocator(IMemAllocator *allocator);

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    // IPersist
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClassID) override;

    // IMediaFilter
    HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart) override;
    HRESULT STDMETHODCALLTYPE Pause() override;
    HRESULT STDMETHODCALLTYPE Stop() override;

    HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *pState) override;

    HRESULT STDMETHODCALLTYPE SetSyncSource(IReferenceClock *pClock) override;
    HRESULT STDMETHODCALLTYPE GetSyncSource(IReferenceClock **ppClock) override;

    // IBaseFilter
    HRESULT STDMETHODCALLTYPE EnumPins(IEnumPins **ppEnum) override;
    HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR Id, IPin **ppPin) override;

    HRESULT STDMETHODCALLTYPE JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName) override;

    HRESULT STDMETHODCALLTYPE QueryFilterInfo(FILTER_INFO *pInfo) override;
    HRESULT STDMETHODCALLTYPE QueryVendorInfo(LPWSTR *pVendorInfo) override;

    // IAMFilterMiscFlags
    ULONG STDMETHODCALLTYPE GetMiscFlags() override;

    // IPin
    HRESULT STDMETHODCALLTYPE Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt) override;
    HRESULT STDMETHODCALLTYPE ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt) override;
    HRESULT STDMETHODCALLTYPE Disconnect() override;
    HRESULT STDMETHODCALLTYPE ConnectedTo(IPin **ppPin) override;

    HRESULT STDMETHODCALLTYPE ConnectionMediaType(AM_MEDIA_TYPE *pmt) override;

    HRESULT STDMETHODCALLTYPE QueryPinInfo(PIN_INFO *pInfo) override;
    HRESULT STDMETHODCALLTYPE QueryId(LPWSTR *Id) override;

    HRESULT STDMETHODCALLTYPE QueryAccept(const AM_MEDIA_TYPE *pmt) override;

    HRESULT STDMETHODCALLTYPE EnumMediaTypes(IEnumMediaTypes **ppEnum) override;

    HRESULT STDMETHODCALLTYPE QueryInternalConnections(IPin **apPin, ULONG *nPin) override;

    HRESULT STDMETHODCALLTYPE EndOfStream() override;

    HRESULT STDMETHODCALLTYPE BeginFlush() override;
    HRESULT STDMETHODCALLTYPE EndFlush() override;

    HRESULT STDMETHODCALLTYPE NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
                                         double dRate) override;

    HRESULT STDMETHODCALLTYPE QueryDirection(PIN_DIRECTION *pPinDir) override;

private:
    volatile LONG m_ref;
    FILTER_STATE m_state;
    DirectShowIOReader *m_reader;
    DirectShowEventLoop *m_loop;
    IFilterGraph *m_graph;
    IReferenceClock *m_clock;
    IMemAllocator *m_allocator;
    IPin *m_peerPin;
    DirectShowMediaType m_connectionMediaType;
    QList<DirectShowMediaType> m_supportedMediaTypes;
    QString m_filterName;
    const QString m_pinId;
    bool m_queriedForAsyncReader;
    QMutex m_mutex;
};

QT_END_NAMESPACE

#endif
