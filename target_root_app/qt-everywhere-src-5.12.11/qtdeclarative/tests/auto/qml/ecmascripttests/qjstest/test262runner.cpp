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
#include "test262runner.h"

#include <qfile.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qdebug.h>
#include <qprocess.h>
#include <qtemporaryfile.h>

#include <private/qv4script_p.h>
#include "private/qv4globalobject_p.h"
#include "private/qqmlbuiltinfunctions_p.h"
#include "private/qv4arraybuffer_p.h"

#include "qrunnable.h"

static const char *excludedFeatures[] = {
    "BigInt",
    "class-fields-public",
    "class-fields-private",
    "Promise.prototype.finally",
    "async-iteration",
    "Symbol.asyncIterator",
    "object-rest",
    "object-spread",
    "optional-catch-binding",
    "regexp-dotall",
    "regexp-lookbehind",
    "regexp-named-groups",
    "regexp-unicode-property-escapes",
    "Atomics",
    "SharedArrayBuffer",
    "Array.prototype.flatten",
    "Array.prototype.flatMap",
    "string-trimming",
    "String.prototype.trimEnd",
    "String.prototype.trimStart",
    "numeric-separator-literal",

    // optional features, not supported by us
    "caller",
    nullptr
};

static const char *excludedFilePatterns[] = {
    "realm",
    nullptr
};

QT_BEGIN_NAMESPACE

namespace QV4 {

static ReturnedValue method_detachArrayBuffer(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    if (!argc)
        return scope.engine->throwTypeError();
    Scoped<ArrayBuffer> a(scope, argv[0]);
    if (!a)
        return scope.engine->throwTypeError();

    if (a->isShared())
        return scope.engine->throwTypeError();

    a->d()->detachArrayBuffer();

    return Encode::null();
}

static void initD262(ExecutionEngine *e)
{
    Scope scope(e);
    ScopedObject d262(scope, e->newObject());

    d262->defineDefaultProperty(QStringLiteral("detachArrayBuffer"), method_detachArrayBuffer, 1);

    ScopedString s(scope, e->newString(QStringLiteral("$262")));
    e->globalObject->put(s, d262);
}

}

QT_END_NAMESPACE

Test262Runner::Test262Runner(const QString &command, const QString &dir)
    : command(command), testDir(dir)
{
    if (testDir.endsWith(QLatin1Char('/')))
        testDir = testDir.chopped(1);
}

Test262Runner::~Test262Runner()
{
    delete threadPool;
}

void Test262Runner::cat()
{
    if (!loadTests())
        return;

    if (testCases.size() != 1)
        qWarning() << "test262 --cat: Ambiguous test case, using" << testCases.begin().key();
    TestData data = getTestData(testCases.begin().value());
    printf("%s", data.content.constData());
}

bool Test262Runner::run()
{
    if (!loadTests())
        return false;

    if (flags & Parallel) {
        threadPool = new QThreadPool;
        threadPool->setStackSize(16*1024*1024);
        if (flags & Verbose)
            qDebug() << "Running in parallel with" << QThread::idealThreadCount() << "threads.";
    }

    if (flags & ForceJIT)
        qputenv("QV4_JIT_CALL_THRESHOLD", QByteArray("0"));
    else if (flags & ForceBytecode)
        qputenv("QV4_FORCE_INTERPRETER", QByteArray("1"));

    if (flags & WithTestExpectations)
        loadTestExpectations();

    for (auto it = testCases.constBegin(); it != testCases.constEnd(); ++it) {
        auto c = it.value();
        if (!c.skipTestCase) {
            int result = runSingleTest(c);
            if (result == -2)
                return false;
        }
    }

    if (threadPool)
        threadPool->waitForDone();

    const bool testsOk = report();

    if (flags & WriteTestExpectations)
        writeTestExpectations();
    else if (flags & UpdateTestExpectations)
        updateTestExpectations();

    return testsOk;
}

