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

#include "qtqmlglobal_p.h"
#include "qquickworkerscript_p.h"
#if QT_CONFIG(qml_list_model)
#include "qqmllistmodel_p.h"
#include "qqmllistmodelworkeragent_p.h"
#endif
#include <private/qqmlengine_p.h>
#include <private/qqmlexpression_p.h>

#include <QtCore/qcoreevent.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtQml/qjsengine.h>
#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qfile.h>
#include <QtCore/qdatetime.h>
#include <QtQml/qqmlinfo.h>
#include <QtQml/qqmlfile.h>
#if QT_CONFIG(qml_network)
#include <QtNetwork/qnetworkaccessmanager.h>
#include "qqmlnetworkaccessmanagerfactory.h"
#endif

#include <private/qv8engine_p.h>
#include <private/qv4serialize_p.h>

#include <private/qv4value_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4script_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4jscall_p.h>

QT_BEGIN_NAMESPACE

class WorkerDataEvent : public QEvent
{
public:
    enum Type { WorkerData = QEvent::User };

    WorkerDataEvent(int workerId, const QByteArray &data);
    virtual ~WorkerDataEvent();

    int workerId() const;
    QByteArray data() const;

private:
    int m_id;
    QByteArray m_data;
};

class WorkerLoadEvent : public QEvent
{
public:
    enum Type { WorkerLoad = WorkerDataEvent::WorkerData + 1 };

    WorkerLoadEvent(int workerId, const QUrl &url);

    int workerId() const;
    QUrl url() const;

private:
    int m_id;
    QUrl m_url;
};

class WorkerRemoveEvent : public QEvent
{
public:
    enum Type { WorkerRemove = WorkerLoadEvent::WorkerLoad + 1 };

    WorkerRemoveEvent(int workerId);

    int workerId() const;

private:
    int m_id;
};

class WorkerErrorEvent : public QEvent
{
public:
    enum Type { WorkerError = WorkerRemoveEvent::WorkerRemove + 1 };

    WorkerErrorEvent(const QQmlError &error);

    QQmlError error() const;

private:
    QQmlError m_error;
};

class QQuickWorkerScriptEnginePrivate : public QObject
{
    Q_OBJECT
public:
    enum WorkerEventTypes {
        WorkerDestroyEvent = QEvent::User + 100
    };

    QQuickWorkerScriptEnginePrivate(QQmlEngine *eng);

    QQmlEngine *qmlengine;

    QMutex m_lock;
    QWaitCondition m_wait;

    struct WorkerScript : public QV8Engine {
        WorkerScript(int id, QQuickWorkerScriptEnginePrivate *parent);
        ~WorkerScript() override;

#if QT_CONFIG(qml_network)
        QNetworkAccessManager *networkAccessManager() override;
#endif

        QQuickWorkerScriptEnginePrivate *p = nullptr;
        QUrl source;
        QQuickWorkerScript *owner = nullptr;
#if QT_CONFIG(qml_network)
        QScopedPointer<QNetworkAccessManager> accessManager;
#endif
        int id = -1;
    };

    QHash<int, WorkerScript *> workers;
    QV4::ReturnedValue getWorker(WorkerScript *);

    int m_nextId;

    static QV4::ReturnedValue method_sendMessage(const QV4::FunctionObject *, const QV4::Value *thisObject, const QV4::Value *argv, int argc);

signals:
    void stopThread();

protected:
    bool event(QEvent *) override;

private:
    void processMessage(int, const QByteArray &);
    void processLoad(int, const QUrl &);
    void reportScriptException(WorkerScript *, const QQmlError &error);
};

QQuickWorkerScriptEnginePrivate::QQuickWorkerScriptEnginePrivate(QQmlEngine *engine)
: qmlengine(engine), m_nextId(0)
{
}

