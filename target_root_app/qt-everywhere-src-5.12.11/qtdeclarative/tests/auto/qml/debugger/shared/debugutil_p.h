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

#ifndef DEBUGUTIL_P_H
#define DEBUGUTIL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <../../../shared/util.h>
#include <private/qqmldebugclient_p.h>

class QQmlDebugProcess;
class QQmlDebugTest : public QQmlDataTest
{
    Q_OBJECT
public:
    static bool waitForSignal(QObject *receiver, const char *member, int timeout = 5000);
    static QList<QQmlDebugClient *> createOtherClients(QQmlDebugConnection *connection);
    static QString clientStateString(const QQmlDebugClient *client);
    static QString connectionStateString(const QQmlDebugConnection *connection);

protected:
    enum ConnectResult {
        ConnectSuccess,
        ProcessFailed,
        SessionFailed,
        ConnectionFailed,
        ClientsFailed,
        EnableFailed,
        RestrictFailed
    };

    ConnectResult connect(const QString &executable, const QString &services,
                          const QString &extraArgs, bool block);

    virtual QQmlDebugProcess *createProcess(const QString &executable);
    virtual QQmlDebugConnection *createConnection();
    virtual QList<QQmlDebugClient *> createClients();

    QQmlDebugProcess *m_process = nullptr;
    QQmlDebugConnection *m_connection = nullptr;
    QList<QQmlDebugClient *> m_clients;

protected slots:
    virtual void cleanup();
};

class QQmlDebugTestClient : public QQmlDebugClient
{
    Q_OBJECT
public:
    QQmlDebugTestClient(const QString &s, QQmlDebugConnection *c);

    QByteArray waitForResponse();

signals:
    void serverMessage(const QByteArray &);

protected:
    virtual void messageReceived(const QByteArray &ba);

private:
    QByteArray lastMsg;
};

class QQmlInspectorResultRecipient : public QObject
{
    Q_OBJECT
public:
    QQmlInspectorResultRecipient(QObject *parent = 0) :
        QObject(parent), lastResponseId(-1), lastResult(false) {}

    int lastResponseId;
    bool lastResult;

    void recordResponse(int requestId, bool result)
    {
        lastResponseId = requestId;
        lastResult = result;
    }
};

class ClientStateHandler : public QObject
{
    Q_OBJECT
public:
    ClientStateHandler(const QList<QQmlDebugClient *> &clients,
                       const QList<QQmlDebugClient *> &others,
                       QQmlDebugClient::State expectedOthers);

    ~ClientStateHandler();

    bool allEnabled() const { return m_allEnabled; }
    bool othersAsExpected() const { return m_othersAsExpected; }

signals:
    void allOk();

private:
    void checkStates();

    const QList<QQmlDebugClient *> m_clients;
    const QList<QQmlDebugClient *> m_others;
    const QQmlDebugClient::State m_expectedOthers;

    bool m_allEnabled = false;
    bool m_othersAsExpected = false;
};

QString debugJsServerPath(const QString &selfPath);

#endif // DEBUGUTIL_P_H
