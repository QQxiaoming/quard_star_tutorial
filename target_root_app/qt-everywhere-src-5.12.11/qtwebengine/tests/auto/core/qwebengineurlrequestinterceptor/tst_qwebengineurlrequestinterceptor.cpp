/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "../../widgets/util.h"
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestinfo.h>
#include <QtWebEngineCore/qwebengineurlrequestinterceptor.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebenginesettings.h>

#include <httpserver.h>
#include <httpreqrep.h>

class tst_QWebEngineUrlRequestInterceptor : public QObject
{
    Q_OBJECT

public:
    tst_QWebEngineUrlRequestInterceptor();
    ~tst_QWebEngineUrlRequestInterceptor();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void interceptRequest();
    void ipv6HostEncoding();
    void requestedUrl();
    void setUrlSameUrl();
    void firstPartyUrl();
    void firstPartyUrlNestedIframes_data();
    void firstPartyUrlNestedIframes();
    void requestInterceptorByResourceType_data();
    void requestInterceptorByResourceType();
    void firstPartyUrlHttp();
    void passRefererHeader();
};

tst_QWebEngineUrlRequestInterceptor::tst_QWebEngineUrlRequestInterceptor()
{
}

tst_QWebEngineUrlRequestInterceptor::~tst_QWebEngineUrlRequestInterceptor()
{
}

void tst_QWebEngineUrlRequestInterceptor::init()
{
}

void tst_QWebEngineUrlRequestInterceptor::cleanup()
{
}

void tst_QWebEngineUrlRequestInterceptor::initTestCase()
{
     QSKIP("Interceptor has race condition in 5.12, skipping the test.");
}

void tst_QWebEngineUrlRequestInterceptor::cleanupTestCase()
{
}

struct RequestInfo {
    RequestInfo(QWebEngineUrlRequestInfo &info)
        : requestUrl(info.requestUrl())
        , firstPartyUrl(info.firstPartyUrl())
        , resourceType(info.resourceType())
    {}

    QUrl requestUrl;
    QUrl firstPartyUrl;
    int resourceType;
};

static const QByteArray kHttpHeaderReferrerValue = QByteArrayLiteral("http://somereferrer.com/");
static const QByteArray kHttpHeaderRefererName = QByteArrayLiteral("referer");

class TestRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    QList<RequestInfo> requestInfos;
    bool shouldIntercept;

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        // Since 63 we also intercept some unrelated blob requests..
        if (info.requestUrl().scheme() == QLatin1String("blob"))
            return;
        info.block(info.requestMethod() != QByteArrayLiteral("GET"));
        if (shouldIntercept && info.requestUrl().toString().endsWith(QLatin1String("__placeholder__")))
            info.redirect(QUrl("qrc:///resources/content.html"));

        // Set referrer header
        info.setHttpHeader(kHttpHeaderRefererName, kHttpHeaderReferrerValue);

        requestInfos.append(info);
    }

    bool shouldSkipRequest(const RequestInfo &requestInfo)
    {
        if (requestInfo.resourceType ==  QWebEngineUrlRequestInfo::ResourceTypeMainFrame ||
                requestInfo.resourceType == QWebEngineUrlRequestInfo::ResourceTypeSubFrame)
            return false;

        // Skip import documents and sandboxed documents.
        // See Document::SiteForCookies() in chromium/third_party/blink/renderer/core/dom/document.cc.
        //
        // TODO: Change this to empty URL during the next chromium update:
        // https://chromium-review.googlesource.com/c/chromium/src/+/1213082/
        return requestInfo.firstPartyUrl == QUrl("data:,");
    }

    QList<RequestInfo> getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceType type)
    {
        QList<RequestInfo> infos;

        foreach (auto requestInfo, requestInfos) {
            if (shouldSkipRequest(requestInfo))
                continue;

            if (type == requestInfo.resourceType)
                infos.append(requestInfo);
        }

        return infos;
    }

    bool hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceType type)
    {
        foreach (auto requestInfo, requestInfos) {
            if (shouldSkipRequest(requestInfo))
                continue;

            if (type == requestInfo.resourceType)
                return true;
        }

        return false;
    }

    TestRequestInterceptor(bool intercept)
        : shouldIntercept(intercept)
    {
    }
};

