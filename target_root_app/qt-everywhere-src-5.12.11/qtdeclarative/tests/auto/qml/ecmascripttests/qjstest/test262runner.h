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
#ifndef TEST262RUNNER_H
#define TEST262RUNNER_H
#include <qstring.h>
#include <qstringlist.h>
#include <qset.h>
#include <qmap.h>
#include <qmutex.h>
#include <qthreadpool.h>

struct TestCase {
    TestCase() = default;
    TestCase(const QString &test)
        : test(test) {}
    enum Result {
        Skipped,
        Passes,
        Fails,
        Crashes
    };
    bool skipTestCase = false;
    Result strictExpectation = Passes;
    Result sloppyExpectation = Passes;
    Result strictResult = Skipped;
    Result sloppyResult = Skipped;

    QString test;
};

struct TestData : TestCase {
    TestData(const TestCase &testCase)
        : TestCase(testCase) {}
    // flags
    bool negative = false;
    bool runInStrictMode = true;
    bool runInSloppyMode = true;
    bool runAsModuleCode = false;
    bool async = false;

    bool isExcluded = false;

    QList<QByteArray> includes;

    QByteArray harness;
    QByteArray content;
};

class Test262Runner
{
public:
    Test262Runner(const QString &command, const QString &testDir);
    ~Test262Runner();

    enum Mode {
        Sloppy = 0,
        Strict = 1
    };

    enum Flags {
        Verbose = 0x1,
        Parallel = 0x2,
        ForceBytecode = 0x4,
        ForceJIT = 0x8,
        WithTestExpectations = 0x10,
        UpdateTestExpectations = 0x20,
        WriteTestExpectations = 0x40,
    };
    void setFlags(int f) { flags = f; }

    void setFilter(const QString &f) { filter = f; }

    void cat();
    bool run();

    bool report();

private:
    friend class SingleTest;
    bool loadTests();
    void loadTestExpectations();
    void updateTestExpectations();
    void writeTestExpectations();
    int runSingleTest(TestCase testCase);

    TestData getTestData(const TestCase &testCase);
    void parseYaml(const QByteArray &content, TestData *data);

    QByteArray harness(const QByteArray &name);

    void addResult(TestCase result);

    QString command;
    QString testDir;
    int flags = 0;

    QMutex mutex;
    QString filter;

    QMap<QString, TestCase> testCases;
    QHash<QByteArray, QByteArray> harnessFiles;

    QThreadPool *threadPool = nullptr;
};


#endif
