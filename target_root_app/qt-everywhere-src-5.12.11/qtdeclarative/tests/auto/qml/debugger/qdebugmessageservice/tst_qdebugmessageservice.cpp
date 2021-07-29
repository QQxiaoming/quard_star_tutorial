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

//QQmlDebugTest
#include "debugutil_p.h"

#include <private/qqmldebugclient_p.h>
#include <private/qqmldebugconnection_p.h>
#include <private/qpacket_p.h>

#include <QtCore/qstring.h>
#include <QtCore/qlibraryinfo.h>
#include <QtTest/qtest.h>

const char *QMLFILE = "test.qml";

class QQmlDebugMsgClient;
class tst_QDebugMessageService : public QQmlDebugTest
{
    Q_OBJECT

private slots:
    void retrieveDebugOutput();

private:
    QList<QQmlDebugClient *> createClients() override;
    QPointer<QQmlDebugMsgClient> m_client;
};

struct LogEntry {
    LogEntry(QtMsgType _type, QString _message)
        : type(_type), message(_message) {}

    QtMsgType type;
    QString message;
    int line;
    QString file;
    QString function;
    QString category;

    QString toString() const
    {
        return QString::number(type) + ": " + message + " (" + category + ")";
    }
};

bool operator==(const LogEntry &t1, const LogEntry &t2)
{
    return t1.type == t2.type && t1.message == t2.message
            && t1.line == t2.line && t1.file == t2.file
            && t1.function == t2.function && t1.category == t2.category;
}

class QQmlDebugMsgClient : public QQmlDebugClient
{
    Q_OBJECT
public:
    QQmlDebugMsgClient(QQmlDebugConnection *connection)
        : QQmlDebugClient(QLatin1String("DebugMessages"), connection)
    {
    }

    QList<LogEntry> logBuffer;

protected:
    //inherited from QQmlDebugClient
    void messageReceived(const QByteArray &data);

signals:
    void debugOutput();
};

void QQmlDebugMsgClient::messageReceived(const QByteArray &data)
{
    QPacket ds(connection()->currentDataStreamVersion(), data);
    QByteArray command;
    ds >> command;

    if (command == "MESSAGE") {
        int type;
        QByteArray message;
        QByteArray file;
        QByteArray function;
        QByteArray category;
        qint64 timestamp;
        int line;
        ds >> type >> message >> file >> line >> function >> category >> timestamp;
        QVERIFY(ds.atEnd());

        QVERIFY(type >= QtDebugMsg);
        QVERIFY(type <= QtInfoMsg);
        QVERIFY(timestamp > 0);

        LogEntry entry((QtMsgType)type, QString::fromUtf8(message));
        entry.line = line;
        entry.file = QString::fromUtf8(file);
        entry.function = QString::fromUtf8(function);
        entry.category = QString::fromUtf8(category);
        logBuffer << entry;
        emit debugOutput();
    } else {
        QFAIL("Unknown message");
    }
}

QList<QQmlDebugClient *> tst_QDebugMessageService::createClients()
{
    m_client = new QQmlDebugMsgClient(m_connection);
    return QList<QQmlDebugClient *>({m_client});
}

void tst_QDebugMessageService::retrieveDebugOutput()
{
    QCOMPARE(QQmlDebugTest::connect(QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qml",
                                    QString(), testFile(QMLFILE), true), ConnectSuccess);

    QTRY_VERIFY(m_client->logBuffer.size() >= 2);

    const QString path =
            QUrl::fromLocalFile(QQmlDataTest::instance()->testFile(QMLFILE)).toString();
    LogEntry entry1(QtDebugMsg, QLatin1String("console.log"));
    entry1.line = 35;
    entry1.file = path;
    entry1.function = QLatin1String("onCompleted");
    entry1.category = QLatin1String("qml");
    LogEntry entry2(QtDebugMsg, QLatin1String("console.count: 1"));
    entry2.line = 36;
    entry2.file = path;
    entry2.function = QLatin1String("onCompleted");
    entry2.category = QLatin1String("default");

    QVERIFY(m_client->logBuffer.contains(entry1));
    QVERIFY(m_client->logBuffer.contains(entry2));
}

QTEST_MAIN(tst_QDebugMessageService)

#include "tst_qdebugmessageservice.moc"
