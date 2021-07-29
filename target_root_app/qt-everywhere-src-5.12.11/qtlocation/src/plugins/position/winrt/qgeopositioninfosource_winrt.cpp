/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtPositioning module of the Qt Toolkit.
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

#include "qgeopositioninfosource_winrt_p.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qfunctions_winrt.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qmutex.h>
#ifdef Q_OS_WINRT
#include <QtCore/private/qeventdispatcher_winrt_p.h>
#endif

#include <functional>
#include <windows.system.h>
#include <windows.devices.geolocation.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Devices::Geolocation;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;

typedef ITypedEventHandler<Geolocator *, PositionChangedEventArgs *> GeoLocatorPositionHandler;
typedef ITypedEventHandler<Geolocator *, StatusChangedEventArgs *> GeoLocatorStatusHandler;
typedef IAsyncOperationCompletedHandler<Geoposition*> PositionHandler;
typedef IAsyncOperationCompletedHandler<GeolocationAccessStatus> AccessHandler;

Q_DECLARE_LOGGING_CATEGORY(lcPositioningWinRT)

Q_DECLARE_METATYPE(QGeoPositionInfoSource::Error)

QT_BEGIN_NAMESPACE

#ifndef Q_OS_WINRT
namespace QEventDispatcherWinRT {
HRESULT runOnXamlThread(const std::function<HRESULT ()> &delegate, bool waitForRun = true)
{
    Q_UNUSED(waitForRun);
    return delegate();
}
}

static inline HRESULT await(const ComPtr<IAsyncOperation<GeolocationAccessStatus>> &asyncOp,
                            GeolocationAccessStatus *result)
{
    ComPtr<IAsyncInfo> asyncInfo;
    HRESULT hr = asyncOp.As(&asyncInfo);
    if (FAILED(hr))
        return hr;

    AsyncStatus status;
    while (SUCCEEDED(hr = asyncInfo->get_Status(&status)) && status == AsyncStatus::Started)
        QThread::yieldCurrentThread();

    if (FAILED(hr) || status != AsyncStatus::Completed) {
        HRESULT ec;
        hr = asyncInfo->get_ErrorCode(&ec);
        if (FAILED(hr))
            return hr;
        hr = asyncInfo->Close();
        if (FAILED(hr))
            return hr;
        return ec;
    }

    if (FAILED(hr))
        return hr;

    return asyncOp->GetResults(result);
}
#endif // !Q_OS_WINRT

enum class InitializationState {
    Uninitialized,
    Initializing,
    Initialized
};

class QGeoPositionInfoSourceWinRTPrivate {
public:
    ComPtr<IGeolocator> locator;
    QTimer periodicTimer;
    QTimer singleUpdateTimer;
    QGeoPositionInfo lastPosition;
    QGeoPositionInfoSource::Error positionError = QGeoPositionInfoSource::NoError;
    EventRegistrationToken statusToken;
    EventRegistrationToken positionToken;
    QMutex mutex;
    bool updatesOngoing = false;
    int minimumUpdateInterval = -1;
    int updateInterval = -1;
    InitializationState initState = InitializationState::Uninitialized;

    PositionStatus positionStatus = PositionStatus_NotInitialized;
};

QGeoPositionInfoSourceWinRT::QGeoPositionInfoSourceWinRT(QObject *parent)
    : QGeoPositionInfoSource(parent)
    , d_ptr(new QGeoPositionInfoSourceWinRTPrivate)
{
    qRegisterMetaType<QGeoPositionInfoSource::Error>();
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    Q_D(QGeoPositionInfoSourceWinRT);
    d->positionError = QGeoPositionInfoSource::NoError;
    d->updatesOngoing = false;
    d->positionToken.value = 0;
    d->statusToken.value = 0;
}

QGeoPositionInfoSourceWinRT::~QGeoPositionInfoSourceWinRT()
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    CoUninitialize();
}

