/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "../util.h"
#include <QtCore/qbuffer.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebenginecookiestore.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebenginesettings.h>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QtWebEngineWidgets/qwebenginedownloaditem.h>

class tst_QWebEngineProfile : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void privateProfile();
    void testProfile();
    void clearDataFromCache();
    void disableCache();
    void urlSchemeHandlers();
    void urlSchemeHandlerFailRequest();
    void urlSchemeHandlerFailOnRead();
    void urlSchemeHandlerStreaming();
    void urlSchemeHandlerInstallation();
    void customUserAgent();
    void httpAcceptLanguage();
    void downloadItem();
    void changePersistentPath();
    void initiator();
    void badDeleteOrder();
    void qtbug_72299(); // this should be the last test
};

void tst_QWebEngineProfile::initTestCase()
{
    QWebEngineUrlScheme foo("foo");
    QWebEngineUrlScheme stream("stream");
    QWebEngineUrlScheme letterto("letterto");
    QWebEngineUrlScheme aviancarrier("aviancarrier");
    foo.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    stream.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
    stream.setDefaultPort(8080);
    letterto.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    aviancarrier.setSyntax(QWebEngineUrlScheme::Syntax::Path);
    QWebEngineUrlScheme::registerScheme(foo);
    QWebEngineUrlScheme::registerScheme(stream);
    QWebEngineUrlScheme::registerScheme(letterto);
    QWebEngineUrlScheme::registerScheme(aviancarrier);
}

void tst_QWebEngineProfile::init()
{
    //make sure defualt global profile is 'default' across all the tests
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    QVERIFY(!profile->isOffTheRecord());
    QCOMPARE(profile->storageName(), QStringLiteral("Default"));
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile->cachePath(),  QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
             + QStringLiteral("/QtWebEngine/Default"));
    QCOMPARE(profile->persistentStoragePath(),  QStandardPaths::writableLocation(QStandardPaths::DataLocation)
             + QStringLiteral("/QtWebEngine/Default"));
}

void tst_QWebEngineProfile::cleanup()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setCachePath(QString());
    profile->setPersistentStoragePath(QString());
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    profile->removeAllUrlSchemeHandlers();
}

