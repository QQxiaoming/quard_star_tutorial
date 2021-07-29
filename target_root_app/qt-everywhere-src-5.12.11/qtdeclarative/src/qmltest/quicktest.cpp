/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include "quicktest.h"
#include "quicktestresult_p.h"
#include <QtTest/qtestsystem.h>
#include "qtestoptions_p.h"
#include <QtQml/qqml.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/qquickview.h>
#include <QtQml/qjsvalue.h>
#include <QtQml/qjsengine.h>
#include <QtQml/qqmlpropertymap.h>
#include <QtGui/qopengl.h>
#include <QtCore/qurl.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qdiriterator.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>
#include <QtGui/qtextdocument.h>
#include <stdio.h>
#include <QtGui/QGuiApplication>
#include <QtCore/QTranslator>
#include <QtTest/QSignalSpy>
#include <QtQml/QQmlFileSelector>

#include <private/qqmlcomponent_p.h>

#ifdef QT_QMLTEST_WITH_WIDGETS
#include <QtWidgets/QApplication>
#endif

QT_BEGIN_NAMESPACE

class QTestRootObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool windowShown READ windowShown NOTIFY windowShownChanged)
    Q_PROPERTY(bool hasTestCase READ hasTestCase WRITE setHasTestCase NOTIFY hasTestCaseChanged)
    Q_PROPERTY(QObject *defined READ defined)
public:
    QTestRootObject(QObject *parent = nullptr)
        : QObject(parent), hasQuit(false), m_windowShown(false), m_hasTestCase(false)  {
        m_defined = new QQmlPropertyMap(this);
#if defined(QT_OPENGL_ES_2_ANGLE)
        m_defined->insert(QLatin1String("QT_OPENGL_ES_2_ANGLE"), QVariant(true));
#endif
    }

    static QTestRootObject *instance() {
        static QPointer<QTestRootObject> object = new QTestRootObject;
        if (!object) {
            // QTestRootObject was deleted when previous test ended, create a new one
            object = new QTestRootObject;
        }
        return object;
    }

    bool hasQuit:1;
    bool hasTestCase() const { return m_hasTestCase; }
    void setHasTestCase(bool value) { m_hasTestCase = value; emit hasTestCaseChanged(); }

    bool windowShown() const { return m_windowShown; }
    void setWindowShown(bool value) { m_windowShown = value; emit windowShownChanged(); }
    QQmlPropertyMap *defined() const { return m_defined; }

    void init() { setWindowShown(false); setHasTestCase(false); hasQuit = false; }

Q_SIGNALS:
    void windowShownChanged();
    void hasTestCaseChanged();

private Q_SLOTS:
    void quit() { hasQuit = true; }

private:
    bool m_windowShown : 1;
    bool m_hasTestCase :1;
    QQmlPropertyMap *m_defined;
};

static QObject *testRootObject(QQmlEngine *engine, QJSEngine *jsEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(jsEngine);
    return QTestRootObject::instance();
}

static inline QString stripQuotes(const QString &s)
{
    if (s.length() >= 2 && s.startsWith(QLatin1Char('"')) && s.endsWith(QLatin1Char('"')))
        return s.mid(1, s.length() - 2);
    else
        return s;
}

void handleCompileErrors(const QFileInfo &fi, QQuickView *view)
{
    // Error compiling the test - flag failure in the log and continue.
    const QList<QQmlError> errors = view->errors();
    QuickTestResult results;
    results.setTestCaseName(fi.baseName());
    results.startLogging();
    results.setFunctionName(QLatin1String("compile"));
    // Verbose warning output of all messages and relevant parameters
    QString message;
    QTextStream str(&message);
    str << "\n  " << QDir::toNativeSeparators(fi.absoluteFilePath()) << " produced "
        << errors.size() << " error(s):\n";
    for (const QQmlError &e : errors) {
        str << "    ";
        if (e.url().isLocalFile()) {
            str << QDir::toNativeSeparators(e.url().toLocalFile());
        } else {
            str << e.url().toString();
        }
        if (e.line() > 0)
            str << ':' << e.line() << ',' << e.column();
        str << ": " << e.description() << '\n';
    }
    str << "  Working directory: " << QDir::toNativeSeparators(QDir::current().absolutePath()) << '\n';
    if (QQmlEngine *engine = view->engine()) {
        str << "  View: " << view->metaObject()->className() << ", import paths:\n";
        const auto importPaths = engine->importPathList();
        for (const QString &i : importPaths)
            str << "    '" << QDir::toNativeSeparators(i) << "'\n";
        const QStringList pluginPaths = engine->pluginPathList();
        str << "  Plugin paths:\n";
        for (const QString &p : pluginPaths)
            str << "    '" << QDir::toNativeSeparators(p) << "'\n";
    }
    qWarning("%s", qPrintable(message));
    // Fail with error 0.
    results.fail(errors.at(0).description(),
                 errors.at(0).url(), errors.at(0).line());
    results.finishTestData();
    results.finishTestDataCleanup();
    results.finishTestFunction();
    results.setFunctionName(QString());
    results.stopLogging();
}