int QGeoPositionInfoSourceWinRT::init()
{
    Q_D(QGeoPositionInfoSourceWinRT);
    Q_ASSERT(d->initState != InitializationState::Initializing);
    if (d->initState == InitializationState::Initialized)
        return 0;

    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    d->initState = InitializationState::Initializing;
    if (!requestAccess()) {
        d->initState = InitializationState::Uninitialized;
        setError(QGeoPositionInfoSource::AccessError);
        qWarning ("Location access failed.");
        return -1;
    }
    HRESULT hr = QEventDispatcherWinRT::runOnXamlThread([this, d]() {
        HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Devices_Geolocation_Geolocator).Get(),
                                        &d->locator);
        RETURN_HR_IF_FAILED("Could not initialize native location services.");

        if (d->minimumUpdateInterval == -1) {
            UINT32 interval;
            hr = d->locator->get_ReportInterval(&interval);
            RETURN_HR_IF_FAILED("Could not retrieve report interval.");
            d->minimumUpdateInterval = static_cast<int>(interval);
        }
        if (d->updateInterval == -1)
            d->updateInterval = d->minimumUpdateInterval;
        setUpdateInterval(d->updateInterval);

        return hr;
    });
    if (FAILED(hr)) {
        d->initState = InitializationState::Uninitialized;
        setError(QGeoPositionInfoSource::UnknownSourceError);
        return -1;
    }

    d->periodicTimer.setSingleShot(true);
    connect(&d->periodicTimer, &QTimer::timeout, this, &QGeoPositionInfoSourceWinRT::virtualPositionUpdate);

    d->singleUpdateTimer.setSingleShot(true);
    connect(&d->singleUpdateTimer, &QTimer::timeout, this, &QGeoPositionInfoSourceWinRT::singleUpdateTimeOut);

    QGeoPositionInfoSource::PositioningMethods preferredMethods = preferredPositioningMethods();
    if (preferredMethods == QGeoPositionInfoSource::NoPositioningMethods)
        preferredMethods = QGeoPositionInfoSource::AllPositioningMethods;
    setPreferredPositioningMethods(preferredMethods);

    connect(this, &QGeoPositionInfoSourceWinRT::nativePositionUpdate, this, &QGeoPositionInfoSourceWinRT::updateSynchronized);
    d->initState = InitializationState::Initialized;
    return 0;
}

QGeoPositionInfo QGeoPositionInfoSourceWinRT::lastKnownPosition(bool fromSatellitePositioningMethodsOnly) const
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_D(const QGeoPositionInfoSourceWinRT);
    Q_UNUSED(fromSatellitePositioningMethodsOnly)
    return d->lastPosition;
}

QGeoPositionInfoSource::PositioningMethods QGeoPositionInfoSourceWinRT::supportedPositioningMethods() const
{
    return requestAccess() ? QGeoPositionInfoSource::AllPositioningMethods
                           : QGeoPositionInfoSource::NoPositioningMethods;
}

void QGeoPositionInfoSourceWinRT::setPreferredPositioningMethods(QGeoPositionInfoSource::PositioningMethods methods)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__ << methods;
    Q_D(QGeoPositionInfoSourceWinRT);

    PositioningMethods previousPreferredPositioningMethods = preferredPositioningMethods();
    QGeoPositionInfoSource::setPreferredPositioningMethods(methods);
    if (previousPreferredPositioningMethods == preferredPositioningMethods()
            || d->initState == InitializationState::Uninitialized) {
        return;
    }

    const bool needsRestart = d->positionToken.value != 0 || d->statusToken.value != 0;

    if (needsRestart)
        stopHandler();

    PositionAccuracy acc = methods & PositioningMethod::SatellitePositioningMethods ?
                PositionAccuracy::PositionAccuracy_High :
                PositionAccuracy::PositionAccuracy_Default;
    HRESULT hr = QEventDispatcherWinRT::runOnXamlThread([d, acc]() {
        return d->locator->put_DesiredAccuracy(acc);
    });
    RETURN_VOID_IF_FAILED("Could not set positioning accuracy.");

    if (needsRestart)
        startHandler();
}

void QGeoPositionInfoSourceWinRT::setUpdateInterval(int msec)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__ << msec;
    Q_D(QGeoPositionInfoSourceWinRT);
    if (d->initState == InitializationState::Uninitialized) {
        d->updateInterval = msec;
        return;
    }

    // minimumUpdateInterval is initialized to the lowest possible update interval in init().
    // Passing 0 will cause an error on Windows 10.
    // See https://docs.microsoft.com/en-us/uwp/api/windows.devices.geolocation.geolocator.reportinterval
    if (msec != 0 && msec < minimumUpdateInterval())
        msec = minimumUpdateInterval();

    const bool needsRestart = d->positionToken.value != 0 || d->statusToken.value != 0;

    if (needsRestart)
        stopHandler();

    HRESULT hr = d->locator->put_ReportInterval(static_cast<UINT32>(msec));
    if (FAILED(hr)) {
        setError(QGeoPositionInfoSource::UnknownSourceError);
        qErrnoWarning(hr, "Failed to set update interval");
        return;
    }

    d->updateInterval = msec;
    d->periodicTimer.setInterval(d->updateInterval);

    QGeoPositionInfoSource::setUpdateInterval(d->updateInterval);

    if (needsRestart)
        startHandler();
}