void tst_QWebEngineUrlRequestInterceptor::interceptRequest()
{
    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    TestRequestInterceptor interceptor(/* intercept */ true);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));

    page.load(QUrl("qrc:///resources/index.html"));
    QTRY_COMPARE(loadSpy.count(), 1);
    QVariant success = loadSpy.takeFirst().takeFirst();
    QVERIFY(success.toBool());
    loadSpy.clear();
    QVariant ok;

    page.runJavaScript("post();", [&ok](const QVariant result){ ok = result; });
    QTRY_VERIFY(ok.toBool());
    QTRY_COMPARE(loadSpy.count(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // We block non-GET requests, so this should not succeed.
    QVERIFY(!success.toBool());
    loadSpy.clear();

    page.load(QUrl("qrc:///resources/__placeholder__"));
    QTRY_COMPARE(loadSpy.count(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // The redirection for __placeholder__ should succeed.
    QVERIFY(success.toBool());
    loadSpy.clear();
    QCOMPARE(interceptor.requestInfos.count(), 4);

    // Make sure that registering an observer does not modify the request.
    TestRequestInterceptor observer(/* intercept */ false);
    profile.setRequestInterceptor(&observer);
    page.load(QUrl("qrc:///resources/__placeholder__"));
    QTRY_COMPARE(loadSpy.count(), 1);
    success = loadSpy.takeFirst().takeFirst();
    // Since we do not intercept, loading an invalid path should not succeed.
    QVERIFY(!success.toBool());
    QCOMPARE(observer.requestInfos.count(), 1);
}

class LocalhostContentProvider : public QWebEngineUrlRequestInterceptor
{
public:
    LocalhostContentProvider() { }

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        // Since 63 we also intercept the original data requests
        if (info.requestUrl().scheme() == QLatin1String("data"))
            return;
        if (info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeFavicon)
            return;

        requestedUrls.append(info.requestUrl());
        info.redirect(QUrl("data:text/html,<p>hello"));
    }

    QList<QUrl> requestedUrls;
};

void tst_QWebEngineUrlRequestInterceptor::ipv6HostEncoding()
{
    QWebEngineProfile profile;
    LocalhostContentProvider contentProvider;
    profile.setRequestInterceptor(&contentProvider);

    QWebEnginePage page(&profile);
    QSignalSpy spyLoadFinished(&page, SIGNAL(loadFinished(bool)));

    page.setHtml("<p>Hi", QUrl::fromEncoded("http://[::1]/index.html"));
    QTRY_COMPARE(spyLoadFinished.count(), 1);
    QCOMPARE(contentProvider.requestedUrls.count(), 0);

    evaluateJavaScriptSync(&page, "var r = new XMLHttpRequest();"
            "r.open('GET', 'http://[::1]/test.xml', false);"
            "r.send(null);"
            );

    QCOMPARE(contentProvider.requestedUrls.count(), 1);
    QCOMPARE(contentProvider.requestedUrls.at(0), QUrl::fromEncoded("http://[::1]/test.xml"));
}

void tst_QWebEngineUrlRequestInterceptor::requestedUrl()
{
    QWebEngineProfile profile;
    profile.settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    TestRequestInterceptor interceptor(/* intercept */ true);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(interceptor.requestInfos.at(0).requestUrl, QUrl("qrc:///resources/content.html"));
    QCOMPARE(page.requestedUrl(), QUrl("qrc:///resources/__placeholder__"));
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));

    page.setUrl(QUrl("qrc:/non-existent.html"));
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(interceptor.requestInfos.at(2).requestUrl, QUrl("qrc:/non-existent.html"));
    QCOMPARE(page.requestedUrl(), QUrl("qrc:///resources/__placeholder__"));
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));

    page.setUrl(QUrl("http://abcdef.abcdef"));
    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 3, 12000);
    QCOMPARE(interceptor.requestInfos.at(3).requestUrl, QUrl("http://abcdef.abcdef/"));
    QCOMPARE(page.requestedUrl(), QUrl("qrc:///resources/__placeholder__"));
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
}

