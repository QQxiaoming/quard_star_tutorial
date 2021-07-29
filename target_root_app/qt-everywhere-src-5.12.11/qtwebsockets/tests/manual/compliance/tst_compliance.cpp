/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
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
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QHostInfo>
#include <QSslError>
#include <QDebug>
#include <QtWebSockets/QWebSocket>

class tst_ComplianceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void cleanupTestCase();

    void autobahnTest_data();
    void autobahnTest();
};

static const QUrl baseUrl { "ws://localhost:9001" };
static const auto agent = QStringLiteral("QtWebSockets/" QT_VERSION_STR);

void tst_ComplianceTest::cleanupTestCase()
{
    QWebSocket webSocket;
    QSignalSpy spy(&webSocket, &QWebSocket::disconnected);
    auto url = baseUrl;
    url.setPath(QStringLiteral("/updateReports"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("agent"), agent);
    url.setQuery(query);
    webSocket.open(url);
    QVERIFY(spy.wait());
}

void tst_ComplianceTest::autobahnTest_data()
{
    QTest::addColumn<int>("testCase");

    // Ask /getCaseCount how many tests we have
    QWebSocket webSocket;
    QSignalSpy spy(&webSocket, &QWebSocket::disconnected);

    connect(&webSocket, &QWebSocket::textMessageReceived, [](QString message) {
        bool ok;
        const auto numberOfTestCases = message.toInt(&ok);
        if (!ok)
            QSKIP("Unable to parse /getCaseCount result");
        for (auto i = 1; i <= numberOfTestCases; ++i)
            QTest::addRow("%d", i) << i;
    });

    auto url = baseUrl;
    url.setPath(QStringLiteral("/getCaseCount"));
    webSocket.open(url);
    if (!spy.wait())
        QSKIP("AutoBahn test server didn't deliver case-count");
}

void tst_ComplianceTest::autobahnTest()
{
    QFETCH(int, testCase);
    QWebSocket webSocket;
    QSignalSpy spy(&webSocket, &QWebSocket::disconnected);
    connect(&webSocket, &QWebSocket::textMessageReceived,
            &webSocket, &QWebSocket::sendTextMessage);
    connect(&webSocket, &QWebSocket::binaryMessageReceived,
            &webSocket, &QWebSocket::sendBinaryMessage);

    // Ask /runCase?case=<number>&agent=<agent> to run the test-case.
    auto url = baseUrl;
    url.setPath(QStringLiteral("/runCase"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("case"), QString::number(testCase));
    query.addQueryItem(QStringLiteral("agent"), agent);
    url.setQuery(query);
    webSocket.open(url);
    QVERIFY(spy.wait());
}

QTEST_MAIN(tst_ComplianceTest)

#include "tst_compliance.moc"