int QGeoPositionInfoSourceWinRT::minimumUpdateInterval() const
{
    Q_D(const QGeoPositionInfoSourceWinRT);
    return d->minimumUpdateInterval == -1 ? 1000 : d->minimumUpdateInterval;
}

void QGeoPositionInfoSourceWinRT::startUpdates()
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_D(QGeoPositionInfoSourceWinRT);

    setError(QGeoPositionInfoSource::NoError);
    if (init() < 0)
        return;

    if (d->updatesOngoing)
        return;

    if (!startHandler())
        return;
    d->updatesOngoing = true;
    d->periodicTimer.start();
}

void QGeoPositionInfoSourceWinRT::stopUpdates()
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_D(QGeoPositionInfoSourceWinRT);

    if (init() < 0)
        return;

    stopHandler();
    d->updatesOngoing = false;
    d->periodicTimer.stop();
}

bool QGeoPositionInfoSourceWinRT::startHandler()
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_D(QGeoPositionInfoSourceWinRT);

    // Check if already attached
    if (d->positionToken.value != 0)
        return true;

    if (preferredPositioningMethods() == QGeoPositionInfoSource::NoPositioningMethods) {
        setError(QGeoPositionInfoSource::UnknownSourceError);
        return false;
    }

    if (!requestAccess()) {
        setError(QGeoPositionInfoSource::AccessError);
        return false;
    }

    HRESULT hr = QEventDispatcherWinRT::runOnXamlThread([this, d]() {
        HRESULT hr;

        // We need to call this at least once on Windows 10 Mobile.
        // Unfortunately this operation does not have a completion handler
        // registered. That could have helped in the single update case
        ComPtr<IAsyncOperation<Geoposition*>> op;
        hr = d->locator->GetGeopositionAsync(&op);
        RETURN_HR_IF_FAILED("Could not start position operation");

        hr = d->locator->add_PositionChanged(Callback<GeoLocatorPositionHandler>(this,
                                                                                 &QGeoPositionInfoSourceWinRT::onPositionChanged).Get(),
                                             &d->positionToken);
        RETURN_HR_IF_FAILED("Could not add position handler");

        hr = d->locator->add_StatusChanged(Callback<GeoLocatorStatusHandler>(this,
                                                                                 &QGeoPositionInfoSourceWinRT::onStatusChanged).Get(),
                                             &d->statusToken);
        RETURN_HR_IF_FAILED("Could not add status handler");
        return hr;
    });
    if (FAILED(hr)) {
        setError(QGeoPositionInfoSource::UnknownSourceError);
        return false;
    }

    return true;
}

void QGeoPositionInfoSourceWinRT::stopHandler()
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_D(QGeoPositionInfoSourceWinRT);

    if (!d->positionToken.value)
        return;
    QEventDispatcherWinRT::runOnXamlThread([d]() {
        d->locator->remove_PositionChanged(d->positionToken);
        d->locator->remove_StatusChanged(d->statusToken);
        return S_OK;
    });
    d->positionToken.value = 0;
    d->statusToken.value = 0;
}

void QGeoPositionInfoSourceWinRT::requestUpdate(int timeout)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__ << timeout;
    Q_D(QGeoPositionInfoSourceWinRT);

    if (init() < 0)
        return;

    setError(QGeoPositionInfoSource::NoError);
    if (timeout != 0 && timeout < minimumUpdateInterval()) {
        emit updateTimeout();
        return;
    }

    if (timeout == 0)
        timeout = 2*60*1000; // Maximum time for cold start (see Android)

    startHandler();
    d->singleUpdateTimer.start(timeout);
}

void QGeoPositionInfoSourceWinRT::virtualPositionUpdate()
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_D(QGeoPositionInfoSourceWinRT);
    QMutexLocker locker(&d->mutex);

    // The operating system did not provide information in time
    // Hence we send a virtual position update to keep same behavior
    // between backends.
    // This only applies to the periodic timer, not for single requests
    // We can only do this if we received a valid position before
    if (d->lastPosition.isValid()) {
        QGeoPositionInfo sent = d->lastPosition;
        sent.setTimestamp(sent.timestamp().addMSecs(updateInterval()));
        d->lastPosition = sent;
        emit positionUpdated(sent);
    }
    d->periodicTimer.start();
}