void tst_QWebEngineUrlRequestInterceptor::setUrlSameUrl()
{
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ true);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.count(), 1);

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.count(), 2);

    // Now a case without redirect.
    page.setUrl(QUrl("qrc:///resources/content.html"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.count(), 3);

    page.setUrl(QUrl("qrc:///resources/__placeholder__"));
    QVERIFY(spy.wait());
    QCOMPARE(page.url(), QUrl("qrc:///resources/content.html"));
    QCOMPARE(spy.count(), 4);
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrl()
{
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));

    page.setUrl(QUrl("qrc:///resources/firstparty.html"));
    QVERIFY(spy.wait());
    QCOMPARE(interceptor.requestInfos.at(0).requestUrl, QUrl("qrc:///resources/firstparty.html"));
    QCOMPARE(interceptor.requestInfos.at(1).requestUrl, QUrl("qrc:///resources/content.html"));
    QCOMPARE(interceptor.requestInfos.at(0).firstPartyUrl, QUrl("qrc:///resources/firstparty.html"));
    QCOMPARE(interceptor.requestInfos.at(1).firstPartyUrl, QUrl("qrc:///resources/firstparty.html"));
    QCOMPARE(spy.count(), 1);
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrlNestedIframes_data()
{
    QUrl url = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/iframe.html"));

    QTest::addColumn<QUrl>("requestUrl");
    QTest::newRow("file") << url;
    QTest::newRow("qrc") << QUrl("qrc:///resources/iframe.html");
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrlNestedIframes()
{
    QFETCH(QUrl, requestUrl);

    if (requestUrl.scheme() == "file" && !QDir(TESTS_SOURCE_DIR).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'").arg(TESTS_SOURCE_DIR).toLatin1().constData(), SkipAll);

    QString adjustedUrl = requestUrl.adjusted(QUrl::RemoveFilename).toString();

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setUrl(requestUrl);
    QTRY_COMPARE(loadSpy.count(), 1);

    RequestInfo info = interceptor.requestInfos.at(0);
    QCOMPARE(info.requestUrl, requestUrl);
    QCOMPARE(info.firstPartyUrl, requestUrl);
    QCOMPARE(info.resourceType, QWebEngineUrlRequestInfo::ResourceTypeMainFrame);

    info = interceptor.requestInfos.at(1);
    QCOMPARE(info.requestUrl, QUrl(adjustedUrl + "iframe2.html"));
    QCOMPARE(info.firstPartyUrl, requestUrl);
    QCOMPARE(info.resourceType, QWebEngineUrlRequestInfo::ResourceTypeSubFrame);

    info = interceptor.requestInfos.at(2);
    QCOMPARE(info.requestUrl, QUrl(adjustedUrl + "iframe3.html"));
    QCOMPARE(info.firstPartyUrl, requestUrl);
    QCOMPARE(info.resourceType, QWebEngineUrlRequestInfo::ResourceTypeSubFrame);
}

void tst_QWebEngineUrlRequestInterceptor::requestInterceptorByResourceType_data()
{
    QUrl firstPartyUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/resource_in_iframe.html"));
    QUrl styleRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/style.css"));
    QUrl scriptRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/script.js"));
    QUrl fontRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/fontawesome.woff"));
    QUrl xhrRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/test"));
    QUrl imageFirstPartyUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/image_in_iframe.html"));
    QUrl imageRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/icons/favicon.png"));
    QUrl mediaFirstPartyUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/media_in_iframe.html"));
    QUrl mediaRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/media.mp4"));
    QUrl faviconFirstPartyUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/favicon.html"));
    QUrl faviconRequestUrl = QUrl::fromLocalFile(TESTS_SOURCE_DIR + QLatin1String("qwebengineurlrequestinterceptor/resources/icons/favicon.png"));

    QTest::addColumn<QUrl>("requestUrl");
    QTest::addColumn<QUrl>("firstPartyUrl");
    QTest::addColumn<int>("resourceType");

    QTest::newRow("StyleSheet") << styleRequestUrl << firstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeStylesheet);
    QTest::newRow("Script") << scriptRequestUrl << firstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeScript);
    QTest::newRow("Image") << imageRequestUrl << imageFirstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeImage);
    QTest::newRow("FontResource") << fontRequestUrl << firstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeFontResource);
    QTest::newRow("Media") << mediaRequestUrl << mediaFirstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeMedia);
    QTest::newRow("Favicon") << faviconRequestUrl << faviconFirstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeFavicon);
    QTest::newRow("Xhr") << xhrRequestUrl << firstPartyUrl << static_cast<int>(QWebEngineUrlRequestInfo::ResourceTypeXhr);
}

