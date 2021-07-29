/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QtCore/qfile.h>
#include <QtTest/QtTest>
#include <QtWebEngineCore/qwebengineurlrequestjob.h>
#include <QtWebEngineCore/qwebengineurlscheme.h>
#include <QtWebEngineCore/qwebengineurlschemehandler.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebenginesettings.h>
#if defined(WEBSOCKETS)
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebChannel/qwebchannel.h>
#endif
#include <QtWidgets/qaction.h>

#define QSL QStringLiteral
#define QBAL QByteArrayLiteral
#define THIS_DIR TESTS_SOURCE_DIR "origins/"

void registerSchemes()
{
    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax"));
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-Secure"));
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-Secure-ServiceWorkersAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme | QWebEngineUrlScheme::ServiceWorkersAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-Local"));
        scheme.setFlags(QWebEngineUrlScheme::LocalScheme);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-LocalAccessAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::LocalAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-NoAccessAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::NoAccessAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-ServiceWorkersAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::ServiceWorkersAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("PathSyntax-ViewSourceAllowed"));
        scheme.setFlags(QWebEngineUrlScheme::ViewSourceAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostSyntax"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostSyntax-ContentSecurityPolicyIgnored"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
        scheme.setFlags(QWebEngineUrlScheme::ContentSecurityPolicyIgnored);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostAndPortSyntax"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
        scheme.setDefaultPort(42);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    {
        QWebEngineUrlScheme scheme(QBAL("HostPortAndUserInformationSyntax"));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostPortAndUserInformation);
        scheme.setDefaultPort(42);
        QWebEngineUrlScheme::registerScheme(scheme);
    }
}
Q_CONSTRUCTOR_FUNCTION(registerSchemes)

class TstUrlSchemeHandler final : public QWebEngineUrlSchemeHandler {
    Q_OBJECT

public:
    TstUrlSchemeHandler(QWebEngineProfile *profile)
    {
        profile->installUrlSchemeHandler(QBAL("tst"), this);

        profile->installUrlSchemeHandler(QBAL("PathSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-Secure"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-Secure-ServiceWorkersAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-Local"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-LocalAccessAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-NoAccessAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-ServiceWorkersAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("PathSyntax-ViewSourceAllowed"), this);
        profile->installUrlSchemeHandler(QBAL("HostSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("HostSyntax-ContentSecurityPolicyIgnored"), this);
        profile->installUrlSchemeHandler(QBAL("HostAndPortSyntax"), this);
        profile->installUrlSchemeHandler(QBAL("HostPortAndUserInformationSyntax"), this);
    }

private:
    void requestStarted(QWebEngineUrlRequestJob *job) override
    {
        QString pathPrefix = QSL(THIS_DIR);
        QString pathSuffix = job->requestUrl().path();
        QFile *file = new QFile(pathPrefix + pathSuffix, job);
        if (!file->open(QIODevice::ReadOnly)) {
            job->fail(QWebEngineUrlRequestJob::RequestFailed);
            return;
        }
        QByteArray mimeType = QBAL("text/html");
        if (pathSuffix.endsWith(QSL(".js")))
            mimeType = QBAL("application/javascript");
        job->reply(mimeType, file);
    }
};

class tst_Origins final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void jsUrlCanon();
    void jsUrlRelative();
    void jsUrlOrigin();
    void subdirWithAccess();
    void subdirWithoutAccess();
    void mixedSchemes();
    void mixedSchemesWithCsp();
    void mixedXHR();
#if defined(WEBSOCKETS)
    void webSocket();
#endif
    void dedicatedWorker();
    void sharedWorker();
    void serviceWorker();
    void viewSource();
    void createObjectURL();

private:
    bool load(const QUrl &url)
    {
        QSignalSpy spy(m_page, &QWebEnginePage::loadFinished);
        m_page->load(url);
        return (!spy.empty() || spy.wait(20000))
            && spy.front().value(0).toBool();
    }

    QVariant eval(const QString &code)
    {
        return evaluateJavaScriptSync(m_page, code);
    }

    QWebEngineProfile m_profile;
    QWebEnginePage *m_page = nullptr;
    TstUrlSchemeHandler *m_handler = nullptr;
};

void tst_Origins::initTestCase()
{
    m_page = new QWebEnginePage(&m_profile, nullptr);
    m_handler = new TstUrlSchemeHandler(&m_profile);
}

void tst_Origins::cleanupTestCase()
{
    delete m_handler;
    delete m_page;
}

// Test URL parsing and canonicalization in Blink. The implementation of this
// part is mostly shared between Blink and Chromium proper.
void tst_Origins::jsUrlCanon()
{
    QVERIFY(load(QSL("about:blank")));

    // Standard schemes are biased towards the authority part.
    QCOMPARE(eval(QSL("new URL(\"http:foo/bar\").href")),    QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"http:/foo/bar\").href")),   QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"http://foo/bar\").href")),  QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"http:///foo/bar\").href")), QVariant(QSL("http://foo/bar")));

    // The file scheme is however a (particularly) special case.
