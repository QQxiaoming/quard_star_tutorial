/*
    Copyright (C) 2016 The Qt Company Ltd.
    Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
    Copyright (C) 2010 Holger Hans Peter Freyther

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "../util.h"
#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <QByteArray>
#include <QClipboard>
#include <QDir>
#include <QGraphicsWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMimeDatabase>
#include <QNetworkProxy>
#include <QOpenGLWidget>
#include <QPaintEngine>
#include <QPushButton>
#include <QRegExp>
#include <QScreen>
#include <QStateMachine>
#include <QtGui/QClipboard>
#include <QtTest/QtTest>
#include <QTextCharFormat>
#if QT_CONFIG(webengine_webchannel)
#include <QWebChannel>
#endif
#include <httpserver.h>
#include <qnetworkcookiejar.h>
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qwebenginedownloaditem.h>
#include <qwebenginefullscreenrequest.h>
#include <qwebenginehistory.h>
#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginequotarequest.h>
#include <qwebengineregisterprotocolhandlerrequest.h>
#include <qwebenginescript.h>
#include <qwebenginescriptcollection.h>
#include <qwebenginesettings.h>
#include <qwebengineview.h>
#include <qimagewriter.h>

static void removeRecursive(const QString& dirname)
{
    QDir dir(dirname);
    QFileInfoList entries(dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot));
    for (int i = 0; i < entries.count(); ++i)
        if (entries[i].isDir())
            removeRecursive(entries[i].filePath());
        else
            dir.remove(entries[i].fileName());
    QDir().rmdir(dirname);
}

class tst_QWebEnginePage : public QObject
{
    Q_OBJECT

public:
    tst_QWebEnginePage();
    virtual ~tst_QWebEnginePage();

    bool eventFilter(QObject *watched, QEvent *event);

public Q_SLOTS:
    void init();
    void cleanup();
    void cleanupFiles();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void thirdPartyCookiePolicy();
    void comboBoxPopupPositionAfterMove();
    void comboBoxPopupPositionAfterChildMove();
    void contextMenuCopy();
    void contextMenuPopulatedOnce();
    void acceptNavigationRequest();
    void acceptNavigationRequestNavigationType();
    void geolocationRequestJS_data();
    void geolocationRequestJS();
    void loadFinished();
    void actionStates();
    void pasteImage();
    void popupFormSubmission();
    void userStyleSheet();
    void userStyleSheetFromLocalFileUrl();
    void userStyleSheetFromQrcUrl();
    void modified();
    void contextMenuCrash();
    void updatePositionDependentActionsCrash();
    void callbackSpyDeleted();
    void multipleProfilesAndLocalStorage();
    void cursorMovements();
    void textSelection();
    void textEditing();
    void backActionUpdate();
    void testOptionalJSObjects();
    void testLocalStorageVisibility();
    void testEnablePersistentStorage();
    void consoleOutput();
    void errorPageExtension();
    void errorPageExtensionLoadFinished();
    void userAgentNewlineStripping();
    void undoActionHaveCustomText();
    void renderWidgetHostViewNotShowTopLevel();
    void getUserMediaRequest_data();
    void getUserMediaRequest();
    void getUserMediaRequestDesktopAudio();
    void getUserMediaRequestSettingDisabled();
    void getUserMediaRequestDesktopVideoManyPages();
    void getUserMediaRequestDesktopVideoManyRequests();
    void savePage();

    void crashTests_LazyInitializationOfMainFrame();

    void screenshot_data();
    void screenshot();

#if defined(ENABLE_WEBGL) && ENABLE_WEBGL
    void acceleratedWebGLScreenshotWithoutView();
    void unacceleratedWebGLScreenshotWithoutView();
#endif

    void testJSPrompt();
    void testStopScheduledPageRefresh();
    void findText();
    void findTextResult();
    void findTextSuccessiveShouldCallAllCallbacks();
    void supportedContentType();
    // [Qt] tst_QWebEnginePage::infiniteLoopJS() timeouts with DFG JIT
    // https://bugs.webkit.org/show_bug.cgi?id=79040
    // void infiniteLoopJS();
    void deleteQWebEngineViewTwice();
    void renderOnRepaintRequestedShouldNotRecurse();
    void loadSignalsOrder_data();
    void loadSignalsOrder();
    void openWindowDefaultSize();
    void cssMediaTypeGlobalSetting();
    void cssMediaTypePageSetting();

#ifdef Q_OS_MAC
    void macCopyUnicodeToClipboard();
#endif

    void runJavaScript();
    void runJavaScriptDisabled();
    void fullScreenRequested();
    void quotaRequested();


    // Tests from tst_QWebEngineFrame
    void horizontalScrollAfterBack();
    void symmetricUrl();
    void progressSignal();
    void urlChange();
    void requestedUrlAfterSetAndLoadFailures();
    void asyncAndDelete();
    void earlyToHtml();
    void setHtml();
    void setHtmlWithImageResource();
    void setHtmlWithStylesheetResource();
    void setHtmlWithBaseURL();
    void setHtmlWithJSAlert();
    void inputFieldFocus();
    void hitTestContent();
    void baseUrl_data();
    void baseUrl();
    void scrollPosition();
    void scrollbarsOff();
    void evaluateWillCauseRepaint();
    void setContent_data();
    void setContent();
    void setCacheLoadControlAttribute();
    void setUrlWithPendingLoads();
    void setUrlToEmpty();
    void setUrlToInvalid();
    void setUrlToBadDomain();
    void setUrlToBadPort();
    void setUrlHistory();
    void setUrlUsingStateObject();
    void setUrlThenLoads_data();
    void setUrlThenLoads();
    void loadFinishedAfterNotFoundError();
    void loadInSignalHandlers_data();
    void loadInSignalHandlers();
    void loadFromQrc();
#if QT_CONFIG(webengine_webchannel)
    void restoreHistory();
#endif
    void toPlainTextLoadFinishedRace_data();
    void toPlainTextLoadFinishedRace();
    void setZoomFactor();
    void mouseButtonTranslation();
    void mouseMovementProperties();

    void viewSource();
    void viewSourceURL_data();
    void viewSourceURL();
    void viewSourceCredentials();
    void proxyConfigWithUnexpectedHostPortPair();
    void registerProtocolHandler_data();
    void registerProtocolHandler();
    void dataURLFragment();
    void devTools();
    void openLinkInDifferentProfile();
    void triggerActionWithoutMenu();
    void dynamicFrame();

    void editActionsWithExplicitFocus();
    void editActionsWithInitialFocus();
    void editActionsWithFocusOnIframe();

    void customUserAgentInNewTab();

private:
    static QPoint elementCenter(QWebEnginePage *page, const QString &id);

    QWebEngineView* m_view;
    QWebEnginePage* m_page;
    QWebEngineView* m_inputFieldsTestView;
    int m_inputFieldTestPaintCount;
    QString tmpDirPath() const
    {
        static QString tmpd = QDir::tempPath() + "/tst_qwebenginepage-"
            + QDateTime::currentDateTime().toString(QLatin1String("yyyyMMddhhmmss"));
        return tmpd;
    }
};

tst_QWebEnginePage::tst_QWebEnginePage()
{
}

tst_QWebEnginePage::~tst_QWebEnginePage()
{
}

bool tst_QWebEnginePage::eventFilter(QObject* watched, QEvent* event)
{
    // used on the inputFieldFocus test
    if (watched == m_inputFieldsTestView) {
        if (event->type() == QEvent::Paint)
            m_inputFieldTestPaintCount++;
    }
    return QObject::eventFilter(watched, event);
}

void tst_QWebEnginePage::init()
{
    m_view = new QWebEngineView();
    m_page = m_view->page();
    m_page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    m_view->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
}

void tst_QWebEnginePage::cleanup()
{
    delete m_view;
}

void tst_QWebEnginePage::cleanupFiles()
{
    removeRecursive(tmpDirPath());
}

void tst_QWebEnginePage::initTestCase()
{
    QLocale::setDefault(QLocale("en"));
    cleanupFiles(); // In case there are old files from previous runs

    // Set custom path since the CI doesn't install test plugins.
    // Stolen from qtlocation/tests/auto/positionplugintest.
    QString searchPath = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN
    searchPath += QStringLiteral("/..");
#endif
    searchPath += QStringLiteral("/../../../plugins");
    QCoreApplication::addLibraryPath(searchPath);
}

void tst_QWebEnginePage::cleanupTestCase()
{
    cleanupFiles(); // Be nice
}

class NavigationRequestOverride : public QWebEnginePage
{
public:
    NavigationRequestOverride(QWebEngineProfile* profile, bool initialValue) : QWebEnginePage(profile, nullptr), m_acceptNavigationRequest(initialValue) {}

    bool m_acceptNavigationRequest;
protected:
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
    {
        Q_UNUSED(url);
        Q_UNUSED(isMainFrame);
        if (type == QWebEnginePage::NavigationTypeFormSubmitted)
            return m_acceptNavigationRequest;
        return true;
    }
};

void tst_QWebEnginePage::acceptNavigationRequest()
{
    QWebEngineProfile profile;
    NavigationRequestOverride page(&profile, false);

    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml(QString("<html><body><form name='tstform' action='data:text/html,foo'method='get'>"
                            "<input type='text'><input type='submit'></form></body></html>"), QUrl());
    QTRY_COMPARE(loadSpy.count(), 1);

    evaluateJavaScriptSync(&page, "tstform.submit();");
    QTRY_COMPARE(loadSpy.count(), 2);

    // Content hasn't changed so the form submit will still work
    page.m_acceptNavigationRequest = true;
    evaluateJavaScriptSync(&page, "tstform.submit();");
    QTRY_COMPARE(loadSpy.count(), 3);

    // Now the content has changed
    QCOMPARE(toPlainTextSync(&page), QString("foo?"));
}

class JSTestPage : public QWebEnginePage
{
Q_OBJECT
public:
    JSTestPage(QObject* parent = 0)
    : QWebEnginePage(parent) {}

    virtual bool shouldInterruptJavaScript()
    {
        return true;
    }
public Q_SLOTS:
    void requestPermission(const QUrl &origin, QWebEnginePage::Feature feature)
    {
        if (m_allowGeolocation)
            setFeaturePermission(origin, feature, PermissionGrantedByUser);
        else
            setFeaturePermission(origin, feature, PermissionDeniedByUser);
    }

public:
    void setGeolocationPermission(bool allow)
    {
        m_allowGeolocation = allow;
    }

private:
    bool m_allowGeolocation;
};

// [Qt] tst_QWebEnginePage::infiniteLoopJS() timeouts with DFG JIT
// https://bugs.webkit.org/show_bug.cgi?id=79040
/*
void tst_QWebEnginePage::infiniteLoopJS()
{
    JSTestPage newPage(m_view);
    m_view->setPage(&newPage);
    m_view->setHtml(QString("<html><body>test</body></html>"), QUrl());
    m_view->page()->evaluateJavaScript("var run = true; var a = 1; while (run) { a++; }");
}
*/

void tst_QWebEnginePage::geolocationRequestJS_data()
{
    QTest::addColumn<bool>("allowed");
    QTest::addColumn<int>("errorCode");
    QTest::newRow("allowed") << true << 0;
    QTest::newRow("not allowed") << false << 1;
}

void tst_QWebEnginePage::geolocationRequestJS()
{
    QFETCH(bool, allowed);
    QFETCH(int, errorCode);
    QWebEngineView view;
    JSTestPage *newPage = new JSTestPage(&view);
    newPage->setView(&view);
    newPage->setGeolocationPermission(allowed);

    connect(newPage, SIGNAL(featurePermissionRequested(const QUrl&, QWebEnginePage::Feature)),
            newPage, SLOT(requestPermission(const QUrl&, QWebEnginePage::Feature)));

    QSignalSpy spyLoadFinished(newPage, SIGNAL(loadFinished(bool)));
    newPage->setHtml(QString("<html><body>test</body></html>"), QUrl("qrc://secure/origin"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);

    // Geolocation is only enabled for visible WebContents.
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    if (evaluateJavaScriptSync(newPage, QLatin1String("!navigator.geolocation")).toBool())
        W_QSKIP("Geolocation is not supported.", SkipSingle);

    evaluateJavaScriptSync(newPage, "var errorCode = 0; var done = false; function error(err) { errorCode = err.code; done = true; } function success(pos) { done = true; } navigator.geolocation.getCurrentPosition(success, error)");

    QTRY_VERIFY(evaluateJavaScriptSync(newPage, "done").toBool());
    int result = evaluateJavaScriptSync(newPage, "errorCode").toInt();
    if (result == 2)
        QEXPECT_FAIL("", "No location service available.", Continue);
    QCOMPARE(result, errorCode);
}

void tst_QWebEnginePage::loadFinished()
{
    QWebEnginePage page;
    QSignalSpy spyLoadStarted(&page, SIGNAL(loadStarted()));
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("data:text/html,<frameset cols=\"25%,75%\"><frame src=\"data:text/html,"
                                           "<head><meta http-equiv='refresh' content='1'></head>foo \">"
                                           "<frame src=\"data:text/html,bar\"></frameset>"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);

    QEXPECT_FAIL("", "Behavior change: Load signals are emitted only for the main frame in QtWebEngine.", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(spyLoadStarted.count() > 1, 100);
    QEXPECT_FAIL("", "Behavior change: Load signals are emitted only for the main frame in QtWebEngine.", Continue);
    QTRY_VERIFY_WITH_TIMEOUT(spyLoadFinished.count() > 1, 100);

    spyLoadFinished.clear();

    page.load(QUrl("data:text/html,<frameset cols=\"25%,75%\"><frame src=\"data:text/html,"
                                           "foo \"><frame src=\"data:text/html,bar\"></frameset>"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);
    QCOMPARE(spyLoadFinished.count(), 1);
}

void tst_QWebEnginePage::actionStates()
{
    m_page->load(QUrl("qrc:///resources/script.html"));

    QAction* reloadAction = m_page->action(QWebEnginePage::Reload);
    QAction* stopAction = m_page->action(QWebEnginePage::Stop);

    QTRY_VERIFY(reloadAction->isEnabled());
    QTRY_VERIFY(!stopAction->isEnabled());
}

static QImage imageWithoutAlpha(const QImage &image)
{
    QImage result = image;
    QPainter painter(&result);
    painter.fillRect(result.rect(), Qt::green);
    painter.drawImage(0, 0, image);
    return result;
}

void tst_QWebEnginePage::callbackSpyDeleted()
{
    QWebEnginePage *page = m_view->page();
    CallbackSpy<QVariant> spy;
    QString poorManSleep("function wait(ms){"
                         "  var start = new Date().getTime();"
                         "  var end = start;"
                         "  while (start + ms > end) {"
                             "end = new Date().getTime();"
                         "  }"
                         "}"
                        "wait(1000);");
    page->runJavaScript(poorManSleep, spy.ref());
    //spy deleted before callback
}

void tst_QWebEnginePage::pasteImage()
{
    // Pixels with an alpha value of 0 will have different RGB values after the
    // test -> clipboard -> webengine -> test roundtrip.
    // Clear the alpha channel to make QCOMPARE happy.
    const QImage origImage = imageWithoutAlpha(QImage(":/resources/image.png"));
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(origImage);
    QWebEnginePage *page = m_view->page();
    page->load(QUrl("qrc:///resources/pasteimage.html"));
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());
    page->triggerAction(QWebEnginePage::Paste);
    QTRY_VERIFY(evaluateJavaScriptSync(page,
            "window.myImageDataURL ? window.myImageDataURL.length : 0").toInt() > 0);
    QByteArray data = evaluateJavaScriptSync(page, "window.myImageDataURL").toByteArray();
    data.remove(0, data.indexOf(";base64,") + 8);
    QImage image = QImage::fromData(QByteArray::fromBase64(data), "PNG");
    if (image.format() == QImage::Format_RGB32)
        image.reinterpretAsFormat(QImage::Format_ARGB32);
    QCOMPARE(image, origImage);
}

class ConsolePage : public QWebEnginePage
{
public:
    ConsolePage(QObject* parent = 0) : QWebEnginePage(parent) {}

    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
    {
        levels.append(level);
        messages.append(message);
        lineNumbers.append(lineNumber);
        sourceIDs.append(sourceID);
    }

    QList<int> levels;
    QStringList messages;
    QList<int> lineNumbers;
    QStringList sourceIDs;
};

void tst_QWebEnginePage::consoleOutput()
{
    ConsolePage page;
    // We don't care about the result but want this to be synchronous
    evaluateJavaScriptSync(&page, "this is not valid JavaScript");
    QCOMPARE(page.messages.count(), 1);
    QCOMPARE(page.lineNumbers.at(0), 1);
}

class TestPage : public QWebEnginePage {
    Q_OBJECT
public:
    TestPage(QObject* parent = 0) : QWebEnginePage(parent)
    {
        connect(this, SIGNAL(geometryChangeRequested(QRect)), this, SLOT(slotGeometryChangeRequested(QRect)));
    }

    struct Navigation {
        NavigationType type;
        QUrl url;
        bool isMainFrame;
    };
    QList<Navigation> navigations;

    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
    {
        Navigation n;
        n.url = url;
        n.type = type;
        n.isMainFrame = isMainFrame;
        navigations.append(n);
        return true;
    }

    QList<TestPage*> createdWindows;
    virtual QWebEnginePage* createWindow(WebWindowType) {
        TestPage* page = new TestPage(this);
        createdWindows.append(page);
        emit windowCreated();
        return page;
    }

    QRect requestedGeometry;

signals:
    void windowCreated();

private Q_SLOTS:
    void slotGeometryChangeRequested(const QRect& geom) {
        requestedGeometry = geom;
    }
};

void tst_QWebEnginePage::acceptNavigationRequestNavigationType()
{

    TestPage page;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("qrc:///resources/script.html"));
    QTRY_COMPARE(loadSpy.count(), 1);
    QTRY_COMPARE(page.navigations.count(), 1);

    page.load(QUrl("qrc:///resources/content.html"));
    QTRY_COMPARE(loadSpy.count(), 2);
    QTRY_COMPARE(page.navigations.count(), 2);

    page.triggerAction(QWebEnginePage::Stop);
    QVERIFY(page.history()->canGoBack());
    page.triggerAction(QWebEnginePage::Back);

    QTRY_COMPARE(loadSpy.count(), 3);
    QTRY_COMPARE(page.navigations.count(), 3);

    page.triggerAction(QWebEnginePage::Reload);
    QTRY_COMPARE(loadSpy.count(), 4);
    QTRY_COMPARE(page.navigations.count(), 4);

    page.load(QUrl("qrc:///resources/reload.html"));
    QTRY_COMPARE(loadSpy.count(), 6);
    QTRY_COMPARE(page.navigations.count(), 6);

    QList<QWebEnginePage::NavigationType> expectedList;
    expectedList << QWebEnginePage::NavigationTypeTyped
        << QWebEnginePage::NavigationTypeTyped
        << QWebEnginePage::NavigationTypeBackForward
        << QWebEnginePage::NavigationTypeReload
        << QWebEnginePage::NavigationTypeTyped
        << QWebEnginePage::NavigationTypeOther;
    QVERIFY(expectedList.count() == page.navigations.count());
    for (int i = 0; i < expectedList.count(); ++i) {
        QCOMPARE(page.navigations[i].type, expectedList[i]);
    }
}

void tst_QWebEnginePage::popupFormSubmission()
{
    TestPage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));

    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    page.setHtml("<form name='form1' method=get action='' target='myNewWin'>"
                 "  <input type='hidden' name='foo' value='bar'>"
                 "</form>");
    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    page.runJavaScript("window.open('', 'myNewWin', 'width=500,height=300,toolbar=0');");
    evaluateJavaScriptSync(&page, "document.form1.submit();");
    QTRY_COMPARE(windowCreatedSpy.count(), 1);

    // The number of popup created should be one.
    QVERIFY(page.createdWindows.size() == 1);

    QTRY_VERIFY(!page.createdWindows[0]->url().isEmpty());
    QString url = page.createdWindows[0]->url().toString();

    // Check if the form submission was OK.
    QVERIFY(url.contains("?foo=bar"));
}

class TestNetworkManager : public QNetworkAccessManager
{
public:
    TestNetworkManager(QObject* parent) : QNetworkAccessManager(parent) {}

    QList<QUrl> requestedUrls;
    QList<QNetworkRequest> requests;

protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest &request, QIODevice* outgoingData) {
        requests.append(request);
        requestedUrls.append(request.url());
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }
};