void QGeoPositionInfoSourceWinRT::singleUpdateTimeOut()
{
    Q_D(QGeoPositionInfoSourceWinRT);
    QMutexLocker locker(&d->mutex);

    if (d->singleUpdateTimer.isActive()) {
        emit updateTimeout();
        if (!d->updatesOngoing)
            stopHandler();
    }
}

void QGeoPositionInfoSourceWinRT::updateSynchronized(QGeoPositionInfo currentInfo)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__ << currentInfo;
    Q_D(QGeoPositionInfoSourceWinRT);
    QMutexLocker locker(&d->mutex);

    d->periodicTimer.stop();
    d->lastPosition = currentInfo;

    if (d->updatesOngoing)
        d->periodicTimer.start();

    if (d->singleUpdateTimer.isActive()) {
        d->singleUpdateTimer.stop();
        if (!d->updatesOngoing)
        stopHandler();
    }

    emit positionUpdated(currentInfo);
}

QGeoPositionInfoSource::Error QGeoPositionInfoSourceWinRT::error() const
{
    Q_D(const QGeoPositionInfoSourceWinRT);
    qCDebug(lcPositioningWinRT) << __FUNCTION__ << d->positionError;

    // If the last encountered error was "Access denied", it is possible that the location service
    // has been enabled by now so that we are clear again.
    if ((d->positionError == QGeoPositionInfoSource::AccessError
         || d->positionError == QGeoPositionInfoSource::UnknownSourceError) && requestAccess())
        return QGeoPositionInfoSource::NoError;

    return d->positionError;
}

void QGeoPositionInfoSourceWinRT::setError(QGeoPositionInfoSource::Error positionError)
{
    Q_D(QGeoPositionInfoSourceWinRT);

    if (positionError == d->positionError)
        return;

    qCDebug(lcPositioningWinRT) << __FUNCTION__ << positionError;
    d->positionError = positionError;
    if (positionError != QGeoPositionInfoSource::NoError)
        emit QGeoPositionInfoSource::error(positionError);
}

void QGeoPositionInfoSourceWinRT::reactOnError(QGeoPositionInfoSource::Error positionError)
{
    setError(positionError);
    stopUpdates();
}

HRESULT QGeoPositionInfoSourceWinRT::onPositionChanged(IGeolocator *locator, IPositionChangedEventArgs *args)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_UNUSED(locator);

    HRESULT hr;
    ComPtr<IGeoposition> position;
    hr = args->get_Position(&position);
    RETURN_HR_IF_FAILED("Could not access position object.");

    QGeoPositionInfo currentInfo;

    ComPtr<IGeocoordinate> coord;
    hr = position->get_Coordinate(&coord);
    if (FAILED(hr))
        qErrnoWarning(hr, "Could not access coordinate");

    ComPtr<IGeocoordinateWithPoint> pointCoordinate;
    hr = coord.As(&pointCoordinate);
    if (FAILED(hr))
        qErrnoWarning(hr, "Could not cast coordinate.");

    ComPtr<IGeopoint> point;
    hr = pointCoordinate->get_Point(&point);
    if (FAILED(hr))
        qErrnoWarning(hr, "Could not obtain coordinate's point.");

    BasicGeoposition pos;
    hr = point->get_Position(&pos);
    if (FAILED(hr))
        qErrnoWarning(hr, "Could not obtain point's position.");

    DOUBLE lat = pos.Latitude;
    DOUBLE lon = pos.Longitude;
    DOUBLE alt = pos.Altitude;

    bool altitudeAvailable = false;
    ComPtr<IGeoshape> shape;
    hr = point.As(&shape);
    if (SUCCEEDED(hr) && shape) {
        AltitudeReferenceSystem altitudeSystem;
        hr = shape->get_AltitudeReferenceSystem(&altitudeSystem);
        if (SUCCEEDED(hr) && altitudeSystem == AltitudeReferenceSystem_Geoid)
            altitudeAvailable = true;
    }
    if (altitudeAvailable)
        currentInfo.setCoordinate(QGeoCoordinate(lat, lon, alt));
    else
        currentInfo.setCoordinate(QGeoCoordinate(lat, lon));

    DOUBLE accuracy;
    hr = coord->get_Accuracy(&accuracy);
    if (SUCCEEDED(hr))
        currentInfo.setAttribute(QGeoPositionInfo::HorizontalAccuracy, accuracy);

    IReference<double> *altAccuracy;
    hr = coord->get_AltitudeAccuracy(&altAccuracy);
    if (SUCCEEDED(hr) && altAccuracy) {
        double value;
        hr = altAccuracy->get_Value(&value);
        currentInfo.setAttribute(QGeoPositionInfo::VerticalAccuracy, value);
    }

    IReference<double> *speed;
    hr = coord->get_Speed(&speed);
    if (SUCCEEDED(hr) && speed) {
        double value;
        hr = speed->get_Value(&value);
        currentInfo.setAttribute(QGeoPositionInfo::GroundSpeed, value);
    }

    IReference<double> *heading;
    hr = coord->get_Heading(&heading);
    if (SUCCEEDED(hr) && heading) {
        double value;
        hr = heading->get_Value(&value);
        double mod = 0;
        value = modf(value, &mod);
        value += static_cast<int>(mod) % 360;
        if (value >=0 && value <= 359) // get_Value might return nan/-nan
            currentInfo.setAttribute(QGeoPositionInfo::Direction, value);
    }

    DateTime dateTime;
    hr = coord->get_Timestamp(&dateTime);

    if (dateTime.UniversalTime > 0) {
        ULARGE_INTEGER uLarge;
        uLarge.QuadPart = dateTime.UniversalTime;
        FILETIME fileTime;
        fileTime.dwHighDateTime = uLarge.HighPart;
        fileTime.dwLowDateTime = uLarge.LowPart;
        SYSTEMTIME systemTime;
        if (FileTimeToSystemTime(&fileTime, &systemTime)) {
            currentInfo.setTimestamp(QDateTime(QDate(systemTime.wYear, systemTime.wMonth,
                                                     systemTime.wDay),
                                               QTime(systemTime.wHour, systemTime.wMinute,
                                                     systemTime.wSecond, systemTime.wMilliseconds),
                                               Qt::UTC));
        }
    }

    emit nativePositionUpdate(currentInfo);

    return S_OK;
}

