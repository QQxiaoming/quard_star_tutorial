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

#include "qqmltypeloader_p.h"
#include "qqmlabstracturlinterceptor.h"
#include "qqmlexpression_p.h"

#include <private/qqmlengine_p.h>
#include <private/qqmlglobal_p.h>
#include <private/qqmlthread_p.h>
#include <private/qv4codegen_p.h>
#include <private/qqmlcomponent_p.h>
#include <private/qqmlprofiler_p.h>
#include <private/qqmlmemoryprofiler_p.h>
#include <private/qqmltypecompiler_p.h>
#include <private/qqmlpropertyvalidator_p.h>
#include <private/qqmlpropertycachecreator_p.h>
#include <private/qv4module_p.h>

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtCore/qthread.h>
#include <QtQml/qqmlfile.h>
#include <QtCore/qdiriterator.h>
#include <QtQml/qqmlcomponent.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qloggingcategory.h>
#include <QtQml/qqmlextensioninterface.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/qscopeguard.h>

#include <functional>

#if defined (Q_OS_UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#if defined (QT_LINUXBASE)
// LSB doesn't declare NAME_MAX. Use SYMLINK_MAX instead, which seems to
// always be identical to NAME_MAX
#ifndef NAME_MAX
#  define NAME_MAX _POSIX_SYMLINK_MAX
#endif

#endif

// #define DATABLOB_DEBUG

#ifdef DATABLOB_DEBUG

#define ASSERT_MAINTHREAD() do { if (m_thread->isThisThread()) qFatal("QQmlTypeLoader: Caller not in main thread"); } while (false)
#define ASSERT_LOADTHREAD() do { if (!m_thread->isThisThread()) qFatal("QQmlTypeLoader: Caller not in load thread"); } while (false)
#define ASSERT_CALLBACK() do { if (!m_typeLoader || !m_typeLoader->m_thread->isThisThread()) qFatal("QQmlDataBlob: An API call was made outside a callback"); } while (false)

#else

#define ASSERT_MAINTHREAD()
#define ASSERT_LOADTHREAD()
#define ASSERT_CALLBACK()

#endif

DEFINE_BOOL_CONFIG_OPTION(dumpErrors, QML_DUMP_ERRORS);
DEFINE_BOOL_CONFIG_OPTION(disableDiskCache, QML_DISABLE_DISK_CACHE);
DEFINE_BOOL_CONFIG_OPTION(forceDiskCache, QML_FORCE_DISK_CACHE);

Q_DECLARE_LOGGING_CATEGORY(DBG_DISK_CACHE)
Q_LOGGING_CATEGORY(DBG_DISK_CACHE, "qt.qml.diskcache")

QT_BEGIN_NAMESPACE

namespace {

    template<typename LockType>
    struct LockHolder
    {
        LockType& lock;
        LockHolder(LockType *l) : lock(*l) { lock.lock(); }
        ~LockHolder() { lock.unlock(); }
    };
}

#if QT_CONFIG(qml_network)
// This is a lame object that we need to ensure that slots connected to
// QNetworkReply get called in the correct thread (the loader thread).
// As QQmlTypeLoader lives in the main thread, and we can't use
// Qt::DirectConnection connections from a QNetworkReply (because then
// sender() wont work), we need to insert this object in the middle.
class QQmlTypeLoaderNetworkReplyProxy : public QObject
{
    Q_OBJECT
public:
    QQmlTypeLoaderNetworkReplyProxy(QQmlTypeLoader *l);

public slots:
    void finished();
    void downloadProgress(qint64, qint64);
    void manualFinished(QNetworkReply*);

private:
    QQmlTypeLoader *l;
};
#endif // qml_network

class QQmlTypeLoaderThread : public QQmlThread
{
    typedef QQmlTypeLoaderThread This;

public:
    QQmlTypeLoaderThread(QQmlTypeLoader *loader);
#if QT_CONFIG(qml_network)
    QNetworkAccessManager *networkAccessManager() const;
    QQmlTypeLoaderNetworkReplyProxy *networkReplyProxy() const;
#endif // qml_network
    void load(QQmlDataBlob *b);
    void loadAsync(QQmlDataBlob *b);
    void loadWithStaticData(QQmlDataBlob *b, const QByteArray &);
    void loadWithStaticDataAsync(QQmlDataBlob *b, const QByteArray &);
    void loadWithCachedUnit(QQmlDataBlob *b, const QV4::CompiledData::Unit *unit);
    void loadWithCachedUnitAsync(QQmlDataBlob *b, const QV4::CompiledData::Unit *unit);
    void callCompleted(QQmlDataBlob *b);
    void callDownloadProgressChanged(QQmlDataBlob *b, qreal p);
    void initializeEngine(QQmlExtensionInterface *, const char *);

protected:
    void shutdownThread() override;

private:
    void loadThread(QQmlDataBlob *b);
    void loadWithStaticDataThread(QQmlDataBlob *b, const QByteArray &);
    void loadWithCachedUnitThread(QQmlDataBlob *b, const QV4::CompiledData::Unit *unit);
    void callCompletedMain(QQmlDataBlob *b);
    void callDownloadProgressChangedMain(QQmlDataBlob *b, qreal p);
    void initializeEngineMain(QQmlExtensionInterface *iface, const char *uri);

    QQmlTypeLoader *m_loader;
#if QT_CONFIG(qml_network)
    mutable QNetworkAccessManager *m_networkAccessManager;
    mutable QQmlTypeLoaderNetworkReplyProxy *m_networkReplyProxy;
#endif // qml_network
};

#if QT_CONFIG(qml_network)
QQmlTypeLoaderNetworkReplyProxy::QQmlTypeLoaderNetworkReplyProxy(QQmlTypeLoader *l)
: l(l)
{
}

void QQmlTypeLoaderNetworkReplyProxy::finished()
{
    Q_ASSERT(sender());
    Q_ASSERT(qobject_cast<QNetworkReply *>(sender()));
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    l->networkReplyFinished(reply);
}

void QQmlTypeLoaderNetworkReplyProxy::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_ASSERT(sender());
    Q_ASSERT(qobject_cast<QNetworkReply *>(sender()));
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    l->networkReplyProgress(reply, bytesReceived, bytesTotal);
}

// This function is for when you want to shortcut the signals and call directly
void QQmlTypeLoaderNetworkReplyProxy::manualFinished(QNetworkReply *reply)
{
    qint64 replySize = reply->size();
    l->networkReplyProgress(reply, replySize, replySize);
    l->networkReplyFinished(reply);
}
#endif // qml_network

/*!
\class QQmlDataBlob
\brief The QQmlDataBlob encapsulates a data request that can be issued to a QQmlTypeLoader.
\internal

QQmlDataBlob's are loaded by a QQmlTypeLoader.  The user creates the QQmlDataBlob
and then calls QQmlTypeLoader::load() or QQmlTypeLoader::loadWithStaticData() to load it.
The QQmlTypeLoader invokes callbacks on the QQmlDataBlob as data becomes available.
*/

/*!
\enum QQmlDataBlob::Status

This enum describes the status of the data blob.

\list
\li Null The blob has not yet been loaded by a QQmlTypeLoader
\li Loading The blob is loading network data.  The QQmlDataBlob::setData() callback has not yet been
invoked or has not yet returned.
\li WaitingForDependencies The blob is waiting for dependencies to be done before continueing.  This status
only occurs after the QQmlDataBlob::setData() callback has been made, and when the blob has outstanding
dependencies.
\li Complete The blob's data has been loaded and all dependencies are done.
\li Error An error has been set on this blob.
\endlist
*/

/*!
\enum QQmlDataBlob::Type

This enum describes the type of the data blob.

\list
\li QmlFile This is a QQmlTypeData
\li JavaScriptFile This is a QQmlScriptData
\li QmldirFile This is a QQmlQmldirData
\endlist
*/

/*!
Create a new QQmlDataBlob for \a url and of the provided \a type.
*/
QQmlDataBlob::QQmlDataBlob(const QUrl &url, Type type, QQmlTypeLoader *manager)
: m_typeLoader(manager), m_type(type), m_url(url), m_finalUrl(url), m_redirectCount(0),
  m_inCallback(false), m_isDone(false)
{
    //Set here because we need to get the engine from the manager
    if (m_typeLoader->engine() && m_typeLoader->engine()->urlInterceptor())
        m_url = m_typeLoader->engine()->urlInterceptor()->intercept(m_url,
                    (QQmlAbstractUrlInterceptor::DataType)m_type);
}

/*!  \internal */
QQmlDataBlob::~QQmlDataBlob()
{
    Q_ASSERT(m_waitingOnMe.isEmpty());

    cancelAllWaitingFor();
}

/*!
  Must be called before loading can occur.
*/
void QQmlDataBlob::startLoading()
{
    Q_ASSERT(status() == QQmlDataBlob::Null);
    m_data.setStatus(QQmlDataBlob::Loading);
}

/*!
Returns the type provided to the constructor.
*/
QQmlDataBlob::Type QQmlDataBlob::type() const
{
    return m_type;
}

/*!
Returns the blob's status.
*/
QQmlDataBlob::Status QQmlDataBlob::status() const
{
    return m_data.status();
}

/*!
Returns true if the status is Null.
*/
bool QQmlDataBlob::isNull() const
{
    return status() == Null;
}

/*!
Returns true if the status is Loading.
*/
bool QQmlDataBlob::isLoading() const
{
    return status() == Loading;
}

/*!
Returns true if the status is WaitingForDependencies.
*/
bool QQmlDataBlob::isWaiting() const
{
    return status() == WaitingForDependencies ||
            status() == ResolvingDependencies;
}

/*!
Returns true if the status is Complete.
*/
bool QQmlDataBlob::isComplete() const
{
    return status() == Complete;
}

/*!
Returns true if the status is Error.
*/
bool QQmlDataBlob::isError() const
{
    return status() == Error;
}

/*!
Returns true if the status is Complete or Error.
*/
bool QQmlDataBlob::isCompleteOrError() const
{
    Status s = status();
    return s == Error || s == Complete;
}

/*!
Returns the data download progress from 0 to 1.
*/
qreal QQmlDataBlob::progress() const
{
    quint8 p = m_data.progress();
    if (p == 0xFF) return 1.;
    else return qreal(p) / qreal(0xFF);
}

/*!
Returns the physical url of the data.  Initially this is the same as
finalUrl(), but if a URL interceptor is set, it will work on this URL
and leave finalUrl() alone.

\sa finalUrl()
*/
QUrl QQmlDataBlob::url() const
{
    return m_url;
}

QString QQmlDataBlob::urlString() const
{
    if (m_urlString.isEmpty())
        m_urlString = m_url.toString();

    return m_urlString;
}

/*!
Returns the logical URL to be used for resolving further URLs referred to in
the code.

This is the blob url passed to the constructor. If a URL interceptor rewrites
the URL, this one stays the same. If a network redirect happens while fetching
the data, this url is updated to reflect the new location. Therefore, if both
an interception and a redirection happen, the final url will indirectly
incorporate the result of the interception, potentially breaking further
lookups.

\sa url()
*/
QUrl QQmlDataBlob::finalUrl() const
{
    return m_finalUrl;
}

/*!
Returns the finalUrl() as a string.
*/
QString QQmlDataBlob::finalUrlString() const
{
    if (m_finalUrlString.isEmpty())
        m_finalUrlString = m_finalUrl.toString();

    return m_finalUrlString;
}

/*!
Return the errors on this blob.

May only be called from the load thread, or after the blob isCompleteOrError().
*/
QList<QQmlError> QQmlDataBlob::errors() const
{
    Q_ASSERT(isCompleteOrError() || (m_typeLoader && m_typeLoader->m_thread->isThisThread()));
    return m_errors;
}

/*!
Mark this blob as having \a errors.

All outstanding dependencies will be cancelled.  Requests to add new dependencies
will be ignored.  Entry into the Error state is irreversable.

The setError() method may only be called from within a QQmlDataBlob callback.
*/
void QQmlDataBlob::setError(const QQmlError &errors)
{
    ASSERT_CALLBACK();

    QList<QQmlError> l;
    l << errors;
    setError(l);
}

/*!
\overload
*/
void QQmlDataBlob::setError(const QList<QQmlError> &errors)
{
    ASSERT_CALLBACK();

    Q_ASSERT(status() != Error);
    Q_ASSERT(m_errors.isEmpty());

    m_errors = errors; // Must be set before the m_data fence
    m_data.setStatus(Error);

    if (dumpErrors()) {
        qWarning().nospace() << "Errors for " << urlString();
        for (int ii = 0; ii < errors.count(); ++ii)
            qWarning().nospace() << "    " << qPrintable(errors.at(ii).toString());
    }
    cancelAllWaitingFor();

    if (!m_inCallback)
        tryDone();
}

void QQmlDataBlob::setError(const QQmlCompileError &error)
{
    QQmlError e;
    e.setColumn(error.location.column);
    e.setLine(error.location.line);
    e.setDescription(error.description);
    e.setUrl(url());
    setError(e);
}

void QQmlDataBlob::setError(const QVector<QQmlCompileError> &errors)
{
    QList<QQmlError> finalErrors;
    finalErrors.reserve(errors.count());
    for (const QQmlCompileError &error: errors) {
        QQmlError e;
        e.setColumn(error.location.column);
        e.setLine(error.location.line);
        e.setDescription(error.description);
        e.setUrl(url());
        finalErrors << e;
    }
    setError(finalErrors);
}

void QQmlDataBlob::setError(const QString &description)
{
    QQmlError e;
    e.setDescription(description);
    e.setUrl(url());
    setError(e);
}

