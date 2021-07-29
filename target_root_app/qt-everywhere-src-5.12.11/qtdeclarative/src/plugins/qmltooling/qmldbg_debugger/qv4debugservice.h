/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QV4DEBUGSERVICE_H
#define QV4DEBUGSERVICE_H

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

#include "qv4debuggeragent.h"
#include "qv4datacollector.h"
#include <private/qqmlconfigurabledebugservice_p.h>
#include <private/qqmldebugserviceinterfaces_p.h>
#include <private/qv4debugging_p.h>

#include <QtCore/QJsonValue>

QT_BEGIN_NAMESPACE

namespace QV4 { struct ExecutionEngine; }

class VariableCollector;
class V4CommandHandler;
class UnknownV4CommandHandler;
class QV4DebugServiceImpl;

class QV4DebugServiceImpl : public QQmlConfigurableDebugService<QV4DebugService>
{
    Q_OBJECT
public:
    explicit QV4DebugServiceImpl(QObject *parent = nullptr);
    ~QV4DebugServiceImpl() override;

    void engineAdded(QJSEngine *engine) override;
    void engineAboutToBeRemoved(QJSEngine *engine) override;

    void stateAboutToBeChanged(State state) override;

    void signalEmitted(const QString &signal) override;
    void send(QJsonObject v4Payload);

    int selectedFrame() const;
    void selectFrame(int frameNr);

    QV4DebuggerAgent debuggerAgent;

protected:
    void messageReceived(const QByteArray &) override;
    void sendSomethingToSomebody(const char *type, int magicNumber = 1);

private:
    friend class QQmlDebuggerServiceFactory;

    void handleV4Request(const QByteArray &payload);
    static QByteArray packMessage(const QByteArray &command,
                                  const QByteArray &message = QByteArray());
    void processCommand(const QByteArray &command, const QByteArray &data);
    V4CommandHandler *v4CommandHandler(const QString &command) const;

    QStringList breakOnSignals;
    static int sequence;
    int theSelectedFrame;

    void addHandler(V4CommandHandler* handler);
    QHash<QString, V4CommandHandler*> handlers;
    QScopedPointer<UnknownV4CommandHandler> unknownV4CommandHandler;
};

QT_END_NAMESPACE

#endif // QV4DEBUGSERVICE_H
