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

#include "qv4debugjob.h"

#include <private/qv4script_p.h>
#include <private/qqmlcontext_p.h>
#include <private/qv4qmlcontext_p.h>
#include <private/qv4qobjectwrapper_p.h>
#include <private/qqmldebugservice_p.h>
#include <private/qv4jscall_p.h>

#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

QV4DebugJob::~QV4DebugJob()
{
}

JavaScriptJob::JavaScriptJob(QV4::ExecutionEngine *engine, int frameNr, int context,
                             const QString &script) :
    engine(engine), frameNr(frameNr), context(context), script(script),
    resultIsException(false)
{}

void JavaScriptJob::run()
{
    QV4::Scope scope(engine);

    QV4::ScopedContext ctx(scope, engine->currentStackFrame ? engine->currentContext()
                                                            : engine->scriptContext());
    QObject scopeObject;

    QV4::CppStackFrame *frame = engine->currentStackFrame;

    for (int i = 0; frame && i < frameNr; ++i)
        frame = frame->parent;
    if (frameNr > 0 && frame)
        ctx = static_cast<QV4::ExecutionContext *>(&frame->jsFrame->context);

    if (context >= 0) {
        QQmlContext *extraContext = qmlContext(QQmlDebugService::objectForId(context));
        if (extraContext)
            ctx = QV4::QmlContext::create(ctx, QQmlContextData::get(extraContext), &scopeObject);
    } else if (frameNr < 0) { // Use QML context if available
        QQmlEngine *qmlEngine = engine->qmlEngine();
        if (qmlEngine) {
            QQmlContext *qmlRootContext = qmlEngine->rootContext();
            QQmlContextPrivate *ctxtPriv = QQmlContextPrivate::get(qmlRootContext);

            QV4::ScopedObject withContext(scope, engine->newObject());
            QV4::ScopedString k(scope);
            QV4::ScopedValue v(scope);
            for (int ii = 0; ii < ctxtPriv->instances.count(); ++ii) {
                QObject *object = ctxtPriv->instances.at(ii);
                if (QQmlContext *context = qmlContext(object)) {
                    if (QQmlContextData *cdata = QQmlContextData::get(context)) {
                        v = QV4::QObjectWrapper::wrap(engine, object);
                        k = engine->newString(cdata->findObjectId(object));
                        withContext->put(k, v);
                    }
                }
            }
            if (!engine->qmlContext())
                ctx = QV4::QmlContext::create(ctx, QQmlContextData::get(qmlRootContext), &scopeObject);
        }
    }

    QV4::Script script(ctx, QV4::Compiler::ContextType::Eval, this->script);
    if (const QV4::Function *function = frame ? frame->v4Function : engine->globalCode)
        script.strictMode = function->isStrict();

    // In order for property lookups in QML to work, we need to disable fast v4 lookups. That
    // is a side-effect of inheritContext.
    script.inheritContext = true;
    script.parse();
    QV4::ScopedValue result(scope);
    if (!scope.engine->hasException) {
        if (frame) {
            QV4::ScopedValue thisObject(scope, frame->thisObject());
            result = script.run(thisObject);
        } else {
            result = script.run();
        }
    }
    if (scope.engine->hasException) {
        result = scope.engine->catchException();
        resultIsException = true;
    }
    handleResult(result);
}

bool JavaScriptJob::hasExeption() const
{
    return resultIsException;
}

BacktraceJob::BacktraceJob(QV4DataCollector *collector, int fromFrame, int toFrame) :
    CollectJob(collector), fromFrame(fromFrame), toFrame(toFrame)
{
}

void BacktraceJob::run()
{
    QJsonArray frameArray;
    QVector<QV4::StackFrame> frames = collector->engine()->stackTrace(toFrame);
    for (int i = fromFrame; i < toFrame && i < frames.size(); ++i)
        frameArray.push_back(collector->buildFrame(frames[i], i));
    if (frameArray.isEmpty()) {
        result.insert(QStringLiteral("totalFrames"), 0);
    } else {
        result.insert(QStringLiteral("fromFrame"), fromFrame);
        result.insert(QStringLiteral("toFrame"), fromFrame + frameArray.size());
        result.insert(QStringLiteral("frames"), frameArray);
    }
}