bool qWaitForSignal(QObject *obj, const char* signal, int timeout = 5000)
{
    QSignalSpy spy(obj, signal);
    QElapsedTimer timer;
    timer.start();

    while (!spy.size()) {
        int remaining = timeout - int(timer.elapsed());
        if (remaining <= 0)
            break;
        QCoreApplication::processEvents(QEventLoop::AllEvents, remaining);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QTest::qSleep(10);
    }

    return spy.size();
}

void maybeInvokeSetupMethod(QObject *setupObject, const char *member, QGenericArgument val0 = QGenericArgument(nullptr))
{
    // It's OK if it doesn't exist: since we have more than one callback that
    // can be called, it makes sense if the user only implements one of them.
    // We do this the long way rather than just calling the static
    // QMetaObject::invokeMethod(), because that will issue a warning if the
    // function doesn't exist, which we don't want.
    const QMetaObject *setupMetaObject = setupObject->metaObject();
    const int methodIndex = setupMetaObject->indexOfMethod(member);
    if (methodIndex != -1) {
        const QMetaMethod method = setupMetaObject->method(methodIndex);
        method.invoke(setupObject, val0);
    }
}

using namespace QV4::CompiledData;

class TestCaseCollector
{
public:
    typedef QList<QString> TestCaseList;

    TestCaseCollector(const QFileInfo &fileInfo, QQmlEngine *engine)
    {
        QString path = fileInfo.absoluteFilePath();
        if (path.startsWith(QLatin1String(":/")))
            path.prepend(QLatin1String("qrc"));

        QQmlComponent component(engine, path);
        m_errors += component.errors();

        if (component.isReady()) {
            QQmlRefPointer<CompilationUnit> rootCompilationUnit = QQmlComponentPrivate::get(&component)->compilationUnit;
            TestCaseEnumerationResult result = enumerateTestCases(rootCompilationUnit.data());
            m_testCases = result.testCases + result.finalizedPartialTestCases();
            m_errors += result.errors;
        }
    }

    TestCaseList testCases() const { return m_testCases; }
    QList<QQmlError> errors() const { return m_errors; }

private:
    TestCaseList m_testCases;
    QList<QQmlError> m_errors;

    struct TestCaseEnumerationResult
    {
        TestCaseList testCases;
        QList<QQmlError> errors;

        // Partially constructed test cases
        bool isTestCase = false;
        TestCaseList testFunctions;
        QString testCaseName;

        TestCaseList finalizedPartialTestCases() const
        {
            TestCaseList result;
            for (const QString &function : testFunctions)
                result << QString(QStringLiteral("%1::%2")).arg(testCaseName).arg(function);
            return result;
        }

        TestCaseEnumerationResult &operator<<(const TestCaseEnumerationResult &other)
        {
            testCases += other.testCases + other.finalizedPartialTestCases();
            errors += other.errors;
            return *this;
        }
    };