static inline bool isDisabledStatus(PositionStatus status)
{
    return status == PositionStatus_NoData || status == PositionStatus_Disabled
            || status == PositionStatus_NotAvailable;
}

HRESULT QGeoPositionInfoSourceWinRT::onStatusChanged(IGeolocator *, IStatusChangedEventArgs *args)
{
    Q_D(QGeoPositionInfoSourceWinRT);

    const PositionStatus oldStatus = d->positionStatus;
    HRESULT hr = args->get_Status(&d->positionStatus);
    RETURN_HR_IF_FAILED("Could not obtain position status");
    qCDebug(lcPositioningWinRT) << __FUNCTION__ << d->positionStatus;
    QGeoPositionInfoSource::Error error = QGeoPositionInfoSource::NoError;
    switch (d->positionStatus) {
    case PositionStatus::PositionStatus_NotAvailable:
        error = QGeoPositionInfoSource::UnknownSourceError;
        break;
    case PositionStatus::PositionStatus_Disabled:
        error = QGeoPositionInfoSource::AccessError;
        break;
    case PositionStatus::PositionStatus_NoData:
        error = QGeoPositionInfoSource::ClosedError;
        break;
    }
    if (error != QGeoPositionInfoSource::NoError) {
        QMetaObject::invokeMethod(this, "reactOnError", Qt::QueuedConnection,
                                  Q_ARG(QGeoPositionInfoSource::Error,
                                        QGeoPositionInfoSource::UnknownSourceError));
    }

    if (isDisabledStatus(oldStatus) != isDisabledStatus(d->positionStatus))
        emit supportedPositioningMethodsChanged();

    return S_OK;
}

bool QGeoPositionInfoSourceWinRT::requestAccess() const
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    GeolocationAccessStatus accessStatus;
    static ComPtr<IGeolocatorStatics> statics;

    ComPtr<IAsyncOperation<GeolocationAccessStatus>> op;
    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([&op]() {
        HRESULT hr;
        if (!statics) {
            hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Geolocation_Geolocator).Get(),
                                        IID_PPV_ARGS(&statics));
            RETURN_HR_IF_FAILED("Could not access Geolocation Statics.");
        }

        hr = statics->RequestAccessAsync(&op);
        return hr;
    });
    if (FAILED(hr)) {
        qCDebug(lcPositioningWinRT) << __FUNCTION__ << "Requesting access from Xaml thread failed";
        return false;
    }

    // We cannot wait inside the XamlThread as that would deadlock
#ifdef Q_OS_WINRT
    QWinRTFunctions::await(op, &accessStatus);
#else
    await(op, &accessStatus);
#endif
    return accessStatus == GeolocationAccessStatus_Allowed;
}

QT_END_NAMESPACE