FrameJob::FrameJob(QV4DataCollector *collector, int frameNr) :
    CollectJob(collector), frameNr(frameNr), success(false)
{
}

void FrameJob::run()
{
    QVector<QV4::StackFrame> frames = collector->engine()->stackTrace(frameNr + 1);
    if (frameNr >= frames.length()) {
        success = false;
    } else {
        result = collector->buildFrame(frames[frameNr], frameNr);
        success = true;
    }
}

bool FrameJob::wasSuccessful() const
{
    return success;
}

ScopeJob::ScopeJob(QV4DataCollector *collector, int frameNr, int scopeNr) :
    CollectJob(collector), frameNr(frameNr), scopeNr(scopeNr), success(false)
{
}

void ScopeJob::run()
{
    QJsonObject object;
    success = collector->collectScope(&object, frameNr, scopeNr);

    if (success) {
        QVector<QV4::Heap::ExecutionContext::ContextType> scopeTypes =
                collector->getScopeTypes(frameNr);
        result[QLatin1String("type")] = QV4DataCollector::encodeScopeType(scopeTypes[scopeNr]);
    } else {
        result[QLatin1String("type")] = -1;
    }
    result[QLatin1String("index")] = scopeNr;
    result[QLatin1String("frameIndex")] = frameNr;
    result[QLatin1String("object")] = object;
}

bool ScopeJob::wasSuccessful() const
{
    return success;
}

ValueLookupJob::ValueLookupJob(const QJsonArray &handles, QV4DataCollector *collector) :
    CollectJob(collector), handles(handles) {}

void ValueLookupJob::run()
{
    // Open a QML context if we don't have one, yet. We might run into QML objects when looking up
    // refs and that will crash without a valid QML context. Mind that engine->qmlContext() is only
    // set if the engine is currently executing QML code.
    QScopedPointer<QObject> scopeObject;
    QV4::ExecutionEngine *engine = collector->engine();
    QV4::Scope scope(engine);
    QV4::Heap::ExecutionContext *qmlContext = nullptr;
    if (engine->qmlEngine() && !engine->qmlContext()) {
        scopeObject.reset(new QObject);
        qmlContext = QV4::QmlContext::create(engine->currentContext(),
                                QQmlContextData::get(engine->qmlEngine()->rootContext()),
                                scopeObject.data());
    }
    QV4::ScopedStackFrame frame(scope, qmlContext);
    for (const QJsonValue &handle : handles) {
        QV4DataCollector::Ref ref = handle.toInt();
        if (!collector->isValidRef(ref)) {
            exception = QString::fromLatin1("Invalid Ref: %1").arg(ref);
            break;
        }
        result[QString::number(ref)] = collector->lookupRef(ref);
    }
}

const QString &ValueLookupJob::exceptionMessage() const
{
    return exception;
}

ExpressionEvalJob::ExpressionEvalJob(QV4::ExecutionEngine *engine, int frameNr,
                                     int context, const QString &expression,
                                     QV4DataCollector *collector) :
    JavaScriptJob(engine, frameNr, context, expression), collector(collector)
{
}

void ExpressionEvalJob::handleResult(QV4::ScopedValue &value)
{
    if (hasExeption())
        exception = value->toQStringNoThrow();
    result = collector->lookupRef(collector->addValueRef(value));
}

const QString &ExpressionEvalJob::exceptionMessage() const
{
    return exception;
}

const QJsonObject &ExpressionEvalJob::returnValue() const
{
    return result;
}

GatherSourcesJob::GatherSourcesJob(QV4::ExecutionEngine *engine)
    : engine(engine)
{}

void GatherSourcesJob::run()
{
    for (QV4::CompiledData::CompilationUnit *unit : engine->compilationUnits) {
        QString fileName = unit->fileName();
        if (!fileName.isEmpty())
            sources.append(fileName);
    }
}

const QStringList &GatherSourcesJob::result() const
{
    return sources;
}

EvalJob::EvalJob(QV4::ExecutionEngine *engine, const QString &script) :
    JavaScriptJob(engine, /*frameNr*/-1, /*context*/ -1, script), result(false)
{}

void EvalJob::handleResult(QV4::ScopedValue &result)
{
    this->result = result->toBoolean();
}

bool EvalJob::resultAsBoolean() const
{
    return result;
}

QT_END_NAMESPACE
