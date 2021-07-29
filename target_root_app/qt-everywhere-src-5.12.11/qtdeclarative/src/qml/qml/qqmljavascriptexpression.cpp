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

#include "qqmljavascriptexpression_p.h"

#include <private/qqmlexpression_p.h>
#include <private/qv4context_p.h>
#include <private/qv4value_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4script_p.h>
#include <private/qv4errorobject_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4jscall_p.h>
#include <private/qqmlglobal_p.h>
#include <private/qv4qobjectwrapper_p.h>
#include <private/qqmlbuiltinfunctions_p.h>

QT_BEGIN_NAMESPACE

bool QQmlDelayedError::addError(QQmlEnginePrivate *e)
{
    if (!e) return false;

    if (e->inProgressCreations == 0) return false; // Not in construction

    if (prevError) return true; // Already in error chain

    prevError = &e->erroredBindings;
    nextError = e->erroredBindings;
    e->erroredBindings = this;
    if (nextError) nextError->prevError = &nextError;

    return true;
}

void QQmlDelayedError::setErrorLocation(const QQmlSourceLocation &sourceLocation)
{
    m_error.setUrl(QUrl(sourceLocation.sourceFile));
    m_error.setLine(sourceLocation.line);
    m_error.setColumn(sourceLocation.column);
}

void QQmlDelayedError::setErrorDescription(const QString &description)
{
    m_error.setDescription(description);
}

void QQmlDelayedError::setErrorObject(QObject *object)
{
    m_error.setObject(object);
}

void QQmlDelayedError::catchJavaScriptException(QV4::ExecutionEngine *engine)
{
    m_error = engine->catchExceptionAsQmlError();
}


QQmlJavaScriptExpression::QQmlJavaScriptExpression()
    :  m_context(nullptr),
      m_prevExpression(nullptr),
      m_nextExpression(nullptr),
      m_v4Function(nullptr)
{
}

QQmlJavaScriptExpression::~QQmlJavaScriptExpression()
{
    if (m_prevExpression) {
        *m_prevExpression = m_nextExpression;
        if (m_nextExpression)
            m_nextExpression->m_prevExpression = m_prevExpression;
    }

    clearActiveGuards();
    clearError();
    if (m_scopeObject.isT2()) // notify DeleteWatcher of our deletion.
        m_scopeObject.asT2()->_s = nullptr;
}

void QQmlJavaScriptExpression::setNotifyOnValueChanged(bool v)
{
    activeGuards.setFlagValue(v);
    if (!v)
        clearActiveGuards();
}

void QQmlJavaScriptExpression::resetNotifyOnValueChanged()
{
    setNotifyOnValueChanged(false);
}

QQmlSourceLocation QQmlJavaScriptExpression::sourceLocation() const
{
    if (m_v4Function)
        return m_v4Function->sourceLocation();
    return QQmlSourceLocation();
}

void QQmlJavaScriptExpression::setContext(QQmlContextData *context)
{
    if (m_prevExpression) {
        *m_prevExpression = m_nextExpression;
        if (m_nextExpression)
            m_nextExpression->m_prevExpression = m_prevExpression;
        m_prevExpression = nullptr;
        m_nextExpression = nullptr;
    }

    m_context = context;

    if (context) {
        m_nextExpression = context->expressions;
        if (m_nextExpression)
            m_nextExpression->m_prevExpression = &m_nextExpression;
        m_prevExpression = &context->expressions;
        context->expressions = this;
    }
}

QV4::Function *QQmlJavaScriptExpression::function() const
{
    return m_v4Function;
}

void QQmlJavaScriptExpression::refresh()
{
}

QV4::ReturnedValue QQmlJavaScriptExpression::evaluate(bool *isUndefined)
{
    QV4::ExecutionEngine *v4 = m_context->engine->handle();
    QV4::Scope scope(v4);
    QV4::JSCallData jsCall(scope);

    return evaluate(jsCall.callData(), isUndefined);
}