#ifdef Q_OS_WIN
    QCOMPARE(eval(QSL("new URL(\"file:foo/bar\").href")),    QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:/foo/bar\").href")),   QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file://foo/bar\").href")),  QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:///foo/bar\").href")), QVariant(QSL("file:///foo/bar")));
#else
    QCOMPARE(eval(QSL("new URL(\"file:foo/bar\").href")),    QVariant(QSL("file:///foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:/foo/bar\").href")),   QVariant(QSL("file:///foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file://foo/bar\").href")),  QVariant(QSL("file://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"file:///foo/bar\").href")), QVariant(QSL("file:///foo/bar")));
#endif

    // The qrc scheme is a PathSyntax scheme, having only a path and nothing else.
    QCOMPARE(eval(QSL("new URL(\"qrc:foo/bar\").href")),    QVariant(QSL("qrc:foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc:/foo/bar\").href")),   QVariant(QSL("qrc:/foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc://foo/bar\").href")),  QVariant(QSL("qrc://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"qrc:///foo/bar\").href")), QVariant(QSL("qrc:///foo/bar")));

    // Same for unregistered schemes.
    QCOMPARE(eval(QSL("new URL(\"tst:foo/bar\").href")),    QVariant(QSL("tst:foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst:/foo/bar\").href")),   QVariant(QSL("tst:/foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst://foo/bar\").href")),  QVariant(QSL("tst://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"tst:///foo/bar\").href")), QVariant(QSL("tst:///foo/bar")));

    // A HostSyntax scheme is like http without the port & user information.
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:foo/bar\").href")),     QVariant(QSL("hostsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:foo:42/bar\").href")),  QVariant(QSL("hostsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:a:b@foo/bar\").href")), QVariant(QSL("hostsyntax://foo/bar")));

    // A HostAndPortSyntax scheme is like http without the user information.
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:41/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo:41/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:42/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:a:b@foo/bar\").href")),
             QVariant(QSL("hostandportsyntax://foo/bar")));

    // A HostPortAndUserInformationSyntax scheme is exactly like http.
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:41/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://foo:41/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:42/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://foo/bar")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:a:b@foo/bar\").href")),
             QVariant(QSL("hostportanduserinformationsyntax://a:b@foo/bar")));
}

// Test relative URL resolution.
void tst_Origins::jsUrlRelative()
{
    QVERIFY(load(QSL("about:blank")));

    // Schemes with hosts, like http, work as expected.
    QCOMPARE(eval(QSL("new URL('bar', 'http://foo').href")), QVariant(QSL("http://foo/bar")));
    QCOMPARE(eval(QSL("new URL('baz', 'http://foo/bar').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('/baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('./baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('../baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('../../baz', 'http://foo/bar/').href")), QVariant(QSL("http://foo/baz")));
    QCOMPARE(eval(QSL("new URL('//baz', 'http://foo/bar/').href")), QVariant(QSL("http://baz/")));

    // In the case of schemes without hosts, relative URLs only work if the URL
    // starts with a single slash -- and canonicalization does not guarantee
    // this. The following cases all fail with TypeErrors.
    QCOMPARE(eval(QSL("new URL('bar', 'tst:foo').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('baz', 'tst:foo/bar').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('bar', 'tst://foo').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('bar', 'tst:///foo').href")), QVariant());

    // However, registered custom schemes have been patched to allow relative
    // URLs even without an initial slash.
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:foo').href")), QVariant(QSL("qrc:bar")));
    QCOMPARE(eval(QSL("new URL('baz', 'qrc:foo/bar').href")), QVariant(QSL("qrc:foo/baz")));
    QCOMPARE(eval(QSL("new URL('bar', 'qrc://foo').href")), QVariant());
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:///foo').href")), QVariant());

    // With a slash it works the same as http except 'foo' is part of the path and not the host.
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:/foo').href")), QVariant(QSL("qrc:/bar")));
    QCOMPARE(eval(QSL("new URL('bar', 'qrc:/foo/').href")), QVariant(QSL("qrc:/foo/bar")));
    QCOMPARE(eval(QSL("new URL('baz', 'qrc:/foo/bar').href")), QVariant(QSL("qrc:/foo/baz")));
    QCOMPARE(eval(QSL("new URL('baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('/baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/baz")));
    QCOMPARE(eval(QSL("new URL('./baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/foo/bar/baz")));
    QCOMPARE(eval(QSL("new URL('../baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/foo/baz")));
    QCOMPARE(eval(QSL("new URL('../../baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/baz")));
    QCOMPARE(eval(QSL("new URL('../../../baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc:/baz")));

    // If the relative URL begins with >= 2 slashes, then the scheme is treated
    // not as a Syntax::Path scheme but as a Syntax::HostPortAndUserInformation
    // scheme.
    QCOMPARE(eval(QSL("new URL('//baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc://baz/")));
    QCOMPARE(eval(QSL("new URL('///baz', 'qrc:/foo/bar/').href")), QVariant(QSL("qrc://baz/")));
}

// Test origin serialization in Blink, implemented by blink::KURL and
// blink::SecurityOrigin as opposed to GURL and url::Origin.
void tst_Origins::jsUrlOrigin()
{
    QVERIFY(load(QSL("about:blank")));

    // For network protocols the origin string must include the domain and port.
    QCOMPARE(eval(QSL("new URL(\"http://foo.com/page.html\").origin")), QVariant(QSL("http://foo.com")));
    QCOMPARE(eval(QSL("new URL(\"https://foo.com/page.html\").origin")), QVariant(QSL("https://foo.com")));

    // Even though file URL can also have domains, these are not included in the
    // origin string by Chromium. The standard does not specify a value here,
    // but suggests 'null' (https://url.spec.whatwg.org/#origin).
    QCOMPARE(eval(QSL("new URL(\"file:/etc/passwd\").origin")), QVariant(QSL("file://")));
    QCOMPARE(eval(QSL("new URL(\"file://foo.com/etc/passwd\").origin")), QVariant(QSL("file://")));

    // The qrc scheme should behave like file.
    QCOMPARE(eval(QSL("new URL(\"qrc:/crysis.css\").origin")), QVariant(QSL("qrc://")));
    QCOMPARE(eval(QSL("new URL(\"qrc://foo.com/crysis.css\").origin")), QVariant(QSL("qrc://")));

    // Same with unregistered schemes.
    QCOMPARE(eval(QSL("new URL(\"tst:/banana\").origin")), QVariant(QSL("tst://")));
    QCOMPARE(eval(QSL("new URL(\"tst://foo.com/banana\").origin")), QVariant(QSL("tst://")));

    // The non-PathSyntax schemes should have hosts and potentially ports.
    QCOMPARE(eval(QSL("new URL(\"HostSyntax:foo:41/bar\").origin")),
             QVariant(QSL("hostsyntax://foo")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:41/bar\").origin")),
             QVariant(QSL("hostandportsyntax://foo:41")));
    QCOMPARE(eval(QSL("new URL(\"HostAndPortSyntax:foo:42/bar\").origin")),
             QVariant(QSL("hostandportsyntax://foo")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:41/bar\").origin")),
             QVariant(QSL("hostportanduserinformationsyntax://foo:41")));
    QCOMPARE(eval(QSL("new URL(\"HostPortAndUserInformationSyntax:foo:42/bar\").origin")),
             QVariant(QSL("hostportanduserinformationsyntax://foo")));

    // A PathSyntax scheme should have a 'universal' origin.
    QCOMPARE(eval(QSL("new URL(\"PathSyntax:foo\").origin")),
             QVariant(QSL("pathsyntax://")));

    // The NoAccessAllowed flag forces opaque origins.
    QCOMPARE(eval(QSL("new URL(\"PathSyntax-NoAccessAllowed:foo\").origin")),
             QVariant(QSL("null")));
}

