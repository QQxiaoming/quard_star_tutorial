/****************************************************************************
**
** Copyright (C) 2019 Ford Motor Company
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
#include <QMetaType>
#include <QProcess>
#include <QStandardPaths>

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

class tst_Proxy_MultiProcess: public QObject
{
    Q_OBJECT

public:
    enum ObjectMode { NullPointer, ObjectPointer };
    Q_ENUM(ObjectMode)

private slots:
    void initTestCase()
    {
    }

    void cleanup()
    {
        // wait for delivery of RemoveObject events to the source
        QTest::qWait(200);
    }

    void testRun_data()
    {
        QTest::addColumn<bool>("templated");
        QTest::addColumn<ObjectMode>("objectMode");
        QTest::newRow("non-templated, subobject") << false << ObjectPointer;
        QTest::newRow("templated, subobject") << true << ObjectPointer;
        QTest::newRow("non-templated, nullptr") << false << NullPointer;
        QTest::newRow("templated, nullptr") << true << NullPointer;
    }

    void testRun()
    {
        QFETCH(bool, templated);
        QFETCH(ObjectMode, objectMode);

        qDebug() << "Starting server process";
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("ObjectMode", QVariant::fromValue(objectMode).toString());
        if (templated) {
            env.insert("TEMPLATED_REMOTING", "true");
        }
        serverProc.setProcessEnvironment(env);
        serverProc.start(findExecutable("server", {
            QCoreApplication::applicationDirPath() + "/../server/"
        }));
        QVERIFY(serverProc.waitForStarted());

        // wait for server start
        QTest::qWait(200);


        qDebug() << "Starting client process";
        QProcess clientProc;
        clientProc.setProcessChannelMode(QProcess::ForwardedChannels);
        clientProc.setProcessEnvironment(env);
        clientProc.start(findExecutable("client", {
            QCoreApplication::applicationDirPath() + "/../client/"
        }));
        QVERIFY(clientProc.waitForStarted());

        // wait for client start
        QTest::qWait(200);


        qDebug() << "Starting proxy process";
        QProcess proxyProc;
        proxyProc.setProcessChannelMode(QProcess::ForwardedChannels);
        proxyProc.start(findExecutable("proxy", {
            QCoreApplication::applicationDirPath() + "/../proxy/"
        }));
        QVERIFY(proxyProc.waitForStarted());

        // wait for proxy start
        QTest::qWait(200);


        QVERIFY(clientProc.waitForFinished());
        QVERIFY(proxyProc.waitForFinished());
        QVERIFY(serverProc.waitForFinished());

        QCOMPARE(serverProc.exitCode(), 0);
        QCOMPARE(proxyProc.exitCode(), 0);
        QCOMPARE(clientProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Proxy_MultiProcess)

#include "tst_proxy_multiprocess.moc"
