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

#include <QtTest/QtTest>

#include <qwebenginepage.h>

class tst_DevTools : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void attachAndDestroyPageFirst();
    void attachAndDestroyInspectorFirst();
};

void tst_DevTools::attachAndDestroyPageFirst()
{
    // External inspector + manual destruction of page first
    QWebEnginePage* page = new QWebEnginePage();
    QWebEnginePage* inspector = new QWebEnginePage();

    QSignalSpy spy(page, &QWebEnginePage::loadFinished);
    page->load(QUrl("data:text/plain,foobarbaz"));
    QTRY_COMPARE(spy.count(),  1);

    inspector->setInspectedPage(page);
    page->triggerAction(QWebEnginePage::InspectElement);

    // This is deliberately racy:
    QTest::qWait(10);

    delete page;
    delete inspector;
}

void tst_DevTools::attachAndDestroyInspectorFirst()
{
    // External inspector + manual destruction of inspector first
    QWebEnginePage* page = new QWebEnginePage();
    QWebEnginePage* inspector = new QWebEnginePage();
    inspector->setInspectedPage(page);

    QSignalSpy spy(page, &QWebEnginePage::loadFinished);
    page->setHtml(QStringLiteral("<body><h1>FOO BAR!</h1></body>"));
    QTRY_COMPARE(spy.count(),  1);

    page->triggerAction(QWebEnginePage::InspectElement);

    delete inspector;

    page->triggerAction(QWebEnginePage::InspectElement);

    // This is deliberately racy:
    QTest::qWait(10);

    delete page;
}


QTEST_MAIN(tst_DevTools)

#include "tst_devtools.moc"
