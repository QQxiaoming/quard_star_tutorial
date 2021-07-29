/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the V4VM module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "private/qv4object_p.h"
#include "private/qv4runtime_p.h"
#include "private/qv4functionobject_p.h"
#include "private/qv4errorobject_p.h"
#include "private/qv4globalobject_p.h"
#include "private/qv4codegen_p.h"
#include "private/qv4objectproto_p.h"
#include "private/qv4mm_p.h"
#include "private/qv4context_p.h"
#include "private/qv4script_p.h"
#include "private/qv4string_p.h"
#include "private/qv4module_p.h"
#include "private/qqmlbuiltinfunctions_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <private/qqmljsengine_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qqmljsast_p.h>

#include <iostream>

static void showException(QV4::ExecutionContext *ctx, const QV4::Value &exception, const QV4::StackTrace &trace)
{
    QV4::Scope scope(ctx);
    QV4::ScopedValue ex(scope, exception);
    QV4::ErrorObject *e = ex->as<QV4::ErrorObject>();
    if (!e) {
        std::cerr << "Uncaught exception: " << qPrintable(ex->toQString()) << std::endl;
    } else {
        std::cerr << "Uncaught exception: " << qPrintable(e->toQStringNoThrow()) << std::endl;
    }

    for (const QV4::StackFrame &frame : trace) {
        std::cerr << "    at " << qPrintable(frame.function) << " (" << qPrintable(frame.source);
        if (frame.line >= 0)
            std::cerr << ':' << frame.line;
        std::cerr << ')' << std::endl;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));
    QStringList args = app.arguments();
    args.removeFirst();

    bool runAsQml = false;
    bool runAsModule = false;
    bool cache = false;

    if (!args.isEmpty()) {
        if (args.constFirst() == QLatin1String("--jit")) {
            qputenv("QV4_JIT_CALL_THRESHOLD", QByteArray("0"));
            args.removeFirst();
        }
        if (args.constFirst() == QLatin1String("--interpret")) {
            qputenv("QV4_FORCE_INTERPRETER", QByteArray("1"));
            args.removeFirst();
        }

        if (args.constFirst() == QLatin1String("--qml")) {
            runAsQml = true;
            args.removeFirst();
        }

        if (args.constFirst() == QLatin1String("--module")) {
            runAsModule = true;
            args.removeFirst();
        }

        if (args.constFirst() == QLatin1String("--cache")) {
            cache = true;
            args.removeFirst();
        }

        if (args.constFirst() == QLatin1String("--help")) {
            std::cerr << "Usage: qmljs [|--jit|--interpret|--qml] file..." << std::endl;
            return EXIT_SUCCESS;
        }
    }

    QV4::ExecutionEngine vm;

    QV4::Scope scope(&vm);
    QV4::ScopedContext ctx(scope, vm.rootContext());

    QV4::GlobalExtensions::init(vm.globalObject, QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

    for (const QString &fn : qAsConst(args)) {
        QV4::ScopedValue result(scope);
        if (runAsModule) {
            auto moduleUnit = vm.loadModule(QUrl::fromLocalFile(QFileInfo(fn).absoluteFilePath()));
            if (moduleUnit) {
                if (moduleUnit->instantiate(&vm))
                    moduleUnit->evaluate();
            } else {
                vm.throwError(QStringLiteral("Could not load module file"));
            }
        } else {
            QFile file(fn);
            if (!file.open(QFile::ReadOnly)) {
                std::cerr << "Error: cannot open file " << fn.toUtf8().constData() << std::endl;
                return EXIT_FAILURE;
            }
            QScopedPointer<QV4::Script> script;
            if (cache && QFile::exists(fn + QLatin1Char('c'))) {
                QQmlRefPointer<QV4::CompiledData::CompilationUnit> unit = QV4::Compiler::Codegen::createUnitForLoading();
                QString error;
                if (unit->loadFromDisk(QUrl::fromLocalFile(fn), QFileInfo(fn).lastModified(), &error)) {
                    script.reset(new QV4::Script(&vm, nullptr, unit));
                } else {
                    std::cout << "Error loading" << qPrintable(fn) << "from disk cache:" << qPrintable(error) << std::endl;
                }
            }
            if (!script) {
                QByteArray ba = file.readAll();
                const QString code = QString::fromUtf8(ba.constData(), ba.length());
                file.close();

                script.reset(new QV4::Script(ctx, QV4::Compiler::ContextType::Global, code, fn));
                script->parseAsBinding = runAsQml;
                script->parse();
            }
            if (!scope.engine->hasException) {
                const auto unit = script->compilationUnit;
                if (cache && unit && !(unit->unitData()->flags & QV4::CompiledData::Unit::StaticData)) {
                    if (unit->unitData()->sourceTimeStamp == 0) {
                        const_cast<QV4::CompiledData::Unit*>(unit->unitData())->sourceTimeStamp = QFileInfo(fn).lastModified().toMSecsSinceEpoch();
                    }
                    QString saveError;
                    if (!unit->saveToDisk(QUrl::fromLocalFile(fn), &saveError)) {
                        std::cout << "Error saving JS cache file: " << qPrintable(saveError) << std::endl;
                    }
                }
//                QElapsedTimer t; t.start();
                result = script->run();
//                std::cout << t.elapsed() << " ms. elapsed" << std::endl;
            }
        }
        if (scope.engine->hasException) {
            QV4::StackTrace trace;
            QV4::ScopedValue ex(scope, scope.engine->catchException(&trace));
            showException(ctx, ex, trace);
            return EXIT_FAILURE;
        }
        if (!result->isUndefined()) {
            if (! qgetenv("SHOW_EXIT_VALUE").isEmpty())
                std::cout << "exit value: " << qPrintable(result->toQString()) << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
