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

#include <QtTest/QtTest>
#include <QRemoteObjectNode>
#include <QScopedPointer>
#include "rep_pingpong_replica.h"

namespace {

QString findExecutable(const QString &executableName, const QStringList &paths)
{
    const auto path = QStandardPaths::findExecutable(executableName, paths);
    if (!path.isEmpty()) {
        return path;
    }

    qWarning() << "Could not find executable:" << executableName << "in any of" << paths;
    return QString();
}

}

class tst_clientSSL: public QObject
{
    Q_OBJECT
public:
    tst_clientSSL() = default;

private slots:
    void testRun()
    {
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        serverProc.start(findExecutable("sslTestServer", {
            QCoreApplication::applicationDirPath() + "/../sslTestServer/"
        }));
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);
        QRemoteObjectNode m_client;
        auto config = QSslConfiguration::defaultConfiguration();
        config.setCaCertificates(QSslCertificate::fromPath(QStringLiteral(":/sslcert/rootCA.pem")));
        QSslConfiguration::setDefaultConfiguration(config);

        QScopedPointer<QSslSocket> socketClient{new QSslSocket};
        socketClient->setLocalCertificate(QStringLiteral(":/sslcert/client.crt"));
        socketClient->setPrivateKey(QStringLiteral(":/sslcert/client.key"));
        socketClient->setPeerVerifyMode(QSslSocket::VerifyPeer);
        socketClient->connectToHostEncrypted(QStringLiteral("127.0.0.1"), 65111);
        QVERIFY(socketClient->waitForEncrypted(-1));

        connect(socketClient.data(), QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error),
                socketClient.data(), [](QAbstractSocket::SocketError error){
            QCOMPARE(error, QAbstractSocket::RemoteHostClosedError);
        });
        m_client.addClientSideConnection(socketClient.data());

        QScopedPointer<PingPongReplica> pp{m_client.acquire<PingPongReplica>()};
        QVERIFY(pp->waitForSource());

        QString pongStr;
        connect(pp.data(), &PingPongReplica::pong, [&pongStr](const QString &str) {
            pongStr = str;
        });
        pp->ping("yahoo");
        QTRY_COMPARE(pongStr, "Pong yahoo");
        pp->ping("one more");
        QTRY_COMPARE(pongStr, "Pong one more");
        pp->ping("last one");
        QTRY_COMPARE(pongStr, "Pong last one");
        pp->quit();
        QTRY_VERIFY(serverProc.state() != QProcess::Running);
        QCOMPARE(serverProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_clientSSL)

#include "tst_client.moc"