/*!
Wait for \a blob to become complete or to error.  If \a blob is already
complete or in error, or this blob is already complete, this has no effect.

The setError() method may only be called from within a QQmlDataBlob callback.
*/
void QQmlDataBlob::addDependency(QQmlDataBlob *blob)
{
    ASSERT_CALLBACK();

    Q_ASSERT(status() != Null);

    if (!blob ||
        blob->status() == Error || blob->status() == Complete ||
        status() == Error || status() == Complete || m_isDone)
        return;

    for (auto existingDep: qAsConst(m_waitingFor))
        if (existingDep.data() == blob)
            return;

    m_data.setStatus(WaitingForDependencies);

    m_waitingFor.append(blob);
    blob->m_waitingOnMe.append(this);
}

/*!
\fn void QQmlDataBlob::dataReceived(const Data &data)

Invoked when data for the blob is received.  Implementors should use this callback
to determine a blob's dependencies.  Within this callback you may call setError()
or addDependency().
*/

/*!
Invoked once data has either been received or a network error occurred, and all
dependencies are complete.

You can set an error in this method, but you cannot add new dependencies.  Implementors
should use this callback to finalize processing of data.

The default implementation does nothing.

XXX Rename processData() or some such to avoid confusion between done() (processing thread)
and completed() (main thread)
*/
void QQmlDataBlob::done()
{
}

#if QT_CONFIG(qml_network)
/*!
Invoked if there is a network error while fetching this blob.

The default implementation sets an appropriate QQmlError.
*/
void QQmlDataBlob::networkError(QNetworkReply::NetworkError networkError)
{
    Q_UNUSED(networkError);

    QQmlError error;
    error.setUrl(m_url);

    const char *errorString = nullptr;
    switch (networkError) {
        default:
            errorString = "Network error";
            break;
        case QNetworkReply::ConnectionRefusedError:
            errorString = "Connection refused";
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorString = "Remote host closed the connection";
            break;
        case QNetworkReply::HostNotFoundError:
            errorString = "Host not found";
            break;
        case QNetworkReply::TimeoutError:
            errorString = "Timeout";
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::ProxyAuthenticationRequiredError:
        case QNetworkReply::UnknownProxyError:
            errorString = "Proxy error";
            break;
        case QNetworkReply::ContentAccessDenied:
            errorString = "Access denied";
            break;
        case QNetworkReply::ContentNotFoundError:
            errorString = "File not found";
            break;
        case QNetworkReply::AuthenticationRequiredError:
            errorString = "Authentication required";
            break;
    };

    error.setDescription(QLatin1String(errorString));

    setError(error);
}
#endif // qml_network

/*!
Called if \a blob, which was previously waited for, has an error.

The default implementation does nothing.
*/
void QQmlDataBlob::dependencyError(QQmlDataBlob *blob)
{
    Q_UNUSED(blob);
}

/*!
Called if \a blob, which was previously waited for, has completed.

The default implementation does nothing.
*/
void QQmlDataBlob::dependencyComplete(QQmlDataBlob *blob)
{
    Q_UNUSED(blob);
}

/*!
Called when all blobs waited for have completed.  This occurs regardless of
whether they are in error, or complete state.

The default implementation does nothing.
*/
void QQmlDataBlob::allDependenciesDone()
{
    m_data.setStatus(QQmlDataBlob::ResolvingDependencies);
}

/*!
Called when the download progress of this blob changes.  \a progress goes
from 0 to 1.

This callback is only invoked if an asynchronous load for this blob is
made.  An asynchronous load is one in which the Asynchronous mode is
specified explicitly, or one that is implicitly delayed due to a network
operation.

The default implementation does nothing.
*/
void QQmlDataBlob::downloadProgressChanged(qreal progress)
{
    Q_UNUSED(progress);
}

/*!
Invoked on the main thread sometime after done() was called on the load thread.

You cannot modify the blobs state at all in this callback and cannot depend on the
order or timeliness of these callbacks.  Implementors should use this callback to notify
dependencies on the main thread that the blob is done and not a lot else.

This callback is only invoked if an asynchronous load for this blob is
made.  An asynchronous load is one in which the Asynchronous mode is
specified explicitly, or one that is implicitly delayed due to a network
operation.

The default implementation does nothing.
*/
void QQmlDataBlob::completed()
{
}


void QQmlDataBlob::tryDone()
{
    if (status() != Loading && m_waitingFor.isEmpty() && !m_isDone) {
        m_isDone = true;
        addref();

#ifdef DATABLOB_DEBUG
        qWarning("QQmlDataBlob::done() %s", qPrintable(urlString()));
#endif
        done();

        if (status() != Error)
            m_data.setStatus(Complete);

        notifyAllWaitingOnMe();

        // Locking is not required here, as anyone expecting callbacks must
        // already be protected against the blob being completed (as set above);
#ifdef DATABLOB_DEBUG
        qWarning("QQmlDataBlob: Dispatching completed");
#endif
        m_typeLoader->m_thread->callCompleted(this);

        release();
    }
}

void QQmlDataBlob::cancelAllWaitingFor()
{
    while (m_waitingFor.count()) {
        QQmlRefPointer<QQmlDataBlob> blob = m_waitingFor.takeLast();

        Q_ASSERT(blob->m_waitingOnMe.contains(this));

        blob->m_waitingOnMe.removeOne(this);
    }
}

void QQmlDataBlob::notifyAllWaitingOnMe()
{
    while (m_waitingOnMe.count()) {
        QQmlDataBlob *blob = m_waitingOnMe.takeLast();

        Q_ASSERT(std::any_of(blob->m_waitingFor.constBegin(), blob->m_waitingFor.constEnd(),
                             [this](const QQmlRefPointer<QQmlDataBlob> &waiting) { return waiting.data() == this; }));

        blob->notifyComplete(this);
    }
}

void QQmlDataBlob::notifyComplete(QQmlDataBlob *blob)
{
    Q_ASSERT(blob->status() == Error || blob->status() == Complete);
    QQmlCompilingProfiler prof(typeLoader()->profiler(), blob);

    m_inCallback = true;

    QQmlRefPointer<QQmlDataBlob> blobRef;
    for (int i = 0; i < m_waitingFor.count(); ++i) {
        if (m_waitingFor.at(i).data() == blob) {
            blobRef = m_waitingFor.takeAt(i);
            break;
        }
    }
    Q_ASSERT(blobRef);

    if (blob->status() == Error) {
        dependencyError(blob);
    } else if (blob->status() == Complete) {
        dependencyComplete(blob);
    }

    if (!isError() && m_waitingFor.isEmpty())
        allDependenciesDone();

    m_inCallback = false;

    tryDone();
}

#define TD_STATUS_MASK 0x0000FFFF
#define TD_STATUS_SHIFT 0
#define TD_PROGRESS_MASK 0x00FF0000
#define TD_PROGRESS_SHIFT 16
#define TD_ASYNC_MASK 0x80000000

QQmlDataBlob::ThreadData::ThreadData()
: _p(0)
{
}

QQmlDataBlob::Status QQmlDataBlob::ThreadData::status() const
{
    return QQmlDataBlob::Status((_p.load() & TD_STATUS_MASK) >> TD_STATUS_SHIFT);
}

void QQmlDataBlob::ThreadData::setStatus(QQmlDataBlob::Status status)
{
    while (true) {
        int d = _p.load();
        int nd = (d & ~TD_STATUS_MASK) | ((status << TD_STATUS_SHIFT) & TD_STATUS_MASK);
        if (d == nd || _p.testAndSetOrdered(d, nd)) return;
    }
}

bool QQmlDataBlob::ThreadData::isAsync() const
{
    return _p.load() & TD_ASYNC_MASK;
}

void QQmlDataBlob::ThreadData::setIsAsync(bool v)
{
    while (true) {
        int d = _p.load();
        int nd = (d & ~TD_ASYNC_MASK) | (v?TD_ASYNC_MASK:0);
        if (d == nd || _p.testAndSetOrdered(d, nd)) return;
    }
}

quint8 QQmlDataBlob::ThreadData::progress() const
{
    return quint8((_p.load() & TD_PROGRESS_MASK) >> TD_PROGRESS_SHIFT);
}

void QQmlDataBlob::ThreadData::setProgress(quint8 v)
{
    while (true) {
        int d = _p.load();
        int nd = (d & ~TD_PROGRESS_MASK) | ((v << TD_PROGRESS_SHIFT) & TD_PROGRESS_MASK);
        if (d == nd || _p.testAndSetOrdered(d, nd)) return;
    }
}

QQmlTypeLoaderThread::QQmlTypeLoaderThread(QQmlTypeLoader *loader)
: m_loader(loader)
#if QT_CONFIG(qml_network)
, m_networkAccessManager(nullptr), m_networkReplyProxy(nullptr)
#endif // qml_network
{
    // Do that after initializing all the members.
    startup();
}

#if QT_CONFIG(qml_network)
QNetworkAccessManager *QQmlTypeLoaderThread::networkAccessManager() const
{
    Q_ASSERT(isThisThread());
    if (!m_networkAccessManager) {
        m_networkAccessManager = QQmlEnginePrivate::get(m_loader->engine())->createNetworkAccessManager(nullptr);
        m_networkReplyProxy = new QQmlTypeLoaderNetworkReplyProxy(m_loader);
    }

    return m_networkAccessManager;
}

QQmlTypeLoaderNetworkReplyProxy *QQmlTypeLoaderThread::networkReplyProxy() const
{
    Q_ASSERT(isThisThread());
    Q_ASSERT(m_networkReplyProxy); // Must call networkAccessManager() first
    return m_networkReplyProxy;
}
#endif // qml_network

void QQmlTypeLoaderThread::load(QQmlDataBlob *b)
{
    b->addref();
    callMethodInThread(&This::loadThread, b);
}

void QQmlTypeLoaderThread::loadAsync(QQmlDataBlob *b)
{
    b->addref();
    postMethodToThread(&This::loadThread, b);
}

void QQmlTypeLoaderThread::loadWithStaticData(QQmlDataBlob *b, const QByteArray &d)
{
    b->addref();
    callMethodInThread(&This::loadWithStaticDataThread, b, d);
}

void QQmlTypeLoaderThread::loadWithStaticDataAsync(QQmlDataBlob *b, const QByteArray &d)
{
    b->addref();
    postMethodToThread(&This::loadWithStaticDataThread, b, d);
}

void QQmlTypeLoaderThread::loadWithCachedUnit(QQmlDataBlob *b, const QV4::CompiledData::Unit *unit)
{
    b->addref();
    callMethodInThread(&This::loadWithCachedUnitThread, b, unit);
}

void QQmlTypeLoaderThread::loadWithCachedUnitAsync(QQmlDataBlob *b, const QV4::CompiledData::Unit *unit)
{
    b->addref();
    postMethodToThread(&This::loadWithCachedUnitThread, b, unit);
}

void QQmlTypeLoaderThread::callCompleted(QQmlDataBlob *b)
{
    b->addref();
#if !QT_CONFIG(thread)
    if (!isThisThread())
        postMethodToThread(&This::callCompletedMain, b);
#else
    postMethodToMain(&This::callCompletedMain, b);
#endif
}

void QQmlTypeLoaderThread::callDownloadProgressChanged(QQmlDataBlob *b, qreal p)
{
    b->addref();
#if !QT_CONFIG(thread)
    if (!isThisThread())
        postMethodToThread(&This::callDownloadProgressChangedMain, b, p);
#else
    postMethodToMain(&This::callDownloadProgressChangedMain, b, p);
#endif
}

void QQmlTypeLoaderThread::initializeEngine(QQmlExtensionInterface *iface,
                                                    const char *uri)
{
    callMethodInMain(&This::initializeEngineMain, iface, uri);
}

void QQmlTypeLoaderThread::shutdownThread()
{
#if QT_CONFIG(qml_network)
    delete m_networkAccessManager;
    m_networkAccessManager = nullptr;
    delete m_networkReplyProxy;
    m_networkReplyProxy = nullptr;
#endif // qml_network
}

void QQmlTypeLoaderThread::loadThread(QQmlDataBlob *b)
{
    m_loader->loadThread(b);
    b->release();
}

void QQmlTypeLoaderThread::loadWithStaticDataThread(QQmlDataBlob *b, const QByteArray &d)
{
    m_loader->loadWithStaticDataThread(b, d);
    b->release();
}

void QQmlTypeLoaderThread::loadWithCachedUnitThread(QQmlDataBlob *b, const QV4::CompiledData::Unit *unit)
{
    m_loader->loadWithCachedUnitThread(b, unit);
    b->release();
}

void QQmlTypeLoaderThread::callCompletedMain(QQmlDataBlob *b)
{
    QML_MEMORY_SCOPE_URL(b->url());
#ifdef DATABLOB_DEBUG
    qWarning("QQmlTypeLoaderThread: %s completed() callback", qPrintable(b->urlString()));
#endif
    b->completed();
    b->release();
}

void QQmlTypeLoaderThread::callDownloadProgressChangedMain(QQmlDataBlob *b, qreal p)
{
#ifdef DATABLOB_DEBUG
    qWarning("QQmlTypeLoaderThread: %s downloadProgressChanged(%f) callback",
             qPrintable(b->urlString()), p);
#endif
    b->downloadProgressChanged(p);
    b->release();
}

void QQmlTypeLoaderThread::initializeEngineMain(QQmlExtensionInterface *iface,
                                                        const char *uri)
{
    Q_ASSERT(m_loader->engine()->thread() == QThread::currentThread());
    iface->initializeEngine(m_loader->engine(), uri);
}