QV4::ReturnedValue QQmlJavaScriptExpression::evaluate(QV4::CallData *callData, bool *isUndefined)
{
    Q_ASSERT(m_context && m_context->engine);

    QV4::Function *v4Function = function();
    if (!v4Function) {
        if (isUndefined)
            *isUndefined = true;
        return QV4::Encode::undefined();
    }

    QQmlEnginePrivate *ep = QQmlEnginePrivate::get(m_context->engine);

    // All code that follows must check with watcher before it accesses data members
    // incase we have been deleted.
    DeleteWatcher watcher(this);

    Q_ASSERT(notifyOnValueChanged() || activeGuards.isEmpty());
    QQmlPropertyCapture capture(m_context->engine, this, &watcher);

    QQmlPropertyCapture *lastPropertyCapture = ep->propertyCapture;
    ep->propertyCapture = notifyOnValueChanged() ? &capture : nullptr;


    if (notifyOnValueChanged())
        capture.guards.copyAndClearPrepend(activeGuards);

    QV4::ExecutionEngine *v4 = m_context->engine->handle();
    callData->thisObject = v4->globalObject;
    if (scopeObject()) {
         QV4::ReturnedValue scope = QV4::QObjectWrapper::wrap(v4, scopeObject());
        if (QV4::Value::fromReturnedValue(scope).isObject())
            callData->thisObject = scope;
    }

    Q_ASSERT(m_qmlScope.valueRef());
    QV4::ReturnedValue res = v4Function->call(&callData->thisObject, callData->args, callData->argc(), static_cast<QV4::ExecutionContext *>(m_qmlScope.valueRef()));
    QV4::Scope scope(v4);
    QV4::ScopedValue result(scope, res);

    if (scope.hasException()) {
        if (watcher.wasDeleted())
            scope.engine->catchException(); // ignore exception
        else
            delayedError()->catchJavaScriptException(scope.engine);
        if (isUndefined)
            *isUndefined = true;
    } else {
        if (isUndefined)
            *isUndefined = result->isUndefined();

        if (!watcher.wasDeleted() && hasDelayedError())
            delayedError()->clearError();
    }

    if (capture.errorString) {
        for (int ii = 0; ii < capture.errorString->count(); ++ii)
            qWarning("%s", qPrintable(capture.errorString->at(ii)));
        delete capture.errorString;
        capture.errorString = nullptr;
    }

    while (QQmlJavaScriptExpressionGuard *g = capture.guards.takeFirst())
        g->Delete();

    if (!watcher.wasDeleted())
        setTranslationsCaptured(capture.translationCaptured);

    ep->propertyCapture = lastPropertyCapture;

    return result->asReturnedValue();
}

void QQmlPropertyCapture::captureProperty(QQmlNotifier *n)
{
    if (watcher->wasDeleted())
        return;

    Q_ASSERT(expression);
    // Try and find a matching guard
    while (!guards.isEmpty() && !guards.first()->isConnected(n))
        guards.takeFirst()->Delete();

    QQmlJavaScriptExpressionGuard *g = nullptr;
    if (!guards.isEmpty()) {
        g = guards.takeFirst();
        g->cancelNotify();
        Q_ASSERT(g->isConnected(n));
    } else {
        g = QQmlJavaScriptExpressionGuard::New(expression, engine);
        g->connect(n);
    }

    expression->activeGuards.prepend(g);
}

/*! \internal

    \a n is in the signal index range (see QObjectPrivate::signalIndex()).
*/
void QQmlPropertyCapture::captureProperty(QObject *o, int c, int n, bool doNotify)
{
    if (watcher->wasDeleted())
        return;

    Q_ASSERT(expression);
    if (n == -1) {
        if (!errorString) {
            errorString = new QStringList;
            QString preamble = QLatin1String("QQmlExpression: Expression ") +
                    expression->expressionIdentifier() +
                    QLatin1String(" depends on non-NOTIFYable properties:");
            errorString->append(preamble);
        }

        const QMetaObject *metaObj = o->metaObject();
        QMetaProperty metaProp = metaObj->property(c);

        QString error = QLatin1String("    ") +
                QString::fromUtf8(metaObj->className()) +
                QLatin1String("::") +
                QString::fromUtf8(metaProp.name());
        errorString->append(error);
    } else {

        // Try and find a matching guard
        while (!guards.isEmpty() && !guards.first()->isConnected(o, n))
            guards.takeFirst()->Delete();

        QQmlJavaScriptExpressionGuard *g = nullptr;
        if (!guards.isEmpty()) {
            g = guards.takeFirst();
            g->cancelNotify();
            Q_ASSERT(g->isConnected(o, n));
        } else {
            g = QQmlJavaScriptExpressionGuard::New(expression, engine);
            g->connect(o, n, engine, doNotify);
        }

        expression->activeGuards.prepend(g);
    }
}