bool Test262Runner::report()
{
    qDebug() << "Test execution summary:";
    qDebug() << "    Executed" << testCases.size() << "test cases.";
    QStringList crashes;
    QStringList unexpectedFailures;
    QStringList unexpectedPasses;
    for (auto it = testCases.constBegin(); it != testCases.constEnd(); ++it) {
        const auto c = it.value();
        if (c.strictResult == c.strictExpectation && c.sloppyResult == c.sloppyExpectation)
            continue;
        auto report = [&] (TestCase::Result expected, TestCase::Result result, const char *s) {
            if (result == TestCase::Crashes)
                crashes << (it.key() + " crashed in " + s + " mode");
            if (result == TestCase::Fails && expected == TestCase::Passes)
                unexpectedFailures << (it.key() + " failed in " + s + " mode");
            if (result == TestCase::Passes && expected == TestCase::Fails)
                unexpectedPasses << (it.key() + " unexpectedly passed in " + s + " mode");
        };
        report(c.strictExpectation, c.strictResult, "strict");
        report(c.sloppyExpectation, c.sloppyResult, "sloppy");
    }
    if (!crashes.isEmpty()) {
        qDebug() << "    Encountered" << crashes.size() << "crashes in the following files:";
        for (const QString &f : qAsConst(crashes))
            qDebug() << "        " << f;
    }
    if (!unexpectedFailures.isEmpty()) {
        qDebug() << "    Encountered" << unexpectedFailures.size() << "unexpected failures in the following files:";
        for (const QString &f : qAsConst(unexpectedFailures))
            qDebug() << "        " << f;
    }
    if (!unexpectedPasses.isEmpty()) {
        qDebug() << "    Encountered" << unexpectedPasses.size() << "unexpected passes in the following files:";
        for (const QString &f : qAsConst(unexpectedPasses))
            qDebug() << "        " << f;
    }
    return crashes.isEmpty() && unexpectedFailures.isEmpty() && unexpectedPasses.isEmpty();
}

bool Test262Runner::loadTests()
{
    QDir dir(testDir + "/test");
    if (!dir.exists()) {
        qWarning() << "Could not load tests," << dir.path() << "does not exist.";
        return false;
    }

    QString annexB = "annexB";
    QString harness = "harness";
    QString intl402 = "intl402";

    int pathlen = dir.path().length() + 1;
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString file = it.next().mid(pathlen);
        if (!file.endsWith(".js"))
            continue;
        if (file.endsWith("_FIXTURE.js"))
            continue;
        if (!filter.isEmpty() && !file.contains(filter))
            continue;
        if (file.startsWith(annexB) || file.startsWith(harness) || file.startsWith(intl402))
            continue;
        const char **excluded = excludedFilePatterns;
        bool skip = false;
        while (*excluded) {
            if (file.contains(QLatin1String(*excluded)))
                skip = true;
            ++excluded;
        }
        if (skip)
            continue;

        testCases.insert(file, TestCase{ file });
    }
    if (testCases.isEmpty()) {
        qWarning() << "No tests to run.";
        return false;
    }

    return true;
}


struct TestExpectationLine {
    TestExpectationLine(const QByteArray &line);
    enum State {
        Fails,
        SloppyFails,
        StrictFails,
        Skip,
        Passes
    } state;
    QString testCase;

    QByteArray toLine() const;
    void update(const TestCase &testCase);

    static TestExpectationLine fromTestCase(const TestCase &testCase);
private:
    TestExpectationLine() = default;
    static State stateFromTestCase(const TestCase &testCase);
};

TestExpectationLine::TestExpectationLine(const QByteArray &line)
{
    int space = line.indexOf(' ');

    testCase = QString::fromUtf8(space > 0 ? line.left(space) : line);
    if (!testCase.endsWith(".js"))
        testCase += ".js";

    state = Fails;
    if (space < 0)
        return;
    QByteArray qualifier = line.mid(space + 1);
    if (qualifier == "skip")
        state = Skip;
    else if (qualifier == "strictFails")
        state = StrictFails;
    else if (qualifier == "sloppyFails")
        state = SloppyFails;
    else if (qualifier == "fails")
        state = Fails;
    else
        qWarning() << "illegal format in TestExpectations, line" << line;
}

