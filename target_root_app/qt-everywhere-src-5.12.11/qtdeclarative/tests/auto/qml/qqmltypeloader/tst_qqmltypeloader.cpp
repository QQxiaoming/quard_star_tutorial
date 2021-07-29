/****************************************************************************
**
** Copyright (C) 2016 Canonical Limited and/or its subsidiary(-ies).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <QtTest/QtTest>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlnetworkaccessmanagerfactory.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#if QT_CONFIG(process)
#include <QtCore/qprocess.h>
#endif
#include <QtQml/private/qqmlengine_p.h>
#include <QtQml/private/qqmltypeloader_p.h>
#include "../../shared/testhttpserver.h"
#include "../../shared/util.h"

class tst_QQMLTypeLoader : public QQmlDataTest
{
    Q_OBJECT

private slots:
    void testLoadComplete();
    void loadComponentSynchronously();
    void trimCache();
    void trimCache2();
    void trimCache3();
    void keepSingleton();
    void keepRegistrations();
    void intercept();
    void redirect();
    void qmlSingletonWithinModule();
    void multiSingletonModule();
    void implicitComponentModule();
    void qrcRootPathUrl();
};

void tst_QQMLTypeLoader::testLoadComplete()
{
    QQuickView *window = new QQuickView();
    window->engine()->addImportPath(QT_TESTCASE_BUILDDIR);
    qDebug() << window->engine()->importPathList();
    window->setGeometry(0,0,240,320);
    window->setSource(testFileUrl("test_load_complete.qml"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QObject *rootObject = window->rootObject();
    QTRY_VERIFY(rootObject != nullptr);
    QTRY_COMPARE(rootObject->property("created").toInt(), 2);
    QTRY_COMPARE(rootObject->property("loaded").toInt(), 2);
    delete window;
}

void tst_QQMLTypeLoader::loadComponentSynchronously()
{
    QQmlEngine engine;
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(
                             QLatin1String(".*nonprotocol::1:1: QtObject is not a type.*")));
    QQmlComponent component(&engine, testFileUrl("load_synchronous.qml"));
    QScopedPointer<QObject> o(component.create());
    QVERIFY(o);
}

void tst_QQMLTypeLoader::trimCache()
{
    QQmlEngine engine;
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(&engine)->typeLoader;
    for (int i = 0; i < 256; ++i) {
        QUrl url = testFileUrl("trim_cache.qml");
        url.setQuery(QString::number(i));

        QQmlTypeData *data = loader.getType(url).take();
        // Run an event loop to receive the callback that release()es.
        QTRY_COMPARE(data->count(), 2);

        // keep references to some of them so that they aren't trimmed. References to either the
        // QQmlTypeData or its compiledData() should prevent the trimming.
        if (i % 10 == 0) {
            // keep ref on data, don't add ref on data->compiledData()
        } else if (i % 5 == 0) {
            data->compilationUnit()->addref();
            data->release();
        } else {
            data->release();
        }
    }

    for (int i = 0; i < 256; ++i) {
        QUrl url = testFileUrl("trim_cache.qml");
        url.setQuery(QString::number(i));
        if (i % 5 == 0)
            QVERIFY(loader.isTypeLoaded(url));
        else if (i < 128)
            QVERIFY(!loader.isTypeLoaded(url));
        // The cache is free to keep the others.
    }
}

void tst_QQMLTypeLoader::trimCache2()
{
    QScopedPointer<QQuickView> window(new QQuickView());
    window->setSource(testFileUrl("trim_cache2.qml"));
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(window->engine())->typeLoader;
    // in theory if gc has already run this could be false
    // QCOMPARE(loader.isTypeLoaded(testFileUrl("MyComponent2.qml")), true);
    window->engine()->collectGarbage();
    QTest::qWait(1);    // force event loop
    window->engine()->trimComponentCache();
    QCOMPARE(loader.isTypeLoaded(testFileUrl("MyComponent2.qml")), false);
}

// test trimming the cache of an item that contains sub-items created via incubation
void tst_QQMLTypeLoader::trimCache3()
{
    QScopedPointer<QQuickView> window(new QQuickView());
    window->setSource(testFileUrl("trim_cache3.qml"));
    QQmlTypeLoader &loader = QQmlEnginePrivate::get(window->engine())->typeLoader;
    QCOMPARE(loader.isTypeLoaded(testFileUrl("ComponentWithIncubator.qml")), true);

    QQmlProperty::write(window->rootObject(), "source", QString());

    // handle our deleteLater and cleanup
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
    window->engine()->collectGarbage();

    window->engine()->trimComponentCache();

    QCOMPARE(loader.isTypeLoaded(testFileUrl("ComponentWithIncubator.qml")), false);
}

static void checkSingleton(const QString &dataDirectory)
{
    QQmlEngine engine;
    engine.addImportPath(dataDirectory);
    QQmlComponent component(&engine);
    component.setData("import ClusterDemo 1.0\n"
                      "import QtQuick 2.6\n"
                      "import \"..\"\n"
                      "Item { property int t: ValueSource.something }",
                      QUrl::fromLocalFile(dataDirectory + "/abc/Xyz.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> o(component.create());
    QVERIFY(o.data());
    QCOMPARE(o->property("t").toInt(), 10);
}

void tst_QQMLTypeLoader::keepSingleton()
{
    qmlRegisterSingletonType(testFileUrl("ValueSource.qml"), "ClusterDemo", 1, 0, "ValueSource");
    checkSingleton(dataDirectory());
    QQmlMetaType::freeUnusedTypesAndCaches();
    checkSingleton(dataDirectory());
}

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject(QObject *parent = nullptr) : QObject(parent) {}
};

QML_DECLARE_TYPE(TestObject)

static void verifyTypes(bool shouldHaveTestObject, bool shouldHaveFast)
{
    bool hasTestObject = false;
    bool hasFast = false;
    for (const QQmlType &type : QQmlMetaType::qmlAllTypes()) {
        if (type.elementName() == QLatin1String("Fast"))
            hasFast = true;
        else if (type.elementName() == QLatin1String("TestObject"))
            hasTestObject = true;
    }
    QCOMPARE(hasTestObject, shouldHaveTestObject);
    QCOMPARE(hasFast, shouldHaveFast);
}

void tst_QQMLTypeLoader::keepRegistrations()
{
    verifyTypes(false, false);
    qmlRegisterType<TestObject>("Test", 1, 0, "TestObject");
    verifyTypes(true, false);

    {
        QQmlEngine engine;
        engine.addImportPath(dataDirectory());
        QQmlComponent component(&engine);
        component.setData("import Fast 1.0\nFast {}", QUrl());
        QVERIFY2(component.errorString().isEmpty(), component.errorString().toUtf8().constData());
        QCOMPARE(component.status(), QQmlComponent::Ready);
        QScopedPointer<QObject> o(component.create());
        QVERIFY(o.data());
        verifyTypes(true, true);
    }

    verifyTypes(true, false); // Fast is gone again, even though an event was still scheduled.
    QQmlMetaType::freeUnusedTypesAndCaches();
    verifyTypes(true, false); // qmlRegisterType creates an undeletable type.
}

class NetworkReply : public QNetworkReply
{
public:
    NetworkReply()
    {
        open(QIODevice::ReadOnly);
    }

    void setData(const QByteArray &data)
    {
        if (isFinished())
            return;
        m_buffer = data;
        emit readyRead();
        setFinished(true);
        emit finished();
    }

    void fail()
    {
        if (isFinished())
            return;
        m_buffer.clear();
        setError(ContentNotFoundError, "content not found");
        emit error(ContentNotFoundError);
        setFinished(true);
        emit finished();
    }

    qint64 bytesAvailable() const override
    {
        return m_buffer.size();
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        if (m_buffer.length() < maxlen)
            maxlen = m_buffer.length();
        std::memcpy(data, m_buffer.data(), maxlen);
        m_buffer.remove(0, maxlen);
        return maxlen;
    }

    void abort() override
    {
        if (isFinished())
            return;
        m_buffer.clear();
        setFinished(true);
        emit finished();
    }

private:
    QByteArray m_buffer;
};

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:

    NetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent)
    {
    }

    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                 QIODevice *outgoingData) override
    {
        QUrl url = request.url();
        QString scheme = url.scheme();
        if (op != GetOperation || !scheme.endsWith("+debug"))
            return QNetworkAccessManager::createRequest(op, request, outgoingData);

        scheme.chop(sizeof("+debug") - 1);
        url.setScheme(scheme);

        NetworkReply *reply = new NetworkReply;
        QString filename = QQmlFile::urlToLocalFileOrQrc(url);
        QTimer::singleShot(10, reply, [this, reply, filename]() {
            if (filename.isEmpty()) {
                reply->fail();
            } else {
                QFile file(filename);
                if (file.open(QIODevice::ReadOnly)) {
                    emit loaded(filename);
                    reply->setData(transformQmldir(filename, file.readAll()));
                } else
                    reply->fail();
            }
        });
        return reply;
    }

    QByteArray transformQmldir(const QString &filename, const QByteArray &content)
    {
        if (!filename.endsWith("/qmldir"))
            return content;

        // Make qmldir plugin paths absolute, so that we don't try to load them over the network
        QByteArray result;
        QByteArray path = filename.toUtf8();
        path.chop(sizeof("qmldir") - 1);
        for (QByteArray line : content.split('\n')) {
            if (line.isEmpty())
                continue;
            QList<QByteArray> segments = line.split(' ');
            if (segments.startsWith("plugin")) {
                if (segments.length() == 2) {
                    segments.append(path);
                } else if (segments.length() == 3) {
                    if (!segments[2].startsWith('/'))
                        segments[2] = path + segments[2];
                } else {
                    // Invalid plugin declaration. Ignore
                }
                result.append(segments.join(' '));
            } else {
                result.append(line);
            }
            result.append('\n');
        }
        return result;
    }

signals:
    void loaded(const QString &filename);
};

class NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QStringList loadedFiles;

    QNetworkAccessManager *create(QObject *parent) override
    {
        NetworkAccessManager *manager = new NetworkAccessManager(parent);
        QObject::connect(manager, &NetworkAccessManager::loaded, [this](const QString &filename) {
            loadedFiles.append(filename);
        });
        return manager;
    }
};

class UrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    QUrl intercept(const QUrl &path, DataType type) override
    {
        Q_UNUSED(type);
        if (!QQmlFile::isLocalFile(path))
            return path;

        QUrl result = path;
        QString scheme = result.scheme();
        if (!scheme.endsWith("+debug"))
            result.setScheme(scheme + "+debug");
        return result;
    }
};

void tst_QQMLTypeLoader::intercept()
{
    qmlClearTypeRegistrations();

    QQmlEngine engine;
    engine.addImportPath(dataDirectory());
    engine.addImportPath(QT_TESTCASE_BUILDDIR);

    UrlInterceptor interceptor;
    NetworkAccessManagerFactory factory;

    engine.setUrlInterceptor(&interceptor);
    engine.setNetworkAccessManagerFactory(&factory);

    QQmlComponent component(&engine, testFileUrl("test_intercept.qml"));

    QVERIFY(component.status() != QQmlComponent::Ready);
    QTRY_VERIFY2(component.status() == QQmlComponent::Ready,
                 component.errorString().toUtf8().constData());

    QScopedPointer<QObject> o(component.create());
    QVERIFY(o.data());

    QTRY_COMPARE(o->property("created").toInt(), 2);
    QTRY_COMPARE(o->property("loaded").toInt(), 2);

    QVERIFY(factory.loadedFiles.length() >= 6);
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/test_intercept.qml"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/Intercept.qml"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/Fast/qmldir"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/Fast/Fast.qml"));
    QVERIFY(factory.loadedFiles.contains(dataDirectory() + "/GenericView.qml"));
    QVERIFY(factory.loadedFiles.contains(QLatin1String(QT_TESTCASE_BUILDDIR) + "/Slow/qmldir"));
}

void tst_QQMLTypeLoader::redirect()
{
    TestHTTPServer server;
    QVERIFY2(server.listen(), qPrintable(server.errorString()));
    QVERIFY(server.serveDirectory(dataDirectory()));
    server.addRedirect("Base.qml", server.urlString("/redirected/Redirected.qml"));

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(server.urlString("/Load.qml"), QQmlComponent::Asynchronous);
    QTRY_VERIFY2(component.isReady(), qPrintable(component.errorString()));

    QObject *object = component.create();
    QTRY_COMPARE(object->property("xy").toInt(), 323232);
}

void tst_QQMLTypeLoader::qmlSingletonWithinModule()
{
    qmlClearTypeRegistrations();
    QQmlEngine engine;
    qmlRegisterSingletonType(testFileUrl("Singleton.qml"), "modulewithsingleton", 1, 0, "Singleton");

    QQmlComponent component(&engine, testFileUrl("singletonuser.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
    QVERIFY(obj->property("ok").toBool());
}

static void checkCleanCacheLoad(const QString &testCase)
{
#if QT_CONFIG(process)
    const char *skipKey = "QT_TST_QQMLTYPELOADER_SKIP_MISMATCH";
    if (qEnvironmentVariableIsSet(skipKey))
        return;
    for (int i = 0; i < 5; ++i) {
        QProcess child;
        child.setProgram(QCoreApplication::applicationFilePath());
        child.setArguments(QStringList(testCase));
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QLatin1String("QT_LOGGING_RULES"), QLatin1String("qt.qml.diskcache.debug=true"));
        env.insert(QLatin1String(skipKey), QLatin1String("1"));
        child.setProcessEnvironment(env);
        child.start();
        QVERIFY(child.waitForFinished());
        QCOMPARE(child.exitCode(), 0);
        QVERIFY(!child.readAllStandardOutput().contains("Checksum mismatch for cached version"));
        QVERIFY(!child.readAllStandardError().contains("Checksum mismatch for cached version"));
    }
#else
    Q_UNUSED(testCase);
#endif
}

void tst_QQMLTypeLoader::multiSingletonModule()
{
    qmlClearTypeRegistrations();
    QQmlEngine engine;
    engine.addImportPath(testFile("imports"));

    qmlRegisterSingletonType(testFileUrl("CppRegisteredSingleton1.qml"), "cppsingletonmodule",
                             1, 0, "CppRegisteredSingleton1");
    qmlRegisterSingletonType(testFileUrl("CppRegisteredSingleton2.qml"), "cppsingletonmodule",
                             1, 0, "CppRegisteredSingleton2");

    QQmlComponent component(&engine, testFileUrl("multisingletonuser.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
    QVERIFY(obj->property("ok").toBool());

    checkCleanCacheLoad(QLatin1String("multiSingletonModule"));
}

void tst_QQMLTypeLoader::implicitComponentModule()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("implicitcomponent.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());

    checkCleanCacheLoad(QLatin1String("implicitComponentModule"));
}

void tst_QQMLTypeLoader::qrcRootPathUrl()
{
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("qrcRootPath.qml"));
    QCOMPARE(component.status(), QQmlComponent::Ready);
}

QTEST_MAIN(tst_QQMLTypeLoader)

#include "tst_qqmltypeloader.moc"