QV4::ReturnedValue QQuickWorkerScriptEnginePrivate::method_sendMessage(const QV4::FunctionObject *b,
                                                                       const QV4::Value *, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    WorkerScript *script = static_cast<WorkerScript *>(scope.engine->v8Engine);

    QV4::ScopedValue v(scope, argc > 0 ? argv[0] : QV4::Value::undefinedValue());
    QByteArray data = QV4::Serialize::serialize(v, scope.engine);

    QMutexLocker locker(&script->p->m_lock);
    if (script && script->owner)
        QCoreApplication::postEvent(script->owner, new WorkerDataEvent(0, data));

    return QV4::Encode::undefined();
}

bool QQuickWorkerScriptEnginePrivate::event(QEvent *event)
{
    if (event->type() == (QEvent::Type)WorkerDataEvent::WorkerData) {
        WorkerDataEvent *workerEvent = static_cast<WorkerDataEvent *>(event);
        processMessage(workerEvent->workerId(), workerEvent->data());
        return true;
    } else if (event->type() == (QEvent::Type)WorkerLoadEvent::WorkerLoad) {
        WorkerLoadEvent *workerEvent = static_cast<WorkerLoadEvent *>(event);
        processLoad(workerEvent->workerId(), workerEvent->url());
        return true;
    } else if (event->type() == (QEvent::Type)WorkerDestroyEvent) {
        emit stopThread();
        return true;
    } else if (event->type() == (QEvent::Type)WorkerRemoveEvent::WorkerRemove) {
        QMutexLocker locker(&m_lock);
        WorkerRemoveEvent *workerEvent = static_cast<WorkerRemoveEvent *>(event);
        QHash<int, WorkerScript *>::iterator itr = workers.find(workerEvent->workerId());
        if (itr != workers.end()) {
            delete itr.value();
            workers.erase(itr);
        }
        return true;
    } else {
        return QObject::event(event);
    }
}

void QQuickWorkerScriptEnginePrivate::processMessage(int id, const QByteArray &data)
{
    WorkerScript *script = workers.value(id);
    if (!script)
        return;

    QV4::ExecutionEngine *v4 = QV8Engine::getV4(script);
    QV4::Scope scope(v4);
    QV4::ScopedString v(scope);
    QV4::ScopedObject worker(scope, v4->globalObject->get((v = v4->newString(QStringLiteral("WorkerScript")))));
    QV4::ScopedFunctionObject onmessage(scope);
    if (worker)
        onmessage = worker->get((v = v4->newString(QStringLiteral("onMessage"))));

    if (!onmessage)
        return;

    QV4::ScopedValue value(scope, QV4::Serialize::deserialize(data, v4));

    QV4::JSCallData jsCallData(scope, 1);
    *jsCallData->thisObject = v4->global();
    jsCallData->args[0] = value;
    onmessage->call(jsCallData);
    if (scope.hasException()) {
        QQmlError error = scope.engine->catchExceptionAsQmlError();
        reportScriptException(script, error);
    }
}

void QQuickWorkerScriptEnginePrivate::processLoad(int id, const QUrl &url)
{
    if (url.isRelative())
        return;

    QString fileName = QQmlFile::urlToLocalFileOrQrc(url);

    WorkerScript *script = workers.value(id);
    if (!script)
        return;

    QV4::ExecutionEngine *v4 = QV8Engine::getV4(script);

    script->source = url;

    if (fileName.endsWith(QLatin1String(".mjs"))) {
        auto moduleUnit = v4->loadModule(url);
        if (moduleUnit) {
            if (moduleUnit->instantiate(v4))
                moduleUnit->evaluate();
        } else {
            v4->throwError(QStringLiteral("Could not load module file"));
        }
    } else {
        QString error;
        QV4::Scope scope(v4);
        QScopedPointer<QV4::Script> program;
        program.reset(QV4::Script::createFromFileOrCache(v4, /*qmlContext*/nullptr, fileName, url, &error));
        if (program.isNull()) {
            if (!error.isEmpty())
                qWarning().nospace() << error;
            return;
        }

        if (!v4->hasException)
            program->run();
    }

    if (v4->hasException) {
        QQmlError error = v4->catchExceptionAsQmlError();
        reportScriptException(script, error);
    }
}