void tst_QWebEnginePage::userStyleSheet()
{
#if !defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    QSKIP("QWEBENGINEPAGE_SETNETWORKACCESSMANAGER");
#else
    TestNetworkManager* networkManager = new TestNetworkManager(m_page);
    m_page->setNetworkAccessManager(networkManager);

    m_page->settings()->setUserStyleSheetUrl(QUrl("data:text/css;charset=utf-8;base64,"
            + QByteArray("p { background-image: url('http://does.not/exist.png');}").toBase64()));
    m_view->setHtml("<p>hello world</p>");
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(networkManager->requestedUrls.count() >= 1);
    QCOMPARE(networkManager->requestedUrls.at(0), QUrl("http://does.not/exist.png"));
#endif
}

void tst_QWebEnginePage::userStyleSheetFromLocalFileUrl()
{
#if !defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    QSKIP("QWEBENGINEPAGE_SETNETWORKACCESSMANAGER");
#else
    TestNetworkManager* networkManager = new TestNetworkManager(m_page);
    m_page->setNetworkAccessManager(networkManager);

    QUrl styleSheetUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebenginepage/resources/user.css"));
    m_page->settings()->setUserStyleSheetUrl(styleSheetUrl);
    m_view->setHtml("<p>hello world</p>");
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(networkManager->requestedUrls.count() >= 1);
    QCOMPARE(networkManager->requestedUrls.at(0), QUrl("http://does.not/exist.png"));
#endif
}

void tst_QWebEnginePage::userStyleSheetFromQrcUrl()
{
#if !defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    QSKIP("QWEBENGINEPAGE_SETNETWORKACCESSMANAGER");
#else
    TestNetworkManager* networkManager = new TestNetworkManager(m_page);
    m_page->setNetworkAccessManager(networkManager);

    m_page->settings()->setUserStyleSheetUrl(QUrl("qrc:///resources/user.css"));
    m_view->setHtml("<p>hello world</p>");
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(networkManager->requestedUrls.count() >= 1);
    QCOMPARE(networkManager->requestedUrls.at(0), QUrl("http://does.not/exist.png"));
#endif
}

void tst_QWebEnginePage::modified()
{
#if !defined(QWEBENGINEPAGE_ISMODIFIED)
    QSKIP("QWEBENGINEPAGE_ISMODIFIED");
#else
    m_page->setUrl(QUrl("data:text/html,<body>blub"));
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    m_page->setUrl(QUrl("data:text/html,<body id=foo contenteditable>blah"));
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(!m_page->isModified());

    m_page->runJavaScript("document.getElementById('foo').focus()");
    evaluateJavaScriptSync(m_page, "document.execCommand('InsertText', true, 'Test');");

    QVERIFY(m_page->isModified());

    evaluateJavaScriptSync(m_page, "document.execCommand('Undo', true);");

    QVERIFY(!m_page->isModified());

    evaluateJavaScriptSync(m_page, "document.execCommand('Redo', true);");

    QVERIFY(m_page->isModified());

    QVERIFY(m_page->history()->canGoBack());
    QVERIFY(!m_page->history()->canGoForward());
    QCOMPARE(m_page->history()->count(), 2);
    QVERIFY(m_page->history()->backItem().isValid());
    QVERIFY(!m_page->history()->forwardItem().isValid());

    m_page->history()->back();
    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(!m_page->history()->canGoBack());
    QVERIFY(m_page->history()->canGoForward());

    QVERIFY(!m_page->isModified());

    QCOMPARE(m_page->history()->currentItemIndex(), 0);

    m_page->history()->setMaximumItemCount(3);
    QCOMPARE(m_page->history()->maximumItemCount(), 3);

    QVariant variant("string test");
    m_page->history()->currentItem().setUserData(variant);
    QVERIFY(m_page->history()->currentItem().userData().toString() == "string test");

    m_page->setUrl(QUrl("data:text/html,<body>This is second page"));
    m_page->setUrl(QUrl("data:text/html,<body>This is third page"));
    QCOMPARE(m_page->history()->count(), 2);
    m_page->setUrl(QUrl("data:text/html,<body>This is fourth page"));
    QCOMPARE(m_page->history()->count(), 2);
    m_page->setUrl(QUrl("data:text/html,<body>This is fifth page"));
    QSignalSpy spy(m_page, &QWebEnginePage::saveFrameStateRequested);
    QVERIFY(spy.wait());
#endif
}

// https://bugs.webkit.org/show_bug.cgi?id=51331
void tst_QWebEnginePage::updatePositionDependentActionsCrash()
{
#if !defined(QWEBENGINEPAGE_UPDATEPOSITIONDEPENDENTACTIONS)
    QSKIP("QWEBENGINEPAGE_UPDATEPOSITIONDEPENDENTACTIONS");
#else
    QWebEngineView view;
    view.setHtml("<p>test");
    QPoint pos(0, 0);
    view.page()->updatePositionDependentActions(pos);
    QMenu* contextMenu = 0;
    const QList<QObject *> children = view.children();
    for (QObject *child : children) {
        contextMenu = qobject_cast<QMenu*>(child);
        if (contextMenu)
            break;
    }
    QVERIFY(!contextMenu);
#endif
}

// https://bugs.webkit.org/show_bug.cgi?id=20357
void tst_QWebEnginePage::contextMenuCrash()
{
#if !defined(QWEBENGINEPAGE_SWALLOWCONTEXTMENUEVENT)
    QSKIP("QWEBENGINEPAGE_SWALLOWCONTEXTMENUEVENT");
#else
    QWebEngineView view;
    view.setHtml("<p>test");
    QPoint pos(0, 0);
    QContextMenuEvent event(QContextMenuEvent::Mouse, pos);
    view.page()->swallowContextMenuEvent(&event);
    view.page()->updatePositionDependentActions(pos);
    QMenu* contextMenu = 0;
    const QList<QObject *> children = view.children();
    for (QObject *child : children) {
        contextMenu = qobject_cast<QMenu*>(child);
        if (contextMenu)
            break;
    }
    QVERIFY(contextMenu);
    delete contextMenu;
#endif
}

void tst_QWebEnginePage::multipleProfilesAndLocalStorage()
{
    QDir dir(tmpDirPath());
    bool success = dir.mkpath("path1");
    success = success && dir.mkdir("path2");
    QVERIFY(success);
    {
        QWebEngineProfile profile1("test1");
        QWebEngineProfile profile2("test2");
        profile1.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        profile2.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        profile1.setPersistentStoragePath(QDir::toNativeSeparators(tmpDirPath() + "/path1"));
        profile2.setPersistentStoragePath(QDir::toNativeSeparators(tmpDirPath() + "/path2"));

        QWebEnginePage page1(&profile1, nullptr);
        QWebEnginePage page2(&profile2, nullptr);
        QSignalSpy loadSpy1(&page1, SIGNAL(loadFinished(bool)));
        QSignalSpy loadSpy2(&page2, SIGNAL(loadFinished(bool)));

        page1.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        page2.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        QTRY_COMPARE(loadSpy1.count(), 1);
        QTRY_COMPARE(loadSpy2.count(), 1);

        evaluateJavaScriptSync(&page1, "localStorage.setItem('test', 'value1');");
        evaluateJavaScriptSync(&page2, "localStorage.setItem('test', 'value2');");

        page1.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        page2.setHtml(QString("<html><body> </body></html>"), QUrl("http://wwww.example.com"));
        QTRY_COMPARE(loadSpy1.count(), 2);
        QTRY_COMPARE(loadSpy2.count(), 2);

        QVariant s1 = evaluateJavaScriptSync(&page1, "localStorage.getItem('test')");
        QCOMPARE(s1.toString(), QString("value1"));
        QVariant s2 = evaluateJavaScriptSync(&page2, "localStorage.getItem('test')");
        QCOMPARE(s2.toString(), QString("value2"));
    }
    // Avoid deleting on-disk dbs before the underlying browser-context has been asynchronously deleted
    QTest::qWait(1000);
    QDir(tmpDirPath() + "/path1").removeRecursively();
    QDir(tmpDirPath() + "/path2").removeRecursively();
}

class CursorTrackedPage : public QWebEnginePage
{
public:

    CursorTrackedPage(QWidget *parent = 0): QWebEnginePage(parent) {
    }

    QString selectedText() {
        return evaluateJavaScriptSync(this, "window.getSelection().toString()").toString();
    }

    int selectionStartOffset() {
        return evaluateJavaScriptSync(this, "window.getSelection().getRangeAt(0).startOffset").toInt();
    }

    int selectionEndOffset() {
        return evaluateJavaScriptSync(this, "window.getSelection().getRangeAt(0).endOffset").toInt();
    }

    // true if start offset == end offset, i.e. no selected text
    int isSelectionCollapsed() {
        return evaluateJavaScriptSync(this, "window.getSelection().getRangeAt(0).collapsed").toBool();
    }
    bool hasSelection()
    {
        return !selectedText().isEmpty();
    }
};

void tst_QWebEnginePage::cursorMovements()
{
#if !defined(QWEBENGINEPAGE_SELECTEDTEXT)
    QSKIP("QWEBENGINEPAGE_SELECTEDTEXT");
#else
    QScopedPointer<CursorTrackedPage> page(new CursorTrackedPage);
    QString content("<html><body><p id=one>The quick brown fox</p><p id=two>jumps over the lazy dog</p><p>May the source<br/>be with you!</p></body></html>");
    page->setHtml(content);

    // this will select the first paragraph
    QString script = "var range = document.createRange(); " \
        "var node = document.getElementById(\"one\"); " \
        "range.selectNode(node); " \
        "getSelection().addRange(range);";
    evaluateJavaScriptSync(page.data(), script);
    QCOMPARE(page->selectedText().trimmed(), QString::fromLatin1("The quick brown fox"));

    QRegExp regExp(" style=\".*\"");
    regExp.setMinimal(true);
    QCOMPARE(page->selectedHtml().trimmed().replace(regExp, ""), QString::fromLatin1("<p id=\"one\">The quick brown fox</p>"));

    // these actions must exist
    QVERIFY(page->action(QWebEnginePage::MoveToNextChar) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToPreviousChar) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToNextWord) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToPreviousWord) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToNextLine) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToPreviousLine) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToStartOfLine) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToEndOfLine) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToStartOfBlock) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToEndOfBlock) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToStartOfDocument) != 0);
    QVERIFY(page->action(QWebEnginePage::MoveToEndOfDocument) != 0);

    // right now they are disabled because contentEditable is false
    QCOMPARE(page->action(QWebEnginePage::MoveToNextChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToPreviousChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToNextWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToPreviousWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToNextLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToPreviousLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToStartOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToEndOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToStartOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToEndOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToStartOfDocument)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::MoveToEndOfDocument)->isEnabled(), false);

    // make it editable before navigating the cursor
    page->setContentEditable(true);

    // here the actions are enabled after contentEditable is true
    QCOMPARE(page->action(QWebEnginePage::MoveToNextChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToPreviousChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToNextWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToPreviousWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToNextLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToPreviousLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToStartOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToEndOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToStartOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToEndOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToStartOfDocument)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::MoveToEndOfDocument)->isEnabled(), true);

    // cursor will be before the word "jump"
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be between 'j' and 'u' in the word "jump"
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 1);

    // cursor will be between 'u' and 'm' in the word "jump"
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 2);

    // cursor will be after the word "jump"
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 5);

    // cursor will be after the word "lazy"
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 19);

    // cursor will be between 'z' and 'y' in "lazy"
    page->triggerAction(QWebEnginePage::MoveToPreviousChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 18);

    // cursor will be between 'a' and 'z' in "lazy"
    page->triggerAction(QWebEnginePage::MoveToPreviousChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 17);

    // cursor will be before the word "lazy"
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 15);

    // cursor will be before the word "quick"
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 4);

    // cursor will be between 'p' and 's' in the word "jumps"
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 4);

    // cursor will be before the word "jumps"
    page->triggerAction(QWebEnginePage::MoveToStartOfLine);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be after the word "dog"
    page->triggerAction(QWebEnginePage::MoveToEndOfLine);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 23);

    // cursor will be between 'w' and 'n' in "brown"
    page->triggerAction(QWebEnginePage::MoveToStartOfLine);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 14);

    // cursor will be after the word "fox"
    page->triggerAction(QWebEnginePage::MoveToEndOfLine);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 19);

    // cursor will be before the word "The"
    page->triggerAction(QWebEnginePage::MoveToStartOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be after the word "you!"
    page->triggerAction(QWebEnginePage::MoveToEndOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);

    // cursor will be before the word "be"
    page->triggerAction(QWebEnginePage::MoveToStartOfBlock);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // cursor will be after the word "you!"
    page->triggerAction(QWebEnginePage::MoveToEndOfBlock);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);

    // try to move before the document start
    page->triggerAction(QWebEnginePage::MoveToStartOfDocument);
    page->triggerAction(QWebEnginePage::MoveToPreviousChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);
    page->triggerAction(QWebEnginePage::MoveToStartOfDocument);
    page->triggerAction(QWebEnginePage::MoveToPreviousWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // try to move past the document end
    page->triggerAction(QWebEnginePage::MoveToEndOfDocument);
    page->triggerAction(QWebEnginePage::MoveToNextChar);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);
    page->triggerAction(QWebEnginePage::MoveToEndOfDocument);
    page->triggerAction(QWebEnginePage::MoveToNextWord);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 12);
#endif
}

void tst_QWebEnginePage::textSelection()
{
    QWebEngineView view;
    CursorTrackedPage *page = new CursorTrackedPage(&view);
    QString content("<html><body><p id=one>The quick brown fox</p>" \
        "<p id=two>jumps over the lazy dog</p>" \
        "<p>May the source<br/>be with you!</p></body></html>");
    page->setView(&view);
    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    page->setHtml(content);
    QTRY_COMPARE(loadSpy.count(), 1);

    // these actions must exist
    QVERIFY(page->action(QWebEnginePage::SelectAll) != 0);
#if defined(QWEBENGINEPAGE_SELECTACTIONS)
    QVERIFY(page->action(QWebEnginePage::SelectNextChar) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectPreviousChar) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectNextWord) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectPreviousWord) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectNextLine) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectPreviousLine) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectStartOfLine) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectEndOfLine) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectStartOfBlock) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectEndOfBlock) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectStartOfDocument) != 0);
    QVERIFY(page->action(QWebEnginePage::SelectEndOfDocument) != 0);

    // right now they are disabled because contentEditable is false and
    // there isn't an existing selection to modify
    QCOMPARE(page->action(QWebEnginePage::SelectNextChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousChar)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectNextWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectNextLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfLine)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfBlock)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfDocument)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfDocument)->isEnabled(), false);
#endif

    // ..but SelectAll is disabled because the page has no focus due to disabled FocusOnNavigationEnabled.
    QCOMPARE(page->action(QWebEnginePage::SelectAll)->isEnabled(), false);

    // Verify hasSelection returns false since there is no selection yet...
    QCOMPARE(page->hasSelection(), false);

    // this will select the first paragraph
    QString selectScript = "var range = document.createRange(); " \
        "var node = document.getElementById(\"one\"); " \
        "range.selectNode(node); " \
        "getSelection().addRange(range);";
    evaluateJavaScriptSync(page, selectScript);
    QCOMPARE(page->selectedText().trimmed(), QString::fromLatin1("The quick brown fox"));
#if defined(QWEBENGINEPAGE_SELECTEDHTML)
    QRegExp regExp(" style=\".*\"");
    regExp.setMinimal(true);
    QCOMPARE(page->selectedHtml().trimmed().replace(regExp, ""), QString::fromLatin1("<p id=\"one\">The quick brown fox</p>"));
#endif
    // Make sure hasSelection returns true, since there is selected text now...
    QCOMPARE(page->hasSelection(), true);

#if defined(QWEBENGINEPAGE_SELECTACTIONS)
    // here the actions are enabled after a selection has been created
    QCOMPARE(page->action(QWebEnginePage::SelectNextChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectNextWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectNextLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfDocument)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfDocument)->isEnabled(), true);

    // make it editable before navigating the cursor
    page->setContentEditable(true);

    // cursor will be before the word "The", this makes sure there is a charet
    page->triggerAction(QWebEnginePage::MoveToStartOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // here the actions are enabled after contentEditable is true
    QCOMPARE(page->action(QWebEnginePage::SelectNextChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousChar)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectNextWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectNextLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectPreviousLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfLine)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfBlock)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectStartOfDocument)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SelectEndOfDocument)->isEnabled(), true);
#endif
}

void tst_QWebEnginePage::textEditing()
{
#if !defined(QWEBENGINEPAGE_EVALUATEJAVASCRIPT)
    QSKIP("QWEBENGINEPAGE_EVALUATEJAVASCRIPT");
#else
    QScopedPointer<CursorTrackedPage> page(new CursorTrackedPage);
    QString content("<html><body><p id=one>The quick brown fox</p>" \
        "<p id=two>jumps over the lazy dog</p>" \
        "<p>May the source<br/>be with you!</p></body></html>");
    page->setHtml(content);

    // these actions must exist
    QVERIFY(page->action(QWebEnginePage::Cut) != 0);
    QVERIFY(page->action(QWebEnginePage::Copy) != 0);
    QVERIFY(page->action(QWebEnginePage::Paste) != 0);
    QVERIFY(page->action(QWebEnginePage::DeleteStartOfWord) != 0);
    QVERIFY(page->action(QWebEnginePage::DeleteEndOfWord) != 0);
    QVERIFY(page->action(QWebEnginePage::SetTextDirectionDefault) != 0);
    QVERIFY(page->action(QWebEnginePage::SetTextDirectionLeftToRight) != 0);
    QVERIFY(page->action(QWebEnginePage::SetTextDirectionRightToLeft) != 0);
    QVERIFY(page->action(QWebEnginePage::ToggleBold) != 0);
    QVERIFY(page->action(QWebEnginePage::ToggleItalic) != 0);
    QVERIFY(page->action(QWebEnginePage::ToggleUnderline) != 0);
    QVERIFY(page->action(QWebEnginePage::InsertParagraphSeparator) != 0);
    QVERIFY(page->action(QWebEnginePage::InsertLineSeparator) != 0);
    QVERIFY(page->action(QWebEnginePage::PasteAndMatchStyle) != 0);
    QVERIFY(page->action(QWebEnginePage::RemoveFormat) != 0);
    QVERIFY(page->action(QWebEnginePage::ToggleStrikethrough) != 0);
    QVERIFY(page->action(QWebEnginePage::ToggleSubscript) != 0);
    QVERIFY(page->action(QWebEnginePage::ToggleSuperscript) != 0);
    QVERIFY(page->action(QWebEnginePage::InsertUnorderedList) != 0);
    QVERIFY(page->action(QWebEnginePage::InsertOrderedList) != 0);
    QVERIFY(page->action(QWebEnginePage::Indent) != 0);
    QVERIFY(page->action(QWebEnginePage::Outdent) != 0);
    QVERIFY(page->action(QWebEnginePage::AlignCenter) != 0);
    QVERIFY(page->action(QWebEnginePage::AlignJustified) != 0);
    QVERIFY(page->action(QWebEnginePage::AlignLeft) != 0);
    QVERIFY(page->action(QWebEnginePage::AlignRight) != 0);

    // right now they are disabled because contentEditable is false
    QCOMPARE(page->action(QWebEnginePage::Cut)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::Paste)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::DeleteStartOfWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::DeleteEndOfWord)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SetTextDirectionDefault)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SetTextDirectionLeftToRight)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::SetTextDirectionRightToLeft)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::ToggleBold)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::ToggleItalic)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::ToggleUnderline)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::InsertParagraphSeparator)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::InsertLineSeparator)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::PasteAndMatchStyle)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::RemoveFormat)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::ToggleStrikethrough)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::ToggleSubscript)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::ToggleSuperscript)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::InsertUnorderedList)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::InsertOrderedList)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::Indent)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::Outdent)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::AlignCenter)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::AlignJustified)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::AlignLeft)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::AlignRight)->isEnabled(), false);

    // Select everything
    page->triggerAction(QWebEnginePage::SelectAll);

    // make sure it is enabled since there is a selection
    QCOMPARE(page->action(QWebEnginePage::Copy)->isEnabled(), true);

    // make it editable before navigating the cursor
    page->setContentEditable(true);

    // clear the selection
    page->triggerAction(QWebEnginePage::MoveToStartOfDocument);
    QVERIFY(page->isSelectionCollapsed());
    QCOMPARE(page->selectionStartOffset(), 0);

    // make sure it is disabled since there isn't a selection
    QCOMPARE(page->action(QWebEnginePage::Copy)->isEnabled(), false);

    // here the actions are enabled after contentEditable is true
    QCOMPARE(page->action(QWebEnginePage::Paste)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::DeleteStartOfWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::DeleteEndOfWord)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SetTextDirectionDefault)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SetTextDirectionLeftToRight)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::SetTextDirectionRightToLeft)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::ToggleBold)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::ToggleItalic)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::ToggleUnderline)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::InsertParagraphSeparator)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::InsertLineSeparator)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::PasteAndMatchStyle)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::ToggleStrikethrough)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::ToggleSubscript)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::ToggleSuperscript)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::InsertUnorderedList)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::InsertOrderedList)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::Indent)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::Outdent)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::AlignCenter)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::AlignJustified)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::AlignLeft)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::AlignRight)->isEnabled(), true);

    // make sure these are disabled since there isn't a selection
    QCOMPARE(page->action(QWebEnginePage::Cut)->isEnabled(), false);
    QCOMPARE(page->action(QWebEnginePage::RemoveFormat)->isEnabled(), false);

    // make sure everything is selected
    page->triggerAction(QWebEnginePage::SelectAll);

    // this is only true if there is an editable selection
    QCOMPARE(page->action(QWebEnginePage::Cut)->isEnabled(), true);
    QCOMPARE(page->action(QWebEnginePage::RemoveFormat)->isEnabled(), true);
