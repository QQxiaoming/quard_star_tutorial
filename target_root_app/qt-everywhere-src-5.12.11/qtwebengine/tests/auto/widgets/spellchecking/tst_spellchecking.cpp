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

#include "util.h"
#include <QtTest/QtTest>
#include <QtWebEngineWidgets/qwebenginecontextmenudata.h>
#include <QtWebEngineWidgets/qwebengineprofile.h>
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineview.h>
#include <qwebenginesettings.h>

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    void activateMenu(QWidget *widget, const QPoint &position)
    {
        QTest::mouseMove(widget, position);
        QTest::mousePress(widget, Qt::RightButton, 0, position);
        QContextMenuEvent evcont(QContextMenuEvent::Mouse, position, mapToGlobal(position));
        event(&evcont);
        QTest::mouseRelease(widget, Qt::RightButton, 0, position);
    }

    const QWebEngineContextMenuData& data()
    {
        return m_data;
    }

signals:
    void menuReady();

protected:
    void contextMenuEvent(QContextMenuEvent *)
    {
        m_data = page()->contextMenuData();
        emit menuReady();
    }
private:
    QWebEngineContextMenuData m_data;
};

class tst_Spellchecking : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void initTestCase();
    void spellCheckLanguage();
    void spellCheckLanguages();
    void spellCheckEnabled();
    void spellcheck();
    void spellcheck_data();

private:
    void load();
    WebView *m_view;
};

void tst_Spellchecking::initTestCase()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    QVERIFY(!profile->isSpellCheckEnabled());
    QVERIFY(profile->spellCheckLanguages().isEmpty());
}

void tst_Spellchecking::init()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setSpellCheckEnabled(false);
    profile->setSpellCheckLanguages(QStringList());
    m_view = new WebView();
}

void tst_Spellchecking::load()
{
    m_view->page()->load(QUrl("qrc:///resources/index.html"));
    m_view->show();
    QSignalSpy spyFinished(m_view->page(), &QWebEnginePage::loadFinished);
    QVERIFY(spyFinished.wait());

}

void tst_Spellchecking::cleanup()
{
    delete m_view;
}

void tst_Spellchecking::spellCheckLanguage()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    profile->setSpellCheckLanguages({"en-US"});
    QVERIFY(profile->spellCheckLanguages() == QStringList({"en-US"}));
}

void tst_Spellchecking::spellCheckLanguages()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    profile->setSpellCheckLanguages({"en-US","de-DE"});
    QVERIFY(profile->spellCheckLanguages() == QStringList({"en-US","de-DE"}));
}


void tst_Spellchecking::spellCheckEnabled()
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    profile->setSpellCheckEnabled(true);
    QVERIFY(profile->isSpellCheckEnabled());
}

void tst_Spellchecking::spellcheck()
{
    QFETCH(QStringList, languages);
    QFETCH(QStringList, suggestions);

    m_view->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QVERIFY(profile);
    profile->setSpellCheckLanguages(languages);
    profile->setSpellCheckEnabled(true);
    load();
    QCOMPARE(profile->spellCheckLanguages(), languages);

    // make textarea editable
    evaluateJavaScriptSync(m_view->page(), "makeEditable();");

    // calcuate position of misspelled word
    QVariantList list = evaluateJavaScriptSync(m_view->page(), "findWordPosition('I lowe Qt ....','lowe');").toList();
    QRect rect(list[0].value<int>(),list[1].value<int>(),list[2].value<int>(),list[3].value<int>());

    //type text, spellchecker needs time
    QTest::mouseMove(m_view->focusWidget(), QPoint(20,20));
    QTest::mousePress(m_view->focusWidget(), Qt::LeftButton, 0, QPoint(20,20));
    QTest::mouseRelease(m_view->focusWidget(), Qt::LeftButton, 0, QPoint(20,20));
    QString text("I lowe Qt ....");
    for (int i = 0; i < text.length(); i++) {
        QTest::keyClicks(m_view->focusWidget(), text.at(i));
        QTest::qWait(60);
    }

    // make sure text is there
    QString result = evaluateJavaScriptSync(m_view->page(), "text();").toString();
    QVERIFY(result == text);

    bool gotMisspelledWord = false; // clumsy QTRY_VERIFY still execs expr after first success
    QString detail;

    // check that spellchecker has done text processing and filled misspelled word
    QTRY_VERIFY2([&] () {
        detail.clear();
        if (gotMisspelledWord)
            return true;

        // open menu on misspelled word
        m_view->activateMenu(m_view->focusWidget(), rect.center());
        QSignalSpy spyMenuReady(m_view, &WebView::menuReady);
        if (!spyMenuReady.wait()) {
            detail = "menu was not shown";
            return false;
        }

        if (!m_view->data().isValid()) {
            detail = "invalid data";
            return false;
        }

        if (!m_view->data().isContentEditable()) {
            detail = "content is not editable";
            return false;
        }

        if (m_view->data().misspelledWord().isEmpty()) {
            detail = "no misspelled word";
            return false;
        };

        gotMisspelledWord = true;
        return true;
    } (), qPrintable(QString("Context menu: %1").arg(detail)));

    // check misspelled word
    QCOMPARE(m_view->data().misspelledWord(), QStringLiteral("lowe"));

    // check suggestions
    QCOMPARE(m_view->data().spellCheckerSuggestions(), suggestions);

    // check replace word
    m_view->page()->replaceMisspelledWord("love");
    text = "I love Qt ....";
    QTRY_VERIFY(evaluateJavaScriptSync(m_view->page(), "text();").toString() == text);
}

void tst_Spellchecking::spellcheck_data()
{
    QTest::addColumn<QStringList>("languages");
    QTest::addColumn<QStringList>("suggestions");
    QTest::newRow("en-US") << QStringList({"en-US"}) << QStringList({"low", "love"});
    QTest::newRow("en-US,de-DE") << QStringList({"en-US", "de-DE"}) << QStringList({"löwe", "low", "love"});
}

QTEST_MAIN(tst_Spellchecking)
#include "tst_spellchecking.moc"