void QQuickWorkerScriptEnginePrivate::reportScriptException(WorkerScript *script,
                                                                  const QQmlError &error)
{
    QMutexLocker locker(&script->p->m_lock);
    if (script->owner)
        QCoreApplication::postEvent(script->owner, new WorkerErrorEvent(error));
}

WorkerDataEvent::WorkerDataEvent(int workerId, const QByteArray &data)
: QEvent((QEvent::Type)WorkerData), m_id(workerId), m_data(data)
{
}

WorkerDataEvent::~WorkerDataEvent()
{
}

int WorkerDataEvent::workerId() const
{
    return m_id;
}

QByteArray WorkerDataEvent::data() const
{
    return m_data;
}

WorkerLoadEvent::WorkerLoadEvent(int workerId, const QUrl &url)
: QEvent((QEvent::Type)WorkerLoad), m_id(workerId), m_url(url)
{
}

int WorkerLoadEvent::workerId() const
{
    return m_id;
}

QUrl WorkerLoadEvent::url() const
{
    return m_url;
}

WorkerRemoveEvent::WorkerRemoveEvent(int workerId)
: QEvent((QEvent::Type)WorkerRemove), m_id(workerId)
{
}

int WorkerRemoveEvent::workerId() const
{
    return m_id;
}

WorkerErrorEvent::WorkerErrorEvent(const QQmlError &error)
: QEvent((QEvent::Type)WorkerError), m_error(error)
{
}

QQmlError WorkerErrorEvent::error() const
{
    return m_error;
}

QQuickWorkerScriptEngine::QQuickWorkerScriptEngine(QQmlEngine *parent)
: QThread(parent), d(new QQuickWorkerScriptEnginePrivate(parent))
{
    d->m_lock.lock();
    connect(d, SIGNAL(stopThread()), this, SLOT(quit()), Qt::DirectConnection);
    start(QThread::LowestPriority);
    d->m_wait.wait(&d->m_lock);
    d->moveToThread(this);
    d->m_lock.unlock();
}

QQuickWorkerScriptEngine::~QQuickWorkerScriptEngine()
{
    d->m_lock.lock();
    QCoreApplication::postEvent(d, new QEvent((QEvent::Type)QQuickWorkerScriptEnginePrivate::WorkerDestroyEvent));
    d->m_lock.unlock();

    //We have to force to cleanup the main thread's event queue here
    //to make sure the main GUI release all pending locks/wait conditions which
    //some worker script/agent are waiting for (QQmlListModelWorkerAgent::sync() for example).
    while (!isFinished()) {
        // We can't simply wait here, because the worker thread will not terminate
        // until the main thread processes the last data event it generates
        QCoreApplication::processEvents();
        yieldCurrentThread();
    }

    d->deleteLater();
}

QQuickWorkerScriptEnginePrivate::WorkerScript::WorkerScript(int id, QQuickWorkerScriptEnginePrivate *parent)
    : QV8Engine(new QV4::ExecutionEngine)
    , p(parent)
    , id(id)
{
    m_v4Engine->v8Engine = this;

    initQmlGlobalObject();

    QV4::Scope scope(m_v4Engine);
    QV4::ScopedObject api(scope, scope.engine->newObject());
    QV4::ScopedString name(scope, m_v4Engine->newString(QStringLiteral("sendMessage")));
    QV4::ScopedValue sendMessage(scope, QV4::FunctionObject::createBuiltinFunction(m_v4Engine, name, method_sendMessage, 1));
    api->put(QV4::ScopedString(scope, scope.engine->newString(QStringLiteral("sendMessage"))), sendMessage);
    m_v4Engine->globalObject->put(QV4::ScopedString(scope, scope.engine->newString(QStringLiteral("WorkerScript"))), api);
}

QQuickWorkerScriptEnginePrivate::WorkerScript::~WorkerScript()
{
    delete m_v4Engine;
}