    TestCaseEnumerationResult enumerateTestCases(CompilationUnit *compilationUnit, const Object *object = nullptr)
    {
        QQmlType testCaseType;
        for (quint32 i = 0, count = compilationUnit->importCount(); i < count; ++i) {
            const Import *import = compilationUnit->importAt(i);
            if (compilationUnit->stringAt(import->uriIndex) != QLatin1Literal("QtTest"))
                continue;

            QString testCaseTypeName(QStringLiteral("TestCase"));
            QString typeQualifier = compilationUnit->stringAt(import->qualifierIndex);
            if (!typeQualifier.isEmpty())
                testCaseTypeName = typeQualifier % QLatin1Char('.') % testCaseTypeName;

            testCaseType = compilationUnit->typeNameCache->query(testCaseTypeName).type;
            if (testCaseType.isValid())
                break;
        }

        TestCaseEnumerationResult result;

        if (!object) // Start at root of compilation unit if not enumerating a specific child
            object = compilationUnit->objectAt(0);

        if (CompilationUnit *superTypeUnit = compilationUnit->resolvedTypes.value(object->inheritedTypeNameIndex)->compilationUnit.data()) {
            // We have a non-C++ super type, which could indicate we're a subtype of a TestCase
            if (testCaseType.isValid() && superTypeUnit->url() == testCaseType.sourceUrl())
                result.isTestCase = true;
            else
                result = enumerateTestCases(superTypeUnit);

            if (result.isTestCase) {
                // Look for override of name in this type
                for (auto binding = object->bindingsBegin(); binding != object->bindingsEnd(); ++binding) {
                    if (compilationUnit->stringAt(binding->propertyNameIndex) == QLatin1Literal("name")) {
                        if (binding->type == QV4::CompiledData::Binding::Type_String) {
                            result.testCaseName = compilationUnit->stringAt(binding->stringIndex);
                        } else {
                            QQmlError error;
                            error.setUrl(compilationUnit->url());
                            error.setLine(binding->location.line);
                            error.setColumn(binding->location.column);
                            error.setDescription(QStringLiteral("the 'name' property of a TestCase must be a literal string"));
                            result.errors << error;
                        }
                        break;
                    }
                }

                // Look for additional functions in this type
                auto functionsEnd = compilationUnit->objectFunctionsEnd(object);
                for (auto function = compilationUnit->objectFunctionsBegin(object); function != functionsEnd; ++function) {
                    QString functionName = compilationUnit->stringAt(function->nameIndex);
                    if (!(functionName.startsWith(QLatin1Literal("test_")) || functionName.startsWith(QLatin1Literal("benchmark_"))))
                        continue;

                    if (functionName.endsWith(QLatin1Literal("_data")))
                        continue;

                    result.testFunctions << functionName;
                }
            }
        }

        for (auto binding = object->bindingsBegin(); binding != object->bindingsEnd(); ++binding) {
            if (binding->type == QV4::CompiledData::Binding::Type_Object) {
                const Object *child = compilationUnit->objectAt(binding->value.objectIndex);
                result << enumerateTestCases(compilationUnit, child);
            }
        }

        return result;
    }
};

int quick_test_main(int argc, char **argv, const char *name, const char *sourceDir)
{
    return quick_test_main_with_setup(argc, argv, name, sourceDir, nullptr);
}

int quick_test_main_with_setup(int argc, char **argv, const char *name, const char *sourceDir, QObject *setup)
{
    // Peek at arguments to check for '-widgets' argument
#ifdef QT_QMLTEST_WITH_WIDGETS
    bool withWidgets = false;
    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "-widgets") == 0) {
            withWidgets = true;
            break;
        }
    }
#endif

    QCoreApplication *app = nullptr;
    if (!QCoreApplication::instance()) {
#ifdef QT_QMLTEST_WITH_WIDGETS
        if (withWidgets)
            app = new QApplication(argc, argv);
        else
#endif
        {
            app = new QGuiApplication(argc, argv);
        }
    }

    if (setup)
        maybeInvokeSetupMethod(setup, "applicationAvailable()");

    // Look for QML-specific command-line options.
    //      -import dir         Specify an import directory.
    //      -plugins dir        Specify a directory where to search for plugins.
    //      -input dir          Specify the input directory for test cases.
    //      -translation file   Specify the translation file.
    //      -file-selector      Specify a file selector
    QStringList imports;
    QStringList pluginPaths;
    QString testPath;
    QString translationFile;
    QStringList fileSelectors;
    int index = 1;
    QScopedArrayPointer<char *> testArgV(new char *[argc + 1]);
    testArgV[0] = argv[0];
    int testArgC = 1;
    while (index < argc) {
        if (strcmp(argv[index], "-import") == 0 && (index + 1) < argc) {
            imports += stripQuotes(QString::fromLocal8Bit(argv[index + 1]));
            index += 2;
        } else if (strcmp(argv[index], "-plugins") == 0 && (index + 1) < argc) {
            pluginPaths += stripQuotes(QString::fromLocal8Bit(argv[index + 1]));
            index += 2;
        } else if (strcmp(argv[index], "-input") == 0 && (index + 1) < argc) {
            testPath = stripQuotes(QString::fromLocal8Bit(argv[index + 1]));
            index += 2;
        } else if (strcmp(argv[index], "-opengl") == 0) {
            ++index;
#ifdef QT_QMLTEST_WITH_WIDGETS
        } else if (strcmp(argv[index], "-widgets") == 0) {
            withWidgets = true;
            ++index;
#endif
        } else if (strcmp(argv[index], "-translation") == 0 && (index + 1) < argc) {
            translationFile = stripQuotes(QString::fromLocal8Bit(argv[index + 1]));
            index += 2;
        } else if (strcmp(argv[index], "-file-selector") == 0 && (index + 1) < argc) {
            fileSelectors += stripQuotes(QString::fromLocal8Bit(argv[index + 1]));
            index += 2;
        } else {
            testArgV[testArgC++] = argv[index++];
        }
    }
    testArgV[testArgC] = 0;

    // Setting currentAppname and currentTestObjectName (via setProgramName) are needed
    // for the code coverage analysis. Must be done before parseArgs is called.
    QuickTestResult::setCurrentAppname(argv[0]);
    QuickTestResult::setProgramName(name);

    QuickTestResult::parseArgs(testArgC, testArgV.data());