class ScopedAttribute {
public:
    ScopedAttribute(QWebEngineSettings *settings, QWebEngineSettings::WebAttribute attribute, bool newValue)
        : m_settings(settings)
        , m_attribute(attribute)
        , m_oldValue(m_settings->testAttribute(m_attribute))
    {
        m_settings->setAttribute(m_attribute, newValue);
    }
    ~ScopedAttribute()
    {
        m_settings->setAttribute(m_attribute, m_oldValue);
    }
private:
    QWebEngineSettings *m_settings;
    QWebEngineSettings::WebAttribute m_attribute;
    bool m_oldValue;
};

// Test same-origin policy of file, qrc and custom schemes.
//
// Note the test case involves the main page trying to load an iframe from a
// file that resides in a parent directory. This is just a small detail to
// demonstrate the difference with Firefox where such access is not allowed.
void tst_Origins::subdirWithAccess()
{
    ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    QVERIFY(load(QSL("file:" THIS_DIR "resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(load(QSL("qrc:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(load(QSL("tst:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));
}

// In this variation the LocalContentCanAccessFileUrls attribute is disabled. As
// a result all file URLs will be considered to have unique/opaque origins, that
// is, they are not the 'same origin as' any other origin.
//
// Note that this applies only to file URLs and not qrc or custom schemes.
//
// See also (in Blink):
//   - the allow_file_access_from_file_urls option and
//   - the blink::SecurityOrigin::BlockLocalAccessFromLocalOrigin() method.
void tst_Origins::subdirWithoutAccess()
{
    ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, false);

    QVERIFY(load(QSL("file:" THIS_DIR "resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant());
    QCOMPARE(eval(QSL("msg[1]")), QVariant());

    QVERIFY(load(QSL("qrc:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));

    QVERIFY(load(QSL("tst:/resources/subdir/index.html")));
    QCOMPARE(eval(QSL("msg[0]")), QVariant(QSL("hello")));
    QCOMPARE(eval(QSL("msg[1]")), QVariant(QSL("world")));
}

// Load the main page over one scheme with an iframe over another scheme.
//
// For file and qrc schemes, the iframe should load but it should not be
// possible for scripts in different frames to interact.
//
// Additionally for unregistered custom schemes and custom schemes without
// LocalAccessAllowed it should not be possible to load an iframe over the
// file: scheme.
void tst_Origins::mixedSchemes()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('file:" THIS_DIR "resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('qrc:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('tst:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(load(QSL("qrc:/resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('file:" THIS_DIR "resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('qrc:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('tst:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(load(QSL("tst:/resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('file:" THIS_DIR "resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("cannotLoad")));
    eval(QSL("setIFrameUrl('qrc:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('tst:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));

    QVERIFY(load(QSL("PathSyntax:/resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('PathSyntax:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-Local:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("cannotLoad")));
    eval(QSL("setIFrameUrl('PathSyntax-LocalAccessAllowed:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-NoAccessAllowed:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(load(QSL("PathSyntax-LocalAccessAllowed:/resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('PathSyntax:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-Local:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-LocalAccessAllowed:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-NoAccessAllowed:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(load(QSL("PathSyntax-NoAccessAllowed:/resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('PathSyntax:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-Local:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("cannotLoad")));
    eval(QSL("setIFrameUrl('PathSyntax-LocalAccessAllowed:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('PathSyntax-NoAccessAllowed:/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(load(QSL("HostSyntax://a/resources/mixedSchemes.html")));
    eval(QSL("setIFrameUrl('HostSyntax://a/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('HostSyntax://b/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
}

// Like mixedSchemes but adds a Content-Security-Policy: frame-src 'none' header.
void tst_Origins::mixedSchemesWithCsp()
{
    QVERIFY(load(QSL("HostSyntax://a/resources/mixedSchemesWithCsp.html")));
    eval(QSL("setIFrameUrl('HostSyntax://a/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
    eval(QSL("setIFrameUrl('HostSyntax://b/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));

    QVERIFY(load(QSL("HostSyntax-ContentSecurityPolicyIgnored://a/resources/mixedSchemesWithCsp.html")));
    eval(QSL("setIFrameUrl('HostSyntax-ContentSecurityPolicyIgnored://a/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadAndAccess")));
    eval(QSL("setIFrameUrl('HostSyntax-ContentSecurityPolicyIgnored://b/resources/mixedSchemes_frame.html')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("canLoadButNotAccess")));
}

// Load the main page over one scheme, then make an XMLHttpRequest to a
// different scheme.
//
// XMLHttpRequests can only be made to http, https, data, and chrome.
void tst_Origins::mixedXHR()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/mixedXHR.html")));
    eval(QSL("sendXHR('file:" THIS_DIR "resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
    eval(QSL("sendXHR('qrc:/resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("error")));
    eval(QSL("sendXHR('tst:/resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("error")));
    eval(QSL("sendXHR('data:,ok')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    QVERIFY(load(QSL("qrc:/resources/mixedXHR.html")));
    eval(QSL("sendXHR('file:" THIS_DIR "resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
    eval(QSL("sendXHR('qrc:/resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
    eval(QSL("sendXHR('tst:/resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("error")));
    eval(QSL("sendXHR('data:,ok')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    QVERIFY(load(QSL("tst:/resources/mixedXHR.html")));
    eval(QSL("sendXHR('file:" THIS_DIR "resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("error")));
    eval(QSL("sendXHR('qrc:/resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("error")));
    eval(QSL("sendXHR('tst:/resources/mixedXHR.txt')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
    eval(QSL("sendXHR('data:,ok')"));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
}

#if defined(WEBSOCKETS)
class EchoServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
public:
    EchoServer() : webSocketServer(QSL("EchoServer"), QWebSocketServer::NonSecureMode)
    {
        connect(&webSocketServer, &QWebSocketServer::newConnection, this, &EchoServer::onNewConnection);
    }

    bool listen()
    {
        if (webSocketServer.listen(QHostAddress::Any)) {
            Q_EMIT urlChanged();
            return true;
        }
        return false;
    }

    QUrl url() const
    {
        return webSocketServer.serverUrl();
    }

Q_SIGNALS:
    void urlChanged();

private:
    void onNewConnection()
    {
        QWebSocket *socket = webSocketServer.nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &EchoServer::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected, socket, &QObject::deleteLater);
    }

    void onTextMessageReceived(const QString &message)
    {
        QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
        socket->sendTextMessage(message);
    }

    QWebSocketServer webSocketServer;
};

// Try opening a WebSocket from pages loaded over various URL schemes.
void tst_Origins::webSocket()
{
    const int kAbnormalClosure = 1006;

    EchoServer echoServer;
    QWebChannel channel;
    channel.registerObject(QSL("echoServer"), &echoServer);
    m_page->setWebChannel(&channel);
    QVERIFY(echoServer.listen());

    QVERIFY(load(QSL("file:" THIS_DIR "resources/websocket.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    QVERIFY(load(QSL("qrc:/resources/websocket.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));

    // Only registered schemes can open WebSockets.
    QVERIFY(load(QSL("tst:/resources/websocket.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(kAbnormalClosure));

    // Even an insecure registered scheme can open WebSockets.
    QVERIFY(load(QSL("PathSyntax:/resources/websocket.html")));
    QTRY_COMPARE(eval(QSL("result")), QVariant(QSL("ok")));
}
#endif
// Create a (Dedicated)Worker. Since dedicated workers can only be accessed from
// one page, there is not much need for security restrictions.
void tst_Origins::dedicatedWorker()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    QVERIFY(load(QSL("qrc:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // Unregistered schemes cannot create Workers.
    QVERIFY(load(QSL("tst:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Access to dedicated workers is denied to origin 'tst://'")));

    // Even an insecure registered scheme can create Workers.
    QVERIFY(load(QSL("PathSyntax:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // But not if the NoAccessAllowed flag is set.
    QVERIFY(load(QSL("PathSyntax-NoAccessAllowed:/resources/dedicatedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("cannot be accessed from origin 'null'")));
}

// Create a SharedWorker. Shared workers can be accessed from multiple pages,
// and therefore the same-origin policy applies.
void tst_Origins::sharedWorker()
{
    {
        ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, false);
        QVERIFY(load(QSL("file:" THIS_DIR "resources/sharedWorker.html")));
        QTRY_VERIFY(eval(QSL("done")).toBool());
        QVERIFY(eval(QSL("error")).toString()
                .contains(QSL("cannot be accessed from origin 'null'")));
    }

    {
        ScopedAttribute sa(m_page->settings(), QWebEngineSettings::LocalContentCanAccessFileUrls, true);
        QVERIFY(load(QSL("file:" THIS_DIR "resources/sharedWorker.html")));
        QTRY_VERIFY(eval(QSL("done")).toBool());
        QCOMPARE(eval(QSL("result")), QVariant(42));
    }

    QVERIFY(load(QSL("qrc:/resources/sharedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    // Unregistered schemes should not create SharedWorkers.

    QVERIFY(load(QSL("PathSyntax:/resources/sharedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("result")), QVariant(42));

    QVERIFY(load(QSL("PathSyntax-NoAccessAllowed:/resources/sharedWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("denied to origin 'null'")));
}

// Service workers have to be explicitly enabled for a scheme.
void tst_Origins::serviceWorker()
{
    QVERIFY(load(QSL("file:" THIS_DIR "resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('file://') is not supported.")));

    QVERIFY(load(QSL("qrc:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('qrc://') is not supported.")));

    QVERIFY(load(QSL("tst:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read property 'register' of undefined")));

    QVERIFY(load(QSL("PathSyntax:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read property 'register' of undefined")));

    QVERIFY(load(QSL("PathSyntax-Secure:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("The URL protocol of the current origin ('pathsyntax-secure://') is not supported.")));

    QVERIFY(load(QSL("PathSyntax-ServiceWorkersAllowed:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read property 'register' of undefined")));

    QVERIFY(load(QSL("PathSyntax-Secure-ServiceWorkersAllowed:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QCOMPARE(eval(QSL("error")), QVariant());

    QVERIFY(load(QSL("PathSyntax-NoAccessAllowed:/resources/serviceWorker.html")));
    QTRY_VERIFY(eval(QSL("done")).toBool());
    QVERIFY(eval(QSL("error")).toString()
            .contains(QSL("Cannot read property 'register' of undefined")));
}

// Support for view-source must be enabled explicitly.
void tst_Origins::viewSource()
{
    QVERIFY(load(QSL("view-source:file:" THIS_DIR "resources/viewSource.html")));
#ifdef Q_OS_WIN
    QCOMPARE(m_page->requestedUrl().toString(), QSL("file:///" THIS_DIR "resources/viewSource.html"));
#else
    QCOMPARE(m_page->requestedUrl().toString(), QSL("file://" THIS_DIR "resources/viewSource.html"));
#endif

    QVERIFY(load(QSL("view-source:qrc:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("qrc:/resources/viewSource.html"));

    QVERIFY(load(QSL("view-source:tst:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("about:blank"));

    QVERIFY(load(QSL("view-source:PathSyntax:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("about:blank"));

    QVERIFY(load(QSL("view-source:PathSyntax-ViewSourceAllowed:/resources/viewSource.html")));
    QCOMPARE(m_page->requestedUrl().toString(), QSL("pathsyntax-viewsourceallowed:/resources/viewSource.html"));
}

void tst_Origins::createObjectURL()
{
    // Legal for registered custom schemes.
    QVERIFY(load(QSL("qrc:/resources/createObjectURL.html")));
    QVERIFY(eval(QSL("result")).toString().startsWith(QSL("blob:qrc:")));

    // Illegal for unregistered schemes (renderer gets terminated).
    qRegisterMetaType<QWebEnginePage::RenderProcessTerminationStatus>("RenderProcessTerminationStatus");
    QSignalSpy loadFinishedSpy(m_page, &QWebEnginePage::loadFinished);
    QSignalSpy renderProcessTerminatedSpy(m_page, &QWebEnginePage::renderProcessTerminated);
    m_page->load(QSL("tst:/resources/createObjectURL.html"));
    QVERIFY(!renderProcessTerminatedSpy.empty() || renderProcessTerminatedSpy.wait(20000));
    QVERIFY(renderProcessTerminatedSpy.front().value(0).value<QWebEnginePage::RenderProcessTerminationStatus>()
            != QWebEnginePage::NormalTerminationStatus);
    QVERIFY(loadFinishedSpy.empty());
}

QTEST_MAIN(tst_Origins)
#include "tst_origins.moc"