void tst_QWebEngineProfile::privateProfile()
{
    QWebEngineProfile otrProfile;
    QVERIFY(otrProfile.isOffTheRecord());
    QCOMPARE(otrProfile.httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    QCOMPARE(otrProfile.persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
    QCOMPARE(otrProfile.cachePath(), QString());
    QCOMPARE(otrProfile.persistentStoragePath(), QString());
    // TBD: setters do not really work
    otrProfile.setCachePath(QStringLiteral("/home/foo/bar"));
    QCOMPARE(otrProfile.cachePath(), QString());
    otrProfile.setPersistentStoragePath(QStringLiteral("/home/foo/bar"));
    QCOMPARE(otrProfile.persistentStoragePath(), QString());
    otrProfile.setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    QCOMPARE(otrProfile.httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    otrProfile.setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QCOMPARE(otrProfile.persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
}


void tst_QWebEngineProfile::testProfile()
{
    QWebEngineProfile profile(QStringLiteral("Test"));
    QVERIFY(!profile.isOffTheRecord());
    QCOMPARE(profile.storageName(), QStringLiteral("Test"));
    QCOMPARE(profile.httpCacheType(), QWebEngineProfile::DiskHttpCache);
    QCOMPARE(profile.persistentCookiesPolicy(), QWebEngineProfile::AllowPersistentCookies);
    QCOMPARE(profile.cachePath(),  QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
             + QStringLiteral("/QtWebEngine/Test"));
    QCOMPARE(profile.persistentStoragePath(),  QStandardPaths::writableLocation(QStandardPaths::DataLocation)
             + QStringLiteral("/QtWebEngine/Test"));
}

void tst_QWebEngineProfile::clearDataFromCache()
{
    QWebEnginePage page;

    QDir cacheDir("./tst_QWebEngineProfile_cacheDir");
    cacheDir.makeAbsolute();
    if (cacheDir.exists())
        cacheDir.removeRecursively();
    cacheDir.mkpath(cacheDir.path());

    QWebEngineProfile *profile = page.profile();
    profile->setCachePath(cacheDir.path());
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    cacheDir.refresh();
    QVERIFY(cacheDir.entryList().contains("Cache"));
    cacheDir.cd("./Cache");
    int filesBeforeClear = cacheDir.entryList().count();

    QFileSystemWatcher fileSystemWatcher;
    fileSystemWatcher.addPath(cacheDir.path());
    QSignalSpy directoryChangedSpy(&fileSystemWatcher, SIGNAL(directoryChanged(const QString &)));

    // It deletes most of the files, but not all of them.
    profile->clearHttpCache();
    QTest::qWait(1000);
    QTRY_VERIFY(directoryChangedSpy.count() > 0);

    cacheDir.refresh();
    QVERIFY(filesBeforeClear > cacheDir.entryList().count());

    cacheDir.removeRecursively();
}

void tst_QWebEngineProfile::disableCache()
{
    QWebEnginePage page;
    QDir cacheDir("./tst_QWebEngineProfile_cacheDir");
    if (cacheDir.exists())
        cacheDir.removeRecursively();
    cacheDir.mkpath(cacheDir.path());

    QWebEngineProfile *profile = page.profile();
    profile->setCachePath(cacheDir.path());
    QVERIFY(!cacheDir.entryList().contains("Cache"));

    profile->setHttpCacheType(QWebEngineProfile::NoCache);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    cacheDir.refresh();
    QVERIFY(!cacheDir.entryList().contains("Cache"));

    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(1).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    cacheDir.refresh();
    QVERIFY(cacheDir.entryList().contains("Cache"));

    cacheDir.removeRecursively();
}

class RedirectingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job)
    {
        job->redirect(QUrl(QStringLiteral("data:text/plain;charset=utf-8,")
                           + job->requestUrl().fileName()));
    }
};

class ReplyingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    ReplyingUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }
    ~ReplyingUrlSchemeHandler()
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job)
    {
        QBuffer *buffer = new QBuffer(job);
        buffer->setData(job->requestUrl().toString().toUtf8());
        m_buffers.append(buffer);
        job->reply("text/plain;charset=utf-8", buffer);
    }

    QList<QPointer<QBuffer>> m_buffers;
};

class StreamingIODevice : public QIODevice {
    Q_OBJECT
public:
    StreamingIODevice(QObject *parent) : QIODevice(parent), m_bytesRead(0), m_bytesAvailable(0)
    {
        setOpenMode(QIODevice::ReadOnly);
        m_timer.start(100, this);
    }
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override
    {
        QMutexLocker lock(&m_mutex);
        return m_bytesAvailable;
    }
    bool atEnd() const override
    {
        QMutexLocker lock(&m_mutex);
        return (m_data.size() >= 1000 && m_bytesRead >= 1000);
    }
protected:
    void timerEvent(QTimerEvent *) override
    {
        QMutexLocker lock(&m_mutex);
        m_bytesAvailable += 200;
        m_data.append(200, 'c');
        emit readyRead();
        if (m_data.size() >= 1000) {
            m_timer.stop();
            emit readChannelFinished();
        }
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        QMutexLocker lock(&m_mutex);
        qint64 len = qMin(qint64(m_bytesAvailable), maxlen);
        if (len) {
            memcpy(data, m_data.constData() + m_bytesRead, len);
            m_bytesAvailable -= len;
            m_bytesRead += len;
        } else if (m_data.size() > 0)
            return -1;

        return len;
    }
    qint64 writeData(const char *, qint64) override
    {
        return 0;
    }

private:
    mutable QMutex m_mutex{QMutex::Recursive};
    QByteArray m_data;
    QBasicTimer m_timer;
    int m_bytesRead;
    int m_bytesAvailable;
};

class StreamingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    StreamingUrlSchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {
    }

    void requestStarted(QWebEngineUrlRequestJob *job)
    {
        job->reply("text/plain;charset=utf-8", new StreamingIODevice(job));
    }
};