#endif
}

void tst_QWebEnginePage::backActionUpdate()
{
    QWebEngineView view;
    view.resize(640, 480);
    view.show();

    QWebEnginePage *page = view.page();
    QSignalSpy loadSpy(page, &QWebEnginePage::loadFinished);
    QAction *action = page->action(QWebEnginePage::Back);
    QVERIFY(!action->isEnabled());

    page->load(QUrl("qrc:///resources/framedindex.html"));
    QTRY_COMPARE_WITH_TIMEOUT(loadSpy.count(), 1, 20000);
    QVERIFY(!action->isEnabled());

    auto firstAnchorCenterInFrame = [](QWebEnginePage *page, const QString &frameName) {
        QVariantList rectList = evaluateJavaScriptSync(page,
            "(function(){"
            "var frame = document.getElementsByName('" + frameName + "')[0];"
            "var anchor = frame.contentDocument.getElementsByTagName('a')[0];"
            "var rect = anchor.getBoundingClientRect();"
            "return [(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2];"
        "})()").toList();

        if (rectList.count() != 2) {
            qWarning("firstAnchorCenterInFrame failed.");
            return QPoint();
        }

        return QPoint(rectList.at(0).toInt(), rectList.at(1).toInt());
    };

    QVERIFY(evaluateJavaScriptSync(page, "document.getElementsByName('frame_b')[0].contentDocument == undefined").toBool());
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, 0, firstAnchorCenterInFrame(page, "frame_c"));
    QTRY_VERIFY(evaluateJavaScriptSync(page, "document.getElementsByName('frame_b')[0].contentDocument != undefined").toBool());
    QTRY_VERIFY(action->isEnabled());
}

#if defined(QWEBENGINEPAGE_SETTINGS)
static inline bool testFlag(QWebEnginePage& webPage, QWebEngineSettings::WebAttribute settingAttribute, const QString& jsObjectName, bool settingValue)
{
    webPage.settings()->setAttribute(settingAttribute, settingValue);
    return evaluateJavaScriptSync(&webPage, QString("(window.%1 != undefined)").arg(jsObjectName)).toBool();
}
#endif

void tst_QWebEnginePage::testOptionalJSObjects()
{
#if !defined(QWEBENGINESETTINGS)
    QSKIP("QWEBENGINSETTINGS");
#else
    // Once a feature is enabled and the JS object is accessed turning off the setting will not turn off
    // the visibility of the JS object any more. For this reason this test uses two QWebEnginePage instances.
    // Part of the test is to make sure that the QWebEnginePage instances do not interfere with each other so turning on
    // a feature for one instance will not turn it on for another.

    QWebEnginePage webPage1;
    QWebEnginePage webPage2;

    webPage1.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("http://www.example.com/"));
    webPage2.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("http://www.example.com/"));

    QEXPECT_FAIL("","Feature enabled/disabled checking problem. Look at bugs.webkit.org/show_bug.cgi?id=29867", Continue);
    QCOMPARE(testFlag(webPage1, QWebEngineSettings::OfflineWebApplicationCacheEnabled, "applicationCache", false), false);
    QCOMPARE(testFlag(webPage2, QWebEngineSettings::OfflineWebApplicationCacheEnabled, "applicationCache", true),  true);
    QEXPECT_FAIL("","Feature enabled/disabled checking problem. Look at bugs.webkit.org/show_bug.cgi?id=29867", Continue);
    QCOMPARE(testFlag(webPage1, QWebEngineSettings::OfflineWebApplicationCacheEnabled, "applicationCache", false), false);
    QCOMPARE(testFlag(webPage2, QWebEngineSettings::OfflineWebApplicationCacheEnabled, "applicationCache", false), true);

    QCOMPARE(testFlag(webPage1, QWebEngineSettings::LocalStorageEnabled, "localStorage", false), false);
    QCOMPARE(testFlag(webPage2, QWebEngineSettings::LocalStorageEnabled, "localStorage", true),  true);
    QCOMPARE(testFlag(webPage1, QWebEngineSettings::LocalStorageEnabled, "localStorage", false), false);
    QCOMPARE(testFlag(webPage2, QWebEngineSettings::LocalStorageEnabled, "localStorage", false), true);
#endif
}

#if defined(QWEBENGINEPAGE_SETTINGS)
static inline bool checkLocalStorageVisibility(QWebEnginePage& webPage, bool localStorageEnabled)
{
    webPage.settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, localStorageEnabled);
    return evaluateJavaScriptSync(&webPage, QString("(window.localStorage != undefined)")).toBool();
}
#endif

void tst_QWebEnginePage::testLocalStorageVisibility()
{
#if !defined(QWEBENGINEPAGE_SETTINGS)
    QSKIP("QWEBENGINEPAGE_SETTINGS");
#else
    // Local storage's visibility depends on its security origin, which depends on base url.
    // Initially, it will test it with base urls that get a globally unique origin, which may not
    // be able to use local storage even if the feature is enabled. Then later the same test is
    // done but with urls that would get a valid origin, so local storage could be used.
    // Before every test case it checks if local storage is not already visible.

    QWebEnginePage webPage;

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl());

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), false);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("invalid"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), false);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("://misparsed.com"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), false);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("http://"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), false);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("about:blank"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), false);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("data:text/html,test"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), false);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("file:///"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), true);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("http://www.example.com"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), true);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("https://www.example.com"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), true);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("ftp://files.example.com"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), true);

    webPage.currentFrame()->setHtml(QString("<html><body>test</body></html>"), QUrl("file:///path/to/index.html"));

    QCOMPARE(checkLocalStorageVisibility(webPage, false), false);
    QCOMPARE(checkLocalStorageVisibility(webPage, true), true);
#endif
}

void tst_QWebEnginePage::testEnablePersistentStorage()
{
#if !defined(QWEBENGINESETTINGS)
    QSKIP("QWEBENGINESETTINGS");
#else
    QWebEnginePage webPage;

    // By default all persistent options should be disabled
    QCOMPARE(webPage.settings()->testAttribute(QWebEngineSettings::LocalStorageEnabled), false);
    QCOMPARE(webPage.settings()->testAttribute(QWebEngineSettings::OfflineStorageDatabaseEnabled), false);
    QCOMPARE(webPage.settings()->testAttribute(QWebEngineSettings::OfflineWebApplicationCacheEnabled), false);
    QVERIFY(webPage.settings()->iconDatabasePath().isEmpty());

    QWebEngineSettings::enablePersistentStorage();


    QTRY_COMPARE(webPage.settings()->testAttribute(QWebEngineSettings::LocalStorageEnabled), true);
    QTRY_COMPARE(webPage.settings()->testAttribute(QWebEngineSettings::OfflineStorageDatabaseEnabled), true);
    QTRY_COMPARE(webPage.settings()->testAttribute(QWebEngineSettings::OfflineWebApplicationCacheEnabled), true);

    QTRY_VERIFY(!webPage.settings()->offlineStoragePath().isEmpty());
    QTRY_VERIFY(!webPage.settings()->offlineWebApplicationCachePath().isEmpty());
    QTRY_VERIFY(!webPage.settings()->iconDatabasePath().isEmpty());
#endif
}


#if defined(QWEBENGINEPAGE_ERRORPAGEEXTENSION)
class ErrorPage : public QWebEnginePage
{
public:

    ErrorPage(QWidget* parent = 0): QWebEnginePage(parent)
    {
    }

    virtual bool supportsExtension(Extension extension) const
    {
        return extension == ErrorPageExtension;
    }

    virtual bool extension(Extension, const ExtensionOption* option, ExtensionReturn* output)
    {
        ErrorPageExtensionReturn* errorPage = static_cast<ErrorPageExtensionReturn*>(output);

        errorPage->contentType = "text/html";
        errorPage->content = "error";
        return true;
    }
};
#endif

void tst_QWebEnginePage::errorPageExtension()
{
#if !defined(QWEBENGINEPAGE_ERRORPAGEEXTENSION)
    QSKIP("QWEBENGINEPAGE_ERRORPAGEEXTENSION");
#else
    ErrorPage page;
    m_view->setPage(&page);

    QSignalSpy spyLoadFinished(m_view, SIGNAL(loadFinished(bool)));

    m_view->setUrl(QUrl("data:text/html,foo"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);

    page.setUrl(QUrl("http://non.existent/url"));
    QTRY_COMPARE(spyLoadFinished.count(), 2);
    QCOMPARE(toPlainTextSync(&page), QString("error"));
    QCOMPARE(page.history()->count(), 2);
    QCOMPARE(page.history()->currentItem().url(), QUrl("http://non.existent/url"));
    QCOMPARE(page.history()->canGoBack(), true);
    QCOMPARE(page.history()->canGoForward(), false);

    page.triggerAction(QWebEnginePage::Back);
    QTRY_COMPARE(page.history()->canGoBack(), false);
    QTRY_COMPARE(page.history()->canGoForward(), true);

    page.triggerAction(QWebEnginePage::Forward);
    QTRY_COMPARE(page.history()->canGoBack(), true);
    QTRY_COMPARE(page.history()->canGoForward(), false);

    page.triggerAction(QWebEnginePage::Back);
    QTRY_COMPARE(page.history()->canGoBack(), false);
    QTRY_COMPARE(page.history()->canGoForward(), true);
    QTRY_COMPARE(page.history()->currentItem().url(), QUrl("data:text/html,foo"));

    m_view->setPage(0);
#endif
}

void tst_QWebEnginePage::errorPageExtensionLoadFinished()
{
#if !defined(QWEBENGINEPAGE_ERRORPAGEEXTENSION)
    QSKIP("QWEBENGINEPAGE_ERRORPAGEEXTENSION");
#else
    ErrorPage page;
    m_view->setPage(&page);

    QSignalSpy spyLoadFinished(m_view, SIGNAL(loadFinished(bool)));
    QSignalSpy spyFrameLoadFinished(m_view->page(), SIGNAL(loadFinished(bool)));

    m_view->setUrl(QUrl("data:text/html,foo"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);
    QTRY_COMPARE(spyFrameLoadFinished.count(), 1);

    const bool loadSucceded = spyLoadFinished.at(0).at(0).toBool();
    QVERIFY(loadSucceded);
    const bool frameLoadSucceded = spyFrameLoadFinished.at(0).at(0).toBool();
    QVERIFY(frameLoadSucceded);

    m_view->page()->setUrl(QUrl("http://non.existent/url"));
    QTRY_COMPARE(spyLoadFinished.count(), 2);
    QTRY_COMPARE(spyFrameLoadFinished.count(), 2);

    const bool nonExistantLoadSucceded = spyLoadFinished.at(1).at(0).toBool();
    QVERIFY(nonExistantLoadSucceded);
    const bool nonExistantFrameLoadSucceded = spyFrameLoadFinished.at(1).at(0).toBool();
    QVERIFY(nonExistantFrameLoadSucceded);

    m_view->setPage(0);
#endif
}

void tst_QWebEnginePage::userAgentNewlineStripping()
{
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);

    profile.setHttpUserAgent(QStringLiteral("My User Agent\nX-New-Http-Header: Oh Noes!"));
    // The user agent will be updated after a page load.
    page.load(QUrl("about:blank"));

    QTRY_COMPARE(evaluateJavaScriptSync(&page, "navigator.userAgent").toString(), QStringLiteral("My User Agent X-New-Http-Header: Oh Noes!"));
}

void tst_QWebEnginePage::crashTests_LazyInitializationOfMainFrame()
{
    {
        QWebEnginePage webPage;
    }
    {
        QWebEnginePage webPage;
        webPage.selectedText();
    }
    {
#if defined(QWEBENGINEPAGE_SELECTEDHTML)
        QWebEnginePage webPage;
        webPage.selectedHtml();
#endif
    }
    {
        QWebEnginePage webPage;
        webPage.triggerAction(QWebEnginePage::Back, true);
    }
    {
#if defined(QWEBENGINEPAGE_UPDATEPOSITIONDEPENDENTACTIONS)
        QWebEnginePage webPage;
        QPoint pos(10,10);
        webPage.updatePositionDependentActions(pos);
#endif
    }
}

#if defined(QWEBENGINEPAGE_RENDER)
static void takeScreenshot(QWebEnginePage* page)
{
    page->setViewportSize(page->contentsSize());
    QImage image(page->viewportSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    page->render(&painter);
    painter.end();
}
#endif

void tst_QWebEnginePage::screenshot_data()
{
    QTest::addColumn<QString>("html");
    QTest::newRow("WithoutPlugin") << "<html><body id='b'>text</body></html>";
    QTest::newRow("WindowedPlugin") << QString("<html><body id='b'>text<embed src='resources/test.swf'></embed></body></html>");
    QTest::newRow("WindowlessPlugin") << QString("<html><body id='b'>text<embed src='resources/test.swf' wmode='transparent'></embed></body></html>");
}

void tst_QWebEnginePage::screenshot()
{
#if !defined(QWEBENGINESETTINGS)
    QSKIP("QWEBENGINESETTINGS");
#else
    if (!QDir(TESTS_SOURCE_DIR).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'").arg(TESTS_SOURCE_DIR).toLatin1().constData(), SkipAll);

    QDir::setCurrent(TESTS_SOURCE_DIR);

    QFETCH(QString, html);
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    page.setHtml(html, QUrl::fromLocalFile(TESTS_SOURCE_DIR));
    QVERIFY(spyFinished.wait(2000));

    // take screenshot without a view
    takeScreenshot(&page);

    QWebEngineView view;
    view.setPage(&page);

    // take screenshot when attached to a view
    takeScreenshot(&page);

    QDir::setCurrent(QApplication::applicationDirPath());
#endif
}

#if defined(ENABLE_WEBGL) && ENABLE_WEBGL
// https://bugs.webkit.org/show_bug.cgi?id=54138
static void webGLScreenshotWithoutView(bool accelerated)
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    page.settings()->setAttribute(QWebEngineSettings::AcceleratedCompositingEnabled, accelerated);
    page.setHtml("<html><body>"
                       "<canvas id='webgl' width='300' height='300'></canvas>"
                       "<script>document.getElementById('webgl').getContext('experimental-webgl')</script>"
                       "</body></html>");

    takeScreenshot(&page);
}

void tst_QWebEnginePage::acceleratedWebGLScreenshotWithoutView()
{
    webGLScreenshotWithoutView(true);
}

void tst_QWebEnginePage::unacceleratedWebGLScreenshotWithoutView()
{
    webGLScreenshotWithoutView(false);
}
#endif

/**
 * Test fixups for https://bugs.webkit.org/show_bug.cgi?id=30914
 *
 * From JS we test the following conditions.
 *
 *   OK     + QString() => SUCCESS, empty string (but not null)
 *   OK     + "text"    => SUCCESS, "text"
 *   CANCEL + QString() => CANCEL, null string
 *   CANCEL + "text"    => CANCEL, null string
 */
class JSPromptPage : public QWebEnginePage {
    Q_OBJECT
public:
    JSPromptPage()
    {}

    bool javaScriptPrompt(const QUrl &securityOrigin, const QString& msg, const QString& defaultValue, QString* result)
    {
        if (msg == QLatin1String("test1")) {
            *result = QString();
            return true;
        } else if (msg == QLatin1String("test2")) {
            *result = QLatin1String("text");
            return true;
        } else if (msg == QLatin1String("test3")) {
            *result = QString();
            return false;
        } else if (msg == QLatin1String("test4")) {
            *result = QLatin1String("text");
            return false;
        }

        qFatal("Unknown msg.");
        return QWebEnginePage::javaScriptPrompt(securityOrigin, msg, defaultValue, result);
    }
};

void tst_QWebEnginePage::testJSPrompt()
{
    JSPromptPage page;
    bool res;
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral("<html><body></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);

    // OK + QString()
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test1');"
            "retval=='' && retval.length == 0;").toBool();
    QVERIFY(res);

    // OK + "text"
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test2');"
            "retval=='text' && retval.length == 4;").toBool();
    QVERIFY(res);

    // Cancel + QString()
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test3');"
            "retval===null;").toBool();
    QVERIFY(res);

    // Cancel + "text"
    res = evaluateJavaScriptSync(&page,
            "var retval = prompt('test4');"
            "retval===null;").toBool();
    QVERIFY(res);
}

void tst_QWebEnginePage::testStopScheduledPageRefresh()
{
#if !defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    QSKIP("QWEBENGINEPAGE_SETNETWORKACCESSMANAGER");
#else
    // Without QWebEnginePage::StopScheduledPageRefresh
    QWebEnginePage page1;
    page1.setNetworkAccessManager(new TestNetworkManager(&page1));
    page1.setHtml("<html><head>"
                                "<meta http-equiv=\"refresh\"content=\"0;URL=qrc:///resources/index.html\">"
                                "</head><body><h1>Page redirects immediately...</h1>"
                                "</body></html>");
    QSignalSpy spyFinished(&page1, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait();
    QTest::qWait(500);
    QCOMPARE(page1.url(), QUrl(QLatin1String("qrc:///resources/index.html")));

    // With QWebEnginePage::StopScheduledPageRefresh
    QWebEnginePage page2;
    page2.setNetworkAccessManager(new TestNetworkManager(&page2));
    page2.setHtml("<html><head>"
                               "<meta http-equiv=\"refresh\"content=\"1;URL=qrc:///resources/index.html\">"
                               "</head><body><h1>Page redirect test with 1 sec timeout...</h1>"
                               "</body></html>");
    page2.triggerAction(QWebEnginePage::StopScheduledPageRefresh);
    QTest::qWait(1500);
    QEXPECT_FAIL("", "https://bugs.webkit.org/show_bug.cgi?id=118673", Continue);
    QCOMPARE(page2.url().toString(), QLatin1String("about:blank"));
#endif
}

void tst_QWebEnginePage::findText()
{
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));
    m_view->setHtml(QString("<html><head></head><body><div>foo bar</div></body></html>"));

    // Showing is required, otherwise all find operations fail.
    m_view->show();
    QTRY_COMPARE(loadSpy.count(), 1);

    // Select whole page contents.
    QTRY_VERIFY(m_view->page()->action(QWebEnginePage::SelectAll)->isEnabled());
    m_view->page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(m_view->hasSelection(), true);

    // Invoking a stopFinding operation will not change or clear the currently selected text,
    // if nothing was found beforehand.
    {
        CallbackSpy<bool> spy;
        m_view->findText("", 0, spy.ref());
        QVERIFY(spy.wasCalled());
        QTRY_COMPARE(m_view->selectedText(), QString("foo bar"));
    }

    // Invoking a startFinding operation with text that won't be found, will clear the current
    // selection.
    {
        CallbackSpy<bool> spy;
        m_view->findText("Will not be found", 0, spy.ref());
        QCOMPARE(spy.waitForResult(), false);
        QTRY_VERIFY(m_view->selectedText().isEmpty());
    }

    // Select whole page contents again.
    m_view->page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(m_view->hasSelection(), true);

    // Invoking a startFinding operation with text that will be found, will clear the current
    // selection as well.
    {
        CallbackSpy<bool> spy;
        m_view->findText("foo", 0, spy.ref());
        QVERIFY(spy.waitForResult());
        QTRY_VERIFY(m_view->selectedText().isEmpty());
    }

    // Invoking a stopFinding operation after text was found, will set the selected text to the
    // found text.
    {
        CallbackSpy<bool> spy;
        m_view->findText("", 0, spy.ref());
        QTRY_VERIFY(spy.wasCalled());
        QTRY_COMPARE(m_view->selectedText(), QString("foo"));
    }
}