/*!
\class QQmlTypeLoader
\brief The QQmlTypeLoader class abstracts loading files and their dependencies over the network.
\internal

The QQmlTypeLoader class is provided for the exclusive use of the QQmlTypeLoader class.

Clients create QQmlDataBlob instances and submit them to the QQmlTypeLoader class
through the QQmlTypeLoader::load() or QQmlTypeLoader::loadWithStaticData() methods.
The loader then fetches the data over the network or from the local file system in an efficient way.
QQmlDataBlob is an abstract class, so should always be specialized.

Once data is received, the QQmlDataBlob::dataReceived() method is invoked on the blob.  The
derived class should use this callback to process the received data.  Processing of the data can
result in an error being set (QQmlDataBlob::setError()), or one or more dependencies being
created (QQmlDataBlob::addDependency()).  Dependencies are other QQmlDataBlob's that
are required before processing can fully complete.

To complete processing, the QQmlDataBlob::done() callback is invoked.  done() is called when
one of these three preconditions are met.

\list 1
\li The QQmlDataBlob has no dependencies.
\li The QQmlDataBlob has an error set.
\li All the QQmlDataBlob's dependencies are themselves "done()".
\endlist

Thus QQmlDataBlob::done() will always eventually be called, even if the blob has an error set.
*/

void QQmlTypeLoader::invalidate()
{
    if (m_thread) {
        shutdownThread();
        delete m_thread;
        m_thread = nullptr;
    }

#if QT_CONFIG(qml_network)
    // Need to delete the network replies after
    // the loader thread is shutdown as it could be
    // getting new replies while we clear them
    for (NetworkReplies::Iterator iter = m_networkReplies.begin(); iter != m_networkReplies.end(); ++iter)
        (*iter)->release();
    m_networkReplies.clear();
#endif // qml_network
}

#if QT_CONFIG(qml_debug)
void QQmlTypeLoader::setProfiler(QQmlProfiler *profiler)
{
    Q_ASSERT(!m_profiler);
    m_profiler.reset(profiler);
}
#endif

struct PlainLoader {
    void loadThread(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->loadThread(blob);
    }
    void load(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->m_thread->load(blob);
    }
    void loadAsync(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->m_thread->loadAsync(blob);
    }
};

struct StaticLoader {
    const QByteArray &data;
    StaticLoader(const QByteArray &data) : data(data) {}

    void loadThread(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->loadWithStaticDataThread(blob, data);
    }
    void load(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->m_thread->loadWithStaticData(blob, data);
    }
    void loadAsync(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->m_thread->loadWithStaticDataAsync(blob, data);
    }
};

struct CachedLoader {
    const QV4::CompiledData::Unit *unit;
    CachedLoader(const QV4::CompiledData::Unit *unit) :  unit(unit) {}

    void loadThread(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->loadWithCachedUnitThread(blob, unit);
    }
    void load(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->m_thread->loadWithCachedUnit(blob, unit);
    }
    void loadAsync(QQmlTypeLoader *loader, QQmlDataBlob *blob) const
    {
        loader->m_thread->loadWithCachedUnitAsync(blob, unit);
    }
};

template<typename Loader>
void QQmlTypeLoader::doLoad(const Loader &loader, QQmlDataBlob *blob, Mode mode)
{
#ifdef DATABLOB_DEBUG
    qWarning("QQmlTypeLoader::doLoad(%s): %s thread", qPrintable(blob->urlString()),
             m_thread->isThisThread()?"Compile":"Engine");
#endif
    blob->startLoading();

    if (m_thread->isThisThread()) {
        unlock();
        loader.loadThread(this, blob);
        lock();
    } else if (mode == Asynchronous) {
        blob->m_data.setIsAsync(true);
        unlock();
        loader.loadAsync(this, blob);
        lock();
    } else {
        unlock();
        loader.load(this, blob);
        lock();
        if (mode == PreferSynchronous) {
            if (!blob->isCompleteOrError())
                blob->m_data.setIsAsync(true);
        } else {
            Q_ASSERT(mode == Synchronous);
            while (!blob->isCompleteOrError()) {
                unlock();
                m_thread->waitForNextMessage();
                lock();
            }
        }
    }
}

/*!
Load the provided \a blob from the network or filesystem.

The loader must be locked.
*/
void QQmlTypeLoader::load(QQmlDataBlob *blob, Mode mode)
{
    doLoad(PlainLoader(), blob, mode);
}

/*!
Load the provided \a blob with \a data.  The blob's URL is not used by the data loader in this case.

The loader must be locked.
*/
void QQmlTypeLoader::loadWithStaticData(QQmlDataBlob *blob, const QByteArray &data, Mode mode)
{
    doLoad(StaticLoader(data), blob, mode);
}

void QQmlTypeLoader::loadWithCachedUnit(QQmlDataBlob *blob, const QV4::CompiledData::Unit *unit, Mode mode)
{
    doLoad(CachedLoader(unit), blob, mode);
}

void QQmlTypeLoader::loadWithStaticDataThread(QQmlDataBlob *blob, const QByteArray &data)
{
    ASSERT_LOADTHREAD();

    setData(blob, data);
}

void QQmlTypeLoader::loadWithCachedUnitThread(QQmlDataBlob *blob, const QV4::CompiledData::Unit *unit)
{
    ASSERT_LOADTHREAD();

    setCachedUnit(blob, unit);
}

void QQmlTypeLoader::loadThread(QQmlDataBlob *blob)
{
    ASSERT_LOADTHREAD();

    // Don't continue loading if we've been shutdown
    if (m_thread->isShutdown()) {
        QQmlError error;
        error.setDescription(QLatin1String("Interrupted by shutdown"));
        blob->setError(error);
        return;
    }

    if (blob->m_url.isEmpty()) {
        QQmlError error;
        error.setDescription(QLatin1String("Invalid null URL"));
        blob->setError(error);
        return;
    }

    QML_MEMORY_SCOPE_URL(blob->m_url);

    if (QQmlFile::isSynchronous(blob->m_url)) {
        const QString fileName = QQmlFile::urlToLocalFileOrQrc(blob->m_url);
        if (!QQml_isFileCaseCorrect(fileName)) {
            blob->setError(QLatin1String("File name case mismatch"));
            return;
        }

        blob->m_data.setProgress(0xFF);
        if (blob->m_data.isAsync())
            m_thread->callDownloadProgressChanged(blob, 1.);

        setData(blob, fileName);

    } else {
#if QT_CONFIG(qml_network)
        QNetworkReply *reply = m_thread->networkAccessManager()->get(QNetworkRequest(blob->m_url));
        QQmlTypeLoaderNetworkReplyProxy *nrp = m_thread->networkReplyProxy();
        blob->addref();
        m_networkReplies.insert(reply, blob);

        if (reply->isFinished()) {
            nrp->manualFinished(reply);
        } else {
            QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                             nrp, SLOT(downloadProgress(qint64,qint64)));
            QObject::connect(reply, SIGNAL(finished()),
                             nrp, SLOT(finished()));
        }

#ifdef DATABLOB_DEBUG
        qWarning("QQmlDataBlob: requested %s", qPrintable(blob->urlString()));
#endif // DATABLOB_DEBUG
#endif // qml_network
    }
}

#define DATALOADER_MAXIMUM_REDIRECT_RECURSION 16

#ifndef TYPELOADER_MINIMUM_TRIM_THRESHOLD
#define TYPELOADER_MINIMUM_TRIM_THRESHOLD 64
#endif

#if QT_CONFIG(qml_network)
void QQmlTypeLoader::networkReplyFinished(QNetworkReply *reply)
{
    Q_ASSERT(m_thread->isThisThread());

    reply->deleteLater();

    QQmlDataBlob *blob = m_networkReplies.take(reply);

    Q_ASSERT(blob);

    blob->m_redirectCount++;

    if (blob->m_redirectCount < DATALOADER_MAXIMUM_REDIRECT_RECURSION) {
        QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (redirect.isValid()) {
            QUrl url = reply->url().resolved(redirect.toUrl());
            blob->m_finalUrl = url;
            blob->m_finalUrlString.clear();

            QNetworkReply *reply = m_thread->networkAccessManager()->get(QNetworkRequest(url));
            QObject *nrp = m_thread->networkReplyProxy();
            QObject::connect(reply, SIGNAL(finished()), nrp, SLOT(finished()));
            m_networkReplies.insert(reply, blob);
#ifdef DATABLOB_DEBUG
            qWarning("QQmlDataBlob: redirected to %s", qPrintable(blob->finalUrlString()));
#endif
            return;
        }
    }

    if (reply->error()) {
        blob->networkError(reply->error());
    } else {
        QByteArray data = reply->readAll();
        setData(blob, data);
    }

    blob->release();
}

void QQmlTypeLoader::networkReplyProgress(QNetworkReply *reply,
                                                  qint64 bytesReceived, qint64 bytesTotal)
{
    Q_ASSERT(m_thread->isThisThread());

    QQmlDataBlob *blob = m_networkReplies.value(reply);

    Q_ASSERT(blob);

    if (bytesTotal != 0) {
        quint8 progress = 0xFF * (qreal(bytesReceived) / qreal(bytesTotal));
        blob->m_data.setProgress(progress);
        if (blob->m_data.isAsync())
            m_thread->callDownloadProgressChanged(blob, blob->m_data.progress());
    }
}
#endif // qml_network

/*!
Return the QQmlEngine associated with this loader
*/
QQmlEngine *QQmlTypeLoader::engine() const
{
    return m_engine;
}

/*!
Call the initializeEngine() method on \a iface.  Used by QQmlImportDatabase to ensure it
gets called in the correct thread.
*/
void QQmlTypeLoader::initializeEngine(QQmlExtensionInterface *iface,
                                              const char *uri)
{
    Q_ASSERT(m_thread->isThisThread() || engine()->thread() == QThread::currentThread());

    if (m_thread->isThisThread()) {
        m_thread->initializeEngine(iface, uri);
    } else {
        Q_ASSERT(engine()->thread() == QThread::currentThread());
        iface->initializeEngine(engine(), uri);
    }
}


void QQmlTypeLoader::setData(QQmlDataBlob *blob, const QByteArray &data)
{
    QML_MEMORY_SCOPE_URL(blob->url());
    QQmlDataBlob::SourceCodeData d;
    d.inlineSourceCode = QString::fromUtf8(data);
    d.hasInlineSourceCode = true;
    setData(blob, d);
}

void QQmlTypeLoader::setData(QQmlDataBlob *blob, const QString &fileName)
{
    QML_MEMORY_SCOPE_URL(blob->url());
    QQmlDataBlob::SourceCodeData d;
    d.fileInfo = QFileInfo(fileName);
    setData(blob, d);
}

void QQmlTypeLoader::setData(QQmlDataBlob *blob, const QQmlDataBlob::SourceCodeData &d)
{
    QML_MEMORY_SCOPE_URL(blob->url());
    QQmlCompilingProfiler prof(profiler(), blob);

    blob->m_inCallback = true;

    blob->dataReceived(d);

    if (!blob->isError() && !blob->isWaiting())
        blob->allDependenciesDone();

    if (blob->status() != QQmlDataBlob::Error)
        blob->m_data.setStatus(QQmlDataBlob::WaitingForDependencies);

    blob->m_inCallback = false;

    blob->tryDone();
}

void QQmlTypeLoader::setCachedUnit(QQmlDataBlob *blob, const QV4::CompiledData::Unit *unit)
{
    QML_MEMORY_SCOPE_URL(blob->url());
    QQmlCompilingProfiler prof(profiler(), blob);

    blob->m_inCallback = true;

    blob->initializeFromCachedUnit(unit);

    if (!blob->isError() && !blob->isWaiting())
        blob->allDependenciesDone();

    if (blob->status() != QQmlDataBlob::Error)
        blob->m_data.setStatus(QQmlDataBlob::WaitingForDependencies);

    blob->m_inCallback = false;

    blob->tryDone();
}

void QQmlTypeLoader::shutdownThread()
{
    if (m_thread && !m_thread->isShutdown())
        m_thread->shutdown();
}

QQmlTypeLoader::Blob::Blob(const QUrl &url, QQmlDataBlob::Type type, QQmlTypeLoader *loader)
  : QQmlDataBlob(url, type, loader), m_importCache(loader)
{
}

QQmlTypeLoader::Blob::~Blob()
{
}

bool QQmlTypeLoader::Blob::fetchQmldir(const QUrl &url, const QV4::CompiledData::Import *import, int priority, QList<QQmlError> *errors)
{
    QQmlRefPointer<QQmlQmldirData> data = typeLoader()->getQmldir(url);

    data->setImport(this, import);
    data->setPriority(this, priority);

    if (data->status() == Error) {
        // This qmldir must not exist - which is not an error
        return true;
    } else if (data->status() == Complete) {
        // This data is already available
        return qmldirDataAvailable(data, errors);
    }

    // Wait for this data to become available
    addDependency(data.data());
    return true;
}

bool QQmlTypeLoader::Blob::updateQmldir(const QQmlRefPointer<QQmlQmldirData> &data, const QV4::CompiledData::Import *import, QList<QQmlError> *errors)
{
    QString qmldirIdentifier = data->urlString();
    QString qmldirUrl = qmldirIdentifier.left(qmldirIdentifier.lastIndexOf(QLatin1Char('/')) + 1);

    typeLoader()->setQmldirContent(qmldirIdentifier, data->content());

    if (!m_importCache.updateQmldirContent(typeLoader()->importDatabase(), stringAt(import->uriIndex), stringAt(import->qualifierIndex), qmldirIdentifier, qmldirUrl, errors))
        return false;

    QHash<const QV4::CompiledData::Import *, int>::iterator it = m_unresolvedImports.find(import);
    if (it != m_unresolvedImports.end()) {
        *it = data->priority(this);
    }

    // Release this reference at destruction
    m_qmldirs << data;

    const QString &importQualifier = stringAt(import->qualifierIndex);
    if (!importQualifier.isEmpty()) {
        // Does this library contain any qualified scripts?
        QUrl libraryUrl(qmldirUrl);
        const QQmlTypeLoaderQmldirContent qmldir = typeLoader()->qmldirContent(qmldirIdentifier);
        const auto qmldirScripts = qmldir.scripts();
        for (const QQmlDirParser::Script &script : qmldirScripts) {
            QUrl scriptUrl = libraryUrl.resolved(QUrl(script.fileName));
            QQmlRefPointer<QQmlScriptBlob> blob = typeLoader()->getScript(scriptUrl);
            addDependency(blob.data());

            scriptImported(blob, import->location, script.nameSpace, importQualifier);
        }
    }

    return true;
}