QByteArray TestExpectationLine::toLine() const {
    const char *res = nullptr;
    switch (state) {
    case Fails:
        res = " fails\n";
        break;
    case SloppyFails:
        res = " sloppyFails\n";
        break;
    case StrictFails:
        res = " strictFails\n";
        break;
    case Skip:
        res = " skip\n";
        break;
    case Passes:
        // no need for an entry
        return QByteArray();
    }
    QByteArray result = testCase.toUtf8() + res;
    return result;
}

void TestExpectationLine::update(const TestCase &testCase)
{
    Q_ASSERT(testCase.test == this->testCase);

    State resultState = stateFromTestCase(testCase);
    switch (resultState) {
    case Fails:
        // no improvement, don't update
        break;
    case SloppyFails:
        if (state == Fails)
            state = SloppyFails;
        else if (state == StrictFails)
            // we have a regression in sloppy mode, but strict now passes
            state = Passes;
        break;
    case StrictFails:
        if (state == Fails)
            state = StrictFails;
        else if (state == SloppyFails)
            // we have a regression in strict mode, but sloppy now passes
            state = Passes;
        break;
    case Skip:
        Q_ASSERT(state == Skip);
        // nothing to do
        break;
    case Passes:
        state = Passes;
    }
}

TestExpectationLine TestExpectationLine::fromTestCase(const TestCase &testCase)
{
    TestExpectationLine l;
    l.testCase = testCase.test;
    l.state = stateFromTestCase(testCase);
    return l;
}

TestExpectationLine::State TestExpectationLine::stateFromTestCase(const TestCase &testCase)
{
    // keep skipped tests
    if (testCase.skipTestCase)
        return Skip;

    bool strictFails = (testCase.strictResult == TestCase::Crashes || testCase.strictResult == TestCase::Fails);
    bool sloppyFails = (testCase.sloppyResult == TestCase::Crashes || testCase.sloppyResult == TestCase::Fails);
    if (strictFails && sloppyFails)
        return Fails;
    if (strictFails)
        return StrictFails;
    if (sloppyFails)
        return SloppyFails;
    return Passes;
}


void Test262Runner::loadTestExpectations()
{
    QFile file("TestExpectations");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Could not open TestExpectations file.";
        return;
    }

    int line = 0;
    while (!file.atEnd()) {
        ++line;
        QByteArray line = file.readLine().trimmed();
        if (line.startsWith('#') || line.isEmpty())
            continue;
        TestExpectationLine expectation(line);
        if (!filter.isEmpty() && !expectation.testCase.contains(filter))
            continue;

        if (!testCases.contains(expectation.testCase))
            qWarning() << "Unknown test case" << expectation.testCase << "in TestExpectations file.";
        //qDebug() << "TestExpectations:" << expectation.testCase << expectation.state;
        TestCase &s = testCases[expectation.testCase];
        switch (expectation.state) {
        case TestExpectationLine::Fails:
            s.strictExpectation = TestCase::Fails;
            s.sloppyExpectation = TestCase::Fails;
            break;
        case TestExpectationLine::SloppyFails:
            s.strictExpectation = TestCase::Passes;
            s.sloppyExpectation = TestCase::Fails;
            break;
        case TestExpectationLine::StrictFails:
            s.strictExpectation = TestCase::Fails;
            s.sloppyExpectation = TestCase::Passes;
            break;
        case TestExpectationLine::Skip:
            s.skipTestCase = true;
            break;
        case TestExpectationLine::Passes:
            Q_UNREACHABLE();
        }
    }
}