void tst_QWebEnginePage::findTextResult()
{
    // findText will abort in blink if the view has an empty size.
    m_view->resize(800, 600);
    m_view->show();

    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));
    m_view->setHtml(QString("<html><head></head><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);

    QCOMPARE(findTextSync(m_page, ""), false);

    const QStringList words = { "foo", "bar" };
    for (const QString &subString : words) {
        QCOMPARE(findTextSync(m_page, subString), true);
        QCOMPARE(findTextSync(m_page, ""), false);
    }

    QCOMPARE(findTextSync(m_page, "blahhh"), false);
    QCOMPARE(findTextSync(m_page, ""), false);
}

void tst_QWebEnginePage::findTextSuccessiveShouldCallAllCallbacks()
{
    CallbackSpy<bool> spy1;
    CallbackSpy<bool> spy2;
    CallbackSpy<bool> spy3;
    CallbackSpy<bool> spy4;
    CallbackSpy<bool> spy5;
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));
    m_view->setHtml(QString("<html><head></head><body><div>abcdefg abcdefg abcdefg abcdefg abcdefg</div></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);
    m_page->findText("abcde", 0, spy1.ref());
    m_page->findText("abcd", 0, spy2.ref());
    m_page->findText("abc", 0, spy3.ref());
    m_page->findText("ab", 0, spy4.ref());
    m_page->findText("a", 0, spy5.ref());
    spy5.waitForResult();
    QVERIFY(spy1.wasCalled());
    QVERIFY(spy2.wasCalled());
    QVERIFY(spy3.wasCalled());
    QVERIFY(spy4.wasCalled());
    QVERIFY(spy5.wasCalled());
}

#if defined(QWEBENGINEPAGE_SUPPORTEDCONTENTTYPES)
static QString getMimeTypeForExtension(const QString &ext)
{
    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(QStringLiteral("filename.") + ext.toLower(), QMimeDatabase::MatchExtension);
    if (mimeType.isValid() && !mimeType.isDefault())
        return mimeType.name();

    return QString();
}
#endif

void tst_QWebEnginePage::supportedContentType()
{
#if !defined(QWEBENGINEPAGE_SUPPORTEDCONTENTTYPES)
    QSKIP("QWEBENGINEPAGE_SUPPORTEDCONTENTTYPES");
#else
    QStringList contentTypes;

    // Add supported non image types...
    contentTypes << "text/html" << "text/xml" << "text/xsl" << "text/plain" << "text/"
                 << "application/xml" << "application/xhtml+xml" << "application/vnd.wap.xhtml+xml"
                 << "application/rss+xml" << "application/atom+xml" << "application/json";

#if ENABLE_MHTML
    contentTypes << "application/x-mimearchive";
#endif

    // Add supported image types...
    const QList<QByteArray> supportedImageFormats = QImageWriter::supportedImageFormats();
    for (const QByteArray &imageType : supportedImageFormats) {
        const QString mimeType = getMimeTypeForExtension(imageType);
        if (!mimeType.isEmpty())
            contentTypes << mimeType;
    }

    // Get the mime types supported by webengine...
    const QStringList supportedContentTypes = m_page->supportedContentTypes();

    for (const QString &mimeType : qAsConst(contentTypes))
        QVERIFY2(supportedContentTypes.contains(mimeType), QString("'%1' is not a supported content type!").arg(mimeType).toLatin1());

    for (const QString &mimeType : qAsConst(contentTypes))
        QVERIFY2(m_page->supportsContentType(mimeType), QString("Cannot handle content types '%1'!").arg(mimeType).toLatin1());
#endif
}

void tst_QWebEnginePage::thirdPartyCookiePolicy()
{
#if !defined(DUMPRENDERTREESUPPORTQT)
    QSKIP("DUMPRENDERTREESUPPORTQT");
#else
    QWebEngineSettings::globalSettings()->setThirdPartyCookiePolicy(QWebEngineSettings::AlwaysBlockThirdPartyCookies);
    m_page->networkAccessManager()->setCookieJar(new QNetworkCookieJar());
    QVERIFY(m_page->networkAccessManager()->cookieJar());

    // These are all first-party cookies, so should pass.
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://www.example.com"), QUrl("http://example.com")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://www.example.com"), QUrl("http://doc.example.com")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://aaa.www.example.com"), QUrl("http://doc.example.com")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://example.com"), QUrl("http://www.example.com")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://www.example.co.uk"), QUrl("http://example.co.uk")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://www.example.co.uk"), QUrl("http://doc.example.co.uk")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://aaa.www.example.co.uk"), QUrl("http://doc.example.co.uk")));
    QVERIFY(DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://example.co.uk"), QUrl("http://www.example.co.uk")));

    // These are all third-party cookies, so should fail.
    QVERIFY(!DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://www.example.com"), QUrl("http://slashdot.org")));
    QVERIFY(!DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://example.com"), QUrl("http://anotherexample.com")));
    QVERIFY(!DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://anotherexample.com"), QUrl("http://example.com")));
    QVERIFY(!DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://www.example.co.uk"), QUrl("http://slashdot.co.uk")));
    QVERIFY(!DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://example.co.uk"), QUrl("http://anotherexample.co.uk")));
    QVERIFY(!DumpRenderTreeSupportQt::thirdPartyCookiePolicyAllows(m_page->handle(),
            QUrl("http://anotherexample.co.uk"), QUrl("http://example.co.uk")));
#endif
}

static QWindow *findNewTopLevelWindow(const QWindowList &oldTopLevelWindows)
{
    const auto tlws = QGuiApplication::topLevelWindows();
    for (auto w : tlws) {
        if (!oldTopLevelWindows.contains(w)) {
            return w;
        }
    }
    return nullptr;
}

void tst_QWebEnginePage::comboBoxPopupPositionAfterMove()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QWebEngineView view;
    view.move(screen->availableGeometry().topLeft());
    view.resize(640, 480);
    view.show();

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QLatin1String("<html><head></head><body><select id='foo'>"
                               "<option>fran</option><option>troz</option>"
                               "</select></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);
    const auto oldTlws = QGuiApplication::topLevelWindows();
    QWindow *window = view.windowHandle();
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(),
                      elementCenter(view.page(), "foo"));

    QWindow *popup = nullptr;
    QTRY_VERIFY(popup = findNewTopLevelWindow(oldTlws));
    QPoint popupPos = popup->position();

    // Close the popup by clicking somewhere into the page.
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 1));
    QTRY_VERIFY(!QGuiApplication::topLevelWindows().contains(popup));

    // Move the top-level QWebEngineView a little and check the popup's position.
    const QPoint offset(12, 13);
    view.move(screen->availableGeometry().topLeft() + offset);
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(),
                      elementCenter(view.page(), "foo"));
    QTRY_VERIFY(popup = findNewTopLevelWindow(oldTlws));
    QCOMPARE(popupPos + offset, popup->position());
}

void tst_QWebEnginePage::comboBoxPopupPositionAfterChildMove()
{
    QWidget mainWidget;
    mainWidget.setLayout(new QHBoxLayout);

    QWidget spacer;
    spacer.setMinimumWidth(50);
    mainWidget.layout()->addWidget(&spacer);

    QWebEngineView view;
    mainWidget.layout()->addWidget(&view);

    QScreen *screen = QGuiApplication::primaryScreen();
    mainWidget.move(screen->availableGeometry().topLeft());
    mainWidget.resize(640, 480);
    mainWidget.show();

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QLatin1String("<html><head></head><body><select autofocus id='foo'>"
                               "<option value=\"narf\">narf</option><option>zort</option>"
                               "</select></body></html>"));
    QTRY_COMPARE(loadSpy.count(), 1);
    const auto oldTlws = QGuiApplication::topLevelWindows();
    QWindow *window = view.window()->windowHandle();
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(),
                      view.mapTo(view.window(), elementCenter(view.page(), "foo")));

    QWindow *popup = nullptr;
    QTRY_VERIFY(popup = findNewTopLevelWindow(oldTlws));
    QPoint popupPos = popup->position();

    // Close the popup by clicking somewhere into the page.
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(),
                      view.mapTo(view.window(), QPoint(1, 1)));
    QTRY_VERIFY(!QGuiApplication::topLevelWindows().contains(popup));

    // Resize the "spacer" widget, and implicitly change the global position of the QWebEngineView.
    spacer.setMinimumWidth(100);
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(),
                      view.mapTo(view.window(), elementCenter(view.page(), "foo")));
    QTRY_VERIFY(popup = findNewTopLevelWindow(oldTlws));
    QCOMPARE(popupPos + QPoint(50, 0), popup->position());
}

#ifdef Q_OS_MAC
void tst_QWebEnginePage::macCopyUnicodeToClipboard()
{
    QString unicodeText = QString::fromUtf8("αβγδεζηθικλμπ");
    m_page->setHtml(QString("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head><body>%1</body></html>").arg(unicodeText));
    m_page->triggerAction(QWebEnginePage::SelectAll);
    m_page->triggerAction(QWebEnginePage::Copy);

    QString clipboardData = QString::fromUtf8(QApplication::clipboard()->mimeData()->data(QLatin1String("text/html")));

    QVERIFY(clipboardData.contains(QLatin1String("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />")));
    QVERIFY(clipboardData.contains(unicodeText));
}
#endif

void tst_QWebEnginePage::contextMenuCopy()
{
#if !defined(QWEBENGINEELEMENT)
    QSKIP("QWEBENGINEELEMENT");
#else
    QWebEngineView view;

    view.setHtml("<a href=\"http://www.google.com\">You cant miss this</a>");

    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QVERIFY(!view.page()->selectedText().isEmpty());

    QWebEngineElement link = view.page()->mainFrame()->findFirstElement("a");
    QPoint pos(link.geometry().center());
    QContextMenuEvent event(QContextMenuEvent::Mouse, pos);
    view.page()->swallowContextMenuEvent(&event);
    view.page()->updatePositionDependentActions(pos);

    QList<QMenu*> contextMenus = view.findChildren<QMenu*>();
    QVERIFY(!contextMenus.isEmpty());
    QMenu* contextMenu = contextMenus.first();
    QVERIFY(contextMenu);

    QList<QAction *> list = contextMenu->actions();
    int index = list.indexOf(view.page()->action(QWebEnginePage::Copy));
    QVERIFY(index != -1);
#endif
}

// https://bugs.webkit.org/show_bug.cgi?id=62139
void tst_QWebEnginePage::contextMenuPopulatedOnce()
{
#if !defined(QWEBENGINEELEMENT)
    QSKIP("QWEBENGINEELEMENT");
#else
    QWebEngineView view;

    view.setHtml("<input type=\"text\">");

    QWebEngineElement link = view.page()->mainFrame()->findFirstElement("input");
    QPoint pos(link.geometry().center());
    QContextMenuEvent event(QContextMenuEvent::Mouse, pos);
    view.page()->swallowContextMenuEvent(&event);
    view.page()->updatePositionDependentActions(pos);

    QList<QMenu*> contextMenus = view.findChildren<QMenu*>();
    QVERIFY(!contextMenus.isEmpty());
    QMenu* contextMenu = contextMenus.first();
    QVERIFY(contextMenu);

    QList<QAction *> list = contextMenu->actions();
    QStringList entries;
    while (!list.isEmpty()) {
        QString entry = list.takeFirst()->text();
        QVERIFY(!entries.contains(entry));
        entries << entry;
    }
#endif
}

void tst_QWebEnginePage::deleteQWebEngineViewTwice()
{
    for (int i = 0; i < 2; ++i) {
        QMainWindow mainWindow;
        QWebEngineView* webView = new QWebEngineView(&mainWindow);
        mainWindow.setCentralWidget(webView);
        webView->load(QUrl("qrc:///resources/frame_a.html"));
        mainWindow.show();
        QSignalSpy spyFinished(webView, &QWebEngineView::loadFinished);
        QVERIFY(spyFinished.wait());
    }
}

#if defined(QWEBENGINEPAGE_RENDER)
class RepaintRequestedRenderer : public QObject {
    Q_OBJECT
public:
    RepaintRequestedRenderer(QWebEnginePage* page, QPainter* painter)
        : m_page(page)
        , m_painter(painter)
        , m_recursionCount(0)
    {
        connect(m_page, SIGNAL(repaintRequested(QRect)), this, SLOT(onRepaintRequested(QRect)));
    }

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void onRepaintRequested(const QRect& rect)
    {
        QCOMPARE(m_recursionCount, 0);

        m_recursionCount++;
        m_page->render(m_painter, rect);
        m_recursionCount--;

        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

private:
    QWebEnginePage* m_page;
    QPainter* m_painter;
    int m_recursionCount;
};
#endif

void tst_QWebEnginePage::renderOnRepaintRequestedShouldNotRecurse()
{
#if !defined(QWEBENGINEPAGE_RENDER)
    QSKIP("QWEBENGINEPAGE_RENDER");
#else
    QSize viewportSize(720, 576);
    QWebEnginePage page;

    QImage image(viewportSize, QImage::Format_ARGB32);
    QPainter painter(&image);

    page.setPreferredContentsSize(viewportSize);
    page.setViewportSize(viewportSize);
    RepaintRequestedRenderer r(&page, &painter);

    page.setHtml("zalan loves trunk", QUrl());

    QSignalSpy spyFinished(&r, &RepaintRequestedRenderer::finished);
    QVERIFY(spyFinished.wait());
#endif
}

class SpyForLoadSignalsOrder : public QStateMachine {
    Q_OBJECT
public:
    SpyForLoadSignalsOrder(QWebEnginePage* page, QObject* parent = 0)
        : QStateMachine(parent)
    {
        connect(page, SIGNAL(loadProgress(int)), SLOT(onLoadProgress(int)));

        QState* waitingForLoadStarted = new QState(this);
        QState* waitingForFirstLoadProgress = new QState(this);
        QState* waitingForLastLoadProgress = new QState(this);
        QState* waitingForLoadFinished = new QState(this);
        QFinalState* final = new QFinalState(this);

        waitingForLoadStarted->addTransition(page, SIGNAL(loadStarted()), waitingForFirstLoadProgress);
        waitingForFirstLoadProgress->addTransition(this, SIGNAL(firstLoadProgress()), waitingForLastLoadProgress);
        waitingForLastLoadProgress->addTransition(this, SIGNAL(lastLoadProgress()), waitingForLoadFinished);
        waitingForLoadFinished->addTransition(page, SIGNAL(loadFinished(bool)), final);

        setInitialState(waitingForLoadStarted);
        start();
    }
    bool isFinished() const
    {
        return !isRunning();
    }
public Q_SLOTS:
    void onLoadProgress(int progress)
    {
        if (progress == 0)
            emit firstLoadProgress();
        else if (progress == 100)
            emit lastLoadProgress();
    }
Q_SIGNALS:
    void firstLoadProgress();
    void lastLoadProgress();
};

void tst_QWebEnginePage::loadSignalsOrder_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::newRow("inline data") << QUrl("data:text/html,This is first page");
    QTest::newRow("simple page") << QUrl("qrc:///resources/content.html");
    QTest::newRow("frameset page") << QUrl("qrc:///resources/index.html");
}

void tst_QWebEnginePage::loadSignalsOrder()
{
    QFETCH(QUrl, url);
    QWebEnginePage page;
    SpyForLoadSignalsOrder loadSpy(&page);
    QSignalSpy spyLoadSpy(&loadSpy, &SpyForLoadSignalsOrder::started);
    QVERIFY(spyLoadSpy.wait(500));
    page.load(url);
    QTRY_VERIFY(loadSpy.isFinished());
}

void tst_QWebEnginePage::undoActionHaveCustomText()
{
#if !defined(QWEBENGINEPAGE_UNDOACTION)
    QSKIP("QWEBENGINEPAGE_UNDOACTION");
#else
    m_page->setHtml("<div id=test contenteditable></div>");
    evaluateJavaScriptSync(m_page, "document.getElementById('test').focus()");

    evaluateJavaScriptSync(m_page, "document.execCommand('insertText', true, 'Test');");
    QString typingActionText = m_page->action(QWebEnginePage::Undo)->text();

    evaluateJavaScriptSync(m_page, "document.execCommand('indent', true);");
    QString alignActionText = m_page->action(QWebEnginePage::Undo)->text();

    QVERIFY(typingActionText != alignActionText);
#endif
}

void tst_QWebEnginePage::renderWidgetHostViewNotShowTopLevel()
{
    QWebEnginePage page;
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("http://qt-project.org"));
    if (!spyLoadFinished.wait(10000) || !spyLoadFinished.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");
    spyLoadFinished.clear();

    // Loading a different domain will force the creation of a separate render
    // process and should therefore create a new RenderWidgetHostViewQtDelegateWidget.
    page.load(QUrl("http://www.wikipedia.org/"));
    if (!spyLoadFinished.wait(10000) || !spyLoadFinished.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    // Make sure that RenderWidgetHostViewQtDelegateWidgets are not shown as top-level.
    // They should only be made visible when parented to a QWebEngineView.
    const QList<QWidget *> widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets)
        QCOMPARE(widget->isVisible(), false);
}

class GetUserMediaTestPage : public QWebEnginePage {
Q_OBJECT

public:
    GetUserMediaTestPage()
        : m_gotRequest(false)
        , m_loadSucceeded(false)
    {
        connect(this, &QWebEnginePage::featurePermissionRequested, this, &GetUserMediaTestPage::onFeaturePermissionRequested);
        connect(this, &QWebEnginePage::loadFinished, [this](bool success){
            m_loadSucceeded = success;
        });
        // We need to load content from a resource in order for the securityOrigin to be valid.
        load(QUrl("qrc:///resources/content.html"));
    }

    void jsGetUserMedia(const QString & constraints)
    {
        evaluateJavaScriptSync(this,
            QStringLiteral(
                "var promiseFulfilled = false;"
                "var promiseRejected = false;"
                "navigator.mediaDevices.getUserMedia(%1)"
                ".then(stream => { promiseFulfilled = true})"
                ".catch(err => { promiseRejected = true})")
            .arg(constraints));
    }

    bool jsPromiseFulfilled()
    {
        return evaluateJavaScriptSync(this, QStringLiteral("promiseFulfilled")).toBool();
    }

    bool jsPromiseRejected()
    {
        return evaluateJavaScriptSync(this, QStringLiteral("promiseRejected")).toBool();
    }

    void rejectPendingRequest()
    {
        setFeaturePermission(m_requestSecurityOrigin, m_requestedFeature, QWebEnginePage::PermissionDeniedByUser);
        m_gotRequest = false;
    }
    void acceptPendingRequest()
    {
        setFeaturePermission(m_requestSecurityOrigin, m_requestedFeature, QWebEnginePage::PermissionGrantedByUser);
        m_gotRequest = false;
    }

    bool gotFeatureRequest(QWebEnginePage::Feature feature)
    {
        return m_gotRequest && m_requestedFeature == feature;
    }

    bool gotFeatureRequest() const
    {
        return m_gotRequest;
    }

    bool loadSucceeded() const
    {
        return m_loadSucceeded;
    }

private Q_SLOTS:
    void onFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
    {
        m_requestedFeature = feature;
        m_requestSecurityOrigin = securityOrigin;
        m_gotRequest = true;
    }

private:
    bool m_gotRequest;
    bool m_loadSucceeded;
    QWebEnginePage::Feature m_requestedFeature;
    QUrl m_requestSecurityOrigin;

};

void tst_QWebEnginePage::getUserMediaRequest_data()
{
    QTest::addColumn<QString>("constraints");
    QTest::addColumn<QWebEnginePage::Feature>("feature");

    QTest::addRow("device audio")
        << "{audio: true}" << QWebEnginePage::MediaAudioCapture;
    QTest::addRow("device video")
        << "{video: true}" << QWebEnginePage::MediaVideoCapture;
    QTest::addRow("device audio+video")
        << "{audio: true, video: true}" << QWebEnginePage::MediaAudioVideoCapture;
    QTest::addRow("desktop video")
        << "{video: { mandatory: { chromeMediaSource: 'desktop' }}}"
        << QWebEnginePage::DesktopVideoCapture;
    QTest::addRow("desktop audio+video")
        << "{audio: { mandatory: { chromeMediaSource: 'desktop' }}, video: { mandatory: { chromeMediaSource: 'desktop' }}}"
        << QWebEnginePage::DesktopAudioVideoCapture;
}