static bool loadSync(QWebEngineView *view, const QUrl &url, int timeout = 5000)
{
    // Ripped off QTRY_VERIFY.
    QSignalSpy loadFinishedSpy(view, SIGNAL(loadFinished(bool)));
    view->load(url);
    if (loadFinishedSpy.isEmpty())
        QTest::qWait(0);
    for (int i = 0; i < timeout; i += 50) {
        if (!loadFinishedSpy.isEmpty())
            return true;
        QTest::qWait(50);
    }
    return false;
}

void tst_QWebEngineProfile::urlSchemeHandlers()
{
    RedirectingUrlSchemeHandler lettertoHandler;
    QWebEngineProfile profile(QStringLiteral("urlSchemeHandlers"));
    profile.installUrlSchemeHandler("letterto", &lettertoHandler);
    QWebEngineView view;
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QString emailAddress = QStringLiteral("egon@olsen-banden.dk");
    QVERIFY(loadSync(&view, QUrl(QStringLiteral("letterto:") + emailAddress)));
    QCOMPARE(toPlainTextSync(view.page()), emailAddress);

    // Install a gopher handler after the view has been fully initialized.
    ReplyingUrlSchemeHandler gopherHandler;
    profile.installUrlSchemeHandler("gopher", &gopherHandler);
    QUrl url = QUrl(QStringLiteral("gopher://olsen-banden.dk/benny"));
    QVERIFY(loadSync(&view, url));
    QCOMPARE(toPlainTextSync(view.page()), url.toString());

    // Remove the letterto scheme, and check whether it is not handled anymore.
    profile.removeUrlScheme("letterto");
    emailAddress = QStringLiteral("kjeld@olsen-banden.dk");
    QVERIFY(loadSync(&view, QUrl(QStringLiteral("letterto:") + emailAddress)));
    QVERIFY(toPlainTextSync(view.page()) != emailAddress);

    // Check if gopher is still working after removing letterto.
    url = QUrl(QStringLiteral("gopher://olsen-banden.dk/yvonne"));
    QVERIFY(loadSync(&view, url));
    QCOMPARE(toPlainTextSync(view.page()), url.toString());

    // Does removeAll work?
    profile.removeAllUrlSchemeHandlers();
    url = QUrl(QStringLiteral("gopher://olsen-banden.dk/harry"));
    QVERIFY(loadSync(&view, url));
    QVERIFY(toPlainTextSync(view.page()) != url.toString());

    // Install a handler that is owned by the view. Make sure this doesn't crash on shutdown.
    profile.installUrlSchemeHandler("aviancarrier", new ReplyingUrlSchemeHandler(&view));
    url = QUrl(QStringLiteral("aviancarrier:inspector.mortensen@politistyrke.dk"));
    QVERIFY(loadSync(&view, url));
    QCOMPARE(toPlainTextSync(view.page()), url.toString());

    // Check that all buffers got deleted
    QCOMPARE(gopherHandler.m_buffers.count(), 2);
    for (int i = 0; i < gopherHandler.m_buffers.count(); ++i)
        QVERIFY(gopherHandler.m_buffers.at(i).isNull());
}

class FailingUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }
};

class FailingIODevice : public QIODevice
{
public:
    FailingIODevice(QWebEngineUrlRequestJob *job) : m_job(job)
    {
    }

    qint64 readData(char *, qint64) Q_DECL_OVERRIDE
    {
        m_job->fail(QWebEngineUrlRequestJob::RequestFailed);
        return -1;
    }
    qint64 writeData(const char *, qint64) Q_DECL_OVERRIDE
    {
        m_job->fail(QWebEngineUrlRequestJob::RequestFailed);
        return -1;
    }
    void close() Q_DECL_OVERRIDE
    {
        QIODevice::close();
        deleteLater();
    }

private:
    QWebEngineUrlRequestJob *m_job;
};

class FailOnReadUrlSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        job->reply(QByteArrayLiteral("text/plain"), new FailingIODevice(job));
    }
};


void tst_QWebEngineProfile::urlSchemeHandlerFailRequest()
{
    FailingUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("foo", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("foo://bar")));
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(toPlainTextSync(view.page()), QString());
}

void tst_QWebEngineProfile::urlSchemeHandlerFailOnRead()
{
    FailOnReadUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("foo", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("foo://bar")));
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(toPlainTextSync(view.page()), QString());
}