bool QQmlTypeLoader::Blob::addImport(const QV4::CompiledData::Import *import, QList<QQmlError> *errors)
{
    Q_ASSERT(errors);

    QQmlImportDatabase *importDatabase = typeLoader()->importDatabase();

    const QString &importUri = stringAt(import->uriIndex);
    const QString &importQualifier = stringAt(import->qualifierIndex);
    if (import->type == QV4::CompiledData::Import::ImportScript) {
        QUrl scriptUrl = finalUrl().resolved(QUrl(importUri));
        QQmlRefPointer<QQmlScriptBlob> blob = typeLoader()->getScript(scriptUrl);
        addDependency(blob.data());

        scriptImported(blob, import->location, importQualifier, QString());
    } else if (import->type == QV4::CompiledData::Import::ImportLibrary) {
        QString qmldirFilePath;
        QString qmldirUrl;

        if (QQmlMetaType::isLockedModule(importUri, import->majorVersion)) {
            //Locked modules are checked first, to save on filesystem checks
            if (!m_importCache.addLibraryImport(importDatabase, importUri, importQualifier, import->majorVersion,
                                          import->minorVersion, QString(), QString(), false, errors))
                return false;

        } else if (m_importCache.locateQmldir(importDatabase, importUri, import->majorVersion, import->minorVersion,
                                 &qmldirFilePath, &qmldirUrl)) {
            // This is a local library import
            if (!m_importCache.addLibraryImport(importDatabase, importUri, importQualifier, import->majorVersion,
                                          import->minorVersion, qmldirFilePath, qmldirUrl, false, errors))
                return false;

            if (!importQualifier.isEmpty()) {
                // Does this library contain any qualified scripts?
                QUrl libraryUrl(qmldirUrl);
                const QQmlTypeLoaderQmldirContent qmldir = typeLoader()->qmldirContent(qmldirFilePath);
                const auto qmldirScripts = qmldir.scripts();
                for (const QQmlDirParser::Script &script : qmldirScripts) {
                    QUrl scriptUrl = libraryUrl.resolved(QUrl(script.fileName));
                    QQmlRefPointer<QQmlScriptBlob> blob = typeLoader()->getScript(scriptUrl);
                    addDependency(blob.data());

                    scriptImported(blob, import->location, script.nameSpace, importQualifier);
                }
            }
        } else {
            // Is this a module?
            if (QQmlMetaType::isAnyModule(importUri)) {
                if (!m_importCache.addLibraryImport(importDatabase, importUri, importQualifier, import->majorVersion,
                                              import->minorVersion, QString(), QString(), false, errors))
                    return false;
            } else {
                // We haven't yet resolved this import
                m_unresolvedImports.insert(import, 0);

                QQmlAbstractUrlInterceptor *interceptor = typeLoader()->engine()->urlInterceptor();

                // Query any network import paths for this library.
                // Interceptor might redirect local paths.
                QStringList remotePathList = importDatabase->importPathList(
                            interceptor ? QQmlImportDatabase::LocalOrRemote
                                        : QQmlImportDatabase::Remote);
                if (!remotePathList.isEmpty()) {
                    // Add this library and request the possible locations for it
                    if (!m_importCache.addLibraryImport(importDatabase, importUri, importQualifier, import->majorVersion,
                                                  import->minorVersion, QString(), QString(), true, errors))
                        return false;

                    // Probe for all possible locations
                    int priority = 0;
                    const QStringList qmlDirPaths = QQmlImports::completeQmldirPaths(importUri, remotePathList, import->majorVersion, import->minorVersion);
                    for (const QString &qmldirPath : qmlDirPaths) {
                        if (interceptor) {
                            QUrl url = interceptor->intercept(
                                        QQmlImports::urlFromLocalFileOrQrcOrUrl(qmldirPath),
                                        QQmlAbstractUrlInterceptor::QmldirFile);
                            if (!QQmlFile::isLocalFile(url)
                                    && !fetchQmldir(url, import, ++priority, errors)) {
                                return false;
                            }
                        } else if (!fetchQmldir(QUrl(qmldirPath), import, ++priority, errors)) {
                            return false;
                        }

                    }
                }
            }
        }
    } else {
        Q_ASSERT(import->type == QV4::CompiledData::Import::ImportFile);

        bool incomplete = false;

        QUrl importUrl(importUri);
        QString path = importUrl.path();
        path.append(QLatin1String(path.endsWith(QLatin1Char('/')) ? "qmldir" : "/qmldir"));
        importUrl.setPath(path);
        QUrl qmldirUrl = finalUrl().resolved(importUrl);
        if (!QQmlImports::isLocal(qmldirUrl)) {
            // This is a remote file; the import is currently incomplete
            incomplete = true;
        }

        if (!m_importCache.addFileImport(importDatabase, importUri, importQualifier, import->majorVersion,
                                   import->minorVersion, incomplete, errors))
            return false;

        if (incomplete) {
            if (!fetchQmldir(qmldirUrl, import, 1, errors))
                return false;
        }
    }

    return true;
}

void QQmlTypeLoader::Blob::dependencyComplete(QQmlDataBlob *blob)
{
    if (blob->type() == QQmlDataBlob::QmldirFile) {
        QQmlQmldirData *data = static_cast<QQmlQmldirData *>(blob);

        const QV4::CompiledData::Import *import = data->import(this);

        QList<QQmlError> errors;
        if (!qmldirDataAvailable(data, &errors)) {
            Q_ASSERT(errors.size());
            QQmlError error(errors.takeFirst());
            error.setUrl(m_importCache.baseUrl());
            error.setLine(import->location.line);
            error.setColumn(import->location.column);
            errors.prepend(error); // put it back on the list after filling out information.
            setError(errors);
        }
    }
}

bool QQmlTypeLoader::Blob::isDebugging() const
{
    return typeLoader()->engine()->handle()->debugger() != nullptr;
}

bool QQmlTypeLoader::Blob::qmldirDataAvailable(const QQmlRefPointer<QQmlQmldirData> &data, QList<QQmlError> *errors)
{
    bool resolve = true;

    const QV4::CompiledData::Import *import = data->import(this);
    data->setImport(this, nullptr);

    int priority = data->priority(this);
    data->setPriority(this, 0);

    if (import) {
        // Do we need to resolve this import?
        QHash<const QV4::CompiledData::Import *, int>::iterator it = m_unresolvedImports.find(import);
        if (it != m_unresolvedImports.end()) {
            resolve = (*it == 0) || (*it > priority);
        }

        if (resolve) {
            // This is the (current) best resolution for this import
            if (!updateQmldir(data, import, errors)) {
                return false;
            }

            if (it != m_unresolvedImports.end())
                *it = priority;
            return true;
        }
    }

    return true;
}


QQmlTypeLoaderQmldirContent::QQmlTypeLoaderQmldirContent()
{
}

bool QQmlTypeLoaderQmldirContent::hasError() const
{
    return m_parser.hasError();
}

QList<QQmlError> QQmlTypeLoaderQmldirContent::errors(const QString &uri) const
{
    return m_parser.errors(uri);
}

QString QQmlTypeLoaderQmldirContent::typeNamespace() const
{
    return m_parser.typeNamespace();
}

void QQmlTypeLoaderQmldirContent::setContent(const QString &location, const QString &content)
{
    m_hasContent = true;
    m_location = location;
    m_parser.parse(content);
}

void QQmlTypeLoaderQmldirContent::setError(const QQmlError &error)
{
    m_parser.setError(error);
}

QQmlDirComponents QQmlTypeLoaderQmldirContent::components() const
{
    return m_parser.components();
}

QQmlDirScripts QQmlTypeLoaderQmldirContent::scripts() const
{
    return m_parser.scripts();
}

QQmlDirPlugins QQmlTypeLoaderQmldirContent::plugins() const
{
    return m_parser.plugins();
}

QString QQmlTypeLoaderQmldirContent::pluginLocation() const
{
    return m_location;
}

bool QQmlTypeLoaderQmldirContent::designerSupported() const
{
    return m_parser.designerSupported();
}

/*!
Constructs a new type loader that uses the given \a engine.
*/
QQmlTypeLoader::QQmlTypeLoader(QQmlEngine *engine)
    : m_engine(engine)
    , m_thread(new QQmlTypeLoaderThread(this))
    , m_mutex(m_thread->mutex())
    , m_typeCacheTrimThreshold(TYPELOADER_MINIMUM_TRIM_THRESHOLD)
{
}

/*!
Destroys the type loader, first clearing the cache of any information about
loaded files.
*/
QQmlTypeLoader::~QQmlTypeLoader()
{
    // Stop the loader thread before releasing resources
    shutdownThread();

    clearCache();

    invalidate();
}

QQmlImportDatabase *QQmlTypeLoader::importDatabase() const
{
    return &QQmlEnginePrivate::get(engine())->importDatabase;
}

QUrl QQmlTypeLoader::normalize(const QUrl &unNormalizedUrl)
{
    QUrl normalized(unNormalizedUrl);
    if (normalized.scheme() == QLatin1String("qrc"))
        normalized.setHost(QString()); // map qrc:///a.qml to qrc:/a.qml
    return normalized;
}

/*!
Returns a QQmlTypeData for the specified \a url.  The QQmlTypeData may be cached.
*/
QQmlRefPointer<QQmlTypeData> QQmlTypeLoader::getType(const QUrl &unNormalizedUrl, Mode mode)
{
    Q_ASSERT(!unNormalizedUrl.isRelative() &&
            (QQmlFile::urlToLocalFileOrQrc(unNormalizedUrl).isEmpty() ||
             !QDir::isRelativePath(QQmlFile::urlToLocalFileOrQrc(unNormalizedUrl))));

    const QUrl url = normalize(unNormalizedUrl);

    LockHolder<QQmlTypeLoader> holder(this);

    QQmlTypeData *typeData = m_typeCache.value(url);

    if (!typeData) {
        // Trim before adding the new type, so that we don't immediately trim it away
        if (m_typeCache.size() >= m_typeCacheTrimThreshold)
            trimCache();

        typeData = new QQmlTypeData(url, this);
        // TODO: if (compiledData == 0), is it safe to omit this insertion?
        m_typeCache.insert(url, typeData);
        QQmlMetaType::CachedUnitLookupError error = QQmlMetaType::CachedUnitLookupError::NoError;
        if (const QV4::CompiledData::Unit *cachedUnit = QQmlMetaType::findCachedCompilationUnit(typeData->url(), &error)) {
            QQmlTypeLoader::loadWithCachedUnit(typeData, cachedUnit, mode);
        } else {
            typeData->setCachedUnitStatus(error);
            QQmlTypeLoader::load(typeData, mode);
        }
    } else if ((mode == PreferSynchronous || mode == Synchronous) && QQmlFile::isSynchronous(url)) {
        // this was started Asynchronous, but we need to force Synchronous
        // completion now (if at all possible with this type of URL).

        if (!m_thread->isThisThread()) {
            // this only works when called directly from the UI thread, but not
            // when recursively called on the QML thread via resolveTypes()

            while (!typeData->isCompleteOrError()) {
                unlock();
                m_thread->waitForNextMessage();
                lock();
            }
        }
    }

    return typeData;
}

/*!
Returns a QQmlTypeData for the given \a data with the provided base \a url.  The
QQmlTypeData will not be cached.
*/
QQmlRefPointer<QQmlTypeData> QQmlTypeLoader::getType(const QByteArray &data, const QUrl &url, Mode mode)
{
    LockHolder<QQmlTypeLoader> holder(this);

    QQmlTypeData *typeData = new QQmlTypeData(url, this);
    QQmlTypeLoader::loadWithStaticData(typeData, data, mode);

    return QQmlRefPointer<QQmlTypeData>(typeData, QQmlRefPointer<QQmlTypeData>::Adopt);
}

/*!
Return a QQmlScriptBlob for \a url.  The QQmlScriptData may be cached.
*/
QQmlRefPointer<QQmlScriptBlob> QQmlTypeLoader::getScript(const QUrl &unNormalizedUrl)
{
    Q_ASSERT(!unNormalizedUrl.isRelative() &&
            (QQmlFile::urlToLocalFileOrQrc(unNormalizedUrl).isEmpty() ||
             !QDir::isRelativePath(QQmlFile::urlToLocalFileOrQrc(unNormalizedUrl))));

    const QUrl url = normalize(unNormalizedUrl);

    LockHolder<QQmlTypeLoader> holder(this);

    QQmlScriptBlob *scriptBlob = m_scriptCache.value(url);

    if (!scriptBlob) {
        scriptBlob = new QQmlScriptBlob(url, this);
        m_scriptCache.insert(url, scriptBlob);

        QQmlMetaType::CachedUnitLookupError error;
        if (const QV4::CompiledData::Unit *cachedUnit = QQmlMetaType::findCachedCompilationUnit(scriptBlob->url(), &error)) {
            QQmlTypeLoader::loadWithCachedUnit(scriptBlob, cachedUnit);
        } else {
            scriptBlob->setCachedUnitStatus(error);
            QQmlTypeLoader::load(scriptBlob);
        }
    }

    return scriptBlob;
}

/*!
Returns a QQmlQmldirData for \a url.  The QQmlQmldirData may be cached.
*/
QQmlRefPointer<QQmlQmldirData> QQmlTypeLoader::getQmldir(const QUrl &url)
{
    Q_ASSERT(!url.isRelative() &&
            (QQmlFile::urlToLocalFileOrQrc(url).isEmpty() ||
             !QDir::isRelativePath(QQmlFile::urlToLocalFileOrQrc(url))));
    LockHolder<QQmlTypeLoader> holder(this);

    QQmlQmldirData *qmldirData = m_qmldirCache.value(url);

    if (!qmldirData) {
        qmldirData = new QQmlQmldirData(url, this);
        m_qmldirCache.insert(url, qmldirData);
        QQmlTypeLoader::load(qmldirData);
    }

    return qmldirData;
}