void tst_QWebEnginePage::getUserMediaRequest()
{
    QFETCH(QString, constraints);
    QFETCH(QWebEnginePage::Feature, feature);

    GetUserMediaTestPage page;
    if (feature == QWebEnginePage::DesktopVideoCapture || feature == QWebEnginePage::DesktopAudioVideoCapture) {
        // Desktop capture needs to be on a desktop.
        QWebEngineView view;
        view.setPage(&page);
        view.resize(640, 480);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
    }

    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);

    // 1. Rejecting request on C++ side should reject promise on JS side.
    page.jsGetUserMedia(constraints);
    QTRY_VERIFY(page.gotFeatureRequest(feature));
    page.rejectPendingRequest();
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());

    // 2. Accepting request on C++ side should either fulfill or reject the
    // Promise on JS side. Due to the potential lack of physical media devices
    // deeper in the content layer we cannot guarantee that the promise will
    // always be fulfilled, however in this case an error should be returned to
    // JS instead of leaving the Promise in limbo.
    page.jsGetUserMedia(constraints);
    QTRY_VERIFY(page.gotFeatureRequest(feature));
    page.acceptPendingRequest();
    QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());

    // 3. Media feature permissions are not remembered.
    page.jsGetUserMedia(constraints);
    QTRY_VERIFY(page.gotFeatureRequest(feature));
    page.acceptPendingRequest();
    QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());
}

void tst_QWebEnginePage::getUserMediaRequestDesktopAudio()
{
    GetUserMediaTestPage page;
    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);

    // Audio-only desktop capture is not supported. JS Promise should be
    // rejected immediately.

    page.jsGetUserMedia(
        QStringLiteral("{audio: { mandatory: { chromeMediaSource: 'desktop' }}}"));
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());

    page.jsGetUserMedia(
        QStringLiteral("{audio: { mandatory: { chromeMediaSource: 'desktop' }}, video: true}"));
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());
}

void tst_QWebEnginePage::getUserMediaRequestSettingDisabled()
{
    GetUserMediaTestPage page;
    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);

    // With the setting disabled, the JS Promise should be rejected without
    // asking for permission first.

    page.jsGetUserMedia(QStringLiteral("{video: { mandatory: { chromeMediaSource: 'desktop' }}}"));
    QTRY_VERIFY(!page.jsPromiseFulfilled() && page.jsPromiseRejected());
}

// Try to trigger any possible race condition between the UI thread (permission
// management) and the audio/device thread (desktop capture initialization).
void tst_QWebEnginePage::getUserMediaRequestDesktopVideoManyPages()
{
    const QString constraints = QStringLiteral("{video: { mandatory: { chromeMediaSource: 'desktop' }}}");
    const QWebEnginePage::Feature feature = QWebEnginePage::DesktopVideoCapture;
    std::vector<GetUserMediaTestPage> pages(10);

    // Desktop capture needs to be on a desktop
    std::vector<QWebEngineView> views(10);
    for (size_t i = 0; i < views.size(); ++i) {
        QWebEngineView *view = &(views[i]);
        GetUserMediaTestPage *page = &(pages[i]);
        view->setPage(page);
        view->resize(640, 480);
        view->show();
        QVERIFY(QTest::qWaitForWindowExposed(view));
    }

    for (GetUserMediaTestPage &page : pages)
        QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    for (GetUserMediaTestPage &page : pages)
        page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    for (GetUserMediaTestPage &page : pages)
        page.jsGetUserMedia(constraints);
    for (GetUserMediaTestPage &page : pages)
        QTRY_VERIFY(page.gotFeatureRequest(feature));
    for (GetUserMediaTestPage &page : pages)
        page.acceptPendingRequest();
    for (GetUserMediaTestPage &page : pages)
        QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());
}

// Try to trigger any possible race condition between the UI or audio/device
// threads and the desktop capture thread, where the capture actually happens.
void tst_QWebEnginePage::getUserMediaRequestDesktopVideoManyRequests()
{
    const QString constraints = QStringLiteral("{video: { mandatory: { chromeMediaSource: 'desktop' }}}");
    const QWebEnginePage::Feature feature = QWebEnginePage::DesktopVideoCapture;
    GetUserMediaTestPage page;

    // Desktop capture needs to be on a desktop
    QWebEngineView view;
    view.setPage(&page);
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QTRY_VERIFY_WITH_TIMEOUT(page.loadSucceeded(), 20000);
    page.settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    for (int i = 0; i != 100; ++i) {
        page.jsGetUserMedia(constraints);
        QTRY_VERIFY(page.gotFeatureRequest(feature));
        page.acceptPendingRequest();
        QTRY_VERIFY(page.jsPromiseFulfilled() || page.jsPromiseRejected());
    }
}

void tst_QWebEnginePage::savePage()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();

    connect(page->profile(), &QWebEngineProfile::downloadRequested,
            [] (QWebEngineDownloadItem *item)
    {
        connect(item, &QWebEngineDownloadItem::finished,
                &QTestEventLoop::instance(), &QTestEventLoop::exitLoop, Qt::QueuedConnection);
    });

    const QString urlPrefix = QStringLiteral("data:text/html,<h1>");
    const QString text = QStringLiteral("There is Thingumbob shouting!");
    page->load(QUrl(urlPrefix + text));
    QSignalSpy spyFinished(page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(toPlainTextSync(page), text);

    // Save the loaded page as HTML.
    QTemporaryDir tempDir(QDir::tempPath() + "/tst_qwebengineview-XXXXXX");
    const QString filePath = tempDir.path() + "/thingumbob.html";
    page->save(filePath, QWebEngineDownloadItem::CompleteHtmlSaveFormat);
    QTestEventLoop::instance().enterLoop(10);

    // Load something else.
    page->load(QUrl(urlPrefix + QLatin1String("It's a Snark!")));
    QVERIFY(spyFinished.wait());
    QVERIFY(toPlainTextSync(page) != text);

    // Load the saved page and compare the contents.
    page->load(QUrl::fromLocalFile(filePath));
    QVERIFY(spyFinished.wait());
    QCOMPARE(toPlainTextSync(page), text);
}

void tst_QWebEnginePage::openWindowDefaultSize()
{
    TestPage page;
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));
    QWebEngineView view;
    page.setView(&view);
    view.show();

    page.settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    // Open a default window.
    page.runJavaScript("window.open()");
    QTRY_COMPARE(windowCreatedSpy.count(), 1);
    // Open a too small window.
    evaluateJavaScriptSync(&page, "window.open('','about:blank','width=10,height=10')");
    QTRY_COMPARE(windowCreatedSpy.count(), 2);

    // The number of popups created should be two.
    QCOMPARE(page.createdWindows.size(), 2);

    QRect requestedGeometry = page.createdWindows[0]->requestedGeometry;
    // Check default size has been requested.
    QCOMPARE(requestedGeometry.width(), 0);
    QCOMPARE(requestedGeometry.height(), 0);

    requestedGeometry = page.createdWindows[1]->requestedGeometry;
    // Check minimum size has been requested.
    QCOMPARE(requestedGeometry.width(), 100);
    QCOMPARE(requestedGeometry.height(), 100);
}

void tst_QWebEnginePage::cssMediaTypeGlobalSetting()
{
#if !defined(QWEBENGINESETTINGS_SETCSSMEDIATYPE)
    QSKIP("QWEBENGINESETTINGS_SETCSSMEDIATYPE");
#else
    QString testHtml("<style>@media tv {body{background-color:red;}}@media handheld {body{background-color:green;}}@media screen {body{background-color:blue;}}</style>");
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));

    QWebEngineSettings::globalSettings()->setCSSMediaType("tv");
    // Clear page specific setting to read from global setting
    m_view->page()->settings()->setCSSMediaType(QString());
    m_view->setHtml(testHtml);
    QTRY_COMPARE(loadSpy.count(), 1);
    QVERIFY(evaluateJavaScriptSync(m_view->page(), "window.matchMedia('tv').matches == true").toBool());
    QVERIFY(QWebEngineSettings::globalSettings()->cssMediaType() == "tv");

    QWebEngineSettings::globalSettings()->setCSSMediaType("handheld");
    // Clear page specific setting to read from global setting
    m_view->page()->settings()->setCSSMediaType(QString());
    m_view->setHtml(testHtml);
    QTRY_COMPARE(loadSpy.count(), 2);
    QVERIFY(evaluateJavaScriptSync(m_view->page(), "window.matchMedia('handheld').matches == true").toBool());
    QVERIFY(QWebEngineSettings::globalSettings()->cssMediaType() == "handheld");

    QWebEngineSettings::globalSettings()->setCSSMediaType("screen");
    // Clear page specific setting to read from global setting
    m_view->page()->settings()->setCSSMediaType(QString());
    m_view->setHtml(testHtml);
    QTRY_COMPARE(loadSpy.count(), 3);
    QVERIFY(evaluateJavaScriptSync(m_view->page(), "window.matchMedia('screen').matches == true").toBool());
    QVERIFY(QWebEngineSettings::globalSettings()->cssMediaType() == "screen");
#endif
}

void tst_QWebEnginePage::cssMediaTypePageSetting()
{
#if !defined(QWEBENGINESETTINGS_SETCSSMEDIATYPE)
    QSKIP("QWEBENGINESETTINGS_SETCSSMEDIATYPE");
#else
    QString testHtml("<style>@media tv {body{background-color:red;}}@media handheld {body{background-color:green;}}@media screen {body{background-color:blue;}}</style>");
    QSignalSpy loadSpy(m_view, SIGNAL(loadFinished(bool)));

    m_view->page()->settings()->setCSSMediaType("tv");
    m_view->setHtml(testHtml);
    QTRY_COMPARE(loadSpy.count(), 1);
    QVERIFY(evaluateJavaScriptSync(m_view->page(), "window.matchMedia('tv').matches == true").toBool());
    QVERIFY(m_view->page()->settings()->cssMediaType() == "tv");

    m_view->page()->settings()->setCSSMediaType("handheld");
    m_view->setHtml(testHtml);
    QTRY_COMPARE(loadSpy.count(), 2);
    QVERIFY(evaluateJavaScriptSync(m_view->page(), "window.matchMedia('handheld').matches == true").toBool());
    QVERIFY(m_view->page()->settings()->cssMediaType() == "handheld");

    m_view->page()->settings()->setCSSMediaType("screen");
    m_view->setHtml(testHtml);
    QTRY_COMPARE(loadSpy.count(), 3);
    QVERIFY(evaluateJavaScriptSync(m_view->page(), "window.matchMedia('screen').matches == true").toBool());
    QVERIFY(m_view->page()->settings()->cssMediaType() == "screen");
#endif
}

class JavaScriptCallbackBase
{
public:
    JavaScriptCallbackBase()
    {
        if (watcher)
            QMetaObject::invokeMethod(watcher, "add");
    }

    void operator() (const QVariant &result)
    {
        check(result);
        if (watcher)
            QMetaObject::invokeMethod(watcher, "notify");
    }

protected:
    virtual void check(const QVariant &result) = 0;

private:
    friend class JavaScriptCallbackWatcher;
    static QPointer<QObject> watcher;
};

QPointer<QObject> JavaScriptCallbackBase::watcher = 0;

class JavaScriptCallback : public JavaScriptCallbackBase
{
public:
    JavaScriptCallback() { }
    JavaScriptCallback(const QVariant& _expected) : expected(_expected) { }

    void check(const QVariant& result) Q_DECL_OVERRIDE
    {
        QVERIFY(result.isValid());
        QCOMPARE(result, expected);
    }

private:
    QVariant expected;
};

class JavaScriptCallbackNull : public JavaScriptCallbackBase
{
public:
    void check(const QVariant& result) Q_DECL_OVERRIDE
    {
        QVERIFY(result.isNull());
// FIXME: Returned null values are currently invalid QVariants.
//        QVERIFY(result.isValid());
    }
};

class JavaScriptCallbackUndefined : public JavaScriptCallbackBase
{
public:
    void check(const QVariant& result) Q_DECL_OVERRIDE
    {
        QVERIFY(result.isNull());
        QVERIFY(!result.isValid());
    }
};

class JavaScriptCallbackWatcher : public QObject
{
    Q_OBJECT
public:
    JavaScriptCallbackWatcher()
    {
        Q_ASSERT(!JavaScriptCallbackBase::watcher);
        JavaScriptCallbackBase::watcher = this;
    }

    Q_INVOKABLE void add()
    {
        available++;
    }

    Q_INVOKABLE void notify()
    {
        called++;
        if (called == available)
            emit allCalled();
    }

    bool wait(int maxSeconds = 30)
    {
        if (called == available)
            return true;

        QTestEventLoop loop;
        connect(this, SIGNAL(allCalled()), &loop, SLOT(exitLoop()));
        loop.enterLoop(maxSeconds);
        return !loop.timeout();
    }

signals:
    void allCalled();

private:
    int available = 0;
    int called = 0;
};


void tst_QWebEnginePage::runJavaScript()
{
    TestPage page;
    JavaScriptCallbackWatcher watcher;

    JavaScriptCallback callbackBool(QVariant(false));
    page.runJavaScript("false", QWebEngineCallback<const QVariant&>(callbackBool));

    JavaScriptCallback callbackInt(QVariant(2));
    page.runJavaScript("2", QWebEngineCallback<const QVariant&>(callbackInt));

    JavaScriptCallback callbackDouble(QVariant(2.5));
    page.runJavaScript("2.5", QWebEngineCallback<const QVariant&>(callbackDouble));

    JavaScriptCallback callbackString(QVariant(QStringLiteral("Test")));
    page.runJavaScript("\"Test\"", QWebEngineCallback<const QVariant&>(callbackString));

    QVariantList list;
    JavaScriptCallback callbackList(list);
    page.runJavaScript("[]", QWebEngineCallback<const QVariant&>(callbackList));

    QVariantMap map;
    map.insert(QStringLiteral("test"), QVariant(2));
    JavaScriptCallback callbackMap(map);
    page.runJavaScript("var el = {\"test\": 2}; el", QWebEngineCallback<const QVariant&>(callbackMap));

    JavaScriptCallbackNull callbackNull;
    page.runJavaScript("null", QWebEngineCallback<const QVariant&>(callbackNull));

    JavaScriptCallbackUndefined callbackUndefined;
    page.runJavaScript("undefined", QWebEngineCallback<const QVariant&>(callbackUndefined));

    JavaScriptCallback callbackDate(QVariant(42.0));
    page.runJavaScript("new Date(42000)", QWebEngineCallback<const QVariant&>(callbackDate));

    JavaScriptCallback callbackBlob(QVariant(QByteArray(8, 0)));
    page.runJavaScript("new ArrayBuffer(8)", QWebEngineCallback<const QVariant&>(callbackBlob));

    JavaScriptCallbackUndefined callbackFunction;
    page.runJavaScript("(function(){})", QWebEngineCallback<const QVariant&>(callbackFunction));

    JavaScriptCallback callbackPromise(QVariant(QVariantMap{}));
    page.runJavaScript("new Promise(function(){})", QWebEngineCallback<const QVariant&>(callbackPromise));

    QVERIFY(watcher.wait());
}

void tst_QWebEnginePage::runJavaScriptDisabled()
{
    QWebEnginePage page;
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    // Settings changes take effect asynchronously. The load and wait ensure
    // that the settings are applied by the time we start to execute JavaScript.
    page.load(QStringLiteral("about:blank"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, QStringLiteral("1+1"), QWebEngineScript::MainWorld),
             QVariant());
    QCOMPARE(evaluateJavaScriptSyncInWorld(&page, QStringLiteral("1+1"), QWebEngineScript::ApplicationWorld),
             QVariant(2));
}

void tst_QWebEnginePage::fullScreenRequested()
{
    JavaScriptCallbackWatcher watcher;
    QWebEngineView view;
    QWebEnginePage* page = view.page();
    view.show();

    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    page->load(QUrl("qrc:///resources/fullscreen.html"));
    QTRY_COMPARE(loadSpy.count(), 1);

    page->runJavaScript("document.webkitFullscreenEnabled", JavaScriptCallback(true));
    page->runJavaScript("document.webkitIsFullScreen", JavaScriptCallback(false));
    QVERIFY(watcher.wait());

    // FullscreenRequest must be a user gesture
    bool acceptRequest = true;
    connect(page, &QWebEnginePage::fullScreenRequested,
        [&acceptRequest](QWebEngineFullScreenRequest request) {
        if (acceptRequest) request.accept(); else request.reject();
    });

    QTest::keyPress(view.focusProxy(), Qt::Key_Space);
    QTRY_VERIFY(evaluateJavaScriptSync(page, "document.webkitIsFullScreen").toBool());
    page->runJavaScript("document.webkitExitFullscreen()", JavaScriptCallbackUndefined());
    QVERIFY(watcher.wait());

    acceptRequest = false;

    page->runJavaScript("document.webkitFullscreenEnabled", JavaScriptCallback(true));
    QTest::keyPress(view.focusProxy(), Qt::Key_Space);
    QVERIFY(watcher.wait());
    page->runJavaScript("document.webkitIsFullScreen", JavaScriptCallback(false));
    QVERIFY(watcher.wait());
}

void tst_QWebEnginePage::quotaRequested()
{
    ConsolePage page;
    QWebEngineView view;
    view.setPage(&page);
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl("qrc:///resources/content.html"));
    QVERIFY(loadFinishedSpy.wait());

    connect(&page, &QWebEnginePage::quotaRequested,
            [] (QWebEngineQuotaRequest request)
    {
        if (request.requestedSize() <= 5000)
            request.accept();
        else
            request.reject();
    });

    evaluateJavaScriptSync(&page,
        "navigator.webkitPersistentStorage.requestQuota(1024, function(grantedSize) {" \
            "console.log(grantedSize);" \
        "});");
    QTRY_COMPARE(page.messages.count(), 1);
    QTRY_COMPARE(page.messages[0], QString("1024"));

    evaluateJavaScriptSync(&page,
        "navigator.webkitPersistentStorage.requestQuota(6000, function(grantedSize) {" \
            "console.log(grantedSize);" \
        "});");
    QTRY_COMPARE(page.messages.count(), 2);
    QTRY_COMPARE(page.messages[1], QString("1024"));

    evaluateJavaScriptSync(&page,
        "navigator.webkitPersistentStorage.queryUsageAndQuota(function(usedBytes, grantedBytes) {" \
            "console.log(usedBytes + ', ' + grantedBytes);" \
        "});");
    QTRY_COMPARE(page.messages.count(), 3);
    QTRY_COMPARE(page.messages[2], QString("0, 1024"));
}

void tst_QWebEnginePage::symmetricUrl()
{
    QWebEngineView view;
    QSignalSpy loadFinishedSpy(view.page(), SIGNAL(loadFinished(bool)));

    QVERIFY(view.url().isEmpty());

    QCOMPARE(view.history()->count(), 0);

    QUrl dataUrl("data:text/html,<h1>Test");

    view.setUrl(dataUrl);
    QCOMPARE(view.url(), dataUrl);
    QCOMPARE(view.history()->count(), 0);

    // loading is _not_ immediate, so the text isn't set just yet.
    QVERIFY(toPlainTextSync(view.page()).isEmpty());

    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    QCOMPARE(view.history()->count(), 1);
    QCOMPARE(toPlainTextSync(view.page()), QString("Test"));

    QUrl dataUrl2("data:text/html,<h1>Test2");
    QUrl dataUrl3("data:text/html,<h1>Test3");

    view.setUrl(dataUrl2);
    view.setUrl(dataUrl3);

    QCOMPARE(view.url(), dataUrl3);

    // setUrl(dataUrl3) might override the pending load for dataUrl2. Or not.
    QTRY_VERIFY(loadFinishedSpy.count() >= 2);
    QTRY_VERIFY(loadFinishedSpy.count() <= 3);

    // setUrl(dataUrl3) might stop Chromium from adding a navigation entry for dataUrl2,
    // depending on whether the load of dataUrl2 could be completed in time.
    QVERIFY(view.history()->count() >= 2);
    QVERIFY(view.history()->count() <= 3);

    QCOMPARE(toPlainTextSync(view.page()), QString("Test3"));
}