void tst_QWebEngineUrlRequestInterceptor::requestInterceptorByResourceType()
{
    if (!QDir(TESTS_SOURCE_DIR).exists())
        W_QSKIP(QString("This test requires access to resources found in '%1'").arg(TESTS_SOURCE_DIR).toLatin1().constData(), SkipAll);

    QFETCH(QUrl, requestUrl);
    QFETCH(QUrl, firstPartyUrl);
    QFETCH(int, resourceType);

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    page.setUrl(firstPartyUrl);
    QTRY_COMPARE(loadSpy.count(), 1);

    QTRY_COMPARE(interceptor.getUrlRequestForType(static_cast<QWebEngineUrlRequestInfo::ResourceType>(resourceType)).count(), 1);
    QList<RequestInfo> infos = interceptor.getUrlRequestForType(static_cast<QWebEngineUrlRequestInfo::ResourceType>(resourceType));
    QCOMPARE(infos.at(0).requestUrl, requestUrl);
    QCOMPARE(infos.at(0).firstPartyUrl, firstPartyUrl);
    QCOMPARE(infos.at(0).resourceType, resourceType);
}

void tst_QWebEngineUrlRequestInterceptor::firstPartyUrlHttp()
{
    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(/* intercept */ false);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy loadSpy(&page, SIGNAL(loadFinished(bool)));
    QUrl firstPartyUrl = QUrl("https://www.w3schools.com/tags/tryit.asp?filename=tryhtml5_video");
    page.setUrl(QUrl(firstPartyUrl));
    if (!loadSpy.wait(15000) || !loadSpy.at(0).at(0).toBool())
        QSKIP("Couldn't load page from network, skipping test.");

    QList<RequestInfo> infos;

    // SubFrame
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeSubFrame));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeSubFrame);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Stylesheet
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeStylesheet));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeStylesheet);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Script
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeScript));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeScript);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Image
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeImage));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeImage);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // FontResource
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFontResource));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFontResource);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Media
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeMedia));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeMedia);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // Favicon
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFavicon));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeFavicon);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);

    // XMLHttpRequest
    QTRY_VERIFY(interceptor.hasUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeXhr));
    infos = interceptor.getUrlRequestForType(QWebEngineUrlRequestInfo::ResourceTypeXhr);
    foreach (auto info, infos)
        QCOMPARE(info.firstPartyUrl, firstPartyUrl);
}

void tst_QWebEngineUrlRequestInterceptor::passRefererHeader()
{
    // Create HTTP Server to parse the request.
    HttpServer httpServer;

    if (!httpServer.start())
        QSKIP("Failed to start http server");

    bool succeeded = false;
    connect(&httpServer, &HttpServer::newRequest, [&succeeded](HttpReqRep *rr) {
        const QByteArray headerValue = rr->requestHeader(kHttpHeaderRefererName);
        QCOMPARE(headerValue, kHttpHeaderReferrerValue);
        succeeded = headerValue == kHttpHeaderReferrerValue;
        rr->setResponseStatus(200);
        rr->sendResponse();
    });

    QWebEngineProfile profile;
    TestRequestInterceptor interceptor(true);
    profile.setRequestInterceptor(&interceptor);

    QWebEnginePage page(&profile);
    QSignalSpy spy(&page, SIGNAL(loadFinished(bool)));
    QWebEngineHttpRequest httpRequest;
    QUrl requestUrl = httpServer.url();
    httpRequest.setUrl(requestUrl);
    page.load(httpRequest);

    QVERIFY(spy.wait());
    (void) httpServer.stop();
    QVERIFY(succeeded);
}

QTEST_MAIN(tst_QWebEngineUrlRequestInterceptor)
#include "tst_qwebengineurlrequestinterceptor.moc"