// #### Qt 6: Remove this function, it exists only for binary compatibility.
/*!
 * \internal
 */
bool QQmlEngine::addNamedBundle(const QString &name, const QString &fileName)
{
    Q_UNUSED(name)
    Q_UNUSED(fileName)
    return false;
}

/*!
Returns the absolute filename of path via a directory cache.
Returns a empty string if the path does not exist.

Why a directory cache?  QML checks for files in many paths with
invalid directories.  By caching whether a directory exists
we avoid many stats.  We also cache the files' existence in the
directory, for the same reason.
*/
QString QQmlTypeLoader::absoluteFilePath(const QString &path)
{
    if (path.isEmpty())
        return QString();
    if (path.at(0) == QLatin1Char(':')) {
        // qrc resource
        QFileInfo fileInfo(path);
        return fileInfo.isFile() ? fileInfo.absoluteFilePath() : QString();
    } else if (path.count() > 3 && path.at(3) == QLatin1Char(':') &&
               path.startsWith(QLatin1String("qrc"), Qt::CaseInsensitive)) {
        // qrc resource url
        QFileInfo fileInfo(QQmlFile::urlToLocalFileOrQrc(path));
        return fileInfo.isFile() ? fileInfo.absoluteFilePath() : QString();
    }
#if defined(Q_OS_ANDROID)
    else if (path.count() > 7 && path.at(6) == QLatin1Char(':') && path.at(7) == QLatin1Char('/') &&
           path.startsWith(QLatin1String("assets"), Qt::CaseInsensitive)) {
        // assets resource url
        QFileInfo fileInfo(QQmlFile::urlToLocalFileOrQrc(path));
        return fileInfo.isFile() ? fileInfo.absoluteFilePath() : QString();
    }
#endif

    int lastSlash = path.lastIndexOf(QLatin1Char('/'));
    QString dirPath(path.left(lastSlash));

    LockHolder<QQmlTypeLoader> holder(this);
    if (!m_importDirCache.contains(dirPath)) {
        bool exists = QDir(dirPath).exists();
        QCache<QString, bool> *entry = exists ? new QCache<QString, bool> : nullptr;
        m_importDirCache.insert(dirPath, entry);
    }
    QCache<QString, bool> *fileSet = m_importDirCache.object(dirPath);
    if (!fileSet)
        return QString();

    QString absoluteFilePath;
    QString fileName(path.mid(lastSlash+1, path.length()-lastSlash-1));

    bool *value = fileSet->object(fileName);
    if (value) {
        if (*value)
            absoluteFilePath = path;
    } else {
        bool exists = QFile::exists(path);
        fileSet->insert(fileName, new bool(exists));
        if (exists)
            absoluteFilePath = path;
    }

    if (absoluteFilePath.length() > 2 && absoluteFilePath.at(0) != QLatin1Char('/') && absoluteFilePath.at(1) != QLatin1Char(':'))
        absoluteFilePath = QFileInfo(absoluteFilePath).absoluteFilePath();

    return absoluteFilePath;
}

bool QQmlTypeLoader::fileExists(const QString &path, const QString &file)
{
    if (path.isEmpty())
        return false;
    Q_ASSERT(path.endsWith(QLatin1Char('/')));
    if (path.at(0) == QLatin1Char(':')) {
        // qrc resource
        QFileInfo fileInfo(path + file);
        return fileInfo.isFile();
    } else if (path.count() > 3 && path.at(3) == QLatin1Char(':') &&
               path.startsWith(QLatin1String("qrc"), Qt::CaseInsensitive)) {
        // qrc resource url
        QFileInfo fileInfo(QQmlFile::urlToLocalFileOrQrc(path + file));
        return fileInfo.isFile();
    }
#if defined(Q_OS_ANDROID)
    else if (path.count() > 7 && path.at(6) == QLatin1Char(':') && path.at(7) == QLatin1Char('/') &&
           path.startsWith(QLatin1String("assets"), Qt::CaseInsensitive)) {
        // assets resource url
        QFileInfo fileInfo(QQmlFile::urlToLocalFileOrQrc(path + file));
        return fileInfo.isFile();
    }
#endif

    LockHolder<QQmlTypeLoader> holder(this);
    if (!m_importDirCache.contains(path)) {
        bool exists = QDir(path).exists();
        QCache<QString, bool> *entry = exists ? new QCache<QString, bool> : nullptr;
        m_importDirCache.insert(path, entry);
    }
    QCache<QString, bool> *fileSet = m_importDirCache.object(path);
    if (!fileSet)
        return false;

    QString absoluteFilePath;

    bool *value = fileSet->object(file);
    if (value) {
        return *value;
    } else {
        bool exists = QFile::exists(path + file);
        fileSet->insert(file, new bool(exists));
        return exists;
    }
}


/*!
Returns true if the path is a directory via a directory cache.  Cache is
shared with absoluteFilePath().
*/
bool QQmlTypeLoader::directoryExists(const QString &path)
{
    if (path.isEmpty())
        return false;

    bool isResource = path.at(0) == QLatin1Char(':');
#if defined(Q_OS_ANDROID)
    isResource = isResource || path.startsWith(QLatin1String("assets:/"));
#endif

    if (isResource) {
        // qrc resource
        QFileInfo fileInfo(path);
        return fileInfo.exists() && fileInfo.isDir();
    }

    int length = path.length();
    if (path.endsWith(QLatin1Char('/')))
        --length;
    QString dirPath(path.left(length));

    LockHolder<QQmlTypeLoader> holder(this);
    if (!m_importDirCache.contains(dirPath)) {
        bool exists = QDir(dirPath).exists();
        QCache<QString, bool> *files = exists ? new QCache<QString, bool> : nullptr;
        m_importDirCache.insert(dirPath, files);
    }

    QCache<QString, bool> *fileSet = m_importDirCache.object(dirPath);
    return fileSet != nullptr;
}


/*!
Return a QQmlTypeLoaderQmldirContent for absoluteFilePath.  The QQmlTypeLoaderQmldirContent may be cached.

\a filePath is a local file path.

It can also be a remote path for a remote directory import, but it will have been cached by now in this case.
*/
const QQmlTypeLoaderQmldirContent QQmlTypeLoader::qmldirContent(const QString &filePathIn)
{
    LockHolder<QQmlTypeLoader> holder(this);

    QString filePath;

    // Try to guess if filePathIn is already a URL. This is necessarily fragile, because
    // - paths can contain ':', which might make them appear as URLs with schemes.
    // - windows drive letters appear as schemes (thus "< 2" below).
    // - a "file:" URL is equivalent to the respective file, but will be treated differently.
    // Yet, this heuristic is the best we can do until we pass more structured information here,
    // for example a QUrl also for local files.
    QUrl url(filePathIn);
    if (url.scheme().length() < 2) {
        filePath = filePathIn;
    } else {
        filePath = QQmlFile::urlToLocalFileOrQrc(url);
        if (filePath.isEmpty()) { // Can't load the remote here, but should be cached
            if (auto entry = m_importQmlDirCache.value(filePathIn))
                return **entry;
            else
                return QQmlTypeLoaderQmldirContent();
        }
    }

    QQmlTypeLoaderQmldirContent **val = m_importQmlDirCache.value(filePath);
    if (val)
        return **val;
    QQmlTypeLoaderQmldirContent *qmldir = new QQmlTypeLoaderQmldirContent;

#define ERROR(description) { QQmlError e; e.setDescription(description); qmldir->setError(e); }
#define NOT_READABLE_ERROR QString(QLatin1String("module \"$$URI$$\" definition \"%1\" not readable"))
#define CASE_MISMATCH_ERROR QString(QLatin1String("cannot load module \"$$URI$$\": File name case mismatch for \"%1\""))

    QFile file(filePath);
    if (!QQml_isFileCaseCorrect(filePath)) {
        ERROR(CASE_MISMATCH_ERROR.arg(filePath));
    } else if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        qmldir->setContent(filePath, QString::fromUtf8(data));
    } else {
        ERROR(NOT_READABLE_ERROR.arg(filePath));
    }

#undef ERROR
#undef NOT_READABLE_ERROR
#undef CASE_MISMATCH_ERROR

    m_importQmlDirCache.insert(filePath, qmldir);
    return *qmldir;
}

void QQmlTypeLoader::setQmldirContent(const QString &url, const QString &content)
{
    QQmlTypeLoaderQmldirContent *qmldir;
    QQmlTypeLoaderQmldirContent **val = m_importQmlDirCache.value(url);
    if (val) {
        qmldir = *val;
    } else {
        qmldir = new QQmlTypeLoaderQmldirContent;
        m_importQmlDirCache.insert(url, qmldir);
    }

    qmldir->setContent(url, content);
}

/*!
Clears cached information about loaded files, including any type data, scripts
and qmldir information.
*/
void QQmlTypeLoader::clearCache()
{
    for (TypeCache::Iterator iter = m_typeCache.begin(), end = m_typeCache.end(); iter != end; ++iter)
        (*iter)->release();
    for (ScriptCache::Iterator iter = m_scriptCache.begin(), end = m_scriptCache.end(); iter != end; ++iter)
        (*iter)->release();
    for (QmldirCache::Iterator iter = m_qmldirCache.begin(), end = m_qmldirCache.end(); iter != end; ++iter)
        (*iter)->release();

    qDeleteAll(m_importQmlDirCache);

    m_typeCache.clear();
    m_typeCacheTrimThreshold = TYPELOADER_MINIMUM_TRIM_THRESHOLD;
    m_scriptCache.clear();
    m_qmldirCache.clear();
    m_importDirCache.clear();
    m_importQmlDirCache.clear();
    QQmlMetaType::freeUnusedTypesAndCaches();
}

void QQmlTypeLoader::updateTypeCacheTrimThreshold()
{
    int size = m_typeCache.size();
    if (size > m_typeCacheTrimThreshold)
        m_typeCacheTrimThreshold = size * 2;
    if (size < m_typeCacheTrimThreshold / 2)
        m_typeCacheTrimThreshold = qMax(size * 2, TYPELOADER_MINIMUM_TRIM_THRESHOLD);
}

void QQmlTypeLoader::trimCache()
{
    while (true) {
        QList<TypeCache::Iterator> unneededTypes;
        for (TypeCache::Iterator iter = m_typeCache.begin(), end = m_typeCache.end(); iter != end; ++iter)  {
            QQmlTypeData *typeData = iter.value();

            // typeData->m_compiledData may be set early on in the proccess of loading a file, so
            // it's important to check the general loading status of the typeData before making any
            // other decisions.
            if (typeData->count() == 1 && (typeData->isError() || typeData->isComplete())
                    && (!typeData->m_compiledData || typeData->m_compiledData->count() == 1)) {
                // There are no live objects of this type
                unneededTypes.append(iter);
            }
        }

        if (unneededTypes.isEmpty())
            break;

        while (!unneededTypes.isEmpty()) {
            TypeCache::Iterator iter = unneededTypes.takeLast();

            iter.value()->release();
            m_typeCache.erase(iter);
        }
    }

    updateTypeCacheTrimThreshold();

    QQmlMetaType::freeUnusedTypesAndCaches();

    // TODO: release any scripts which are no longer referenced by any types
}

bool QQmlTypeLoader::isTypeLoaded(const QUrl &url) const
{
    LockHolder<QQmlTypeLoader> holder(const_cast<QQmlTypeLoader *>(this));
    return m_typeCache.contains(url);
}

bool QQmlTypeLoader::isScriptLoaded(const QUrl &url) const
{
    LockHolder<QQmlTypeLoader> holder(const_cast<QQmlTypeLoader *>(this));
    return m_scriptCache.contains(url);
}

QQmlTypeData::TypeDataCallback::~TypeDataCallback()
{
}

QString QQmlTypeData::TypeReference::qualifiedName() const
{
    QString result;
    if (!prefix.isEmpty()) {
        result = prefix + QLatin1Char('.');
    }
    result.append(type.qmlTypeName());
    return result;
}

QQmlTypeData::QQmlTypeData(const QUrl &url, QQmlTypeLoader *manager)
: QQmlTypeLoader::Blob(url, QmlFile, manager),
   m_typesResolved(false), m_implicitImportLoaded(false)
{

}

QQmlTypeData::~QQmlTypeData()
{
    m_scripts.clear();
    m_compositeSingletons.clear();
    m_resolvedTypes.clear();
}

const QList<QQmlTypeData::ScriptReference> &QQmlTypeData::resolvedScripts() const
{
    return m_scripts;
}

QV4::CompiledData::CompilationUnit *QQmlTypeData::compilationUnit() const
{
    return m_compiledData.data();
}

void QQmlTypeData::registerCallback(TypeDataCallback *callback)
{
    Q_ASSERT(!m_callbacks.contains(callback));
    m_callbacks.append(callback);
}

void QQmlTypeData::unregisterCallback(TypeDataCallback *callback)
{
    Q_ASSERT(m_callbacks.contains(callback));
    m_callbacks.removeOne(callback);
    Q_ASSERT(!m_callbacks.contains(callback));
}