#if QT_CONFIG(qml_network)
QNetworkAccessManager *QQuickWorkerScriptEnginePrivate::WorkerScript::networkAccessManager()
{
    if (!accessManager) {
        if (p->qmlengine && p->qmlengine->networkAccessManagerFactory()) {
            accessManager.reset(p->qmlengine->networkAccessManagerFactory()->create(p));
        } else {
            accessManager.reset(new QNetworkAccessManager(p));
        }
    }
    return accessManager.data();
}
#endif

int QQuickWorkerScriptEngine::registerWorkerScript(QQuickWorkerScript *owner)
{
    typedef QQuickWorkerScriptEnginePrivate::WorkerScript WorkerScript;
    WorkerScript *script = new WorkerScript(d->m_nextId++, d);

    script->owner = owner;

    d->m_lock.lock();
    d->workers.insert(script->id, script);
    d->m_lock.unlock();

    return script->id;
}

void QQuickWorkerScriptEngine::removeWorkerScript(int id)
{
    QQuickWorkerScriptEnginePrivate::WorkerScript* script = d->workers.value(id);
    if (script) {
        script->owner = nullptr;
        QCoreApplication::postEvent(d, new WorkerRemoveEvent(id));
    }
}

void QQuickWorkerScriptEngine::executeUrl(int id, const QUrl &url)
{
    QCoreApplication::postEvent(d, new WorkerLoadEvent(id, url));
}

void QQuickWorkerScriptEngine::sendMessage(int id, const QByteArray &data)
{
    QCoreApplication::postEvent(d, new WorkerDataEvent(id, data));
}

void QQuickWorkerScriptEngine::run()
{
    d->m_lock.lock();

    d->m_wait.wakeAll();

    d->m_lock.unlock();

    exec();

    qDeleteAll(d->workers);
    d->workers.clear();
}


/*!
    \qmltype WorkerScript
    \instantiates QQuickWorkerScript
    \ingroup qtquick-threading
    \inqmlmodule QtQuick
    \brief Enables the use of threads in a Qt Quick application.

    Use WorkerScript to run operations in a new thread.
    This is useful for running operations in the background so
    that the main GUI thread is not blocked.

    Messages can be passed between the new thread and the parent thread
    using \l sendMessage() and the \c onMessage() handler.

    An example:

    \snippet qml/workerscript/workerscript.qml 0

    The above worker script specifies a JavaScript file, "script.mjs", that handles
    the operations to be performed in the new thread. Here is \c script.mjs:

    \quotefile qml/workerscript/script.mjs

    When the user clicks anywhere within the rectangle, \c sendMessage() is
    called, triggering the \tt WorkerScript.onMessage() handler in
    \tt script.mjs. This in turn sends a reply message that is then received
    by the \tt onMessage() handler of \tt myWorker.

    The example uses a script that is an ECMAScript module, because it has the ".mjs" extension.
    It can use import statements to access functionality from other modules and it is run in JavaScript
    strict mode.

    If a worker script has the extension ".js" instead, then it is considered to contain plain JavaScript
    statements and it is run in non-strict mode.

    \note Each WorkerScript element will instantiate a separate JavaScript engine to ensure perfect
    isolation and thread-safety. If the impact of that results in a memory consumption that is too
    high for your environment, then consider sharing a WorkerScript element.

    \section3 Restrictions

    Since the \c WorkerScript.onMessage() function is run in a separate thread, the
    JavaScript file is evaluated in a context separate from the main QML engine. This means
    that unlike an ordinary JavaScript file that is imported into QML, the \c script.mjs
    in the above example cannot access the properties, methods or other attributes
    of the QML item, nor can it access any context properties set on the QML object
    through QQmlContext.

    Additionally, there are restrictions on the types of values that can be passed to and
    from the worker script. See the sendMessage() documentation for details.

    Worker scripts that are plain JavaScript sources can not use \l {qtqml-javascript-imports.html}{.import} syntax.
    Scripts that are ECMAScript modules can freely use import and export statements.

    \sa {Qt Quick Examples - Threading},
        {Threaded ListModel Example}
*/
QQuickWorkerScript::QQuickWorkerScript(QObject *parent)
: QObject(parent), m_engine(nullptr), m_scriptId(-1), m_componentComplete(true)
{
}