void tst_QWebEnginePage::progressSignal()
{
    QSignalSpy progressSpy(m_view, SIGNAL(loadProgress(int)));

    QUrl dataUrl("data:text/html,<h1>Test");
    m_view->setUrl(dataUrl);

    QSignalSpy spyFinished(m_view, &QWebEngineView::loadFinished);
    QVERIFY(spyFinished.wait());

    QVERIFY(progressSpy.size() >= 2);
    int previousValue = -1;
    for (QSignalSpy::ConstIterator it = progressSpy.begin(); it < progressSpy.end(); ++it) {
        int current = (*it).first().toInt();
        // verbose output for faulty condition
        if (!(current >= previousValue)) {
            qDebug() << "faulty progress values:";
            for (QSignalSpy::ConstIterator it2 = progressSpy.begin(); it2 < progressSpy.end(); ++it2)
                qDebug() << (*it2).first().toInt();
            QVERIFY(current >= previousValue);
        }
        previousValue = current;
    }

    // But we always end at 100%
    QCOMPARE(progressSpy.last().first().toInt(), 100);
}

void tst_QWebEnginePage::urlChange()
{
    QSignalSpy urlSpy(m_page, &QWebEnginePage::urlChanged);

    QUrl dataUrl("data:text/html,<h1>Test");
    m_view->setUrl(dataUrl);

    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), dataUrl);

    QUrl dataUrl2("data:text/html,<html><head><title>title</title></head><body><h1>Test</body></html>");
    m_view->setUrl(dataUrl2);

    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), dataUrl2);

    QUrl testUrl("http://test.qt.io/");
    m_view->setHtml(QStringLiteral("<h1>Test</h1"), testUrl);

    QTRY_COMPARE(urlSpy.size(), 1);
    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), testUrl);
}

class FakeReply : public QNetworkReply {
    Q_OBJECT

public:
    static const QUrl urlFor404ErrorWithoutContents;

    FakeReply(const QNetworkRequest& request, QObject* parent = 0)
        : QNetworkReply(parent)
    {
        setOperation(QNetworkAccessManager::GetOperation);
        setRequest(request);
        setUrl(request.url());
        if (request.url() == QUrl("qrc:/test1.html")) {
            setHeader(QNetworkRequest::LocationHeader, QString("qrc:/test2.html"));
            setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl("qrc:/test2.html"));
            QTimer::singleShot(0, this, SLOT(continueRedirect()));
        }
#ifndef QT_NO_OPENSSL
        else if (request.url() == QUrl("qrc:/fake-ssl-error.html")) {
            setError(QNetworkReply::SslHandshakeFailedError, tr("Fake error!"));
            QTimer::singleShot(0, this, SLOT(continueError()));
        }
#endif
        else if (request.url().host() == QLatin1String("abcdef.abcdef")) {
            setError(QNetworkReply::HostNotFoundError, tr("Invalid URL"));
            QTimer::singleShot(0, this, SLOT(continueError()));
        } else if (request.url() == FakeReply::urlFor404ErrorWithoutContents) {
            setError(QNetworkReply::ContentNotFoundError, "Not found");
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);
            QTimer::singleShot(0, this, SLOT(continueError()));
        }

        open(QIODevice::ReadOnly);
    }
    ~FakeReply()
    {
        close();
    }
    virtual void abort() {}
    virtual void close() {}

protected:
    qint64 readData(char*, qint64)
    {
        return 0;
    }

private Q_SLOTS:
    void continueRedirect()
    {
        emit metaDataChanged();
        emit finished();
    }

    void continueError()
    {
        emit error(this->error());
        emit finished();
    }
};

const QUrl FakeReply::urlFor404ErrorWithoutContents = QUrl("http://this.will/return-http-404-error-without-contents.html");

class FakeNetworkManager : public QNetworkAccessManager {
    Q_OBJECT

public:
    FakeNetworkManager(QObject* parent) : QNetworkAccessManager(parent) { }

protected:
    virtual QNetworkReply* createRequest(Operation op, const QNetworkRequest& request, QIODevice* outgoingData)
    {
        QString url = request.url().toString();
        if (op == QNetworkAccessManager::GetOperation) {
#ifndef QT_NO_OPENSSL
            if (url == "qrc:/fake-ssl-error.html") {
                FakeReply* reply = new FakeReply(request, this);
                QList<QSslError> errors;
                emit sslErrors(reply, errors << QSslError(QSslError::UnspecifiedError));
                return reply;
            }
#endif
            if (url == "qrc:/test1.html" || url == "http://abcdef.abcdef/" || request.url() == FakeReply::urlFor404ErrorWithoutContents)
                return new FakeReply(request, this);
        }

        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }
};

void tst_QWebEnginePage::requestedUrlAfterSetAndLoadFailures()
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    const QUrl first("http://abcdef.abcdef/");
    page.setUrl(first);
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 20000);
    QCOMPARE(page.url(), first);
    QCOMPARE(page.requestedUrl(), first);
    QVERIFY(!spy.at(0).first().toBool());

    const QUrl second("http://abcdef.abcdef/another_page.html");
    QVERIFY(first != second);

    page.load(second);
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 2, 20000);
    QCOMPARE(page.url(), second);
    QCOMPARE(page.requestedUrl(), second);
    QVERIFY(!spy.at(1).first().toBool());
}

void tst_QWebEnginePage::asyncAndDelete()
{
    QScopedPointer<QWebEnginePage> page(new QWebEnginePage);
    CallbackSpy<QString> plainTextSpy;
    CallbackSpy<QString> htmlSpy;
    page->toPlainText(plainTextSpy.ref());
    page->toHtml(htmlSpy.ref());

    page.reset();
    // Pending callbacks should be called with an empty value in the page's destructor.
    QCOMPARE(plainTextSpy.waitForResult(), QString());
    QVERIFY(plainTextSpy.wasCalled());
    QCOMPARE(htmlSpy.waitForResult(), QString());
    QVERIFY(htmlSpy.wasCalled());
}

void tst_QWebEnginePage::earlyToHtml()
{
    QString html("<html><head></head><body></body></html>");
    QCOMPARE(toHtmlSync(m_view->page()), html);
}

void tst_QWebEnginePage::setHtml()
{
    QString html("<html><head></head><body><p>hello world</p></body></html>");
    QSignalSpy spy(m_view->page(), SIGNAL(loadFinished(bool)));
    m_view->page()->setHtml(html);
    QVERIFY(spy.wait());
    QCOMPARE(toHtmlSync(m_view->page()), html);
}

void tst_QWebEnginePage::setHtmlWithImageResource()
{
    // We allow access to qrc resources from any security origin, including local and anonymous

    QLatin1String html("<html><body><p>hello world</p><img src='qrc:/resources/image.png'/></body></html>");
    QWebEnginePage page;

    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(html, QUrl("file:///path/to/file"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 12000);

    QCOMPARE(evaluateJavaScriptSync(&page, "document.images.length").toInt(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].width").toInt(), 128);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].height").toInt(), 128);

    // Now we test the opposite: without a baseUrl as a local file, we can still request qrc resources.

    page.setHtml(html);
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images.length").toInt(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].width").toInt(), 128);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].height").toInt(), 128);
}

void tst_QWebEnginePage::setHtmlWithStylesheetResource()
{
    const char* htmlData =
        "<html>"
            "<head>"
                "<link rel='stylesheet' href='qrc:/resources/style.css' type='text/css' />"
            "</head>"
            "<body>"
                "<p id='idP'>some text</p>"
            "</body>"
        "</html>";
    QLatin1String html(htmlData);
    QWebEnginePage page;
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);

    // We allow access to qrc resources from any security origin, including local and anonymous
    page.setHtml(html, QUrl("file:///path/to/file"));
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getComputedStyle(document.getElementById('idP')).color").toString(), QString("rgb(255, 0, 0)"));

    page.setHtml(html, QUrl(QLatin1String("qrc:/")));
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getComputedStyle(document.getElementById('idP')).color").toString(), QString("rgb(255, 0, 0)"));

    // Now we test the opposite: without a baseUrl as a local file, we can still request qrc resources.
    page.setHtml(html);
    QVERIFY(spyFinished.wait());
    QCOMPARE(evaluateJavaScriptSync(&page, "window.getComputedStyle(document.getElementById('idP')).color").toString(), QString("rgb(255, 0, 0)"));
}

void tst_QWebEnginePage::setHtmlWithBaseURL()
{
    // This tests if baseUrl is indeed affecting the relative paths from resources.
    // As we are using a local file as baseUrl, its security origin should be able to load local resources.

    if (!QDir(TESTS_SOURCE_DIR).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'").arg(TESTS_SOURCE_DIR).toLatin1().constData(), SkipAll);

    QDir::setCurrent(TESTS_SOURCE_DIR);

    QString html("<html><body><p>hello world</p><img src='resources/image2.png'/></body></html>");

    QWebEnginePage page;

    // in few seconds, the image should be completey loaded
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setHtml(html, QUrl::fromLocalFile(TESTS_SOURCE_DIR));
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(spy.count(), 1);

    QCOMPARE(evaluateJavaScriptSync(&page, "document.images.length").toInt(), 1);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].width").toInt(), 128);
    QCOMPARE(evaluateJavaScriptSync(&page, "document.images[0].height").toInt(), 128);

    // no history item has to be added.
    QCOMPARE(m_view->page()->history()->count(), 0);
}

class MyPage : public QWebEnginePage
{
public:
    MyPage() :  QWebEnginePage(), alerts(0) {}
    int alerts;

protected:
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
    {
        alerts++;
        QCOMPARE(securityOrigin, QUrl(QStringLiteral("http://test.origin.com/")));
        QCOMPARE(msg, QString("foo"));
    }
};

void tst_QWebEnginePage::setHtmlWithJSAlert()
{
    QString html("<html><head></head><body><script>alert('foo');</script><p>hello world</p></body></html>");
    MyPage page;
    page.setHtml(html, QUrl(QStringLiteral("http://test.origin.com/path#fragment")));
    QSignalSpy spyFinished(&page, &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());
    QCOMPARE(page.alerts, 1);
    QCOMPARE(toHtmlSync(&page), html);
}

void tst_QWebEnginePage::inputFieldFocus()
{
#if !defined(QWEBENGINEELEMENT)
    QSKIP("QWEBENGINEELEMENT");
#else
    QWebEngineView view;
    view.setHtml("<html><body><input type=\"text\"></input></body></html>");
    view.resize(400, 100);
    view.show();
    QTest::qWaitForWindowExposed(&view);
    view.activateWindow();
    view.setFocus();
    QTRY_VERIFY(view.hasFocus());

    // double the flashing time, should at least blink once already
    int delay = qApp->cursorFlashTime() * 2;

    // focus the lineedit and check if it blinks
    bool autoSipEnabled = qApp->autoSipEnabled();
    qApp->setAutoSipEnabled(false);
    const QWebEngineElement inputElement = view.page()->documentElement().findFirst(QLatin1String("input[type=text]"));
    QTest::mouseClick(&view, Qt::LeftButton, 0, inputElement.geometry().center());
    m_inputFieldsTestView = &view;
    view.installEventFilter( this );
    QTest::qWait(delay);
    QVERIFY2(m_inputFieldTestPaintCount >= 3,
             "The input field should have a blinking caret");
    qApp->setAutoSipEnabled(autoSipEnabled);
#endif
}

void tst_QWebEnginePage::hitTestContent()
{
#if !defined(QWEBENGINEELEMENT)
    QSKIP("QWEBENGINEELEMENT");
#else
    QString html("<html><body><p>A paragraph</p><br/><br/><br/><a href=\"about:blank\" target=\"_foo\" id=\"link\">link text</a></body></html>");

    QWebEnginePage page;
    page.setHtml(html);
    page.setViewportSize(QSize(200, 0)); //no height so link is not visible
    const QWebEngineElement linkElement = page.documentElement().findFirst(QLatin1String("a#link"));
    QWebEngineHitTestResult result = page.hitTestContent(linkElement.geometry().center());
    QCOMPARE(result.linkText(), QString("link text"));
    QWebEngineElement link = result.linkElement();
    QCOMPARE(link.attribute("target"), QString("_foo"));
#endif
}

void tst_QWebEnginePage::baseUrl_data()
{
    QTest::addColumn<QString>("html");
    QTest::addColumn<QUrl>("loadUrl");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("baseUrl");

    QTest::newRow("null") << QString() << QUrl()
                          << QUrl("about:blank") << QUrl("about:blank");

    QTest::newRow("foo") << QString() << QUrl("http://foobar.baz/")
                         << QUrl("http://foobar.baz/") << QUrl("http://foobar.baz/");

    QString html = "<html>"
        "<head>"
            "<base href=\"http://foobaz.bar/\" />"
        "</head>"
    "</html>";
    QTest::newRow("customBaseUrl") << html << QUrl("http://foobar.baz/")
                                   << QUrl("http://foobar.baz/") << QUrl("http://foobaz.bar/");
}

void tst_QWebEnginePage::baseUrl()
{
    QFETCH(QString, html);
    QFETCH(QUrl, loadUrl);
    QFETCH(QUrl, url);
    QFETCH(QUrl, baseUrl);

    QSignalSpy loadSpy(m_page, SIGNAL(loadFinished(bool)));
    m_page->setHtml(html, loadUrl);
    QTRY_COMPARE(loadSpy.count(), 1);
    QCOMPARE(m_page->url(), url);
    QEXPECT_FAIL("null", "Slight change: We now translate QUrl() to about:blank for the virtual url, but not for the baseUrl", Continue);
    QCOMPARE(baseUrlSync(m_page), baseUrl);
}

void tst_QWebEnginePage::scrollPosition()
{
    // enlarged image in a small viewport, to provoke the scrollbars to appear
    QString html("<html><body><img src='qrc:/image.png' height=500 width=500/></body></html>");

    QWebEngineView view;
    view.setFixedSize(200,200);
    view.show();

    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadSpy(view.page(), SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.count(), 1);

    // try to set the scroll offset programmatically
    view.page()->runJavaScript("window.scrollTo(23, 29);");
    QTRY_COMPARE(view.page()->scrollPosition().x(), 23 * view.windowHandle()->devicePixelRatio());
    QCOMPARE(view.page()->scrollPosition().y(), 29 * view.windowHandle()->devicePixelRatio());

    int x = evaluateJavaScriptSync(view.page(), "window.scrollX").toInt();
    int y = evaluateJavaScriptSync(view.page(), "window.scrollY").toInt();
    QCOMPARE(x, 23);
    QCOMPARE(y, 29);
}

void tst_QWebEnginePage::scrollbarsOff()
{
    QWebEngineView view;
    view.page()->settings()->setAttribute(QWebEngineSettings::ShowScrollBars, false);

    QString html("<html><body>"
                 "   <div style='margin-top:1000px ; margin-left:1000px'>"
                 "       <a id='offscreen' href='a'>End</a>"
                 "   </div>"
                 "</body></html>");

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.count(), 1);
    QVERIFY(evaluateJavaScriptSync(view.page(), "innerWidth == document.documentElement.offsetWidth").toBool());
}

void tst_QWebEnginePage::horizontalScrollAfterBack()
{
#if !defined(QWEBENGINESETTINGS)
    QSKIP("QWEBENGINESETTINGS");
#else
    QWebEngineView view;
    QSignalSpy loadSpy(view.page(), SIGNAL(loadFinished(bool)));

    view.page()->settings()->setMaximumPagesInCache(2);
    view.page()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    view.page()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);

    view.load(QUrl("qrc:/resources/testiframe2.html"));
    view.resize(200, 200);
    QTRY_COMPARE(loadSpy.count(), 1);
    QTRY_VERIFY((view.page()->scrollBarGeometry(Qt::Horizontal)).height());

    view.load(QUrl("qrc:/resources/testiframe.html"));
    QTRY_COMPARE(loadSpy.count(), 2);

    view.page()->triggerAction(QWebEnginePage::Back);
    QTRY_COMPARE(loadSpy.count(), 3);
    QTRY_VERIFY((view.page()->scrollBarGeometry(Qt::Horizontal)).height());
#endif
}

class WebView : public QWebEngineView
{
    Q_OBJECT
signals:
    void repaintRequested();

protected:
    bool event(QEvent *event) {
        if (event->type() == QEvent::UpdateRequest)
            emit repaintRequested();

        return QWebEngineView::event(event);
    }
};

void tst_QWebEnginePage::evaluateWillCauseRepaint()
{
    WebView view;
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QString html("<html><body>"
                 "  top"
                 "  <div id=\"junk\" style=\"display: block;\">junk</div>"
                 "  bottom"
                 "</body></html>");

    QSignalSpy loadSpy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(html);
    QTRY_COMPARE(loadSpy.count(), 1);

    evaluateJavaScriptSync(view.page(), "document.getElementById('junk').style.display = 'none';");
    QSignalSpy repaintSpy(&view, &WebView::repaintRequested);
    QVERIFY(repaintSpy.wait());
}

void tst_QWebEnginePage::setContent_data()
{
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<QByteArray>("testContents");
    QTest::addColumn<QString>("expected");

    QString str = QString::fromUtf8("ὕαλον ϕαγεῖν δύναμαι· τοῦτο οὔ με βλάπτει");
    QTest::newRow("UTF-8 plain text") << "text/plain; charset=utf-8" << str.toUtf8() << str;

    QTextCodec *utf16 = QTextCodec::codecForName("UTF-16");
    if (utf16)
        QTest::newRow("UTF-16 plain text") << "text/plain; charset=utf-16" << utf16->fromUnicode(str) << str;

    str = QString::fromUtf8("Une chaîne de caractères à sa façon.");
    QTest::newRow("latin-1 plain text") << "text/plain; charset=iso-8859-1" << str.toLatin1() << str;


}

void tst_QWebEnginePage::setContent()
{
    QFETCH(QString, mimeType);
    QFETCH(QByteArray, testContents);
    QFETCH(QString, expected);
    QSignalSpy loadSpy(m_page, SIGNAL(loadFinished(bool)));
    m_view->setContent(testContents, mimeType);
    QVERIFY(loadSpy.wait());
    QCOMPARE(toPlainTextSync(m_view->page()), expected);
}

class CacheNetworkAccessManager : public QNetworkAccessManager {
public:
    CacheNetworkAccessManager(QObject* parent = 0)
        : QNetworkAccessManager(parent)
        , m_lastCacheLoad(QNetworkRequest::PreferNetwork)
    {
    }

    virtual QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*)
    {
        QVariant cacheLoad = request.attribute(QNetworkRequest::CacheLoadControlAttribute);
        if (cacheLoad.isValid())
            m_lastCacheLoad = static_cast<QNetworkRequest::CacheLoadControl>(cacheLoad.toUInt());
        else
            m_lastCacheLoad = QNetworkRequest::PreferNetwork; // default value
        return new FakeReply(request, this);
    }

    QNetworkRequest::CacheLoadControl lastCacheLoad() const
    {
        return m_lastCacheLoad;
    }

private:
    QNetworkRequest::CacheLoadControl m_lastCacheLoad;
};

void tst_QWebEnginePage::setCacheLoadControlAttribute()
{
#if !defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
    QSKIP("QWEBENGINEPAGE_SETNETWORKACCESSMANAGER");
#else
    QWebEnginePage page;
    CacheNetworkAccessManager* manager = new CacheNetworkAccessManager(&page);
    page.setNetworkAccessManager(manager);

    QNetworkRequest request(QUrl("http://abcdef.abcdef/"));

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    page.load(request);
    QCOMPARE(manager->lastCacheLoad(), QNetworkRequest::AlwaysCache);

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    page.load(request);
    QCOMPARE(manager->lastCacheLoad(), QNetworkRequest::PreferCache);

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    page.load(request);
    QCOMPARE(manager->lastCacheLoad(), QNetworkRequest::AlwaysNetwork);

    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
    page.load(request);
    QCOMPARE(manager->lastCacheLoad(), QNetworkRequest::PreferNetwork);
#endif
}

void tst_QWebEnginePage::setUrlWithPendingLoads()
{
    QWebEnginePage page;
    page.setHtml("<img src='dummy:'/>");
    page.setUrl(QUrl("about:blank"));
}