bool QQmlTypeData::tryLoadFromDiskCache()
{
    if (disableDiskCache() && !forceDiskCache())
        return false;

    if (isDebugging())
        return false;

    QV4::ExecutionEngine *v4 = typeLoader()->engine()->handle();
    if (!v4)
        return false;

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> unit = QV4::Compiler::Codegen::createUnitForLoading();
    {
        QString error;
        if (!unit->loadFromDisk(url(), m_backupSourceCode.sourceTimeStamp(), &error)) {
            qCDebug(DBG_DISK_CACHE) << "Error loading" << urlString() << "from disk cache:" << error;
            return false;
        }
    }

    if (unit->unitData()->flags & QV4::CompiledData::Unit::PendingTypeCompilation) {
        restoreIR(unit);
        return true;
    }

    m_compiledData = unit;

    for (int i = 0, count = m_compiledData->objectCount(); i < count; ++i)
        m_typeReferences.collectFromObject(m_compiledData->objectAt(i));

    m_importCache.setBaseUrl(finalUrl(), finalUrlString());

    // For remote URLs, we don't delay the loading of the implicit import
    // because the loading probably requires an asynchronous fetch of the
    // qmldir (so we can't load it just in time).
    if (!finalUrl().scheme().isEmpty()) {
        QUrl qmldirUrl = finalUrl().resolved(QUrl(QLatin1String("qmldir")));
        if (!QQmlImports::isLocal(qmldirUrl)) {
            if (!loadImplicitImport())
                return false;

            // find the implicit import
            for (quint32 i = 0, count = m_compiledData->importCount(); i < count; ++i) {
                const QV4::CompiledData::Import *import = m_compiledData->importAt(i);
                if (m_compiledData->stringAt(import->uriIndex) == QLatin1String(".")
                    && import->qualifierIndex == 0
                    && import->majorVersion == -1
                    && import->minorVersion == -1) {
                    QList<QQmlError> errors;
                    if (!fetchQmldir(qmldirUrl, import, 1, &errors)) {
                        setError(errors);
                        return false;
                    }
                    break;
                }
            }
        }
    }

    for (int i = 0, count = m_compiledData->importCount(); i < count; ++i) {
        const QV4::CompiledData::Import *import = m_compiledData->importAt(i);
        QList<QQmlError> errors;
        if (!addImport(import, &errors)) {
            Q_ASSERT(errors.size());
            QQmlError error(errors.takeFirst());
            error.setUrl(m_importCache.baseUrl());
            error.setLine(import->location.line);
            error.setColumn(import->location.column);
            errors.prepend(error); // put it back on the list after filling out information.
            setError(errors);
            return false;
        }
    }

    return true;
}

void QQmlTypeData::createTypeAndPropertyCaches(const QQmlRefPointer<QQmlTypeNameCache> &typeNameCache,
                                                const QV4::CompiledData::ResolvedTypeReferenceMap &resolvedTypeCache)
{
    Q_ASSERT(m_compiledData);
    m_compiledData->typeNameCache = typeNameCache;
    m_compiledData->resolvedTypes = resolvedTypeCache;

    QQmlEnginePrivate * const engine = QQmlEnginePrivate::get(typeLoader()->engine());

    QQmlPendingGroupPropertyBindings pendingGroupPropertyBindings;

    {
        QQmlPropertyCacheCreator<QV4::CompiledData::CompilationUnit> propertyCacheCreator(&m_compiledData->propertyCaches,
                                                                                          &pendingGroupPropertyBindings,
                                                                                          engine, m_compiledData.data(), &m_importCache);
        QQmlCompileError error = propertyCacheCreator.buildMetaObjects();
        if (error.isSet()) {
            setError(error);
            return;
        }
    }

    QQmlPropertyCacheAliasCreator<QV4::CompiledData::CompilationUnit> aliasCreator(&m_compiledData->propertyCaches, m_compiledData.data());
    aliasCreator.appendAliasPropertiesToMetaObjects();

    pendingGroupPropertyBindings.resolveMissingPropertyCaches(engine, &m_compiledData->propertyCaches);
}

static bool addTypeReferenceChecksumsToHash(const QList<QQmlTypeData::TypeReference> &typeRefs, QCryptographicHash *hash, QQmlEngine *engine)
{
    for (const auto &typeRef: typeRefs) {
        if (typeRef.typeData) {
            const auto unit = typeRef.typeData->compilationUnit()->unitData();
            hash->addData(unit->md5Checksum, sizeof(unit->md5Checksum));
        } else if (typeRef.type.isValid()) {
            const auto propertyCache = QQmlEnginePrivate::get(engine)->cache(typeRef.type.metaObject());
            bool ok = false;
            hash->addData(propertyCache->checksum(&ok));
            if (!ok)
                return false;
        }
    }
    return true;
}

void QQmlTypeData::done()
{
    auto cleanup = qScopeGuard([this]{
        m_document.reset();
        m_typeReferences.clear();
        if (isError())
            m_compiledData = nullptr;
    });

    if (isError())
        return;

    // Check all script dependencies for errors
    for (int ii = 0; ii < m_scripts.count(); ++ii) {
        const ScriptReference &script = m_scripts.at(ii);
        Q_ASSERT(script.script->isCompleteOrError());
        if (script.script->isError()) {
            QList<QQmlError> errors = script.script->errors();
            QQmlError error;
            error.setUrl(url());
            error.setLine(script.location.line);
            error.setColumn(script.location.column);
            error.setDescription(QQmlTypeLoader::tr("Script %1 unavailable").arg(script.script->urlString()));
            errors.prepend(error);
            setError(errors);
            return;
        }
    }

    // Check all type dependencies for errors
    for (auto it = m_resolvedTypes.constBegin(), end = m_resolvedTypes.constEnd(); it != end;
         ++it) {
        const TypeReference &type = *it;
        Q_ASSERT(!type.typeData || type.typeData->isCompleteOrError());
        if (type.typeData && type.typeData->isError()) {
            const QString typeName = stringAt(it.key());

            QList<QQmlError> errors = type.typeData->errors();
            QQmlError error;
            error.setUrl(url());
            error.setLine(type.location.line);
            error.setColumn(type.location.column);
            error.setDescription(QQmlTypeLoader::tr("Type %1 unavailable").arg(typeName));
            errors.prepend(error);
            setError(errors);
            return;
        }
    }

    // Check all composite singleton type dependencies for errors
    for (int ii = 0; ii < m_compositeSingletons.count(); ++ii) {
        const TypeReference &type = m_compositeSingletons.at(ii);
        Q_ASSERT(!type.typeData || type.typeData->isCompleteOrError());
        if (type.typeData && type.typeData->isError()) {
            QString typeName = type.type.qmlTypeName();

            QList<QQmlError> errors = type.typeData->errors();
            QQmlError error;
            error.setUrl(url());
            error.setLine(type.location.line);
            error.setColumn(type.location.column);
            error.setDescription(QQmlTypeLoader::tr("Type %1 unavailable").arg(typeName));
            errors.prepend(error);
            setError(errors);
            return;
        }
    }

    QQmlRefPointer<QQmlTypeNameCache> typeNameCache;
    QV4::CompiledData::ResolvedTypeReferenceMap resolvedTypeCache;
    {
        QQmlCompileError error = buildTypeResolutionCaches(&typeNameCache, &resolvedTypeCache);
        if (error.isSet()) {
            setError(error);
            return;
        }
    }

    QQmlEngine *const engine = typeLoader()->engine();

    const auto dependencyHasher = [engine, &resolvedTypeCache, this](QCryptographicHash *hash) {
        if (!resolvedTypeCache.addToHash(hash, engine))
            return false;
        return ::addTypeReferenceChecksumsToHash(m_compositeSingletons, hash, engine);
    };

    // verify if any dependencies changed if we're using a cache
    if (m_document.isNull() && !m_compiledData->verifyChecksum(dependencyHasher)) {
        qCDebug(DBG_DISK_CACHE) << "Checksum mismatch for cached version of" << m_compiledData->fileName();
        if (!loadFromSource())
            return;
        m_backupSourceCode = SourceCodeData();
        m_compiledData = nullptr;
    }

    if (!m_document.isNull()) {
        // Compile component
        compile(typeNameCache, &resolvedTypeCache, dependencyHasher);
    } else {
        createTypeAndPropertyCaches(typeNameCache, resolvedTypeCache);
    }

    if (isError())
        return;

    {
        QQmlEnginePrivate *const enginePrivate = QQmlEnginePrivate::get(engine);
        {
        // Sanity check property bindings
            QQmlPropertyValidator validator(enginePrivate, m_importCache, m_compiledData);
            QVector<QQmlCompileError> errors = validator.validate();
            if (!errors.isEmpty()) {
                setError(errors);
                return;
            }
        }

        m_compiledData->finalizeCompositeType(enginePrivate);
    }

    {
        QQmlType type = QQmlMetaType::qmlType(finalUrl(), true);
        if (m_compiledData && m_compiledData->unitData()->flags & QV4::CompiledData::Unit::IsSingleton) {
            if (!type.isValid()) {
                QQmlError error;
                error.setDescription(QQmlTypeLoader::tr("No matching type found, pragma Singleton files cannot be used by QQmlComponent."));
                setError(error);
                return;
            } else if (!type.isCompositeSingleton()) {
                QQmlError error;
                error.setDescription(QQmlTypeLoader::tr("pragma Singleton used with a non composite singleton type %1").arg(type.qmlTypeName()));
                setError(error);
                return;
            }
        } else {
            // If the type is CompositeSingleton but there was no pragma Singleton in the
            // QML file, lets report an error.
            if (type.isValid() && type.isCompositeSingleton()) {
                QString typeName = type.qmlTypeName();
                setError(QQmlTypeLoader::tr("qmldir defines type as singleton, but no pragma Singleton found in type %1.").arg(typeName));
                return;
            }
        }
    }

    {
        // Collect imported scripts
        m_compiledData->dependentScripts.reserve(m_scripts.count());
        for (int scriptIndex = 0; scriptIndex < m_scripts.count(); ++scriptIndex) {
            const QQmlTypeData::ScriptReference &script = m_scripts.at(scriptIndex);

            QStringRef qualifier(&script.qualifier);
            QString enclosingNamespace;

            const int lastDotIndex = qualifier.lastIndexOf(QLatin1Char('.'));
            if (lastDotIndex != -1) {
                enclosingNamespace = qualifier.left(lastDotIndex).toString();
                qualifier = qualifier.mid(lastDotIndex+1);
            }

            m_compiledData->typeNameCache->add(qualifier.toString(), scriptIndex, enclosingNamespace);
            QQmlRefPointer<QQmlScriptData> scriptData = script.script->scriptData();
            m_compiledData->dependentScripts << scriptData;
        }
    }
}

void QQmlTypeData::completed()
{
    // Notify callbacks
    while (!m_callbacks.isEmpty()) {
        TypeDataCallback *callback = m_callbacks.takeFirst();
        callback->typeDataReady(this);
    }
}

bool QQmlTypeData::loadImplicitImport()
{
    m_implicitImportLoaded = true; // Even if we hit an error, count as loaded (we'd just keep hitting the error)

    m_importCache.setBaseUrl(finalUrl(), finalUrlString());

    QQmlImportDatabase *importDatabase = typeLoader()->importDatabase();
    // For local urls, add an implicit import "." as most overridden lookup.
    // This will also trigger the loading of the qmldir and the import of any native
    // types from available plugins.
    QList<QQmlError> implicitImportErrors;
    m_importCache.addImplicitImport(importDatabase, &implicitImportErrors);

    if (!implicitImportErrors.isEmpty()) {
        setError(implicitImportErrors);
        return false;
    }

    return true;
}

void QQmlTypeData::dataReceived(const SourceCodeData &data)
{
    m_backupSourceCode = data;

    if (tryLoadFromDiskCache())
        return;

    if (isError())
        return;

    if (!m_backupSourceCode.exists() || m_backupSourceCode.isEmpty()) {
        if (m_cachedUnitStatus == QQmlMetaType::CachedUnitLookupError::VersionMismatch)
            setError(QQmlTypeLoader::tr("File was compiled ahead of time with an incompatible version of Qt and the original file cannot be found. Please recompile"));
        else if (!m_backupSourceCode.exists())
            setError(QQmlTypeLoader::tr("No such file or directory"));
        else
            setError(QQmlTypeLoader::tr("File is empty"));
        return;
    }

    if (!loadFromSource())
        return;

    continueLoadFromIR();
}

void QQmlTypeData::initializeFromCachedUnit(const QV4::CompiledData::Unit *unit)
{
    m_document.reset(new QmlIR::Document(isDebugging()));
    QmlIR::IRLoader loader(unit, m_document.data());
    loader.load();
    m_document->jsModule.fileName = urlString();
    m_document->jsModule.finalUrl = finalUrlString();
    m_document->javaScriptCompilationUnit.adopt(new QV4::CompiledData::CompilationUnit(unit));
    continueLoadFromIR();
}

bool QQmlTypeData::loadFromSource()
{
    m_document.reset(new QmlIR::Document(isDebugging()));
    m_document->jsModule.sourceTimeStamp = m_backupSourceCode.sourceTimeStamp();
    QQmlEngine *qmlEngine = typeLoader()->engine();
    QmlIR::IRBuilder compiler(qmlEngine->handle()->v8Engine->illegalNames());

    QString sourceError;
    const QString source = m_backupSourceCode.readAll(&sourceError);
    if (!sourceError.isEmpty()) {
        setError(sourceError);
        return false;
    }

    if (!compiler.generateFromQml(source, finalUrlString(), m_document.data())) {
        QList<QQmlError> errors;
        errors.reserve(compiler.errors.count());
        for (const QQmlJS::DiagnosticMessage &msg : qAsConst(compiler.errors)) {
            QQmlError e;
            e.setUrl(url());
            e.setLine(msg.loc.startLine);
            e.setColumn(msg.loc.startColumn);
            e.setDescription(msg.message);
            errors << e;
        }
        setError(errors);
        return false;
    }
    return true;
}

void QQmlTypeData::restoreIR(QQmlRefPointer<QV4::CompiledData::CompilationUnit> unit)
{
    m_document.reset(new QmlIR::Document(isDebugging()));
    QmlIR::IRLoader loader(unit->unitData(), m_document.data());
    loader.load();
    m_document->jsModule.fileName = urlString();
    m_document->jsModule.finalUrl = finalUrlString();
    m_document->javaScriptCompilationUnit = unit;
    continueLoadFromIR();
}

