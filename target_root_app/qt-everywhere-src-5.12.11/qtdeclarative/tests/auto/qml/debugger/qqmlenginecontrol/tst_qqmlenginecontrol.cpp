/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include "debugutil_p.h"
#include "../../../shared/util.h"

#include <private/qqmldebugclient_p.h>
#include <private/qqmldebugconnection_p.h>
#include <private/qpacket_p.h>
#include <private/qqmlenginecontrolclient_p.h>

#include <QtTest/qtest.h>
#include <QtCore/qlibraryinfo.h>

class QQmlEngineBlocker : public QObject
{
    Q_OBJECT
public:
    QQmlEngineBlocker(QQmlEngineControlClient *parent);

public slots:
    void blockEngine(int engineId, const QString &name);
};

QQmlEngineBlocker::QQmlEngineBlocker(QQmlEngineControlClient *parent): QObject(parent)
{
    connect(parent, &QQmlEngineControlClient::engineAboutToBeAdded,
            this, &QQmlEngineBlocker::blockEngine);
    connect(parent, &QQmlEngineControlClient::engineAboutToBeRemoved,
            this, &QQmlEngineBlocker::blockEngine);
}

void QQmlEngineBlocker::blockEngine(int engineId, const QString &name)
{
    Q_UNUSED(name);
    static_cast<QQmlEngineControlClient *>(parent())->blockEngine(engineId);
}

class tst_QQmlEngineControl : public QQmlDebugTest
{
    Q_OBJECT

private:
    ConnectResult connect(const QString &testFile, bool restrictServices);
    QList<QQmlDebugClient *> createClients() override;

    void engine_data();
    QPointer<QQmlEngineControlClient> m_client;

private slots:
    void startEngine_data();
    void startEngine();
    void stopEngine_data();
    void stopEngine();
};

QQmlDebugTest::ConnectResult tst_QQmlEngineControl::connect(const QString &file,
                                                            bool restrictServices)
{
    return QQmlDebugTest::connect(QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmlscene",
                                  restrictServices ? QStringLiteral("EngineControl") : QString(),
                                  testFile(file), true);
}

QList<QQmlDebugClient *> tst_QQmlEngineControl::createClients()
{
    m_client = new QQmlEngineControlClient(m_connection);
    new QQmlEngineBlocker(m_client);
    return QList<QQmlDebugClient *>({m_client});
}

void tst_QQmlEngineControl::engine_data()
{
    QTest::addColumn<bool>("restrictMode");
    QTest::newRow("unrestricted") << false;
    QTest::newRow("restricted") << true;
}

void tst_QQmlEngineControl::startEngine_data()
{
    engine_data();
}

void tst_QQmlEngineControl::startEngine()
{
    QFETCH(bool, restrictMode);
    QCOMPARE(connect("test.qml", restrictMode), ConnectSuccess);

    QTRY_VERIFY(!m_client->blockedEngines().empty());
    m_client->releaseEngine(m_client->blockedEngines().last());
    QVERIFY(m_client->blockedEngines().isEmpty());

    QVERIFY2(QQmlDebugTest::waitForSignal(m_client, SIGNAL(engineAdded(int,QString))),
             "No engine start message received in time.");

    QVERIFY(m_client->blockedEngines().isEmpty());
}

void tst_QQmlEngineControl::stopEngine_data()
{
    engine_data();
}

void tst_QQmlEngineControl::stopEngine()
{
    QFETCH(bool, restrictMode);

    QCOMPARE(connect("exit.qml", restrictMode), ConnectSuccess);

    QTRY_VERIFY(!m_client->blockedEngines().empty());
    m_client->releaseEngine(m_client->blockedEngines().last());
    QVERIFY(m_client->blockedEngines().isEmpty());

    QVERIFY2(QQmlDebugTest::waitForSignal(m_client, SIGNAL(engineAdded(int,QString))),
             "No engine start message received in time.");
    QVERIFY(m_client->blockedEngines().isEmpty());

    QVERIFY2(QQmlDebugTest::waitForSignal(m_client, SIGNAL(engineAboutToBeRemoved(int,QString))),
             "No engine about to stop message received in time.");
    m_client->releaseEngine(m_client->blockedEngines().last());
    QVERIFY(m_client->blockedEngines().isEmpty());
    QVERIFY2(QQmlDebugTest::waitForSignal(m_client, SIGNAL(engineRemoved(int,QString))),
             "No engine stop message received in time.");
    QVERIFY(m_client->blockedEngines().isEmpty());
}

QTEST_MAIN(tst_QQmlEngineControl)

#include "tst_qqmlenginecontrol.moc"