void tst_QWebEnginePage::setUrlToEmpty()
{
    int expectedLoadFinishedCount = 0;
    const QUrl aboutBlank("about:blank");
    const QUrl url("qrc:/resources/test2.html");

    QWebEnginePage page;
    QCOMPARE(page.url(), QUrl());
    QCOMPARE(page.requestedUrl(), QUrl());
// Chromium now returns about:blank as the base url here:
//     QCOMPARE(baseUrlSync(&page), QUrl());

    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    // Set existing url
    page.setUrl(url);
    expectedLoadFinishedCount++;
    QVERIFY(spy.wait());

    QCOMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.requestedUrl(), url);
    QCOMPARE(baseUrlSync(&page), url);

    // Set empty url
    page.setUrl(QUrl());
    expectedLoadFinishedCount++;

    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), aboutBlank);
    QCOMPARE(page.requestedUrl(), QUrl());
    QCOMPARE(baseUrlSync(&page), aboutBlank);

    // Set existing url
    page.setUrl(url);
    expectedLoadFinishedCount++;

    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.requestedUrl(), url);
    QCOMPARE(baseUrlSync(&page), url);

    // Load empty url
    page.load(QUrl());
    expectedLoadFinishedCount++;

    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(page.url(), aboutBlank);
    QCOMPARE(page.requestedUrl(), QUrl());
    QCOMPARE(baseUrlSync(&page), aboutBlank);
}

void tst_QWebEnginePage::setUrlToInvalid()
{
    QEXPECT_FAIL("", "Unsupported: QtWebEngine doesn't adjust invalid URLs.", Abort);
    QVERIFY(false);

    QWebEnginePage page;

    const QUrl invalidUrl("http:/example.com");
    QVERIFY(!invalidUrl.isEmpty());
    QVERIFY(invalidUrl != QUrl());

    // QWebEnginePage will do its best to accept the URL, possible converting it to a valid equivalent URL.
    const QUrl validUrl("http://example.com/");
    page.setUrl(invalidUrl);
    QCOMPARE(page.url(), validUrl);
    QCOMPARE(page.requestedUrl(), validUrl);
    QCOMPARE(baseUrlSync(&page), validUrl);

    // QUrls equivalent to QUrl() will be treated as such.
    const QUrl aboutBlank("about:blank");
    const QUrl anotherInvalidUrl("1http://bugs.webkit.org");
    QVERIFY(!anotherInvalidUrl.isEmpty()); // and they are not necessarily empty.
    QVERIFY(!anotherInvalidUrl.isValid());
    QCOMPARE(anotherInvalidUrl.toEncoded(), QUrl().toEncoded());

    page.setUrl(anotherInvalidUrl);
    QCOMPARE(page.url(), aboutBlank);
    QCOMPARE(page.requestedUrl().toEncoded(), anotherInvalidUrl.toEncoded());
    QCOMPARE(baseUrlSync(&page), aboutBlank);
}

void tst_QWebEnginePage::setUrlToBadDomain()
{
    // Failing to load a URL should still emit a urlChanged signal.
    //
    // This test is based on the scenario in QTBUG-48995 where the second setUrl
    // call first triggers an unexpected additional urlChanged signal with the
    // original url before the expected signal with the new url.

    // RFC 2606 says the .invalid TLD should be invalid.
    const QUrl url1 = QStringLiteral("http://this.is.definitely.invalid/");
    const QUrl url2 = QStringLiteral("http://this.is.also.invalid/");
    QWebEnginePage page;
    QSignalSpy urlSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleSpy(&page, &QWebEnginePage::titleChanged);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.setUrl(url1);

    QTRY_COMPARE(urlSpy.count(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(titleSpy.count(), 1, 20000);
    QTRY_COMPARE(loadSpy.count(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url1);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url1.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url1);
    QCOMPARE(page.title(), url1.host());

    page.setUrl(url2);

    QTRY_COMPARE(urlSpy.count(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(titleSpy.count(), 1, 20000);
    QTRY_COMPARE(loadSpy.count(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url2);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url2.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url2);
    QCOMPARE(page.title(), url2.host());
}

void tst_QWebEnginePage::setUrlToBadPort()
{
    // Failing to load a URL should still emit a urlChanged signal.

    // Ports 244-245 are hopefully unbound (marked unassigned in RFC1700).
    const QUrl url1 = QStringLiteral("http://127.0.0.1:244/");
    const QUrl url2 = QStringLiteral("http://127.0.0.1:245/");
    QWebEnginePage page;
    QSignalSpy urlSpy(&page, &QWebEnginePage::urlChanged);
    QSignalSpy titleSpy(&page, &QWebEnginePage::titleChanged);
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);

    page.setUrl(url1);

    QTRY_COMPARE(urlSpy.count(), 1);
    QTRY_COMPARE(titleSpy.count(), 2);
    QTRY_COMPARE(loadSpy.count(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url1);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url1.authority());
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url1.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url1);
    QCOMPARE(page.title(), url1.host());

    page.setUrl(url2);

    QTRY_COMPARE(urlSpy.count(), 1);
    QTRY_COMPARE(titleSpy.count(), 2);
    QTRY_COMPARE(loadSpy.count(), 1);

    QCOMPARE(urlSpy.takeFirst().value(0).toUrl(), url2);
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url2.authority());
    QCOMPARE(titleSpy.takeFirst().value(0).toString(), url2.host());
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), false);

    QCOMPARE(page.url(), url2);
    QCOMPARE(page.title(), url2.host());
}

static QStringList collectHistoryUrls(QWebEngineHistory *history)
{
    QStringList urls;
    const QList<QWebEngineHistoryItem> items = history->items();
    for (const QWebEngineHistoryItem &i : items)
        urls << i.url().toString();
    return urls;
}

void tst_QWebEnginePage::setUrlHistory()
{
    const QUrl aboutBlank("about:blank");
    QUrl url;
    int expectedLoadFinishedCount = 0;
    QSignalSpy spy(m_page, SIGNAL(loadFinished(bool)));

    QCOMPARE(m_page->history()->count(), 0);

    m_page->setUrl(QUrl());
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), aboutBlank);
    QCOMPARE(m_page->requestedUrl(), QUrl());
    // Chromium stores navigation entry for every successful loads. The load of the empty page is committed and stored as about:blank.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList() << aboutBlank.toString());

    url = QUrl("http://url.invalid/");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), expectedLoadFinishedCount, 20000);
    // When error page is disabled in case of LoadFail the entry of the unavailable page is not stored.
    // We expect the url of the previously loaded page here.
    QCOMPARE(m_page->url(), aboutBlank);
    QCOMPARE(m_page->requestedUrl(), QUrl());
    // Since the entry of the unavailable page is not stored it will not available in the history.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList() << aboutBlank.toString());

    url = QUrl("qrc:/resources/test1.html");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList() << aboutBlank.toString() << QStringLiteral("qrc:/resources/test1.html"));

    m_page->setUrl(QUrl());
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), aboutBlank);
    QCOMPARE(m_page->requestedUrl(), QUrl());
    // Chromium stores navigation entry for every successful loads. The load of the empty page is committed and stored as about:blank.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList()
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << aboutBlank.toString());

    url = QUrl("qrc:/resources/test1.html");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    // The history count DOES change since the about:blank is in the list.
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList()
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html"));

    url = QUrl("qrc:/resources/test2.html");
    m_page->setUrl(url);
    expectedLoadFinishedCount++;
    QTRY_COMPARE(spy.count(), expectedLoadFinishedCount);
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    QCOMPARE(collectHistoryUrls(m_page->history()), QStringList()
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << aboutBlank.toString()
                                                        << QStringLiteral("qrc:/resources/test1.html")
                                                        << QStringLiteral("qrc:/resources/test2.html"));
}

void tst_QWebEnginePage::setUrlUsingStateObject()
{
    QUrl url;
    QSignalSpy urlChangedSpy(m_page, SIGNAL(urlChanged(QUrl)));
    int expectedUrlChangeCount = 0;

    QCOMPARE(m_page->history()->count(), 0);

    url = QUrl("qrc:/resources/test1.html");
    m_page->setUrl(url);
    expectedUrlChangeCount++;
    QTRY_COMPARE(urlChangedSpy.count(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), url);
    QTRY_COMPARE(m_page->history()->count(), 1);

    evaluateJavaScriptSync(m_page, "window.history.pushState(null, 'push', 'navigate/to/here')");
    expectedUrlChangeCount++;
    QCOMPARE(urlChangedSpy.count(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), QUrl("qrc:/resources/navigate/to/here"));
    QCOMPARE(m_page->history()->count(), 2);
    QVERIFY(m_page->history()->canGoBack());

    evaluateJavaScriptSync(m_page, "window.history.replaceState(null, 'replace', 'another/location')");
    expectedUrlChangeCount++;
    QCOMPARE(urlChangedSpy.count(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), QUrl("qrc:/resources/navigate/to/another/location"));
    QCOMPARE(m_page->history()->count(), 2);
    QVERIFY(!m_page->history()->canGoForward());
    QVERIFY(m_page->history()->canGoBack());

    evaluateJavaScriptSync(m_page, "window.history.back()");
    expectedUrlChangeCount++;
    QTRY_COMPARE(urlChangedSpy.count(), expectedUrlChangeCount);
    QCOMPARE(m_page->url(), QUrl("qrc:/resources/test1.html"));
    QVERIFY(m_page->history()->canGoForward());
    QVERIFY(!m_page->history()->canGoBack());
}

static inline QUrl extractBaseUrl(const QUrl& url)
{
    return url.resolved(QUrl());
}

void tst_QWebEnginePage::setUrlThenLoads_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("baseUrl");

    QTest::newRow("resource file") << QUrl("qrc:/resources/test1.html") << extractBaseUrl(QUrl("qrc:/resources/test1.html"));
    QTest::newRow("base specified in HTML") << QUrl("data:text/html,<head><base href=\"http://different.base/\"></head>") << QUrl("http://different.base/");
}

void tst_QWebEnginePage::setUrlThenLoads()
{
    QFETCH(QUrl, url);
    QFETCH(QUrl, baseUrl);
    QSignalSpy urlChangedSpy(m_page, SIGNAL(urlChanged(QUrl)));
    QSignalSpy startedSpy(m_page, SIGNAL(loadStarted()));
    QSignalSpy finishedSpy(m_page, SIGNAL(loadFinished(bool)));

    m_page->setUrl(url);
    QTRY_COMPARE(startedSpy.count(), 1);
    QTRY_COMPARE(urlChangedSpy.count(), 1);
    QTRY_COMPARE(finishedSpy.count(), 1);
    QVERIFY(finishedSpy.at(0).first().toBool());
    QCOMPARE(m_page->url(), url);
    QCOMPARE(m_page->requestedUrl(), url);
    QCOMPARE(baseUrlSync(m_page), baseUrl);

    const QUrl urlToLoad1("qrc:/resources/test2.html");
    const QUrl urlToLoad2("qrc:/resources/test1.html");

    m_page->load(urlToLoad1);
    QCOMPARE(m_page->url(), urlToLoad1);
    QCOMPARE(m_page->requestedUrl(), urlToLoad1);
    // baseUrlSync spins an event loop and this sometimes return the next result.
    // QCOMPARE(baseUrlSync(m_page), baseUrl);
    QTRY_COMPARE(startedSpy.count(), 2);

    // After first URL changed.
    QTRY_COMPARE(urlChangedSpy.count(), 2);
    QTRY_COMPARE(finishedSpy.count(), 2);
    QVERIFY(finishedSpy.at(1).first().toBool());
    QCOMPARE(m_page->url(), urlToLoad1);
    QCOMPARE(m_page->requestedUrl(), urlToLoad1);
    QCOMPARE(baseUrlSync(m_page), extractBaseUrl(urlToLoad1));

    m_page->load(urlToLoad2);
    QCOMPARE(m_page->url(), urlToLoad2);
    QCOMPARE(m_page->requestedUrl(), urlToLoad2);
    QCOMPARE(baseUrlSync(m_page), extractBaseUrl(urlToLoad1));
    QTRY_COMPARE(startedSpy.count(), 3);

    // After second URL changed.
    QTRY_COMPARE(urlChangedSpy.count(), 3);
    QTRY_COMPARE(finishedSpy.count(), 3);
    QVERIFY(finishedSpy.at(2).first().toBool());
    QCOMPARE(m_page->url(), urlToLoad2);
    QCOMPARE(m_page->requestedUrl(), urlToLoad2);
    QCOMPARE(baseUrlSync(m_page), extractBaseUrl(urlToLoad2));
}