QQmlError QQmlJavaScriptExpression::error(QQmlEngine *engine) const
{
    Q_UNUSED(engine);

    if (m_error)
        return m_error->error();
    else
        return QQmlError();
}

QQmlDelayedError *QQmlJavaScriptExpression::delayedError()
{
    if (!m_error)
        m_error = new QQmlDelayedError;
    return m_error.data();
}

QV4::ReturnedValue
QQmlJavaScriptExpression::evalFunction(QQmlContextData *ctxt, QObject *scopeObject,
                                       const QString &code, const QString &filename, quint16 line)
{
    QQmlEngine *engine = ctxt->engine;
    QQmlEnginePrivate *ep = QQmlEnginePrivate::get(engine);

    QV4::ExecutionEngine *v4 = engine->handle();
    QV4::Scope scope(v4);

    QV4::Scoped<QV4::QmlContext> qmlContext(scope, QV4::QmlContext::create(v4->rootContext(), ctxt, scopeObject));
    QV4::Script script(v4, qmlContext, /*parse as QML binding*/true, code, filename, line);
    QV4::ScopedValue result(scope);
    script.parse();
    if (!v4->hasException)
        result = script.run();
    if (v4->hasException) {
        QQmlError error = v4->catchExceptionAsQmlError();
        if (error.description().isEmpty())
            error.setDescription(QLatin1String("Exception occurred during function evaluation"));
        if (error.line() == -1)
            error.setLine(line);
        if (error.url().isEmpty())
            error.setUrl(QUrl::fromLocalFile(filename));
        error.setObject(scopeObject);
        ep->warning(error);
        return QV4::Encode::undefined();
    }
    return result->asReturnedValue();
}

void QQmlJavaScriptExpression::createQmlBinding(QQmlContextData *ctxt, QObject *qmlScope,
                                          const QString &code, const QString &filename, quint16 line)
{
    QQmlEngine *engine = ctxt->engine;
    QQmlEnginePrivate *ep = QQmlEnginePrivate::get(engine);

    QV4::ExecutionEngine *v4 = engine->handle();
    QV4::Scope scope(v4);

    QV4::Scoped<QV4::QmlContext> qmlContext(scope, QV4::QmlContext::create(v4->rootContext(), ctxt, qmlScope));
    QV4::Script script(v4, qmlContext, /*parse as QML binding*/true, code, filename, line);
    script.parse();
    if (v4->hasException) {
        QQmlDelayedError *error = delayedError();
        error->catchJavaScriptException(v4);
        error->setErrorObject(qmlScope);
        if (!error->addError(ep))
            ep->warning(error->error());
        return;
    }
    setupFunction(qmlContext, script.vmFunction);
}

void QQmlJavaScriptExpression::setupFunction(QV4::ExecutionContext *qmlContext, QV4::Function *f)
{
    if (!qmlContext || !f)
        return;
    m_qmlScope.set(qmlContext->engine(), *qmlContext);
    m_v4Function = f;
    setCompilationUnit(m_v4Function->compilationUnit);
}

void QQmlJavaScriptExpression::setCompilationUnit(const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compilationUnit)
{
    m_compilationUnit = compilationUnit;
}

void QQmlJavaScriptExpression::clearActiveGuards()
{
    while (QQmlJavaScriptExpressionGuard *g = activeGuards.takeFirst())
        g->Delete();
}

void QQmlJavaScriptExpressionGuard_callback(QQmlNotifierEndpoint *e, void **)
{
    QQmlJavaScriptExpression *expression =
        static_cast<QQmlJavaScriptExpressionGuard *>(e)->expression;

    expression->expressionChanged();
}

QT_END_NAMESPACE
