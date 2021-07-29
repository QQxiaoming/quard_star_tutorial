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

class tst_Restart: public QObject
{
    Q_OBJECT

public:
    enum RunMode { Baseline, ServerRestartGraceful, ServerRestartFatal };
    enum ObjectMode { NullPointer, ObjectPointer };
    Q_ENUM(RunMode)
    Q_ENUM(ObjectMode)

private slots:
    void initTestCase()
    {
        QLoggingCategory::setFilterRules("qt.remoteobjects.warning=false");
    }

    void cleanup()
    {
        // wait for delivery of RemoveObject events to the source
        QTest::qWait(200);
    }

    void testRun_data()
    {
        QTest::addColumn<RunMode>("runMode");
        QTest::addColumn<ObjectMode>("objectMode");
        auto runModeMeta = QMetaEnum::fromType<RunMode>();
        auto objectModeMeta = QMetaEnum::fromType<ObjectMode>();
        for (int i = 0; i < runModeMeta.keyCount(); i++) {
            for (int j = 0; j < objectModeMeta.keyCount(); j++) {
                auto ba = QByteArray(runModeMeta.valueToKey(i));
                ba = ba.append("_").append(objectModeMeta.valueToKey(j));
                QTest::newRow(ba.data()) << static_cast<RunMode>(i) << static_cast<ObjectMode>(j);
            }
        }
    }

    void testRun()
    {
        QFETCH(RunMode, runMode);
        QFETCH(ObjectMode, objectMode);

        qDebug() << "Starting server process" << runMode;
        bool serverRestart = runMode == ServerRestartFatal || runMode == ServerRestartGraceful;
        QProcess serverProc;
        serverProc.setProcessChannelMode(QProcess::ForwardedChannels);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("RunMode", QVariant::fromValue(runMode).toString());
        env.insert("ObjectMode", QVariant::fromValue(objectMode).toString());
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

        if (serverRestart) {
            env.insert("RunMode", QVariant::fromValue(Baseline).toString()); // Don't include ServerRestart environment variable this time
            qDebug() << "Waiting for server exit";
            QVERIFY(serverProc.waitForFinished());
            if (runMode == ServerRestartFatal)
                QVERIFY(serverProc.exitCode() != 0);
            else
                QCOMPARE(serverProc.exitCode(), 0);
            qDebug() << "Restarting server";
            serverProc.setProcessEnvironment(env);
            serverProc.start(findExecutable("server", {
                QCoreApplication::applicationDirPath() + "/../server/"
            }));
            QVERIFY(serverProc.waitForStarted());
        }

        QVERIFY(clientProc.waitForFinished());
        QVERIFY(serverProc.waitForFinished());

        QCOMPARE(serverProc.exitCode(), 0);
        QCOMPARE(clientProc.exitCode(), 0);
    }
};

QTEST_MAIN(tst_Restart)

#include "tst_restart.moc"
