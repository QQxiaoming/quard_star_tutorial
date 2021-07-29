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

#include <QtTest/QtTest>

#include <qwebengineview.h>
#include <qwebenginepage.h>
#include <qwebengineprofile.h>
#include <qwebenginesettings.h>

class tst_Schemes : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void unknownUrlSchemePolicy();
};

class AcceptNavigationRequestHandler : public QWebEnginePage
{
public:
    AcceptNavigationRequestHandler(QObject* parent = nullptr)
        : QWebEnginePage(parent)
    {
    }
    int acceptNavigationRequestCalls = 0;
    bool acceptNavigationRequest(const QUrl &/*url*/, NavigationType type, bool /*isMainFrame*/) override
    {
        if (type == QWebEnginePage::NavigationTypeTyped)
            return true;
        this->acceptNavigationRequestCalls++;
        return false;
    }
};

void tst_Schemes::unknownUrlSchemePolicy()
{
    QWebEngineView view;
    AcceptNavigationRequestHandler page;
    QSignalSpy loadFinishedSpy(&page, &QWebEnginePage::loadFinished);
    view.setPage(&page);
    view.resize(400, 40);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QWebEngineSettings *settings = view.page()->profile()->settings();
    settings->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    QWebEngineSettings::UnknownUrlSchemePolicy policies[6] = {QWebEngineSettings::DisallowUnknownUrlSchemes,
                                                              QWebEngineSettings::DisallowUnknownUrlSchemes,
                                                              QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction,
                                                              QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction,
                                                              QWebEngineSettings::AllowAllUnknownUrlSchemes,
                                                              QWebEngineSettings::AllowAllUnknownUrlSchemes};
    // even iterations are for navigation-requests from javascript,
    // odd iterations are for navigations-requests from user-interaction
    for (int i = 0; i < 8; i++) {
        if (i <= 5)
            settings->setUnknownUrlSchemePolicy(policies[i]);
        else
            settings->resetUnknownUrlSchemePolicy();
        loadFinishedSpy.clear();
        page.acceptNavigationRequestCalls = 0;
        bool shouldAccept;

        if (i % 2 == 0) { // navigation request coming from javascript
            shouldAccept = (4 <= i && i <= 5); // only case AllowAllUnknownUrlSchemes
            view.setHtml("<html><script>setTimeout(function(){ window.location.href='nonexistentscheme://somewhere'; }, 10);</script><body>testing...</body></html>");
        } else { // navigation request coming from user interaction
            shouldAccept = (2 <= i); // all cases except DisallowUnknownUrlSchemes
            view.setHtml("<html><body><a id='nonexlink' href='nonexistentscheme://somewhere'>nonexistentscheme://somewhere</a></body></html>");
            QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 1, 15000);
            // focus and trigger the link
            view.page()->runJavaScript("document.getElementById('nonexlink').focus();", [&view](const QVariant &result) {
                Q_UNUSED(result);
                QTest::sendKeyEvent(QTest::Press, view.focusProxy(), Qt::Key_Return, QString("\r"), Qt::NoModifier);
                QTest::sendKeyEvent(QTest::Release, view.focusProxy(), Qt::Key_Return, QString("\r"), Qt::NoModifier);
            });
        }

        QTRY_COMPARE_WITH_TIMEOUT(loadFinishedSpy.size(), 2, 15000);
        QCOMPARE(page.acceptNavigationRequestCalls, shouldAccept ? 1 : 0);
    }
}

QTEST_MAIN(tst_Schemes)
#include "tst_schemes.moc"