void Test262Runner::updateTestExpectations()
{
    QFile file("TestExpectations");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Could not open TestExpectations file.";
        return;
    }

    QTemporaryFile updatedExpectations;
    updatedExpectations.open();

    int line = 0;
    while (!file.atEnd()) {
        ++line;
        QByteArray originalLine = file.readLine();
        QByteArray line = originalLine.trimmed();
        if (line.startsWith('#') || line.isEmpty()) {
            updatedExpectations.write(originalLine);
            continue;
        }

        TestExpectationLine expectation(line);
//        qDebug() << "checking: " << expectation.testCase;
        if (!testCases.contains(expectation.testCase)) {
            updatedExpectations.write(originalLine);
            continue;
        }
        const TestCase &testcase = testCases.value(expectation.testCase);
        expectation.update(testcase);

        line = expectation.toLine();
//        qDebug() << "updated line:" << line;
        updatedExpectations.write(line);
    }
    file.close();
    updatedExpectations.close();
    file.remove();
    qDebug() << updatedExpectations.fileName() << file.fileName();
    updatedExpectations.copy(file.fileName());
    qDebug() << "Updated TestExpectations file written!";
}

void Test262Runner::writeTestExpectations()
{
    QFile file("TestExpectations");

    QTemporaryFile expectations;
    expectations.open();

    for (auto c : qAsConst(testCases)) {
        TestExpectationLine line = TestExpectationLine::fromTestCase(c);
        expectations.write(line.toLine());
    }

    expectations.close();
    if (file.exists())
        file.remove();
    qDebug() << expectations.fileName() << file.fileName();
    expectations.copy(file.fileName());
    qDebug() << "new TestExpectations file written!";

}