void tst_QWebEngineProfile::urlSchemeHandlerStreaming()
{
    StreamingUrlSchemeHandler handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("stream", &handler);
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(&view, SIGNAL(loadFinished(bool)));
    view.setPage(new QWebEnginePage(&profile, &view));
    view.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    view.load(QUrl(QStringLiteral("stream://whatever")));
    QVERIFY(loadFinishedSpy.wait());
    QByteArray result;
    result.append(1000, 'c');
    QCOMPARE(toPlainTextSync(view.page()), QString::fromLatin1(result));
}

void tst_QWebEngineProfile::urlSchemeHandlerInstallation()
{
    FailingUrlSchemeHandler handler;
    QWebEngineProfile profile;

    // Builtin schemes that *cannot* be overridden.
    for (auto scheme : { "about", "blob", "data", "javascript", "qrc", "https", "http", "file",
                         "ftp", "wss", "ws", "filesystem", "FileSystem" }) {
        QTest::ignoreMessage(
                QtWarningMsg,
                QRegularExpression("Cannot install a URL scheme handler overriding internal scheme.*"));
        profile.installUrlSchemeHandler(scheme, &handler);
        QCOMPARE(profile.urlSchemeHandler(scheme), nullptr);
    }

    // Builtin schemes that *can* be overridden.
    for (auto scheme : { "gopher", "GOPHER" }) {
        profile.installUrlSchemeHandler(scheme, &handler);
        QCOMPARE(profile.urlSchemeHandler(scheme), &handler);
        profile.removeUrlScheme(scheme);
    }

    // Other schemes should be registered with QWebEngineUrlScheme first, but
    // handler installation still succeeds to preserve backwards compatibility.
    QTest::ignoreMessage(
            QtWarningMsg,
            QRegularExpression("Please register the custom scheme.*"));
    profile.installUrlSchemeHandler("tst", &handler);
    QCOMPARE(profile.urlSchemeHandler("tst"), &handler);

    // Existing handler cannot be overridden.
    FailingUrlSchemeHandler handler2;
    QTest::ignoreMessage(
            QtWarningMsg,
            QRegularExpression("URL scheme handler already installed.*"));
    profile.installUrlSchemeHandler("tst", &handler2);
    QCOMPARE(profile.urlSchemeHandler("tst"), &handler);
    profile.removeUrlScheme("tst");
}

void tst_QWebEngineProfile::customUserAgent()
{
    QString defaultUserAgent = QWebEngineProfile::defaultProfile()->httpUserAgent();
    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><body>Hello world!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    // First test the user-agent is default
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), defaultUserAgent);

    const QString testUserAgent = QStringLiteral("tst_QWebEngineProfile 1.0");
    QWebEngineProfile testProfile;
    testProfile.setHttpUserAgent(testUserAgent);

    // Test a new profile with custom user-agent works
    QWebEnginePage page2(&testProfile);
    QSignalSpy loadFinishedSpy2(&page2, SIGNAL(loadFinished(bool)));
    page2.setHtml(QStringLiteral("<html><body>Hello again!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy2.count(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page2, QStringLiteral("navigator.userAgent")).toString(), testUserAgent);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), defaultUserAgent);

    // Test an existing page and profile with custom user-agent works
    QWebEngineProfile::defaultProfile()->setHttpUserAgent(testUserAgent);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), testUserAgent);
}

void tst_QWebEngineProfile::httpAcceptLanguage()
{
    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><body>Hello world!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    QStringList defaultLanguages = evaluateJavaScriptSync(&page, QStringLiteral("navigator.languages")).toStringList();

    const QString testLang = QStringLiteral("xx-YY");
    QWebEngineProfile testProfile;
    testProfile.setHttpAcceptLanguage(testLang);

    // Test a completely new profile
    QWebEnginePage page2(&testProfile);
    QSignalSpy loadFinishedSpy2(&page2, SIGNAL(loadFinished(bool)));
    page2.setHtml(QStringLiteral("<html><body>Hello again!</body></html>"));
    QTRY_COMPARE(loadFinishedSpy2.count(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page2, QStringLiteral("navigator.languages")).toStringList(), QStringList(testLang));
    // Test the old one wasn't affected
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.languages")).toStringList(), defaultLanguages);

    // Test changing an existing page and profile
    QWebEngineProfile::defaultProfile()->setHttpAcceptLanguage(testLang);
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.languages")).toStringList(), QStringList(testLang));
}