QQuickWorkerScript::~QQuickWorkerScript()
{
    if (m_scriptId != -1) m_engine->removeWorkerScript(m_scriptId);
}

/*!
    \qmlproperty url WorkerScript::source

    This holds the url of the JavaScript file that implements the
    \tt WorkerScript.onMessage() handler for threaded operations.

    If the file name component of the url ends with ".mjs", then the script
    is parsed as an ECMAScript module and run in strict mode. Otherwise it is considered to be
    plain script.
*/
QUrl QQuickWorkerScript::source() const
{
    return m_source;
}

void QQuickWorkerScript::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;

    if (engine())
        m_engine->executeUrl(m_scriptId, m_source);

    emit sourceChanged();
}

/*!
    \qmlmethod WorkerScript::sendMessage(jsobject message)

    Sends the given \a message to a worker script handler in another
    thread. The other worker script handler can receive this message
    through the onMessage() handler.

    The \c message object may only contain values of the following
    types:

    \list
    \li boolean, number, string
    \li JavaScript objects and arrays
    \li ListModel objects (any other type of QObject* is not allowed)
    \endlist

    All objects and arrays are copied to the \c message. With the exception
    of ListModel objects, any modifications by the other thread to an object
    passed in \c message will not be reflected in the original object.
*/
void QQuickWorkerScript::sendMessage(QQmlV4Function *args)
{
    if (!engine()) {
        qWarning("QQuickWorkerScript: Attempt to send message before WorkerScript establishment");
        return;
    }

    QV4::Scope scope(args->v4engine());
    QV4::ScopedValue argument(scope, QV4::Value::undefinedValue());
    if (args->length() != 0)
        argument = (*args)[0];

    m_engine->sendMessage(m_scriptId, QV4::Serialize::serialize(argument, scope.engine));
}

void QQuickWorkerScript::classBegin()
{
    m_componentComplete = false;
}

QQuickWorkerScriptEngine *QQuickWorkerScript::engine()
{
    if (m_engine) return m_engine;
    if (m_componentComplete) {
        QQmlEngine *engine = qmlEngine(this);
        if (!engine) {
            qWarning("QQuickWorkerScript: engine() called without qmlEngine() set");
            return nullptr;
        }

        m_engine = QQmlEnginePrivate::get(engine)->getWorkerScriptEngine();
        m_scriptId = m_engine->registerWorkerScript(this);

        if (m_source.isValid())
            m_engine->executeUrl(m_scriptId, m_source);

        return m_engine;
    }
    return nullptr;
}

void QQuickWorkerScript::componentComplete()
{
    m_componentComplete = true;
    engine(); // Get it started now.
}

/*!
    \qmlsignal WorkerScript::message(jsobject msg)

    This signal is emitted when a message \a msg is received from a worker
    script in another thread through a call to sendMessage().

    The corresponding handler is \c onMessage.
*/

bool QQuickWorkerScript::event(QEvent *event)
{
    if (event->type() == (QEvent::Type)WorkerDataEvent::WorkerData) {
        QQmlEngine *engine = qmlEngine(this);
        if (engine) {
            WorkerDataEvent *workerEvent = static_cast<WorkerDataEvent *>(event);
            QV4::Scope scope(engine->handle());
            QV4::ScopedValue value(scope, QV4::Serialize::deserialize(workerEvent->data(), scope.engine));
            emit message(QQmlV4Handle(value));
        }
        return true;
    } else if (event->type() == (QEvent::Type)WorkerErrorEvent::WorkerError) {
        WorkerErrorEvent *workerEvent = static_cast<WorkerErrorEvent *>(event);
        QQmlEnginePrivate::warning(qmlEngine(this), workerEvent->error());
        return true;
    } else {
        return QObject::event(event);
    }
}

QT_END_NAMESPACE

#include <qquickworkerscript.moc>

#include "moc_qquickworkerscript_p.cpp"