#if QT_CONFIG(translation)
    QTranslator translator;
    if (!translationFile.isEmpty()) {
        if (translator.load(translationFile)) {
            app->installTranslator(&translator);
        } else {
            qWarning("Could not load the translation file '%s'.", qPrintable(translationFile));
        }
    }
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_WINRT)
    if (testPath.isEmpty())
        testPath = QLatin1String(":/");
#endif

    // Determine where to look for the test data.
    if (testPath.isEmpty() && sourceDir) {
        const QString s = QString::fromLocal8Bit(sourceDir);
        if (QFile::exists(s))
            testPath = s;
    }
    if (testPath.isEmpty()) {
        QDir current = QDir::current();
#ifdef Q_OS_WIN
        // Skip release/debug subfolders
        if (!current.dirName().compare(QLatin1String("Release"), Qt::CaseInsensitive)
            || !current.dirName().compare(QLatin1String("Debug"), Qt::CaseInsensitive))
            current.cdUp();
#endif // Q_OS_WIN
        testPath = current.absolutePath();
    }
    QStringList files;

    const QFileInfo testPathInfo(testPath);
    if (testPathInfo.isFile()) {
        if (!testPath.endsWith(QLatin1String(".qml"))) {
            qWarning("'%s' does not have the suffix '.qml'.", qPrintable(testPath));
            return 1;
        }
        files << testPath;
    } else if (testPathInfo.isDir()) {
        // Scan the test data directory recursively, looking for "tst_*.qml" files.
        const QStringList filters(QStringLiteral("tst_*.qml"));
        QDirIterator iter(testPathInfo.absoluteFilePath(), filters, QDir::Files,
                          QDirIterator::Subdirectories |
                          QDirIterator::FollowSymlinks);
        while (iter.hasNext())
            files += iter.next();
        files.sort();
        if (files.isEmpty()) {
            qWarning("The directory '%s' does not contain any test files matching '%s'",
                     qPrintable(testPath), qPrintable(filters.front()));
            return 1;
        }
    } else {
        qWarning("'%s' does not exist under '%s'.",
                 qPrintable(testPath), qPrintable(QDir::currentPath()));
        return 1;
    }

    qputenv("QT_QTESTLIB_RUNNING", "1");

    // Register the test object
    qmlRegisterSingletonType<QTestRootObject>("Qt.test.qtestroot", 1, 0, "QTestRootObject", testRootObject);

    QSet<QString> commandLineTestFunctions = QTest::testFunctions.toSet();
    const bool filteringTestFunctions = !commandLineTestFunctions.isEmpty();

    // Scan through all of the "tst_*.qml" files and run each of them
    // in turn with a separate QQuickView (for test isolation).
    for (const QString &file : qAsConst(files)) {
        const QFileInfo fi(file);
        if (!fi.exists())
            continue;

        QQmlEngine engine;
        for (const QString &path : qAsConst(imports))
            engine.addImportPath(path);
        for (const QString &path : qAsConst(pluginPaths))
            engine.addPluginPath(path);

        if (!fileSelectors.isEmpty()) {
            QQmlFileSelector* const qmlFileSelector = new QQmlFileSelector(&engine, &engine);
            qmlFileSelector->setExtraSelectors(fileSelectors);
        }

        // Do this down here so that import paths, plugin paths, file selectors, etc. are available
        // in case the user needs access to them. Do it _before_ the TestCaseCollector parses the
        // QML files though, because it attempts to import modules, which might not be available
        // if qmlRegisterType()/QQmlEngine::addImportPath() are called in qmlEngineAvailable().
        if (setup)
            maybeInvokeSetupMethod(setup, "qmlEngineAvailable(QQmlEngine*)", Q_ARG(QQmlEngine*, &engine));

        TestCaseCollector testCaseCollector(fi, &engine);
        if (!testCaseCollector.errors().isEmpty()) {
            for (const QQmlError &error : testCaseCollector.errors())
                qWarning() << error;
            exit(1);
        }

        TestCaseCollector::TestCaseList availableTestFunctions = testCaseCollector.testCases();
        if (QTest::printAvailableFunctions) {
            for (const QString &function : availableTestFunctions)
                qDebug("%s()", qPrintable(function));
            continue;
        }

        const QSet<QString> availableTestSet = availableTestFunctions.toSet();
        if (filteringTestFunctions && !availableTestSet.intersects(commandLineTestFunctions))
            continue;
        commandLineTestFunctions.subtract(availableTestSet);

        QQuickView view(&engine, nullptr);
        view.setFlags(Qt::Window | Qt::WindowSystemMenuHint
                         | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint
                         | Qt::WindowCloseButtonHint);
        QEventLoop eventLoop;
        QObject::connect(view.engine(), SIGNAL(quit()),
                         QTestRootObject::instance(), SLOT(quit()));
        QObject::connect(view.engine(), SIGNAL(quit()),
                         &eventLoop, SLOT(quit()));
        view.rootContext()->setContextProperty
            (QLatin1String("qtest"), QTestRootObject::instance()); // Deprecated. Use QTestRootObject from Qt.test.qtestroot instead

        view.setObjectName(fi.baseName());
        view.setTitle(view.objectName());
        QTestRootObject::instance()->init();
        QString path = fi.absoluteFilePath();
        if (path.startsWith(QLatin1String(":/")))
            view.setSource(QUrl(QLatin1String("qrc:") + path.midRef(1)));
        else
            view.setSource(QUrl::fromLocalFile(path));

        while (view.status() == QQuickView::Loading)
            QTest::qWait(10);
        if (view.status() == QQuickView::Error) {
            handleCompileErrors(fi, &view);
            continue;
        }
        if (!QTestRootObject::instance()->hasQuit) {
            // If the test already quit, then it was performed
            // synchronously during setSource().  Otherwise it is
            // an asynchronous test and we need to show the window
            // and wait for the first frame to be rendered
            // and then wait for quit indication.
            view.setFramePosition(QPoint(50, 50));
            if (view.size().isEmpty()) { // Avoid hangs with empty windows.
                view.resize(200, 200);
            }
            view.show();
            if (!QTest::qWaitForWindowExposed(&view)) {
                qWarning().nospace()
                    << "Test '" << QDir::toNativeSeparators(path) << "' window not exposed after show().";
            }
            view.requestActivate();
            if (!QTest::qWaitForWindowActive(&view)) {
                qWarning().nospace()
                    << "Test '" << QDir::toNativeSeparators(path) << "' window not active after requestActivate().";
            }
            if (view.isExposed()) {
                // Defer property update until event loop has started
                QTimer::singleShot(0, []() {
                    QTestRootObject::instance()->setWindowShown(true);
                });
            } else {
                qWarning().nospace()
                    << "Test '" << QDir::toNativeSeparators(path) << "' window was never exposed! "
                    << "If the test case was expecting windowShown, it will hang.";
            }
            if (!QTestRootObject::instance()->hasQuit && QTestRootObject::instance()->hasTestCase())
                eventLoop.exec();
        }
    }

    if (setup)
        maybeInvokeSetupMethod(setup, "cleanupTestCase()");

    // Flush the current logging stream.
    QuickTestResult::setProgramName(nullptr);
    delete app;

    // Check that all test functions passed on the command line were found
    if (!commandLineTestFunctions.isEmpty()) {
        qWarning() << "Could not find the following test functions:";
        for (const QString &functionName : qAsConst(commandLineTestFunctions))
            qWarning("    %s()", qUtf8Printable(functionName));
        return commandLineTestFunctions.count();
    }

    // Return the number of failures as the exit code.
    return QuickTestResult::exitCode();
}

QT_END_NAMESPACE

#include "quicktest.moc"