static bool executeTest(const QByteArray &data, bool runAsModule = false, const QString &testCasePath = QString(), const QByteArray &harnessForModules = QByteArray())
{
    QString testData = QString::fromUtf8(data.constData(), data.size());

    QV4::ExecutionEngine vm;

    QV4::Scope scope(&vm);

    QV4::GlobalExtensions::init(vm.globalObject, QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
    QV4::initD262(&vm);

    if (runAsModule) {
        const QUrl rootModuleUrl = QUrl::fromLocalFile(testCasePath);
        // inject all modules with the harness
        QVector<QUrl> modulesToLoad = { rootModuleUrl };
        while (!modulesToLoad.isEmpty()) {
            QUrl url = modulesToLoad.takeFirst();
            QQmlRefPointer<QV4::CompiledData::CompilationUnit> module;

            QFile f(url.toLocalFile());
            if (f.open(QIODevice::ReadOnly)) {
                QByteArray content = harnessForModules + f.readAll();
                module = vm.compileModule(url.toString(), QString::fromUtf8(content.constData(), content.length()), QFileInfo(f).lastModified());
                if (vm.hasException)
                    break;
                vm.injectModule(module);
            } else {
                vm.throwError(QStringLiteral("Could not load module"));
                break;
            }

            for (const QString &request: module->moduleRequests()) {
                const QUrl absoluteRequest = module->finalUrl().resolved(QUrl(request));
                if (!vm.modules.contains(absoluteRequest))
                    modulesToLoad << absoluteRequest;
            }
        }

        if (!vm.hasException) {
            if (auto rootModuleUnit = vm.loadModule(rootModuleUrl)) {
                if (rootModuleUnit->instantiate(&vm))
                    rootModuleUnit->evaluate();
            }
        }
    } else {
        QV4::ScopedContext ctx(scope, vm.rootContext());

        QV4::Script script(ctx, QV4::Compiler::ContextType::Global, testData);
        script.parse();

        if (!vm.hasException)
            script.run();
    }
    return !vm.hasException;
}

class SingleTest : public QRunnable
{
public:
    SingleTest(Test262Runner *runner, const TestData &data)
        : runner(runner), data(data)
    {
        command = runner->command;
    }
    void run();

    void runExternalTest();

    QString command;
    Test262Runner *runner;
    TestData data;
};

void SingleTest::run()
{
    if (!command.isEmpty()) {
        runExternalTest();
        return;
    }

    if (data.runInSloppyMode) {
        bool ok = ::executeTest(data.content);
        if (data.negative)
            ok = !ok;

        data.sloppyResult = ok ? TestCase::Passes : TestCase::Fails;
    } else {
        data.sloppyResult = TestCase::Skipped;
    }
    if (data.runInStrictMode) {
        const QString testCasePath = QFileInfo(runner->testDir + "/test/" + data.test).absoluteFilePath();
        QByteArray c = "'use strict';\n" + data.content;
        bool ok = ::executeTest(c, data.runAsModuleCode, testCasePath, data.harness);
        if (data.negative)
            ok = !ok;

        data.strictResult = ok ? TestCase::Passes : TestCase::Fails;
    } else {
        data.strictResult = TestCase::Skipped;
    }
    runner->addResult(data);
}

void SingleTest::runExternalTest()
{
    auto runTest = [this] (const char *header, TestCase::Result *result) {
        QTemporaryFile tempFile;
        tempFile.open();
        tempFile.write(header);
        tempFile.write(data.content);
        tempFile.close();

        QProcess process;
//        if (flags & Verbose)
//            process.setReadChannelMode(QProcess::ForwardedChannels);

        process.start(command, QStringList(tempFile.fileName()));
        if (!process.waitForFinished(-1) || process.error() == QProcess::FailedToStart) {
            qWarning() << "Could not execute" << command;
            *result = TestCase::Crashes;
        }
        if (process.exitStatus() != QProcess::NormalExit) {
            *result = TestCase::Crashes;
        }
        bool ok = (process.exitCode() == EXIT_SUCCESS);
        if (data.negative)
            ok = !ok;
        *result = ok ? TestCase::Passes : TestCase::Fails;
    };

    if (data.runInSloppyMode)
        runTest("", &data.sloppyResult);
    if (data.runInStrictMode)
        runTest("'use strict';\n", &data.strictResult);

    runner->addResult(data);
}

int Test262Runner::runSingleTest(TestCase testCase)
{
    TestData data = getTestData(testCase);
//    qDebug() << "starting test" << data.test;

    if (data.isExcluded || data.async)
        return 0;

    if (threadPool) {
        SingleTest *test = new SingleTest(this, data);
        threadPool->start(test);
        return 0;
    }
    SingleTest test(this, data);
    test.run();
    return 0;
}

void Test262Runner::addResult(TestCase result)
{
    {
        QMutexLocker locker(&mutex);
        Q_ASSERT(result.strictExpectation == testCases[result.test].strictExpectation);
        Q_ASSERT(result.sloppyExpectation == testCases[result.test].sloppyExpectation);
        testCases[result.test] = result;
    }

    if (!(flags & Verbose))
        return;

    QString test = result.test;
    if (result.strictResult == TestCase::Skipped) {
        ;
    } else if (result.strictResult == TestCase::Crashes) {
        qDebug() << "FAIL:" << test << "crashed in strict mode!";
    } else if ((result.strictResult == TestCase::Fails) && (result.strictExpectation == TestCase::Fails)) {
        qDebug() << "PASS:" << test << "failed in strict mode as expected";
    } else if ((result.strictResult == TestCase::Passes) == (result.strictExpectation == TestCase::Passes)) {
        qDebug() << "PASS:" << test << "passed in strict mode";
    } else if (!(result.strictExpectation == TestCase::Fails)) {
        qDebug() << "FAIL:" << test << "failed in strict mode";
    } else {
        qDebug() << "XPASS:" << test << "unexpectedly passed in strict mode";
    }

    if (result.sloppyResult == TestCase::Skipped) {
        ;
    } else if (result.sloppyResult == TestCase::Crashes) {
        qDebug() << "FAIL:" << test << "crashed in sloppy mode!";
    } else if ((result.sloppyResult == TestCase::Fails) && (result.sloppyExpectation == TestCase::Fails)) {
        qDebug() << "PASS:" << test << "failed in sloppy mode as expected";
    } else if ((result.sloppyResult == TestCase::Passes) == (result.sloppyExpectation == TestCase::Passes)) {
        qDebug() << "PASS:" << test << "passed in sloppy mode";
    } else if (!(result.sloppyExpectation == TestCase::Fails)) {
        qDebug() << "FAIL:" << test << "failed in sloppy mode";
    } else {
        qDebug() << "XPASS:" << test << "unexpectedly passed in sloppy mode";
    }
}

TestData Test262Runner::getTestData(const TestCase &testCase)
{
    QFile testFile(testDir + "/test/" + testCase.test);
    if (!testFile.open(QFile::ReadOnly)) {
        qWarning() << "wrong test file" << testCase.test;
        exit(1);
    }
    QByteArray content = testFile.readAll();
    content.replace(QByteArrayLiteral("\r\n"), "\n");

//    qDebug() << "parsing test file" << test;

    TestData data(testCase);
    parseYaml(content, &data);

    data.harness += harness("assert.js");
    data.harness += harness("sta.js");

    for (QByteArray inc : qAsConst(data.includes)) {
        inc = inc.trimmed();
        data.harness += harness(inc);
    }

    if (data.async)
        data.harness += harness("doneprintHandle.js");

    data.content = data.harness + content;

    return data;
}

struct YamlSection {
    YamlSection(const QByteArray &yaml, const char *sectionName);

    bool contains(const char *keyword) const;
    QList<QByteArray> keywords() const;

    QByteArray yaml;
    int start = -1;
    int length = 0;
    bool shortSection = false;
};

YamlSection::YamlSection(const QByteArray &yaml, const char *sectionName)
    : yaml(yaml)
{
    start = yaml.indexOf(sectionName);
    if (start < 0)
        return;
    start += static_cast<int>(strlen(sectionName));
    int end = yaml.indexOf('\n', start + 1);
    if (end < 0)
        end = yaml.length();

    int s = yaml.indexOf('[', start);
    if (s > 0 && s < end) {
        shortSection = true;
        start = s + 1;
        end = yaml.indexOf(']', s);
    } else {
        while (end < yaml.size() - 1 && yaml.at(end + 1) == ' ')
            end = yaml.indexOf('\n', end + 1);
    }
    length = end - start;
}

bool YamlSection::contains(const char *keyword) const
{
    if (start < 0)
        return false;
    int idx = yaml.indexOf(keyword, start);
    if (idx >= start && idx < start + length)
        return true;
    return false;
}

QList<QByteArray> YamlSection::keywords() const
{
    if (start < 0)
        return QList<QByteArray>();

    QByteArray content = yaml.mid(start, length);
    QList<QByteArray> keywords;
    if (shortSection) {
        keywords = content.split(',');
    } else {
        const QList<QByteArray> list = content.split('\n');
        for (const QByteArray &l : list) {
            int i = 0;
            while (i < l.size() && (l.at(i) == ' ' || l.at(i) == '-'))
                ++i;
            QByteArray entry = l.mid(i);
            if (!entry.isEmpty())
                keywords.append(entry);
        }
    }
//    qDebug() << "keywords:" << keywords;
    return keywords;
}


void Test262Runner::parseYaml(const QByteArray &content, TestData *data)
{
    int start = content.indexOf("/*---");
    if (start < 0)
        return;
    start += sizeof("/*---");

    int end = content.indexOf("---*/");
    if (end < 0)
        return;

    QByteArray yaml = content.mid(start, end - start);

    if (yaml.contains("negative:"))
        data->negative = true;

    YamlSection flags(yaml, "flags:");
    data->runInSloppyMode = !flags.contains("onlyStrict");
    data->runInStrictMode = !flags.contains("noStrict") && !flags.contains("raw");
    data->runAsModuleCode = flags.contains("module");
    data->async = flags.contains("async");

    if (data->runAsModuleCode) {
        data->runInStrictMode = true;
        data->runInSloppyMode = false;
    }

    YamlSection includes(yaml, "includes:");
    data->includes = includes.keywords();

    YamlSection features = YamlSection(yaml, "features:");

    const char **f = excludedFeatures;
    while (*f) {
        if (features.contains(*f)) {
            data->isExcluded = true;
            break;
        }
        ++f;
    }

//    qDebug() << "Yaml:\n" << yaml;
}

QByteArray Test262Runner::harness(const QByteArray &name)
{
    if (harnessFiles.contains(name))
        return harnessFiles.value(name);

    QFile h(testDir + QLatin1String("/harness/") + name);
    if (!h.open(QFile::ReadOnly)) {
        qWarning() << "Illegal test harness file" << name;
        exit(1);
    }

    QByteArray content = h.readAll();
    harnessFiles.insert(name, content);
    return content;
}