void QQmlTypeData::continueLoadFromIR()
{
    m_typeReferences.collectFromObjects(m_document->objects.constBegin(), m_document->objects.constEnd());
    m_importCache.setBaseUrl(finalUrl(), finalUrlString());

    // For remote URLs, we don't delay the loading of the implicit import
    // because the loading probably requires an asynchronous fetch of the
    // qmldir (so we can't load it just in time).
    if (!finalUrl().scheme().isEmpty()) {
        QUrl qmldirUrl = finalUrl().resolved(QUrl(QLatin1String("qmldir")));
        if (!QQmlImports::isLocal(qmldirUrl)) {
            if (!loadImplicitImport())
                return;
            // This qmldir is for the implicit import
            QQmlJS::MemoryPool *pool = m_document->jsParserEngine.pool();
            auto implicitImport = pool->New<QV4::CompiledData::Import>();
            implicitImport->uriIndex = m_document->registerString(QLatin1String("."));
            implicitImport->qualifierIndex = 0; // empty string
            implicitImport->majorVersion = -1;
            implicitImport->minorVersion = -1;
            QList<QQmlError> errors;

            if (!fetchQmldir(qmldirUrl, implicitImport, 1, &errors)) {
                setError(errors);
                return;
            }
        }
    }

    QList<QQmlError> errors;

    for (const QV4::CompiledData::Import *import : qAsConst(m_document->imports)) {
        if (!addImport(import, &errors)) {
            Q_ASSERT(errors.size());
            QQmlError error(errors.takeFirst());
            error.setUrl(m_importCache.baseUrl());
            error.setLine(import->location.line);
            error.setColumn(import->location.column);
            errors.prepend(error); // put it back on the list after filling out information.
            setError(errors);
            return;
        }
    }
}

void QQmlTypeData::allDependenciesDone()
{
    QQmlTypeLoader::Blob::allDependenciesDone();

    if (!m_typesResolved) {
        // Check that all imports were resolved
        QList<QQmlError> errors;
        QHash<const QV4::CompiledData::Import *, int>::const_iterator it = m_unresolvedImports.constBegin(), end = m_unresolvedImports.constEnd();
        for ( ; it != end; ++it) {
            if (*it == 0) {
                // This import was not resolved
                for (auto keyIt = m_unresolvedImports.keyBegin(),
                     keyEnd = m_unresolvedImports.keyEnd();
                     keyIt != keyEnd; ++keyIt) {
                    const QV4::CompiledData::Import *import = *keyIt;
                    QQmlError error;
                    error.setDescription(QQmlTypeLoader::tr("module \"%1\" is not installed").arg(stringAt(import->uriIndex)));
                    error.setUrl(m_importCache.baseUrl());
                    error.setLine(import->location.line);
                    error.setColumn(import->location.column);
                    errors.prepend(error);
                }
            }
        }
        if (errors.size()) {
            setError(errors);
            return;
        }

        resolveTypes();
        m_typesResolved = true;
    }
}

void QQmlTypeData::downloadProgressChanged(qreal p)
{
    for (int ii = 0; ii < m_callbacks.count(); ++ii) {
        TypeDataCallback *callback = m_callbacks.at(ii);
        callback->typeDataProgress(this, p);
    }
}

QString QQmlTypeData::stringAt(int index) const
{
    if (m_compiledData)
        return m_compiledData->stringAt(index);
    return m_document->jsGenerator.stringTable.stringForIndex(index);
}

void QQmlTypeData::compile(const QQmlRefPointer<QQmlTypeNameCache> &typeNameCache,
                           QV4::CompiledData::ResolvedTypeReferenceMap *resolvedTypeCache,
                           const QV4::CompiledData::DependentTypesHasher &dependencyHasher)
{
    Q_ASSERT(m_compiledData.isNull());

    const bool typeRecompilation = m_document && m_document->javaScriptCompilationUnit && m_document->javaScriptCompilationUnit->unitData()->flags & QV4::CompiledData::Unit::PendingTypeCompilation;

    QQmlEnginePrivate * const enginePrivate = QQmlEnginePrivate::get(typeLoader()->engine());
    QQmlTypeCompiler compiler(enginePrivate, this, m_document.data(), typeNameCache, resolvedTypeCache, dependencyHasher);
    m_compiledData = compiler.compile();
    if (!m_compiledData) {
        setError(compiler.compilationErrors());
        return;
    }

    const bool trySaveToDisk = (!disableDiskCache() || forceDiskCache()) && !m_document->jsModule.debugMode && !typeRecompilation;
    if (trySaveToDisk) {
        QString errorString;
        if (m_compiledData->saveToDisk(url(), &errorString)) {
            QString error;
            if (!m_compiledData->loadFromDisk(url(), m_backupSourceCode.sourceTimeStamp(), &error)) {
                // ignore error, keep using the in-memory compilation unit.
            }
        } else {
            qCDebug(DBG_DISK_CACHE) << "Error saving cached version of" << m_compiledData->fileName() << "to disk:" << errorString;
        }
    }
}

void QQmlTypeData::resolveTypes()
{
    // Add any imported scripts to our resolved set
    const auto resolvedScripts = m_importCache.resolvedScripts();
    for (const QQmlImports::ScriptReference &script : resolvedScripts) {
        QQmlRefPointer<QQmlScriptBlob> blob = typeLoader()->getScript(script.location);
        addDependency(blob.data());

        ScriptReference ref;
        //ref.location = ...
        if (!script.qualifier.isEmpty())
        {
            ref.qualifier = script.qualifier + QLatin1Char('.') + script.nameSpace;
            // Add a reference to the enclosing namespace
            m_namespaces.insert(script.qualifier);
        } else {
            ref.qualifier = script.nameSpace;
        }

        ref.script = blob;
        m_scripts << ref;
    }

    // Lets handle resolved composite singleton types
    const auto resolvedCompositeSingletons = m_importCache.resolvedCompositeSingletons();
    for (const QQmlImports::CompositeSingletonReference &csRef : resolvedCompositeSingletons) {
        TypeReference ref;
        QString typeName;
        if (!csRef.prefix.isEmpty()) {
            typeName = csRef.prefix + QLatin1Char('.') + csRef.typeName;
            // Add a reference to the enclosing namespace
            m_namespaces.insert(csRef.prefix);
        } else {
            typeName = csRef.typeName;
        }

        int majorVersion = csRef.majorVersion > -1 ? csRef.majorVersion : -1;
        int minorVersion = csRef.minorVersion > -1 ? csRef.minorVersion : -1;

        if (!resolveType(typeName, majorVersion, minorVersion, ref, -1, -1, true,
                         QQmlType::CompositeSingletonType))
            return;

        if (ref.type.isCompositeSingleton()) {
            ref.typeData = typeLoader()->getType(ref.type.sourceUrl());
            if (ref.typeData->status() == QQmlDataBlob::ResolvingDependencies) {
                // TODO: give an error message? If so, we should record and show the path of the cycle.
                continue;
            }
            addDependency(ref.typeData.data());
            ref.prefix = csRef.prefix;

            m_compositeSingletons << ref;
        }
    }

    for (QV4::CompiledData::TypeReferenceMap::ConstIterator unresolvedRef = m_typeReferences.constBegin(), end = m_typeReferences.constEnd();
         unresolvedRef != end; ++unresolvedRef) {

        TypeReference ref; // resolved reference

        const bool reportErrors = unresolvedRef->errorWhenNotFound;

        int majorVersion = -1;
        int minorVersion = -1;

        const QString name = stringAt(unresolvedRef.key());

        if (!resolveType(name, majorVersion, minorVersion, ref, unresolvedRef->location.line,
                         unresolvedRef->location.column, reportErrors,
                         QQmlType::AnyRegistrationType) && reportErrors)
            return;

        if (ref.type.isComposite()) {
            ref.typeData = typeLoader()->getType(ref.type.sourceUrl());
            addDependency(ref.typeData.data());
        }
        ref.majorVersion = majorVersion;
        ref.minorVersion = minorVersion;

        ref.location.line = unresolvedRef->location.line;
        ref.location.column = unresolvedRef->location.column;

        ref.needsCreation = unresolvedRef->needsCreation;

        m_resolvedTypes.insert(unresolvedRef.key(), ref);
    }

    // ### this allows enums to work without explicit import or instantiation of the type
    if (!m_implicitImportLoaded)
        loadImplicitImport();
}

QQmlCompileError QQmlTypeData::buildTypeResolutionCaches(
        QQmlRefPointer<QQmlTypeNameCache> *typeNameCache,
        QV4::CompiledData::ResolvedTypeReferenceMap *resolvedTypeCache
        ) const
{
    typeNameCache->adopt(new QQmlTypeNameCache(m_importCache));

    for (const QString &ns: m_namespaces)
        (*typeNameCache)->add(ns);

    // Add any Composite Singletons that were used to the import cache
    for (const QQmlTypeData::TypeReference &singleton: m_compositeSingletons)
        (*typeNameCache)->add(singleton.type.qmlTypeName(), singleton.type.sourceUrl(), singleton.prefix);

    m_importCache.populateCache(typeNameCache->data());

    QQmlEnginePrivate * const engine = QQmlEnginePrivate::get(typeLoader()->engine());

    for (auto resolvedType = m_resolvedTypes.constBegin(), end = m_resolvedTypes.constEnd(); resolvedType != end; ++resolvedType) {
        QScopedPointer<QV4::CompiledData::ResolvedTypeReference> ref(new QV4::CompiledData::ResolvedTypeReference);
        QQmlType qmlType = resolvedType->type;
        if (resolvedType->typeData) {
            if (resolvedType->needsCreation && qmlType.isCompositeSingleton()) {
                return QQmlCompileError(resolvedType->location, tr("Composite Singleton Type %1 is not creatable.").arg(qmlType.qmlTypeName()));
            }
            ref->compilationUnit = resolvedType->typeData->compilationUnit();
        } else if (qmlType.isValid()) {
            ref->type = qmlType;
            Q_ASSERT(ref->type.isValid());

            if (resolvedType->needsCreation && !ref->type.isCreatable()) {
                QString reason = ref->type.noCreationReason();
                if (reason.isEmpty())
                    reason = tr("Element is not creatable.");
                return QQmlCompileError(resolvedType->location, reason);
            }

            if (ref->type.containsRevisionedAttributes()) {
                ref->typePropertyCache = engine->cache(ref->type,
                                                       resolvedType->minorVersion);
            }
        }
        ref->majorVersion = resolvedType->majorVersion;
        ref->minorVersion = resolvedType->minorVersion;
        ref->doDynamicTypeCheck();
        resolvedTypeCache->insert(resolvedType.key(), ref.take());
    }
    QQmlCompileError noError;
    return noError;
}

bool QQmlTypeData::resolveType(const QString &typeName, int &majorVersion, int &minorVersion,
                               TypeReference &ref, int lineNumber, int columnNumber,
                               bool reportErrors, QQmlType::RegistrationType registrationType)
{
    QQmlImportNamespace *typeNamespace = nullptr;
    QList<QQmlError> errors;

    bool typeFound = m_importCache.resolveType(typeName, &ref.type, &majorVersion, &minorVersion,
                                               &typeNamespace, &errors, registrationType);
    if (!typeNamespace && !typeFound && !m_implicitImportLoaded) {
        // Lazy loading of implicit import
        if (loadImplicitImport()) {
            // Try again to find the type
            errors.clear();
            typeFound = m_importCache.resolveType(typeName, &ref.type, &majorVersion, &minorVersion,
                                                  &typeNamespace, &errors, registrationType);
        } else {
            return false; //loadImplicitImport() hit an error, and called setError already
        }
    }

    if ((!typeFound || typeNamespace) && reportErrors) {
        // Known to not be a type:
        //  - known to be a namespace (Namespace {})
        //  - type with unknown namespace (UnknownNamespace.SomeType {})
        QQmlError error;
        if (typeNamespace) {
            error.setDescription(QQmlTypeLoader::tr("Namespace %1 cannot be used as a type").arg(typeName));
        } else {
            if (errors.size()) {
                error = errors.takeFirst();
            } else {
                // this should not be possible!
                // Description should come from error provided by addImport() function.
                error.setDescription(QQmlTypeLoader::tr("Unreported error adding script import to import database"));
            }
            error.setUrl(m_importCache.baseUrl());
            error.setDescription(QQmlTypeLoader::tr("%1 %2").arg(typeName).arg(error.description()));
        }

        if (lineNumber != -1)
            error.setLine(lineNumber);
        if (columnNumber != -1)
            error.setColumn(columnNumber);

        errors.prepend(error);
        setError(errors);
        return false;
    }

    return true;
}

void QQmlTypeData::scriptImported(const QQmlRefPointer<QQmlScriptBlob> &blob, const QV4::CompiledData::Location &location, const QString &qualifier, const QString &/*nameSpace*/)
{
    ScriptReference ref;
    ref.script = blob;
    ref.location = location;
    ref.qualifier = qualifier;

    m_scripts << ref;
}

QQmlScriptData::QQmlScriptData()
    : typeNameCache(nullptr)
    , m_loaded(false)
{
}

