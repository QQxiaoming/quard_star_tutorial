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
#include <QJSEngine>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <qdebug.h>
#include <stdlib.h>

#include "test262runner.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);


    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption verbose("verbose", "Verbose output");
    parser.addOption(verbose);
    QCommandLineOption commandOption("command", "Javascript command line interpreter", "command");
    parser.addOption(commandOption);
    QCommandLineOption testDir("tests", "path to the tests", "tests", "test262");
    parser.addOption(testDir);
    QCommandLineOption cat("cat", "Print packaged test code that would be run");
    parser.addOption(cat);
    QCommandLineOption parallel("parallel", "Run tests in parallel");
    parser.addOption(parallel);
    QCommandLineOption jit("jit", "JIT all code");
    parser.addOption(jit);
    QCommandLineOption bytecode("interpret", "Run using the bytecode interpreter");
    parser.addOption(bytecode);
    QCommandLineOption withExpectations("with-test-expectations", "Parse TestExpectations to deal with known failures");
    parser.addOption(withExpectations);
    QCommandLineOption updateExpectations("update-expectations", "Update TestExpectations to remove unexepected passes");
    parser.addOption(updateExpectations);
    QCommandLineOption writeExpectations("write-expectations", "Generate a new TestExpectations file based on the results of the run");
    parser.addOption(writeExpectations);
    parser.addPositionalArgument("[filter]", "Only run tests that contain filter in their name");

    parser.process(app);

    Test262Runner testRunner(parser.value(commandOption), parser.value(testDir));

    QStringList otherArgs = parser.positionalArguments();
    if (otherArgs.size() > 1) {
        qWarning() << "too many arguments";
        return 1;
    } else if (otherArgs.size()) {
        testRunner.setFilter(otherArgs.at(0));
    }

    if (parser.isSet(cat)) {
        testRunner.cat();
        return 0;
    }

    if (parser.isSet(updateExpectations) && parser.isSet(writeExpectations)) {
        qWarning() << "Can only specify one of --update-expectations and --write-expectations.";
        exit(1);
    }

    if (parser.isSet(jit) && parser.isSet(bytecode)) {
        qWarning() << "Can only specify one of --jit and --interpret.";
        exit(1);
    }

    int flags = 0;
    if (parser.isSet(verbose))
        flags |= Test262Runner::Verbose;
    if (parser.isSet(parallel))
        flags |= Test262Runner::Parallel;
    if (parser.isSet(jit))
        flags |= Test262Runner::ForceJIT;
    if (parser.isSet(bytecode))
        flags |= Test262Runner::ForceBytecode;
    if (parser.isSet(withExpectations))
        flags |= Test262Runner::WithTestExpectations;
    if (parser.isSet(updateExpectations))
        flags |= Test262Runner::UpdateTestExpectations;
    if (parser.isSet(writeExpectations))
        flags |= Test262Runner::WriteTestExpectations;
    testRunner.setFlags(flags);

    if (testRunner.run())
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
