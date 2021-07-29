/****************************************************************************
**
** Copyright (C) 2018 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
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

#include "mytestserver.h"

#include <QCoreApplication>
#include <QTcpServer>
#include <QtTest/QtTest>

const QUrl registryUrl = QUrl(QStringLiteral("tcp://127.0.0.1:65212"));
const QUrl extUrl = QUrl(QStringLiteral("exttcp://127.0.0.1:65213"));
const QUrl extUrl2 = QUrl(QStringLiteral("exttcp://127.0.0.1:65214"));

class tst_Server_Process : public QObject
{
    Q_OBJECT

    struct Device
    {
        Device(QUrl url) : srcNode(url, registryUrl, QRemoteObjectHost::AllowExternalRegistration)
        {
            tcpServer.listen(QHostAddress(url.host()), url.port());
            QVERIFY(srcNode.waitForRegistry(3000));
            QObject::connect(&tcpServer, &QTcpServer::newConnection, [this]() {
                auto conn = this->tcpServer.nextPendingConnection();
                this->srcNode.addHostSideConnection(conn);
            });
        }
        QTcpServer tcpServer;
        QRemoteObjectHost srcNode;
    };

private Q_SLOTS:
    void testRun()
    {
        QRemoteObjectRegistryHost registry(registryUrl);

        Device dev1(extUrl);
        MyTestServer myTestServer;
        bool templated = qEnvironmentVariableIsSet("TEMPLATED_REMOTING");
        if (templated)
            QVERIFY(dev1.srcNode.enableRemoting<MyInterfaceSourceAPI>(&myTestServer));
        else
            QVERIFY(dev1.srcNode.enableRemoting(&myTestServer));

        qDebug() << "Waiting for incoming connections";

        QSignalSpy waitForStartedSpy(&myTestServer, &MyTestServer::startedChanged);
        QVERIFY(waitForStartedSpy.isValid());
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), true);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Client connected";

        // BEGIN: Testing

        // make sure continuous changes to enums don't mess up the protocol
        myTestServer.setEnum1(MyTestServer::Second);
        myTestServer.setEnum1(MyTestServer::Third);

        emit myTestServer.advance();

        waitForStartedSpy.clear();
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), false);

        bool next = false;
        connect(&myTestServer, &MyTestServer::nextStep, [&next]{ next = true; });
        QTRY_VERIFY_WITH_TIMEOUT(next, 5000);
        dev1.srcNode.disableRemoting(&myTestServer);

        // Change a value while replica is suspect
        myTestServer.setEnum1(MyTestServer::First);

        // Share the object on a different "device", make sure registry updates and connects
        Device dev2(extUrl2);
        dev2.srcNode.enableRemoting(&myTestServer);

        // wait for quit
        bool quit = false;
        connect(&myTestServer, &MyTestServer::quitApp, [&quit]{quit = true;});
        QTRY_VERIFY_WITH_TIMEOUT(quit, 5000);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Done. Shutting down.";
    }
};

QTEST_MAIN(tst_Server_Process)

#include "main.moc"