QQmlContextData *QQmlScriptData::qmlContextDataForContext(QQmlContextData *parentQmlContextData)
{
    Q_ASSERT(parentQmlContextData && parentQmlContextData->engine);

    if (m_precompiledScript->isESModule())
        return nullptr;

    auto qmlContextData = new QQmlContextData();

    qmlContextData->isInternal = true;
    qmlContextData->isJSContext = true;
    if (m_precompiledScript->isSharedLibrary())
        qmlContextData->isPragmaLibraryContext = true;
    else
        qmlContextData->isPragmaLibraryContext = parentQmlContextData->isPragmaLibraryContext;
    qmlContextData->baseUrl = url;
    qmlContextData->baseUrlString = urlString;

    // For backward compatibility, if there are no imports, we need to use the
    // imports from the parent context.  See QTBUG-17518.
    if (!typeNameCache->isEmpty()) {
        qmlContextData->imports = typeNameCache;
    } else if (!m_precompiledScript->isSharedLibrary()) {
        qmlContextData->imports = parentQmlContextData->imports;
        qmlContextData->importedScripts = parentQmlContextData->importedScripts;
    }

    if (!m_precompiledScript->isSharedLibrary()) {
        qmlContextData->setParent(parentQmlContextData);
    } else {
        qmlContextData->engine = parentQmlContextData->engine; // Fix for QTBUG-21620
    }

    QV4::ExecutionEngine *v4 = parentQmlContextData->engine->handle();
    QV4::Scope scope(v4);
    QV4::ScopedObject scriptsArray(scope);
    if (qmlContextData->importedScripts.isNullOrUndefined()) {
        scriptsArray = v4->newArrayObject(scripts.count());
        qmlContextData->importedScripts.set(v4, scriptsArray);
    } else {
        scriptsArray = qmlContextData->importedScripts.valueRef();
    }
    QV4::ScopedValue v(scope);
    for (int ii = 0; ii < scripts.count(); ++ii)
        scriptsArray->put(ii, (v = scripts.at(ii)->scriptData()->scriptValueForContext(qmlContextData)));

    return qmlContextData;
}

QV4::ReturnedValue QQmlScriptData::scriptValueForContext(QQmlContextData *parentQmlContextData)
{
    if (m_loaded)
        return m_value.value();

    Q_ASSERT(parentQmlContextData && parentQmlContextData->engine);
    QV4::ExecutionEngine *v4 = parentQmlContextData->engine->handle();
    QV4::Scope scope(v4);

    if (!hasEngine()) {
        addToEngine(parentQmlContextData->engine);
        addref();
    }

    QQmlContextDataRef qmlContextData = qmlContextDataForContext(parentQmlContextData);
    QV4::Scoped<QV4::QmlContext> qmlExecutionContext(scope);
    if (qmlContextData)
        qmlExecutionContext =
            QV4::QmlContext::create(v4->rootContext(), qmlContextData, /* scopeObject: */ nullptr);

    QV4::Scoped<QV4::Module> module(scope, m_precompiledScript->instantiate(v4));
    if (module) {
        if (qmlContextData) {
            module->d()->scope->outer.set(v4, qmlExecutionContext->d());
            qmlExecutionContext->d()->qml()->module.set(v4, module->d());
        }

        module->evaluate();
    }

    if (v4->hasException) {
        QQmlError error = v4->catchExceptionAsQmlError();
        if (error.isValid())
            QQmlEnginePrivate::get(v4)->warning(error);
    }

    QV4::ScopedValue value(scope);
    if (qmlContextData)
        value = qmlExecutionContext->d()->qml();
    else if (module)
        value = module->d();

    if (m_precompiledScript->isSharedLibrary() || m_precompiledScript->isESModule()) {
        m_loaded = true;
        m_value.set(v4, value);
    }

    return value->asReturnedValue();
}

void QQmlScriptData::clear()
{
    if (typeNameCache) {
        typeNameCache->release();
        typeNameCache = nullptr;
    }

    scripts.clear();

    // An addref() was made when the QQmlCleanup was added to the engine.
    release();
}

QQmlScriptBlob::QQmlScriptBlob(const QUrl &url, QQmlTypeLoader *loader)
    : QQmlTypeLoader::Blob(url, JavaScriptFile, loader)
    , m_isModule(url.path().endsWith(QLatin1String(".mjs")))
{
}

QQmlScriptBlob::~QQmlScriptBlob()
{
}

QQmlRefPointer<QQmlScriptData> QQmlScriptBlob::scriptData() const
{
    return m_scriptData;
}

void QQmlScriptBlob::dataReceived(const SourceCodeData &data)
{
    if (!disableDiskCache() || forceDiskCache()) {
        QQmlRefPointer<QV4::CompiledData::CompilationUnit> unit = QV4::Compiler::Codegen::createUnitForLoading();
        QString error;
        if (unit->loadFromDisk(url(), data.sourceTimeStamp(), &error)) {
            initializeFromCompilationUnit(unit);
            return;
        } else {
            qCDebug(DBG_DISK_CACHE()) << "Error loading" << urlString() << "from disk cache:" << error;
        }
    }

    if (!data.exists()) {
        if (m_cachedUnitStatus == QQmlMetaType::CachedUnitLookupError::VersionMismatch)
            setError(QQmlTypeLoader::tr("File was compiled ahead of time with an incompatible version of Qt and the original file cannot be found. Please recompile"));
        else
            setError(QQmlTypeLoader::tr("No such file or directory"));
        return;
    }

    QString error;
    QString source = data.readAll(&error);
    if (!error.isEmpty()) {
        setError(error);
        return;
    }

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> unit;

    if (m_isModule) {
        QList<QQmlJS::DiagnosticMessage> diagnostics;
        unit = QV4::ExecutionEngine::compileModule(isDebugging(), urlString(), source, data.sourceTimeStamp(), &diagnostics);
        QList<QQmlError> errors = QQmlEnginePrivate::qmlErrorFromDiagnostics(urlString(), diagnostics);
        if (!errors.isEmpty()) {
            setError(errors);
            return;
        }
    } else {
        QmlIR::Document irUnit(isDebugging());

        irUnit.jsModule.sourceTimeStamp = data.sourceTimeStamp();

        QmlIR::ScriptDirectivesCollector collector(&irUnit);
        irUnit.jsParserEngine.setDirectives(&collector);

        QList<QQmlError> errors;
        unit = QV4::Script::precompile(
                    &irUnit.jsModule, &irUnit.jsParserEngine, &irUnit.jsGenerator, urlString(), finalUrlString(),
                    source, &errors, QV4::Compiler::ContextType::ScriptImportedByQML);
        // No need to addref on unit, it's initial refcount is 1
        source.clear();
        if (!errors.isEmpty()) {
            setError(errors);
            return;
        }
        if (!unit) {
            unit.adopt(new QV4::CompiledData::CompilationUnit);
        }
        irUnit.javaScriptCompilationUnit = unit;

        QmlIR::QmlUnitGenerator qmlGenerator;
        qmlGenerator.generate(irUnit);
    }

    if ((!disableDiskCache() || forceDiskCache()) && !isDebugging()) {
        QString errorString;
        if (unit->saveToDisk(url(), &errorString)) {
            QString error;
            if (!unit->loadFromDisk(url(), data.sourceTimeStamp(), &error)) {
                // ignore error, keep using the in-memory compilation unit.
            }
        } else {
            qCDebug(DBG_DISK_CACHE()) << "Error saving cached version of" << unit->fileName() << "to disk:" << errorString;
        }
    }

    initializeFromCompilationUnit(unit);
}

void QQmlScriptBlob::initializeFromCachedUnit(const QV4::CompiledData::Unit *unit)
{
    QQmlRefPointer<QV4::CompiledData::CompilationUnit> compilationUnit;
    compilationUnit.adopt(new QV4::CompiledData::CompilationUnit(unit, urlString(), finalUrlString()));
    initializeFromCompilationUnit(compilationUnit);
}

void QQmlScriptBlob::done()
{
    if (isError())
        return;

    // Check all script dependencies for errors
    for (int ii = 0; ii < m_scripts.count(); ++ii) {
        const ScriptReference &script = m_scripts.at(ii);
        Q_ASSERT(script.script->isCompleteOrError());
        if (script.script->isError()) {
            QList<QQmlError> errors = script.script->errors();
            QQmlError error;
            error.setUrl(url());
            error.setLine(script.location.line);
            error.setColumn(script.location.column);
            error.setDescription(QQmlTypeLoader::tr("Script %1 unavailable").arg(script.script->urlString()));
            errors.prepend(error);
            setError(errors);
            return;
        }
    }

    if (!m_isModule) {
        m_scriptData->typeNameCache = new QQmlTypeNameCache(m_importCache);

        QSet<QString> ns;

        for (int scriptIndex = 0; scriptIndex < m_scripts.count(); ++scriptIndex) {
            const ScriptReference &script = m_scripts.at(scriptIndex);

            m_scriptData->scripts.append(script.script);

            if (!script.nameSpace.isNull()) {
                if (!ns.contains(script.nameSpace)) {
                    ns.insert(script.nameSpace);
                    m_scriptData->typeNameCache->add(script.nameSpace);
                }
            }
            m_scriptData->typeNameCache->add(script.qualifier, scriptIndex, script.nameSpace);
        }

        m_importCache.populateCache(m_scriptData->typeNameCache);
    }
    m_scripts.clear();
}

QString QQmlScriptBlob::stringAt(int index) const
{
    return m_scriptData->m_precompiledScript->stringAt(index);
}

void QQmlScriptBlob::scriptImported(const QQmlRefPointer<QQmlScriptBlob> &blob, const QV4::CompiledData::Location &location, const QString &qualifier, const QString &nameSpace)
{
    ScriptReference ref;
    ref.script = blob;
    ref.location = location;
    ref.qualifier = qualifier;
    ref.nameSpace = nameSpace;

    m_scripts << ref;
}

void QQmlScriptBlob::initializeFromCompilationUnit(const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &unit)
{
    Q_ASSERT(!m_scriptData);
    m_scriptData.adopt(new QQmlScriptData());
    m_scriptData->url = finalUrl();
    m_scriptData->urlString = finalUrlString();
    m_scriptData->m_precompiledScript = unit;

    m_importCache.setBaseUrl(finalUrl(), finalUrlString());

    QQmlRefPointer<QV4::CompiledData::CompilationUnit> script = m_scriptData->m_precompiledScript;

    if (!m_isModule) {
        QList<QQmlError> errors;
        for (quint32 i = 0, count = script->importCount(); i < count; ++i) {
            const QV4::CompiledData::Import *import = script->importAt(i);
            if (!addImport(import, &errors)) {
                Q_ASSERT(errors.size());
                QQmlError error(errors.takeFirst());
                error.setUrl(m_importCache.baseUrl());
                error.setLine(import->location.line);
                error.setColumn(import->location.column);
                errors.prepend(error); // put it back on the list after filling out information.
                setError(errors);
                return;
            }
        }
    }

    auto *v4 = QQmlEnginePrivate::getV4Engine(typeLoader()->engine());

    v4->injectModule(unit);

    for (const QString &request: unit->moduleRequests()) {
        if (v4->moduleForUrl(QUrl(request), unit.data()))
            continue;

        const QUrl absoluteRequest = unit->finalUrl().resolved(QUrl(request));
        QQmlRefPointer<QQmlScriptBlob> blob = typeLoader()->getScript(absoluteRequest);
        addDependency(blob.data());
        scriptImported(blob, /* ### */QV4::CompiledData::Location(), /*qualifier*/QString(), /*namespace*/QString());
    }
}

QQmlQmldirData::QQmlQmldirData(const QUrl &url, QQmlTypeLoader *loader)
: QQmlTypeLoader::Blob(url, QmldirFile, loader)
{
}

const QString &QQmlQmldirData::content() const
{
    return m_content;
}

const QV4::CompiledData::Import *QQmlQmldirData::import(QQmlTypeLoader::Blob *blob) const
{
    QHash<QQmlTypeLoader::Blob *, const QV4::CompiledData::Import *>::const_iterator it =
        m_imports.find(blob);
    if (it == m_imports.end())
        return nullptr;
    return *it;
}

void QQmlQmldirData::setImport(QQmlTypeLoader::Blob *blob, const QV4::CompiledData::Import *import)
{
    m_imports[blob] = import;
}

int QQmlQmldirData::priority(QQmlTypeLoader::Blob *blob) const
{
    QHash<QQmlTypeLoader::Blob *, int>::const_iterator it = m_priorities.find(blob);
    if (it == m_priorities.end())
        return 0;
    return *it;
}

void QQmlQmldirData::setPriority(QQmlTypeLoader::Blob *blob, int priority)
{
    m_priorities[blob] = priority;
}

void QQmlQmldirData::dataReceived(const SourceCodeData &data)
{
    QString error;
    m_content = data.readAll(&error);
    if (!error.isEmpty()) {
        setError(error);
        return;
    }
}

void QQmlQmldirData::initializeFromCachedUnit(const QV4::CompiledData::Unit *)
{
    Q_UNIMPLEMENTED();
}

QString QQmlDataBlob::SourceCodeData::readAll(QString *error) const
{
    error->clear();
    if (hasInlineSourceCode)
        return inlineSourceCode;

    QFile f(fileInfo.absoluteFilePath());
    if (!f.open(QIODevice::ReadOnly)) {
        *error = f.errorString();
        return QString();
    }

    const qint64 fileSize = fileInfo.size();

    if (uchar *mappedData = f.map(0, fileSize)) {
        QString source = QString::fromUtf8(reinterpret_cast<const char *>(mappedData), fileSize);
        f.unmap(mappedData);
        return source;
    }

    QByteArray data(fileSize, Qt::Uninitialized);
    if (f.read(data.data(), data.length()) != data.length()) {
        *error = f.errorString();
        return QString();
    }
    return QString::fromUtf8(data);
}

QDateTime QQmlDataBlob::SourceCodeData::sourceTimeStamp() const
{
    if (hasInlineSourceCode)
        return QDateTime();

    return fileInfo.lastModified();
}

bool QQmlDataBlob::SourceCodeData::exists() const
{
    if (hasInlineSourceCode)
        return true;
    return fileInfo.exists();
}

bool QQmlDataBlob::SourceCodeData::isEmpty() const
{
    if (hasInlineSourceCode)
        return inlineSourceCode.isEmpty();
    return fileInfo.size() == 0;
}

QT_END_NAMESPACE

#include "qqmltypeloader.moc"
