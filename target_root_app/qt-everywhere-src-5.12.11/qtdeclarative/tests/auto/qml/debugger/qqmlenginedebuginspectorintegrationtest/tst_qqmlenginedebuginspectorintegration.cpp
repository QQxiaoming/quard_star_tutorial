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

#include "qqmlinspectorclient.h"
#include "qqmlenginedebugclient.h"
#include "../shared/debugutil_p.h"
#include "../../../shared/util.h"

#include <private/qqmldebugconnection_p.h>

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtNetwork/qhostaddress.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include <QtCore/qthread.h>
#include <QtCore/qlibraryinfo.h>

class tst_QQmlEngineDebugInspectorIntegration : public QQmlDebugTest
{
    Q_OBJECT

private:
    ConnectResult init(bool restrictServices);
    QList<QQmlDebugClient *> createClients() override;

    QmlDebugObjectReference findRootObject();

    QPointer<QQmlInspectorClient> m_inspectorClient;
    QPointer<QQmlEngineDebugClient> m_engineDebugClient;
    QPointer<QQmlInspectorResultRecipient> m_recipient;

private slots:
    void connect_data();
    void connect();
    void objectLocationLookup();
    void select();
    void createObject();
    void moveObject();
    void destroyObject();
};

QmlDebugObjectReference tst_QQmlEngineDebugInspectorIntegration::findRootObject()
{
    bool success = false;
    m_engineDebugClient->queryAvailableEngines(&success);

    if (!QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())))
        return QmlDebugObjectReference();

    m_engineDebugClient->queryRootContexts(m_engineDebugClient->engines()[0].debugId, &success);
    if (!QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())))
        return QmlDebugObjectReference();

    int count = m_engineDebugClient->rootContext().contexts.count();
    m_engineDebugClient->queryObject(
                m_engineDebugClient->rootContext().contexts[count - 1].objects[0], &success);
    if (!QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())))
        return QmlDebugObjectReference();
    return m_engineDebugClient->object();
}

QQmlDebugTest::ConnectResult tst_QQmlEngineDebugInspectorIntegration::init(bool restrictServices)
{
    return QQmlDebugTest::connect(
                QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qml",
                restrictServices ? QStringLiteral("QmlDebugger,QmlInspector") : QString(),
                testFile("qtquick2.qml"), true);
}

QList<QQmlDebugClient *> tst_QQmlEngineDebugInspectorIntegration::createClients()
{
    m_inspectorClient = new QQmlInspectorClient(m_connection);
    m_engineDebugClient = new QQmlEngineDebugClient(m_connection);
    m_recipient = new QQmlInspectorResultRecipient(m_inspectorClient);
    QObject::connect(m_inspectorClient.data(), &QQmlInspectorClient::responseReceived,
                     m_recipient.data(), &QQmlInspectorResultRecipient::recordResponse);
    return QList<QQmlDebugClient *>({m_inspectorClient, m_engineDebugClient});
}

void tst_QQmlEngineDebugInspectorIntegration::connect_data()
{
    QTest::addColumn<bool>("restrictMode");
    QTest::newRow("unrestricted") << false;
    QTest::newRow("restricted") << true;
}

void tst_QQmlEngineDebugInspectorIntegration::connect()
{
    QFETCH(bool, restrictMode);
    QCOMPARE(init(restrictMode), ConnectSuccess);
}

void tst_QQmlEngineDebugInspectorIntegration::objectLocationLookup()
{
    QCOMPARE(init(true), ConnectSuccess);

    bool success = false;
    QmlDebugObjectReference rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    const QString fileName = QFileInfo(rootObject.source.url.toString()).fileName();
    int lineNumber = rootObject.source.lineNumber;
    int columnNumber = rootObject.source.columnNumber;
    m_engineDebugClient->queryObjectsForLocation(fileName, lineNumber,
                                        columnNumber, &success);
    QVERIFY(success);
    QVERIFY(QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())));

    foreach (QmlDebugObjectReference child, rootObject.children) {
        success = false;
        lineNumber = child.source.lineNumber;
        columnNumber = child.source.columnNumber;
        m_engineDebugClient->queryObjectsForLocation(fileName, lineNumber,
                                       columnNumber, &success);
        QVERIFY(success);
        QVERIFY(QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())));
    }
}

void tst_QQmlEngineDebugInspectorIntegration::select()
{
    QCOMPARE(init(true), ConnectSuccess);

    QmlDebugObjectReference rootObject = findRootObject();
    QList<int> childIds;
    int requestId = 0;
    foreach (const QmlDebugObjectReference &child, rootObject.children) {
        requestId = m_inspectorClient->select(QList<int>() << child.debugId);
        QTRY_COMPARE(m_recipient->lastResponseId, requestId);
        QVERIFY(m_recipient->lastResult);
        childIds << child.debugId;
    }
    requestId = m_inspectorClient->select(childIds);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);
}

void tst_QQmlEngineDebugInspectorIntegration::createObject()
{
    QCOMPARE(init(true), ConnectSuccess);

    QString qml = QLatin1String("Rectangle {\n"
                                "  id: xxxyxxx\n"
                                "  width: 10\n"
                                "  height: 10\n"
                                "  color: \"blue\"\n"
                                "}");

    QmlDebugObjectReference rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    QCOMPARE(rootObject.children.length(), 2);

    int requestId = m_inspectorClient->createObject(
                qml, rootObject.debugId, QStringList() << QLatin1String("import QtQuick 2.0"),
                QLatin1String("testcreate.qml"));
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);

    rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    QCOMPARE(rootObject.children.length(), 3);
    QCOMPARE(rootObject.children[2].idString, QLatin1String("xxxyxxx"));
}

void tst_QQmlEngineDebugInspectorIntegration::moveObject()
{
    QCOMPARE(init(true), ConnectSuccess);

    QCOMPARE(m_inspectorClient->state(), QQmlDebugClient::Enabled);
    QmlDebugObjectReference rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    QCOMPARE(rootObject.children.length(), 2);

    int childId = rootObject.children[0].debugId;
    int requestId = m_inspectorClient->moveObject(childId, rootObject.children[1].debugId);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);

    rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    QCOMPARE(rootObject.children.length(), 1);
    bool success = false;
    m_engineDebugClient->queryObject(rootObject.children[0], &success);
    QVERIFY(success);
    QVERIFY(QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())));
    QCOMPARE(m_engineDebugClient->object().children.length(), 1);
    QCOMPARE(m_engineDebugClient->object().children[0].debugId, childId);
}

void tst_QQmlEngineDebugInspectorIntegration::destroyObject()
{
    QCOMPARE(init(true), ConnectSuccess);

    QCOMPARE(m_inspectorClient->state(), QQmlDebugClient::Enabled);
    QmlDebugObjectReference rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    QCOMPARE(rootObject.children.length(), 2);

    int requestId = m_inspectorClient->destroyObject(rootObject.children[0].debugId);
    QTRY_COMPARE(m_recipient->lastResponseId, requestId);
    QVERIFY(m_recipient->lastResult);

    rootObject = findRootObject();
    QVERIFY(rootObject.debugId != -1);
    QCOMPARE(rootObject.children.length(), 1);
    bool success = false;
    m_engineDebugClient->queryObject(rootObject.children[0], &success);
    QVERIFY(success);
    QVERIFY(QQmlDebugTest::waitForSignal(m_engineDebugClient, SIGNAL(result())));
    QCOMPARE(m_engineDebugClient->object().children.length(), 0);
}

QTEST_MAIN(tst_QQmlEngineDebugInspectorIntegration)

#include "tst_qqmlenginedebuginspectorintegration.moc"