void tst_QWebEnginePage::loadFinishedAfterNotFoundError()
{
    QWebEnginePage page;
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    page.setUrl(QUrl("http://non.existent/url"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 20000);

    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    page.setUrl(QUrl("http://another.non.existent/url"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 2, 20000);
}

class URLSetter : public QObject {
    Q_OBJECT

public:
    enum Signal {
        LoadStarted,
        LoadFinished,
    };

    enum Type {
        UseLoad,
        UseSetUrl
    };

    URLSetter(QWebEnginePage*, Signal, Type, const QUrl&);

public Q_SLOTS:
    void execute();

Q_SIGNALS:
    void finished();

private:
    QWebEnginePage* m_page;
    QUrl m_url;
    Type m_type;
};

Q_DECLARE_METATYPE(URLSetter::Signal)
Q_DECLARE_METATYPE(URLSetter::Type)

URLSetter::URLSetter(QWebEnginePage* page, Signal signal, URLSetter::Type type, const QUrl& url)
    : m_page(page), m_url(url), m_type(type)
{
    if (signal == LoadStarted)
        connect(m_page, SIGNAL(loadStarted()), SLOT(execute()));
    else if (signal == LoadFinished)
        connect(m_page, SIGNAL(loadFinished(bool)), SLOT(execute()));
}

void URLSetter::execute()
{
    // We track only the first emission.
    m_page->disconnect(this);
    connect(m_page, SIGNAL(loadFinished(bool)), SIGNAL(finished()));
    if (m_type == URLSetter::UseLoad)
        m_page->load(m_url);
    else
        m_page->setUrl(m_url);
}

void tst_QWebEnginePage::loadInSignalHandlers_data()
{
    QTest::addColumn<URLSetter::Type>("type");
    QTest::addColumn<URLSetter::Signal>("signal");
    QTest::addColumn<QUrl>("url");

    const QUrl validUrl("qrc:/resources/test2.html");
    const QUrl invalidUrl("qrc:/invalid");

    QTest::newRow("call load() in loadStarted() after valid url") << URLSetter::UseLoad << URLSetter::LoadStarted << validUrl;
    QTest::newRow("call load() in loadStarted() after invalid url") << URLSetter::UseLoad << URLSetter::LoadStarted << invalidUrl;
    QTest::newRow("call load() in loadFinished() after valid url") << URLSetter::UseLoad << URLSetter::LoadFinished << validUrl;
    QTest::newRow("call load() in loadFinished() after invalid url") << URLSetter::UseLoad << URLSetter::LoadFinished << invalidUrl;

    QTest::newRow("call setUrl() in loadStarted() after valid url") << URLSetter::UseSetUrl << URLSetter::LoadStarted << validUrl;
    QTest::newRow("call setUrl() in loadStarted() after invalid url") << URLSetter::UseSetUrl << URLSetter::LoadStarted << invalidUrl;
    QTest::newRow("call setUrl() in loadFinished() after valid url") << URLSetter::UseSetUrl << URLSetter::LoadFinished << validUrl;
    QTest::newRow("call setUrl() in loadFinished() after invalid url") << URLSetter::UseSetUrl << URLSetter::LoadFinished << invalidUrl;
}

void tst_QWebEnginePage::loadInSignalHandlers()
{
    QFETCH(URLSetter::Type, type);
    QFETCH(URLSetter::Signal, signal);
    QFETCH(QUrl, url);

    const QUrl urlForSetter("qrc:/resources/test1.html");
    URLSetter setter(m_page, signal, type, urlForSetter);
    QSignalSpy spy(&setter, &URLSetter::finished);
    m_page->load(url);
    // every loadStarted() call should have also loadFinished()
    if (signal == URLSetter::LoadStarted)
        QTRY_COMPARE(spy.count(), 2);
    else
        QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(m_page->url(), urlForSetter);
}

void tst_QWebEnginePage::loadFromQrc()
{
    QWebEnginePage page;
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);

    // Standard case.
    page.load(QStringLiteral("qrc:///resources/foo.txt"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("foo\n"));

    // Query and fragment parts are ignored.
    page.load(QStringLiteral("qrc:///resources/bar.txt?foo=1#bar"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("bar\n"));

    // Literal spaces are OK.
    page.load(QStringLiteral("qrc:///resources/path with spaces.txt"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("contents with spaces\n"));

    // Escaped spaces are OK too.
    page.load(QStringLiteral("qrc:///resources/path%20with%20spaces.txt"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), true);
    QCOMPARE(toPlainTextSync(&page), QStringLiteral("contents with spaces\n"));

    // Resource not found, loading fails.
    page.load(QStringLiteral("qrc:///nope"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().value(0).toBool(), false);
}

#if QT_CONFIG(webengine_webchannel)
void tst_QWebEnginePage::restoreHistory()
{
    QWebChannel channel;
    QWebEnginePage page;
    page.setWebChannel(&channel);

    QWebEngineScript script;
    script.setName(QStringLiteral("script"));
    page.scripts().insert(script);

    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    page.load(QUrl(QStringLiteral("qrc:/resources/test1.html")));
    QTRY_COMPARE(spy.count(), 1);

    QCOMPARE(page.webChannel(), &channel);
    QVERIFY(page.scripts().contains(script));

    QByteArray data;
    QDataStream out(&data, QIODevice::ReadWrite);
    out << *page.history();
    QDataStream in(&data, QIODevice::ReadOnly);
    in >> *page.history();
    QTRY_COMPARE(spy.count(), 2);

    QCOMPARE(page.webChannel(), &channel);
    QVERIFY(page.scripts().contains(script));
}
#endif

void tst_QWebEnginePage::toPlainTextLoadFinishedRace_data()
{
    QTest::addColumn<bool>("enableErrorPage");
    QTest::newRow("disableErrorPage") << false;
    QTest::newRow("enableErrorPage") << true;
}

void tst_QWebEnginePage::toPlainTextLoadFinishedRace()
{
    QFETCH(bool, enableErrorPage);

    QScopedPointer<QWebEnginePage> page(new QWebEnginePage);
    page->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, enableErrorPage);
    QSignalSpy spy(page.data(), SIGNAL(loadFinished(bool)));

    page->load(QUrl("data:text/plain,foobarbaz"));
    QTRY_VERIFY(spy.count() == 1);
    QCOMPARE(toPlainTextSync(page.data()), QString("foobarbaz"));

    page->load(QUrl("http://fail.invalid/"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 2, 20000);
    QString s = toPlainTextSync(page.data());
    QVERIFY(s.contains("foobarbaz") == !enableErrorPage);

    page->load(QUrl("data:text/plain,lalala"));
    QTRY_COMPARE(spy.count(), 3);
    QTRY_COMPARE(toPlainTextSync(page.data()), QString("lalala"));
    page.reset();
    QCOMPARE(spy.count(), 3);
}

void tst_QWebEnginePage::setZoomFactor()
{
    QWebEnginePage page;

    QVERIFY(qFuzzyCompare(page.zoomFactor(), 1.0));
    page.setZoomFactor(2.5);
    QVERIFY(qFuzzyCompare(page.zoomFactor(), 2.5));

    const QUrl urlToLoad("qrc:/resources/test1.html");

    QSignalSpy finishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.load(urlToLoad);
    QTRY_COMPARE(finishedSpy.count(), 1);
    QVERIFY(finishedSpy.at(0).first().toBool());
    QVERIFY(qFuzzyCompare(page.zoomFactor(), 2.5));

    page.setZoomFactor(5.5);
    QVERIFY(qFuzzyCompare(page.zoomFactor(), 2.5));

    page.setZoomFactor(0.1);
    QVERIFY(qFuzzyCompare(page.zoomFactor(), 2.5));
}

void tst_QWebEnginePage::mouseButtonTranslation()
{
    QWebEngineView view;

    QSignalSpy spy(&view, SIGNAL(loadFinished(bool)));
    view.setHtml(QStringLiteral(
                      "<html><head><script>\
                           var lastEvent = { 'button' : -1 }; \
                           function saveLastEvent(event) { console.log(event); lastEvent = event; }; \
                      </script></head>\
                      <body>\
                      <div style=\"height:600px;\" onmousedown=\"saveLastEvent(event)\">\
                      </div>\
                      </body></html>"));
    view.resize(640, 480);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QTRY_VERIFY(spy.count() == 1);

    QVERIFY(view.focusProxy() != nullptr);

    QMouseEvent evpres(QEvent::MouseButtonPress, view.rect().center(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QGuiApplication::sendEvent(view.focusProxy(), &evpres);

    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.button").toInt(), 0);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.buttons").toInt(), 1);

    QMouseEvent evpres2(QEvent::MouseButtonPress, view.rect().center(), Qt::RightButton, Qt::LeftButton | Qt::RightButton, Qt::NoModifier);
    QGuiApplication::sendEvent(view.focusProxy(), &evpres2);

    QTRY_COMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.button").toInt(), 2);
    QCOMPARE(evaluateJavaScriptSync(view.page(), "lastEvent.buttons").toInt(), 3);
}

void tst_QWebEnginePage::mouseMovementProperties()
{
    QWebEngineView view;
    ConsolePage page;
    view.setPage(&page);
    view.resize(640, 480);
    QTest::mouseMove(&view, QPoint(10, 10));
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    page.setHtml(QStringLiteral(
                        "<html><head><script>\
                            function onMouseMove(event) { console.log(event.movementX + \", \" + event.movementY); }; \
                        </script></head>\
                        <body>\
                            <div style=\"height:600px;\" onmousemove=\"onMouseMove(event)\">\
                        </div>\
                        </body></html>"));
    loadFinishedSpy.wait();

    QTest::mouseMove(&view, QPoint(20, 20));
    QTRY_COMPARE(page.messages.count(), 1);

    QTest::mouseMove(&view, QPoint(30, 30));
    QTRY_COMPARE(page.messages.count(), 2);
    QTRY_COMPARE(page.messages[1], QString("10, 10"));

    QTest::mouseMove(&view, QPoint(20, 20));
    QTRY_COMPARE(page.messages.count(), 3);
    QTRY_COMPARE(page.messages[2], QString("-10, -10"));
}

QPoint tst_QWebEnginePage::elementCenter(QWebEnginePage *page, const QString &id)
{
    QVariantList rectList = evaluateJavaScriptSync(page,
            "(function(){"
            "var elem = document.getElementById('" + id + "');"
            "var rect = elem.getBoundingClientRect();"
            "return [(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2];"
            "})()").toList();

    if (rectList.count() != 2) {
        qWarning("elementCenter failed.");
        return QPoint();
    }

    return QPoint(rectList.at(0).toInt(), rectList.at(1).toInt());
}

void tst_QWebEnginePage::viewSource()
{
    TestPage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));
    const QUrl url("qrc:/resources/test1.html");

    page.load(url);
    QTRY_COMPARE(loadFinishedSpy.count(), 1);
    QCOMPARE(page.title(), QStringLiteral("Test page 1"));
    QVERIFY(page.action(QWebEnginePage::ViewSource)->isEnabled());

    page.triggerAction(QWebEnginePage::ViewSource);
    QTRY_COMPARE(windowCreatedSpy.count(), 1);
    QCOMPARE(page.createdWindows.size(), 1);

    QTRY_COMPARE(page.createdWindows[0]->url().toString(), QStringLiteral("view-source:%1").arg(url.toString()));
    // The requested URL should not be about:blank if the qrc scheme is supported
    QTRY_COMPARE(page.createdWindows[0]->requestedUrl(), url);
    QTRY_COMPARE(page.createdWindows[0]->title(), QStringLiteral("view-source:%1").arg(url.toString()));
    QVERIFY(!page.createdWindows[0]->action(QWebEnginePage::ViewSource)->isEnabled());
}

void tst_QWebEnginePage::viewSourceURL_data()
{
    QTest::addColumn<QUrl>("userInputUrl");
    QTest::addColumn<bool>("loadSucceed");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QUrl>("requestedUrl");
    QTest::addColumn<QString>("title");

    QTest::newRow("view-source:") << QUrl("view-source:") << true << QUrl("view-source:") << QUrl("about:blank") << QString("view-source:");
    QTest::newRow("view-source:about:blank") << QUrl("view-source:about:blank") << true << QUrl("view-source:about:blank") << QUrl("about:blank") << QString("view-source:about:blank");

    QString localFilePath = QString("%1qwebenginepage/resources/test1.html").arg(TESTS_SOURCE_DIR);
    QUrl testLocalUrl = QUrl(QString("view-source:%1").arg(QUrl::fromLocalFile(localFilePath).toString()));
    QUrl testLocalUrlWithoutScheme = QUrl(QString("view-source:%1").arg(localFilePath));
    QTest::newRow(testLocalUrl.toString().toStdString().c_str()) << testLocalUrl << true << testLocalUrl << QUrl::fromLocalFile(localFilePath) << QString("test1.html");
    QTest::newRow(testLocalUrlWithoutScheme.toString().toStdString().c_str()) << testLocalUrlWithoutScheme << true << testLocalUrl << QUrl::fromLocalFile(localFilePath) << QString("test1.html");

    QString resourcePath = QLatin1String("qrc:/resources/test1.html");
    QUrl testResourceUrl = QUrl(QString("view-source:%1").arg(resourcePath));
    QTest::newRow(testResourceUrl.toString().toStdString().c_str()) << testResourceUrl << true << testResourceUrl << QUrl(resourcePath) << testResourceUrl.toString();

    QTest::newRow("view-source:http://non.existent") << QUrl("view-source:non.existent") << false << QUrl("view-source:http://non.existent/") << QUrl("http://non.existent/") << QString("non.existent");
    QTest::newRow("view-source:non.existent") << QUrl("view-source:non.existent") << false << QUrl("view-source:http://non.existent/") << QUrl("http://non.existent/") << QString("non.existent");
}

void tst_QWebEnginePage::viewSourceURL()
{
    if (!QDir(TESTS_SOURCE_DIR).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'").arg(TESTS_SOURCE_DIR).toLatin1().constData(), SkipAll);

    QFETCH(QUrl, userInputUrl);
    QFETCH(bool, loadSucceed);
    QFETCH(QUrl, url);
    QFETCH(QUrl, requestedUrl);
    QFETCH(QString, title);

    QWebEnginePage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(userInputUrl);
    QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.count(), 1, 12000);
    QList<QVariant> arguments = loadFinishedSpy.takeFirst();

    QCOMPARE(arguments.at(0).toBool(), loadSucceed);
    QCOMPARE(page.url(), url);
    QCOMPARE(page.requestedUrl(), requestedUrl);
    QCOMPARE(page.title(), title);
    QVERIFY(!page.action(QWebEnginePage::ViewSource)->isEnabled());
}

void tst_QWebEnginePage::viewSourceCredentials()
{
    TestPage page;
    QSignalSpy loadFinishedSpy(&page, SIGNAL(loadFinished(bool)));
    QSignalSpy windowCreatedSpy(&page, SIGNAL(windowCreated()));
    QUrl url("http://user:passwd@httpbin.org/basic-auth/user/passwd");

    // Test explicit view-source URL with credentials
    page.load(QUrl(QString("view-source:" + url.toString())));
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    QCOMPARE(page.url().toString(), QString("view-source:" + url.toDisplayString(QUrl::RemoveUserInfo)));
    QCOMPARE(page.requestedUrl(), url);
    QCOMPARE(page.title(), QString("view-source:" + url.toDisplayString(QUrl::RemoveScheme | QUrl::RemoveUserInfo).remove(0, 2)));
    loadFinishedSpy.clear();
    windowCreatedSpy.clear();

    // Test ViewSource web action on URL with credentials
    page.load(url);
    if (!loadFinishedSpy.wait(10000) || !loadFinishedSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");
    QVERIFY(page.action(QWebEnginePage::ViewSource)->isEnabled());

    page.triggerAction(QWebEnginePage::ViewSource);
    QTRY_COMPARE(windowCreatedSpy.count(), 1);
    QCOMPARE(page.createdWindows.size(), 1);

    QTRY_COMPARE(page.createdWindows[0]->url().toString(), QString("view-source:" + url.toDisplayString(QUrl::RemoveUserInfo)));
    QTRY_COMPARE(page.createdWindows[0]->requestedUrl(), url);
    QTRY_COMPARE(page.createdWindows[0]->title(), QString("view-source:" + url.toDisplayString(QUrl::RemoveScheme | QUrl::RemoveUserInfo).remove(0, 2)));
}

Q_DECLARE_METATYPE(QNetworkProxy::ProxyType);

void tst_QWebEnginePage::proxyConfigWithUnexpectedHostPortPair()
{
    // Chromium expects a proxy of type NoProxy to not have a host or port set.

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    proxy.setHostName(QStringLiteral("127.0.0.1"));
    proxy.setPort(244);
    QNetworkProxy::setApplicationProxy(proxy);

    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));
    m_page->load(QStringLiteral("http://127.0.0.1:245/"));
    QTRY_COMPARE(loadFinishedSpy.count(), 1);
}

void tst_QWebEnginePage::registerProtocolHandler_data()
{
    QTest::addColumn<bool>("permission");
    QTest::newRow("accept") << true;
    QTest::newRow("reject") << false;
}

void tst_QWebEnginePage::registerProtocolHandler()
{
    QFETCH(bool, permission);

    HttpServer server;
    int mailRequestCount = 0;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        if (rr->requestMethod() == "GET" && rr->requestPath() == "/") {
            rr->setResponseBody(QByteArrayLiteral("<html><body><a id=\"link\" href=\"mailto:foo@bar.com\">some text here</a></body></html>"));
            rr->sendResponse();
        } else if (rr->requestMethod() == "GET" && rr->requestPath() == "/mail?uri=mailto%3Afoo%40bar.com") {
            mailRequestCount++;
            rr->sendResponse();
        } else {
            rr->setResponseStatus(404);
            rr->sendResponse();
        }
    });
    QVERIFY(server.start());

    QWebEnginePage page;
    QSignalSpy loadSpy(&page, &QWebEnginePage::loadFinished);
    QSignalSpy permissionSpy(&page, &QWebEnginePage::registerProtocolHandlerRequested);

    page.setUrl(server.url("/"));
    QTRY_COMPARE(loadSpy.count(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), true);

    QString callFormat = QStringLiteral("window.navigator.registerProtocolHandler(\"%1\", \"%2\", \"%3\")");
    QString scheme = QStringLiteral("mailto");
    QString url = server.url("/mail").toString() + QStringLiteral("?uri=%s");
    QString title;
    QString call = callFormat.arg(scheme).arg(url).arg(title);
    page.runJavaScript(call);

    QTRY_COMPARE(permissionSpy.count(), 1);
    auto request = permissionSpy.takeFirst().value(0).value<QWebEngineRegisterProtocolHandlerRequest>();
    QCOMPARE(request.origin(), QUrl(url));
    QCOMPARE(request.scheme(), scheme);
    if (permission)
        request.accept();
    else
        request.reject();

    page.runJavaScript(QStringLiteral("document.getElementById(\"link\").click()"));

    QTRY_COMPARE(loadSpy.count(), 1);
    QCOMPARE(loadSpy.takeFirst().value(0).toBool(), permission);
    QCOMPARE(mailRequestCount, permission ? 1 : 0);
    QVERIFY(server.stop());
}

void tst_QWebEnginePage::dataURLFragment()
{
    m_view->resize(800, 600);
    m_view->show();
    QSignalSpy loadFinishedSpy(m_page, SIGNAL(loadFinished(bool)));

    m_page->setHtml("<html><body>"
                    "<a id='link' href='#anchor'>anchor</a>"
                    "</body></html>");
    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    QSignalSpy urlChangedSpy(m_page, SIGNAL(urlChanged(QUrl)));
    QTest::mouseClick(m_view->focusProxy(), Qt::LeftButton, 0, elementCenter(m_page, "link"));
    QVERIFY(urlChangedSpy.wait());
    QCOMPARE(m_page->url().fragment(), QStringLiteral("anchor"));


    m_page->setHtml("<html><body>"
                    "<a id='link' href='#anchor'>anchor</a>"
                    "</body></html>", QUrl("http://test.qt.io/mytest.html"));
    QTRY_COMPARE(loadFinishedSpy.count(), 2);

    QTest::mouseClick(m_view->focusProxy(), Qt::LeftButton, 0, elementCenter(m_page, "link"));
    QVERIFY(urlChangedSpy.wait());
    QCOMPARE(m_page->url(), QUrl("http://test.qt.io/mytest.html#anchor"));
}

void tst_QWebEnginePage::devTools()
{
    QWebEngineProfile profile;
    QWebEnginePage inspectedPage1(&profile);
    QWebEnginePage inspectedPage2(&profile);
    QWebEnginePage devToolsPage(&profile);
    QSignalSpy spy(&devToolsPage, &QWebEnginePage::loadFinished);

    inspectedPage1.setDevToolsPage(&devToolsPage);

    QCOMPARE(inspectedPage1.devToolsPage(), &devToolsPage);
    QCOMPARE(inspectedPage1.inspectedPage(), nullptr);
    QCOMPARE(inspectedPage2.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage2.inspectedPage(), nullptr);
    QCOMPARE(devToolsPage.devToolsPage(), nullptr);
    QCOMPARE(devToolsPage.inspectedPage(), &inspectedPage1);

    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().value(0).toBool());

    devToolsPage.setInspectedPage(&inspectedPage2);

    QCOMPARE(inspectedPage1.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage1.inspectedPage(), nullptr);
    QCOMPARE(inspectedPage2.devToolsPage(), &devToolsPage);
    QCOMPARE(inspectedPage2.inspectedPage(), nullptr);
    QCOMPARE(devToolsPage.devToolsPage(), nullptr);
    QCOMPARE(devToolsPage.inspectedPage(), &inspectedPage2);

    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().value(0).toBool());

    devToolsPage.setInspectedPage(nullptr);

    QCOMPARE(inspectedPage1.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage1.inspectedPage(), nullptr);
    QCOMPARE(inspectedPage2.devToolsPage(), nullptr);
    QCOMPARE(inspectedPage2.inspectedPage(), nullptr);
    QCOMPARE(devToolsPage.devToolsPage(), nullptr);
    QCOMPARE(devToolsPage.inspectedPage(), nullptr);
}

void tst_QWebEnginePage::openLinkInDifferentProfile()
{
    class Page : public QWebEnginePage {
    public:
        QWebEnginePage *targetPage = nullptr;
        Page(QWebEngineProfile *profile) : QWebEnginePage(profile) {}
    private:
        QWebEnginePage *createWindow(WebWindowType) override { return targetPage; }
    };
    QWebEngineProfile profile1, profile2;
    Page page1(&profile1), page2(&profile2);
    QWebEngineView view;
    view.resize(500, 500);
    view.setPage(&page1);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QSignalSpy spy1(&page1, &QWebEnginePage::loadFinished), spy2(&page2, &QWebEnginePage::loadFinished);
    page1.setHtml("<html><body>"
                  "<a id='link' href='data:,hello'>link</a>"
                  "</body></html>");
    QTRY_COMPARE(spy1.count(), 1);
    QVERIFY(spy1.takeFirst().value(0).toBool());
    page1.targetPage = &page2;
    QTest::mouseClick(view.focusProxy(), Qt::MiddleButton, 0, elementCenter(&page1, "link"));
    QTRY_COMPARE(spy2.count(), 1);
    QVERIFY(spy2.takeFirst().value(0).toBool());
}

void tst_QWebEnginePage::triggerActionWithoutMenu()
{
    // Calling triggerAction should not crash even when for
    // context-menu-specific actions without a context menu.
    QWebEngineProfile profile;
    QWebEnginePage page(&profile);
    page.triggerAction(QWebEnginePage::DownloadLinkToDisk);
}

void tst_QWebEnginePage::dynamicFrame()
{
    QWebEnginePage page;
    page.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);
    page.load(QStringLiteral("qrc:/resources/dynamicFrame.html"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(toPlainTextSync(&page).trimmed(), QStringLiteral("foo"));
}

void tst_QWebEnginePage::editActionsWithExplicitFocus()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    // The view is hidden and no focus on the page. Edit actions should be disabled.
    QVERIFY(!view.isVisible());
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    page->setHtml(QString("<html><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    // Still no focus because focus on navigation is disabled. Edit actions don't do anything (should not crash).
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QCOMPARE(selectionChangedSpy.count(), 0);
    QCOMPARE(page->hasSelection(), false);

    // Focus content by focusing window from JavaScript. Edit actions should be enabled and functional.
    evaluateJavaScriptSync(page, "window.focus();");
    QTRY_COMPARE(actionChangedSpy.count(), 1);
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.count(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("foo bar"));
}

void tst_QWebEnginePage::editActionsWithInitialFocus()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    // The view is hidden and no focus on the page. Edit actions should be disabled.
    QVERIFY(!view.isVisible());
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    page->setHtml(QString("<html><body><div>foo bar</div></body></html>"));
    QTRY_COMPARE(loadFinishedSpy.count(), 1);

    // Content gets initial focus.
    QTRY_COMPARE(actionChangedSpy.count(), 1);
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.count(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("foo bar"));
}

void tst_QWebEnginePage::editActionsWithFocusOnIframe()
{
    QWebEngineView view;
    QWebEnginePage *page = view.page();
    view.settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

    QSignalSpy loadFinishedSpy(page, &QWebEnginePage::loadFinished);
    QSignalSpy selectionChangedSpy(page, &QWebEnginePage::selectionChanged);
    QSignalSpy actionChangedSpy(page->action(QWebEnginePage::SelectAll), &QAction::changed);

    // The view is hidden and no focus on the page. Edit actions should be disabled.
    QVERIFY(!view.isVisible());
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    page->load(QUrl("qrc:///resources/iframe2.html"));
    QTRY_COMPARE(loadFinishedSpy.count(), 1);
    QVERIFY(!page->action(QWebEnginePage::SelectAll)->isEnabled());

    // Focusing an iframe.
    evaluateJavaScriptSync(page, "document.getElementsByTagName('iframe')[0].contentWindow.focus()");
    QTRY_COMPARE(actionChangedSpy.count(), 1);
    QVERIFY(page->action(QWebEnginePage::SelectAll)->isEnabled());
    view.page()->triggerAction(QWebEnginePage::SelectAll);
    QTRY_COMPARE(selectionChangedSpy.count(), 1);
    QCOMPARE(page->hasSelection(), true);
    QCOMPARE(page->selectedText(), QStringLiteral("inner"));
}

void tst_QWebEnginePage::customUserAgentInNewTab()
{
    HttpServer server;
    QByteArray lastUserAgent;
    connect(&server, &HttpServer::newRequest, [&](HttpReqRep *rr) {
        QCOMPARE(rr->requestMethod(), "GET");
        lastUserAgent = rr->requestHeader("user-agent");
        rr->setResponseBody(QByteArrayLiteral("<html><body>Test</body></html>"));
        rr->sendResponse();
    });
    QVERIFY(server.start());

    class Page : public QWebEnginePage {
    public:
        QWebEngineProfile *targetProfile = nullptr;
        QScopedPointer<QWebEnginePage> newPage;
        Page(QWebEngineProfile *profile) : QWebEnginePage(profile) {}
    private:
        QWebEnginePage *createWindow(WebWindowType) override
        {
            newPage.reset(new QWebEnginePage(targetProfile ? targetProfile : profile(), nullptr));
            return newPage.data();
        }
    };
    QWebEngineProfile profile1, profile2;
    profile1.setHttpUserAgent(QStringLiteral("custom 1"));
    profile2.setHttpUserAgent(QStringLiteral("custom 2"));
    Page page(&profile1);
    QWebEngineView view;
    view.resize(500, 500);
    view.setPage(&page);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QSignalSpy spy(&page, &QWebEnginePage::loadFinished);

    // First check we can get the user-agent passed through normally
    page.setHtml(QString("<html><body><a id='link' target='_blank' href='") +
                 server.url("/test1").toEncoded() +
                 QString("'>link</a></body></html>"));
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().value(0).toBool());
    QCOMPARE(evaluateJavaScriptSync(&page, QStringLiteral("navigator.userAgent")).toString(), profile1.httpUserAgent());
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, 0, elementCenter(&page, "link"));
    QTRY_VERIFY(page.newPage);
    QTRY_VERIFY(!lastUserAgent.isEmpty());
    QCOMPARE(lastUserAgent, profile1.httpUserAgent().toUtf8());

    // Now check we can get the new user-agent of the profile
    page.newPage.reset();
    page.targetProfile = &profile2;
    spy.clear();
    lastUserAgent = { };
    page.setHtml(QString("<html><body><a id='link' target='_blank' href='") +
                 server.url("/test2").toEncoded() +
                 QString("'>link</a></body></html>"));
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().value(0).toBool());
    QTest::mouseClick(view.focusProxy(), Qt::LeftButton, 0, elementCenter(&page, "link"));
    QTRY_VERIFY(page.newPage);
    QTRY_VERIFY(!lastUserAgent.isEmpty());
    QCOMPARE(lastUserAgent, profile2.httpUserAgent().toUtf8());
}

static QByteArrayList params = {QByteArrayLiteral("--use-fake-device-for-media-stream")};
W_QTEST_MAIN(tst_QWebEnginePage, params)

#include "tst_qwebenginepage.moc"