void tst_QWebEngineProfile::downloadItem()
{
    qRegisterMetaType<QWebEngineDownloadItem *>();
    QWebEngineProfile testProfile;
    QWebEnginePage page(&testProfile);
    QSignalSpy downloadSpy(&testProfile, SIGNAL(downloadRequested(QWebEngineDownloadItem *)));
    connect(&testProfile, &QWebEngineProfile::downloadRequested, this, [=] (QWebEngineDownloadItem *item) { item->accept(); });
    page.load(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()));
    QTRY_COMPARE(downloadSpy.count(), 1);
}

void tst_QWebEngineProfile::changePersistentPath()
{
    QWebEngineProfile testProfile(QStringLiteral("Test"));
    const QString oldPath = testProfile.persistentStoragePath();
    QVERIFY(oldPath.endsWith(QStringLiteral("Test")));

    // Make sure the profile has been used and the url-request-context-getter instantiated:
    QWebEnginePage page(&testProfile);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("http://qt-project.org"));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    // Test we do not crash (QTBUG-55322):
    testProfile.setPersistentStoragePath(oldPath + QLatin1Char('2'));
    const QString newPath = testProfile.persistentStoragePath();
    QVERIFY(newPath.endsWith(QStringLiteral("Test2")));
}

class InitiatorSpy : public QWebEngineUrlSchemeHandler
{
public:
    QUrl initiator;
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        initiator = job->initiator();
        job->fail(QWebEngineUrlRequestJob::RequestDenied);
    }
};

void tst_QWebEngineProfile::initiator()
{
    InitiatorSpy handler;
    QWebEngineProfile profile;
    profile.installUrlSchemeHandler("foo", &handler);
    QWebEnginePage page(&profile);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

    // about:blank has a unique origin, so initiator should be QUrl("null")
    evaluateJavaScriptSync(&page, "window.location = 'foo:bar'");
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(handler.initiator, QUrl("null"));

    page.setHtml("", QUrl("http://test:123/foo%20bar"));
    QVERIFY(loadFinishedSpy.wait());

    // baseUrl determines the origin, so QUrl("http://test:123")
    evaluateJavaScriptSync(&page, "window.location = 'foo:bar'");
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(handler.initiator, QUrl("http://test:123"));

    // Directly calling load/setUrl should have initiator QUrl(), meaning
    // browser-initiated, trusted.
    page.load(QUrl("foo:bar"));
    QVERIFY(loadFinishedSpy.wait());
    QCOMPARE(handler.initiator, QUrl());
}

void tst_QWebEngineProfile::badDeleteOrder()
{
    QWebEngineProfile *profile = new QWebEngineProfile();
    QWebEngineView *view = new QWebEngineView();
    view->resize(640, 480);
    view->show();
    QVERIFY(QTest::qWaitForWindowExposed(view));
    QWebEnginePage *page = new QWebEnginePage(profile, view);
    view->setPage(page);

    QSignalSpy spyLoadFinished(page, SIGNAL(loadFinished(bool)));
    page->setHtml(QStringLiteral("<html><body><h1>Badly handled page!</h1></body></html>"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);

    delete profile;
    delete view;
}

void tst_QWebEngineProfile::qtbug_72299()
{
    QWebEngineView view;
    QSignalSpy loadSpy(view.page(), SIGNAL(loadFinished(bool)));
    view.setUrl(QUrl("https://www.qt.io"));
    view.show();
    view.page()->profile()->clearHttpCache();
    view.page()->profile()->setHttpCacheType(QWebEngineProfile::NoCache);
    view.page()->profile()->cookieStore()->deleteAllCookies();
    view.page()->profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.count(), 1, 20000);
    QVERIFY(loadSpy.front().front().toBool());
}


QTEST_MAIN(tst_QWebEngineProfile)
#include "tst_qwebengineprofile.moc"
