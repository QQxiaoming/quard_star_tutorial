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
#include "qqmldebugprocess_p.h"
#include "../shared/qqmlenginedebugclient.h"
#include "../../../shared/util.h"

#include <private/qqmldebugclient_p.h>
#include <private/qqmldebugconnection_p.h>
#include <private/qpacket_p.h>

#include <QtTest/qtest.h>
#include <QtTest/qtestsystem.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtimer.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qmutex.h>
#include <QtCore/qlibraryinfo.h>
#include <QtQml/qjsengine.h>

const char *V8REQUEST = "v8request";
const char *V8MESSAGE = "v8message";
const char *SEQ = "seq";
const char *TYPE = "type";
const char *COMMAND = "command";
const char *ARGUMENTS = "arguments";
const char *STEPACTION = "stepaction";
const char *STEPCOUNT = "stepcount";
const char *EXPRESSION = "expression";
const char *FRAME = "frame";
const char *CONTEXT = "context";
const char *GLOBAL = "global";
const char *DISABLEBREAK = "disable_break";
const char *HANDLES = "handles";
const char *INCLUDESOURCE = "includeSource";
const char *FROMFRAME = "fromFrame";
const char *TOFRAME = "toFrame";
const char *BOTTOM = "bottom";
const char *NUMBER = "number";
const char *FRAMENUMBER = "frameNumber";
const char *TYPES = "types";
const char *IDS = "ids";
const char *FILTER = "filter";
const char *FROMLINE = "fromLine";
const char *TOLINE = "toLine";
const char *TARGET = "target";
const char *LINE = "line";
const char *COLUMN = "column";
const char *ENABLED = "enabled";
const char *CONDITION = "condition";
const char *IGNORECOUNT = "ignoreCount";
const char *BREAKPOINT = "breakpoint";
const char *FLAGS = "flags";

const char *CONTINEDEBUGGING = "continue";
const char *EVALUATE = "evaluate";
const char *LOOKUP = "lookup";
const char *BACKTRACE = "backtrace";
const char *SCOPE = "scope";
const char *SCOPES = "scopes";
const char *SCRIPTS = "scripts";
const char *SOURCE = "source";
const char *SETBREAKPOINT = "setbreakpoint";
const char *CLEARBREAKPOINT = "clearbreakpoint";
const char *CHANGEBREAKPOINT = "changebreakpoint";
const char *SETEXCEPTIONBREAK = "setexceptionbreak";
const char *VERSION = "version";
const char *DISCONNECT = "disconnect";
const char *GARBAGECOLLECTOR = "gc";

const char *CONNECT = "connect";
const char *INTERRUPT = "interrupt";

const char *REQUEST = "request";
const char *IN = "in";
const char *NEXT = "next";
const char *OUT = "out";

const char *SCRIPT = "script";
const char *SCRIPTREGEXP = "scriptRegExp";
const char *EVENT = "event";

const char *ALL = "all";
const char *UNCAUGHT = "uncaught";

const char *BLOCKMODE = "-qmljsdebugger=port:3771,3800,block";
const char *NORMALMODE = "-qmljsdebugger=port:3771,3800";
const char *BLOCKRESTRICTEDMODE = "-qmljsdebugger=port:3771,3800,block,services:V8Debugger";
const char *NORMALRESTRICTEDMODE = "-qmljsdebugger=port:3771,3800,services:V8Debugger";
const char *TEST_QMLFILE = "test.qml";
const char *TEST_JSFILE = "test.js";
const char *TIMER_QMLFILE = "timer.qml";
const char *LOADJSFILE_QMLFILE = "loadjsfile.qml";
const char *EXCEPTION_QMLFILE = "exception.qml";
const char *ONCOMPLETED_QMLFILE = "oncompleted.qml";
const char *CREATECOMPONENT_QMLFILE = "createComponent.qml";
const char *CONDITION_QMLFILE = "condition.qml";
const char *QUIT_QMLFILE = "quit.qml";
const char *CHANGEBREAKPOINT_QMLFILE = "changeBreakpoint.qml";
const char *STEPACTION_QMLFILE = "stepAction.qml";
const char *BREAKPOINTRELOCATION_QMLFILE = "breakpointRelocation.qml";
const char *ENCODEQMLSCOPE_QMLFILE = "encodeQmlScope.qml";
const char *BREAKONANCHOR_QMLFILE = "breakOnAnchor.qml";

#define VARIANTMAPINIT \
    QString obj("{}"); \
    QJSValue jsonVal = parser.call(QJSValueList() << obj); \
    jsonVal.setProperty(SEQ,QJSValue(seq++)); \
    jsonVal.setProperty(TYPE,REQUEST);


#undef QVERIFY
#define QVERIFY(statement) \
do {\
    if (!QTest::qVerify((statement), #statement, "", __FILE__, __LINE__)) {\
        if (QTest::currentTestFailed()) \
          qDebug().nospace() << "\nDEBUGGEE OUTPUT:\n" << m_process->output();\
        return;\
    }\
} while (0)


class QJSDebugClient;

class tst_QQmlDebugJS : public QQmlDebugTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;

    void connect_data();
    void connect();
    void interrupt_data() { targetData(); }
    void interrupt();
    void getVersion_data() { targetData(); }
    void getVersion();
    void getVersionWhenAttaching_data() { targetData(); }
    void getVersionWhenAttaching();

    void disconnect_data() { targetData(); }
    void disconnect();

    void setBreakpointInScriptOnCompleted_data() { targetData(); }
    void setBreakpointInScriptOnCompleted();
    void setBreakpointInScriptOnComponentCreated_data() { targetData(); }
    void setBreakpointInScriptOnComponentCreated();
    void setBreakpointInScriptOnTimerCallback_data() { targetData(); }
    void setBreakpointInScriptOnTimerCallback();
    void setBreakpointInScriptInDifferentFile_data() { targetData(); }
    void setBreakpointInScriptInDifferentFile();
    void setBreakpointInScriptOnComment_data() { targetData(); }
    void setBreakpointInScriptOnComment();
    void setBreakpointInScriptOnEmptyLine_data() { targetData(); }
    void setBreakpointInScriptOnEmptyLine();
    void setBreakpointInScriptOnOptimizedBinding_data() { targetData(); }
    void setBreakpointInScriptOnOptimizedBinding();
    void setBreakpointInScriptWithCondition_data() { targetData(); }
    void setBreakpointInScriptWithCondition();
    void setBreakpointInScriptThatQuits_data() { targetData(); }
    void setBreakpointInScriptThatQuits();
    void setBreakpointWhenAttaching();

    void clearBreakpoint_data() { targetData(); }
    void clearBreakpoint();

    void changeBreakpoint_data() { targetData(); }
    void changeBreakpoint();

    void setExceptionBreak_data() { targetData(); }
    void setExceptionBreak();

    void stepNext_data() { targetData(); }
    void stepNext();
    void stepIn_data() { targetData(); }
    void stepIn();
    void stepOut_data() { targetData(); }
    void stepOut();
    void continueDebugging_data() { targetData(); }
    void continueDebugging();

    void backtrace_data() { targetData(); }
    void backtrace();

    void getFrameDetails_data() { targetData(); }
    void getFrameDetails();

    void getScopeDetails_data() { targetData(); }
    void getScopeDetails();

    void evaluateInGlobalScope();
    void evaluateInLocalScope_data() { targetData(); }
    void evaluateInLocalScope();

    void evaluateInContext();

    void getScripts_data() { targetData(); }
    void getScripts();

    void encodeQmlScope();
    void breakOnAnchor();

private:
    ConnectResult init(bool qmlscene, const QString &qmlFile = QString(TEST_QMLFILE),
                    bool blockMode = true, bool restrictServices = false);
    QList<QQmlDebugClient *> createClients() override;
    QPointer<QJSDebugClient> m_client;

    void targetData();
    bool waitForClientSignal(const char *signal, int timeout = 30000);
    void checkVersionParameters();

    QTime t;
};

class QJSDebugClient : public QQmlDebugClient
{
    Q_OBJECT
public:
    enum StepAction
    {
        Continue,
        In,
        Out,
        Next
    };

    enum Exception
    {
        All,
        Uncaught
    };

    QJSDebugClient(QQmlDebugConnection *connection)
        : QQmlDebugClient(QLatin1String("V8Debugger"), connection),
          seq(0)
    {
        parser = jsEngine.evaluate(QLatin1String("JSON.parse"));
        stringify = jsEngine.evaluate(QLatin1String("JSON.stringify"));
        QObject::connect(this, &QQmlDebugClient::stateChanged,
                         this, &QJSDebugClient::onStateChanged);
    }

    void connect();
    void interrupt();

    void continueDebugging(StepAction stepAction);
    void evaluate(QString expr, int frame = -1, int context = -1);
    void lookup(QList<int> handles, bool includeSource = false);
    void backtrace(int fromFrame = -1, int toFrame = -1, bool bottom = false);
    void frame(int number = -1);
    void scope(int number = -1, int frameNumber = -1);
    void scripts(int types = 4, QList<int> ids = QList<int>(), bool includeSource = false, QVariant filter = QVariant());
    void setBreakpoint(QString target, int line = -1, int column = -1, bool enabled = true,
                       QString condition = QString(), int ignoreCount = -1);
    void clearBreakpoint(int breakpoint);
    void changeBreakpoint(int breakpoint, bool enabled);
    void setExceptionBreak(Exception type, bool enabled = false);
    void version();
    void disconnect();

protected:
    //inherited from QQmlDebugClient
    void onStateChanged(State state);
    void messageReceived(const QByteArray &data);

signals:
    void connected();
    void interruptRequested();
    void result();
    void failure();
    void stopped();

private:
    void sendMessage(const QByteArray &);
    void flushSendBuffer();
    QByteArray packMessage(const QByteArray &type, const QByteArray &message = QByteArray());

private:
    QJSEngine jsEngine;
    int seq;

    QList<QByteArray> sendBuffer;
public:
    QJSValue parser;
    QJSValue stringify;
    QByteArray response;

};

void QJSDebugClient::connect()
{
    const QJSValue jsonVal = parser.call(QJSValueList() << QLatin1String("{}"));
    sendMessage(packMessage(CONNECT,
                            stringify.call(QJSValueList() << jsonVal).toString().toUtf8()));
}

void QJSDebugClient::interrupt()
{
    sendMessage(packMessage(INTERRUPT));
}

void QJSDebugClient::continueDebugging(StepAction action)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "continue",
    //      "arguments" : { "stepaction" : <"in", "next" or "out">,
    //                      "stepcount"  : <number of steps (default 1)>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(CONTINEDEBUGGING)));

    if (action != Continue) {
        QJSValue args = parser.call(QJSValueList() << obj);
        switch (action) {
        case In: args.setProperty(QLatin1String(STEPACTION),QJSValue(QLatin1String(IN)));
            break;
        case Out: args.setProperty(QLatin1String(STEPACTION),QJSValue(QLatin1String(OUT)));
            break;
        case Next: args.setProperty(QLatin1String(STEPACTION),QJSValue(QLatin1String(NEXT)));
            break;
        default:break;
        }
        if (!args.isUndefined()) {
            jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
        }
    }
    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::evaluate(QString expr, int frame, int context)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "evaluate",
    //      "arguments" : { "expression"    : <expression to evaluate>,
    //                      "frame"         : <number>,
    //                      "context"       : <object ID>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(EVALUATE)));

    QJSValue args = parser.call(QJSValueList() << obj);
    args.setProperty(QLatin1String(EXPRESSION),QJSValue(expr));

    if (frame != -1)
        args.setProperty(QLatin1String(FRAME),QJSValue(frame));

    if (context != -1)
        args.setProperty(QLatin1String(CONTEXT), QJSValue(context));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::lookup(QList<int> handles, bool includeSource)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "lookup",
    //      "arguments" : { "handles"       : <array of handles>,
    //                      "includeSource" : <boolean indicating whether the source will be included when script objects are returned>,
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(LOOKUP)));

    QJSValue args = parser.call(QJSValueList() << obj);

    QString arr("[]");
    QJSValue array = parser.call(QJSValueList() << arr);
    int index = 0;
    foreach (int handle, handles) {
        array.setProperty(index++,QJSValue(handle));
    }
    args.setProperty(QLatin1String(HANDLES),array);

    if (includeSource)
        args.setProperty(QLatin1String(INCLUDESOURCE),QJSValue(includeSource));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::backtrace(int fromFrame, int toFrame, bool bottom)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "backtrace",
    //      "arguments" : { "fromFrame" : <number>
    //                      "toFrame" : <number>
    //                      "bottom" : <boolean, set to true if the bottom of the stack is requested>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(BACKTRACE)));

    QJSValue args = parser.call(QJSValueList() << obj);

    if (fromFrame != -1)
        args.setProperty(QLatin1String(FROMFRAME),QJSValue(fromFrame));

    if (toFrame != -1)
        args.setProperty(QLatin1String(TOFRAME),QJSValue(toFrame));

    if (bottom)
        args.setProperty(QLatin1String(BOTTOM),QJSValue(bottom));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::frame(int number)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "frame",
    //      "arguments" : { "number" : <frame number>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(FRAME)));

    if (number != -1) {
        QJSValue args = parser.call(QJSValueList() << obj);
        args.setProperty(QLatin1String(NUMBER),QJSValue(number));

        if (!args.isUndefined()) {
            jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
        }
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::scope(int number, int frameNumber)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "scope",
    //      "arguments" : { "number" : <scope number>
    //                      "frameNumber" : <frame number, optional uses selected frame if missing>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(SCOPE)));

    if (number != -1) {
        QJSValue args = parser.call(QJSValueList() << obj);
        args.setProperty(QLatin1String(NUMBER),QJSValue(number));

        if (frameNumber != -1)
            args.setProperty(QLatin1String(FRAMENUMBER),QJSValue(frameNumber));

        if (!args.isUndefined()) {
            jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
        }
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::scripts(int types, QList<int> ids, bool includeSource, QVariant /*filter*/)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "scripts",
    //      "arguments" : { "types"         : <types of scripts to retrieve
    //                                           set bit 0 for native scripts
    //                                           set bit 1 for extension scripts
    //                                           set bit 2 for normal scripts
    //                                         (default is 4 for normal scripts)>
    //                      "ids"           : <array of id's of scripts to return. If this is not specified all scripts are requrned>
    //                      "includeSource" : <boolean indicating whether the source code should be included for the scripts returned>
    //                      "filter"        : <string or number: filter string or script id.
    //                                         If a number is specified, then only the script with the same number as its script id will be retrieved.
    //                                         If a string is specified, then only scripts whose names contain the filter string will be retrieved.>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(SCRIPTS)));

    QJSValue args = parser.call(QJSValueList() << obj);
    args.setProperty(QLatin1String(TYPES),QJSValue(types));

    if (ids.count()) {
        QString arr("[]");
        QJSValue array = parser.call(QJSValueList() << arr);
        int index = 0;
        foreach (int id, ids) {
            array.setProperty(index++,QJSValue(id));
        }
        args.setProperty(QLatin1String(IDS),array);
    }

    if (includeSource)
        args.setProperty(QLatin1String(INCLUDESOURCE),QJSValue(includeSource));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::setBreakpoint(QString target, int line, int column, bool enabled,
                                   QString condition, int ignoreCount)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "setbreakpoint",
    //      "arguments" : { "type"        : "scriptRegExp"
    //                      "target"      : <function expression or script identification>
    //                      "line"        : <line in script or function>
    //                      "column"      : <character position within the line>
    //                      "enabled"     : <initial enabled state. True or false, default is true>
    //                      "condition"   : <string with break point condition>
    //                      "ignoreCount" : <number specifying the number of break point hits to ignore, default value is 0>
    //                    }
    //    }

    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(SETBREAKPOINT)));

    QJSValue args = parser.call(QJSValueList() << obj);

    args.setProperty(QLatin1String(TYPE),QJSValue(QLatin1String(SCRIPTREGEXP)));
    args.setProperty(QLatin1String(TARGET),QJSValue(target));

    if (line != -1)
        args.setProperty(QLatin1String(LINE),QJSValue(line));

    if (column != -1)
        args.setProperty(QLatin1String(COLUMN),QJSValue(column));

    args.setProperty(QLatin1String(ENABLED),QJSValue(enabled));

    if (!condition.isEmpty())
        args.setProperty(QLatin1String(CONDITION),QJSValue(condition));

    if (ignoreCount != -1)
        args.setProperty(QLatin1String(IGNORECOUNT),QJSValue(ignoreCount));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::clearBreakpoint(int breakpoint)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "clearbreakpoint",
    //      "arguments" : { "breakpoint" : <number of the break point to clear>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(CLEARBREAKPOINT)));

    QJSValue args = parser.call(QJSValueList() << obj);

    args.setProperty(QLatin1String(BREAKPOINT),QJSValue(breakpoint));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::changeBreakpoint(int breakpoint, bool enabled)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "changebreakpoint",
    //      "arguments" : { "breakpoint" : <number of the break point to change>
    //                      "enabled" : <bool: enables the break type if true, disables if false>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND), QLatin1String(CHANGEBREAKPOINT));

    QJSValue args = parser.call(QJSValueList() << obj);

    args.setProperty(QLatin1String(BREAKPOINT), breakpoint);
    args.setProperty(QLatin1String(ENABLED), enabled);
    jsonVal.setProperty(QLatin1String(ARGUMENTS), args);

    const QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::setExceptionBreak(Exception type, bool enabled)
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "setexceptionbreak",
    //      "arguments" : { "type"    : <string: "all", or "uncaught">,
    //                      "enabled" : <optional bool: enables the break type if true>
    //                    }
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(SETEXCEPTIONBREAK)));

    QJSValue args = parser.call(QJSValueList() << obj);

    if (type == All)
        args.setProperty(QLatin1String(TYPE),QJSValue(QLatin1String(ALL)));
    else if (type == Uncaught)
        args.setProperty(QLatin1String(TYPE),QJSValue(QLatin1String(UNCAUGHT)));

    if (enabled)
        args.setProperty(QLatin1String(ENABLED),QJSValue(enabled));

    if (!args.isUndefined()) {
        jsonVal.setProperty(QLatin1String(ARGUMENTS),args);
    }

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::version()
{
    //    { "seq"       : <number>,
    //      "type"      : "request",
    //      "command"   : "version",
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(VERSION)));

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(V8REQUEST, json.toString().toUtf8()));
}

void QJSDebugClient::disconnect()
{
    //    { "seq"     : <number>,
    //      "type"    : "request",
    //      "command" : "disconnect",
    //    }
    VARIANTMAPINIT;
    jsonVal.setProperty(QLatin1String(COMMAND),QJSValue(QLatin1String(DISCONNECT)));

    QJSValue json = stringify.call(QJSValueList() << jsonVal);
    sendMessage(packMessage(DISCONNECT, json.toString().toUtf8()));
}

void QJSDebugClient::onStateChanged(State state)
{
    if (state == Enabled)
        flushSendBuffer();
}

void QJSDebugClient::messageReceived(const QByteArray &data)
{
    QPacket ds(connection()->currentDataStreamVersion(), data);
    QByteArray command;
    ds >> command;

    if (command == "V8DEBUG") {
        QByteArray type;
        ds >> type >> response;

        if (type == CONNECT) {
            emit connected();

        } else if (type == INTERRUPT) {
            emit interruptRequested();

        } else if (type == V8MESSAGE) {
            QString jsonString(response);
            QVariantMap value = parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();
            QString type = value.value("type").toString();

            if (type == "response") {

                if (!value.value("success").toBool()) {
                    emit failure();
                    qDebug() << "Received success == false response from application:"
                             << value.value("message").toString();
                    return;
                }

                QString debugCommand(value.value("command").toString());
                if (debugCommand == "backtrace" ||
                        debugCommand == "lookup" ||
                        debugCommand == "setbreakpoint" ||
                        debugCommand == "evaluate" ||
                        debugCommand == "version" ||
                        debugCommand == "disconnect" ||
                        debugCommand == "gc" ||
                        debugCommand == "changebreakpoint" ||
                        debugCommand == "clearbreakpoint" ||
                        debugCommand == "frame" ||
                        debugCommand == "scope" ||
                        debugCommand == "scopes" ||
                        debugCommand == "scripts" ||
                        debugCommand == "source" ||
                        debugCommand == "setexceptionbreak" /*||
                        debugCommand == "profile"*/) {
                    emit result();
                } else {
                    // DO NOTHING
                }

            } else if (type == QLatin1String(EVENT)) {
                QString event(value.value(QLatin1String(EVENT)).toString());

                if (event == "break" ||
                        event == "exception")
                    emit stopped();
                }

        }
    }
}

void QJSDebugClient::sendMessage(const QByteArray &msg)
{
    if (state() == Enabled) {
        QQmlDebugClient::sendMessage(msg);
    } else {
        sendBuffer.append(msg);
    }
}

void QJSDebugClient::flushSendBuffer()
{
    foreach (const QByteArray &msg, sendBuffer)
        QQmlDebugClient::sendMessage(msg);
    sendBuffer.clear();
}

QByteArray QJSDebugClient::packMessage(const QByteArray &type, const QByteArray &message)
{
    QPacket rs(connection()->currentDataStreamVersion());
    QByteArray cmd = "V8DEBUG";
    rs << cmd << type << message;
    return rs.data();
}

void tst_QQmlDebugJS::initTestCase()
{
    QQmlDebugTest::initTestCase();
    t.start();
}

QQmlDebugTest::ConnectResult tst_QQmlDebugJS::init(bool qmlscene, const QString &qmlFile,
                                                   bool blockMode, bool restrictServices)
{
    const QString executable = qmlscene
            ? QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmlscene"
            : debugJsServerPath("qqmldebugjs");
    return QQmlDebugTest::connect(
                executable, restrictServices ? QStringLiteral("V8Debugger") : QString(),
                testFile(qmlFile), blockMode);
}

void tst_QQmlDebugJS::connect_data()
{
    QTest::addColumn<bool>("blockMode");
    QTest::addColumn<bool>("restrictMode");
    QTest::addColumn<bool>("qmlscene");
    QTest::newRow("normal / unrestricted / custom")   << false << false << false;
    QTest::newRow("block  / unrestricted / custom")   << true  << false << false;
    QTest::newRow("normal / restricted   / custom")   << false << true  << false;
    QTest::newRow("block  / restricted   / custom")   << true  << true  << false;
    QTest::newRow("normal / unrestricted / qmlscene") << false << false << true;
    QTest::newRow("block  / unrestricted / qmlscene") << true  << false << true;
    QTest::newRow("normal / restricted   / qmlscene") << false << true  << true;
    QTest::newRow("block  / restricted   / qmlscene") << true  << true  << true;
}

void tst_QQmlDebugJS::connect()
{
    QFETCH(bool, blockMode);
    QFETCH(bool, restrictMode);
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene, QString(TEST_QMLFILE), blockMode, restrictMode), ConnectSuccess);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(connected())));
}

void tst_QQmlDebugJS::interrupt()
{
    //void connect()
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene), ConnectSuccess);
    m_client->connect();

    m_client->interrupt();
    QVERIFY(waitForClientSignal(SIGNAL(interruptRequested())));
}

void tst_QQmlDebugJS::getVersion()
{
    //void version()
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene), ConnectSuccess);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(connected())));

    m_client->version();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
    checkVersionParameters();
}

void tst_QQmlDebugJS::getVersionWhenAttaching()
{
    //void version()
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene, QLatin1String(TIMER_QMLFILE), false), ConnectSuccess);
    m_client->connect();

    m_client->version();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
    checkVersionParameters();
}

void tst_QQmlDebugJS::disconnect()
{
    //void disconnect()
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene), ConnectSuccess);
    m_client->connect();

    m_client->disconnect();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
}

void tst_QQmlDebugJS::setBreakpointInScriptOnCompleted()
{
    //void setBreakpoint(QString type, QString target, int line = -1, int column = -1, bool enabled = false, QString condition = QString(), int ignoreCount = -1)
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    QCOMPARE(init(qmlscene, ONCOMPLETED_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(ONCOMPLETED_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(ONCOMPLETED_QMLFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptOnComponentCreated()
{
    //void setBreakpoint(QString type, QString target, int line = -1, int column = -1, bool enabled = false, QString condition = QString(), int ignoreCount = -1)
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    QCOMPARE(init(qmlscene, CREATECOMPONENT_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(ONCOMPLETED_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(ONCOMPLETED_QMLFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptOnTimerCallback()
{
    QFETCH(bool, qmlscene);

    int sourceLine = 35;
    QCOMPARE(init(qmlscene, TIMER_QMLFILE), ConnectSuccess);

    m_client->connect();
    //We can set the breakpoint after connect() here because the timer is repeating and if we miss
    //its first iteration we can still catch the second one.
    m_client->setBreakpoint(QLatin1String(TIMER_QMLFILE), sourceLine, -1, true);
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(TIMER_QMLFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptInDifferentFile()
{
    //void setBreakpoint(QString type, QString target, int line = -1, int column = -1, bool enabled = false, QString condition = QString(), int ignoreCount = -1)
    QFETCH(bool, qmlscene);

    int sourceLine = 31;
    QCOMPARE(init(qmlscene, LOADJSFILE_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(TEST_JSFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(TEST_JSFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptOnComment()
{
    //void setBreakpoint(QString type, QString target, int line = -1, int column = -1, bool enabled = false, QString condition = QString(), int ignoreCount = -1)
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    int actualLine = 36;
    QCOMPARE(init(qmlscene, BREAKPOINTRELOCATION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(BREAKPOINTRELOCATION_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QEXPECT_FAIL("", "Relocation of breakpoints is disabled right now", Abort);
    QVERIFY(waitForClientSignal(SIGNAL(stopped()), 1));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), actualLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(BREAKPOINTRELOCATION_QMLFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptOnEmptyLine()
{
    //void setBreakpoint(QString type, QString target, int line = -1, int column = -1, bool enabled = false, QString condition = QString(), int ignoreCount = -1)
    QFETCH(bool, qmlscene);

    int sourceLine = 35;
    int actualLine = 36;
    QCOMPARE(init(qmlscene, BREAKPOINTRELOCATION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(BREAKPOINTRELOCATION_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QEXPECT_FAIL("", "Relocation of breakpoints is disabled right now", Abort);
    QVERIFY(waitForClientSignal(SIGNAL(stopped()), 1));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), actualLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(BREAKPOINTRELOCATION_QMLFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptOnOptimizedBinding()
{
    //void setBreakpoint(QString type, QString target, int line = -1, int column = -1, bool enabled = false, QString condition = QString(), int ignoreCount = -1)
    QFETCH(bool, qmlscene);

    int sourceLine = 39;
    QCOMPARE(init(qmlscene, BREAKPOINTRELOCATION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(BREAKPOINTRELOCATION_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(BREAKPOINTRELOCATION_QMLFILE));
}

void tst_QQmlDebugJS::setBreakpointInScriptWithCondition()
{
    QFETCH(bool, qmlscene);

    int out = 10;
    int sourceLine = 37;
    QCOMPARE(init(qmlscene, CONDITION_QMLFILE), ConnectSuccess);

    m_client->connect();
    //The breakpoint is in a timer loop so we can set it after connect().
    m_client->setBreakpoint(QLatin1String(CONDITION_QMLFILE), sourceLine, 1, true, QLatin1String("a > 10"));
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    //Get the frame index
    QString jsonString = m_client->response;
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    {
        QVariantMap body = value.value("body").toMap();
        int frameIndex = body.value("index").toInt();

        //Verify the value of 'result'
        m_client->evaluate(QLatin1String("a"),frameIndex);
        QVERIFY(waitForClientSignal(SIGNAL(result())));
    }

    jsonString = m_client->response;
    QJSValue val = m_client->parser.call(QJSValueList() << QJSValue(jsonString));
    QVERIFY(val.isObject());
    QJSValue body = val.property(QStringLiteral("body"));
    QVERIFY(body.isObject());
    val = body.property("value");
    QVERIFY(val.isNumber());

    const int a = val.toInt();
    QVERIFY(a > out);
}

void tst_QQmlDebugJS::setBreakpointInScriptThatQuits()
{
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene, QUIT_QMLFILE), ConnectSuccess);

    int sourceLine = 36;

    m_client->setBreakpoint(QLatin1String(QUIT_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(QUIT_QMLFILE));

    m_client->continueDebugging(QJSDebugClient::Continue);
    QVERIFY(m_process->waitForFinished());
    QCOMPARE(m_process->exitStatus(), QProcess::NormalExit);
}

void tst_QQmlDebugJS::setBreakpointWhenAttaching()
{
    int sourceLine = 35;
    QCOMPARE(init(true, QLatin1String(TIMER_QMLFILE), false), ConnectSuccess);

    m_client->connect();

    QSKIP("\nThe breakpoint may not hit because the engine may run in JIT mode or not have debug\n"
          "instructions, as we've connected in non-blocking mode above. That means we may have\n"
          "connected after the engine was already running, with all the QML already compiled.");

    //The breakpoint is in a timer loop so we can set it after connect().
    m_client->setBreakpoint(QLatin1String(TIMER_QMLFILE), sourceLine);

    QVERIFY(waitForClientSignal(SIGNAL(stopped())));
}

void tst_QQmlDebugJS::clearBreakpoint()
{
    //void clearBreakpoint(int breakpoint);
    QFETCH(bool, qmlscene);

    int sourceLine1 = 37;
    int sourceLine2 = 38;
    QCOMPARE(init(qmlscene, CHANGEBREAKPOINT_QMLFILE), ConnectSuccess);

    m_client->connect();
    //The breakpoints are in a timer loop so we can set them after connect().
    //Furthermore the breakpoints should be hit in the right order because setting of breakpoints
    //can only occur in the QML event loop. (see QCOMPARE for sourceLine2 below)
    m_client->setBreakpoint(QLatin1String(CHANGEBREAKPOINT_QMLFILE), sourceLine1, -1, true);
    m_client->setBreakpoint(QLatin1String(CHANGEBREAKPOINT_QMLFILE), sourceLine2, -1, true);

    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    //Will hit 1st brakpoint, change this breakpoint enable = false
    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();
    QList<QVariant> breakpointsHit = body.value("breakpoints").toList();

    int breakpoint = breakpointsHit.at(0).toInt();
    m_client->clearBreakpoint(breakpoint);

    QVERIFY(waitForClientSignal(SIGNAL(result())));

    //Continue with debugging
    m_client->continueDebugging(QJSDebugClient::Continue);
    //Hit 2nd breakpoint
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    //Continue with debugging
    m_client->continueDebugging(QJSDebugClient::Continue);
    //Should stop at 2nd breakpoint
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    jsonString = m_client->response;
    value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine2);
}

void tst_QQmlDebugJS::changeBreakpoint()
{
    //void clearBreakpoint(int breakpoint);
    QFETCH(bool, qmlscene);

    int sourceLine2 = 37;
    int sourceLine1 = 38;
    QCOMPARE(init(qmlscene, CHANGEBREAKPOINT_QMLFILE), ConnectSuccess);

    bool isStopped = false;
    QObject::connect(m_client.data(), &QJSDebugClient::stopped, this, [&]() { isStopped = true; });

    auto continueDebugging = [&]() {
        m_client->continueDebugging(QJSDebugClient::Continue);
        isStopped = false;
    };

    m_client->connect();

    auto extractBody = [&]() {
        const QVariantMap value = m_client->parser.call(
                    QJSValueList() << QJSValue(QString(m_client->response))).toVariant().toMap();
        return value.value("body").toMap();
    };

    auto extractBreakPointId = [&](const QVariantMap &body) {
        const QList<QVariant> breakpointsHit = body.value("breakpoints").toList();
        if (breakpointsHit.size() != 1)
            return -1;
        return breakpointsHit[0].toInt();
    };

    auto setBreakPoint = [&](int sourceLine, bool enabled) {
        int id = -1;
        auto connection = QObject::connect(m_client.data(), &QJSDebugClient::result, [&]() {
            id = extractBody().value("breakpoint").toInt();
        });

        m_client->setBreakpoint(QLatin1String(CHANGEBREAKPOINT_QMLFILE), sourceLine, -1, enabled);
        bool success = QTest::qWaitFor([&]() { return id >= 0; });
        Q_UNUSED(success);

        QObject::disconnect(connection);
        return id;
    };

    //The breakpoints are in a timer loop so we can set them after connect().
    //Furthermore the breakpoints should be hit in the right order because setting of breakpoints
    //can only occur in the QML event loop. (see QCOMPARE for sourceLine2 below)
    const int breakpoint1 = setBreakPoint(sourceLine1, false);
    QVERIFY(breakpoint1 >= 0);

    const int breakpoint2 = setBreakPoint(sourceLine2, true);
    QVERIFY(breakpoint2 >= 0);

    auto verifyBreakpoint = [&](int sourceLine, int breakpointId) {
        QTRY_VERIFY_WITH_TIMEOUT(isStopped, 30000);
        const QVariantMap body = extractBody();
        QCOMPARE(body.value("sourceLine").toInt(), sourceLine);
        QCOMPARE(extractBreakPointId(body), breakpointId);
    };

    verifyBreakpoint(sourceLine2, breakpoint2);

    continueDebugging();
    verifyBreakpoint(sourceLine2, breakpoint2);

    m_client->changeBreakpoint(breakpoint2, false);
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    m_client->changeBreakpoint(breakpoint1, true);
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    continueDebugging();
    verifyBreakpoint(sourceLine1, breakpoint1);

    continueDebugging();
    verifyBreakpoint(sourceLine1, breakpoint1);

    m_client->changeBreakpoint(breakpoint2, true);
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    m_client->changeBreakpoint(breakpoint1, false);
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    for (int i = 0; i < 3; ++i) {
        continueDebugging();
        verifyBreakpoint(sourceLine2, breakpoint2);
    }
}

void tst_QQmlDebugJS::setExceptionBreak()
{
    //void setExceptionBreak(QString type, bool enabled = false);
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene, EXCEPTION_QMLFILE), ConnectSuccess);
    m_client->setExceptionBreak(QJSDebugClient::All,true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));
}

void tst_QQmlDebugJS::stepNext()
{
    //void continueDebugging(StepAction stepAction, int stepCount = 1);
    QFETCH(bool, qmlscene);

    int sourceLine = 37;
    QCOMPARE(init(qmlscene, STEPACTION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(STEPACTION_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->continueDebugging(QJSDebugClient::Next);
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine + 1);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(STEPACTION_QMLFILE));
}

static QVariantMap responseBody(QJSDebugClient *client)
{
    const QString jsonString(client->response);
    const QVariantMap value = client->parser.call(QJSValueList() << QJSValue(jsonString))
            .toVariant().toMap();
    return value.value("body").toMap();
}

void tst_QQmlDebugJS::stepIn()
{
    //void continueDebugging(StepAction stepAction, int stepCount = 1);
    QFETCH(bool, qmlscene);

    int sourceLine = 41;
    int actualLine = 36;
    QCOMPARE(init(qmlscene, STEPACTION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(STEPACTION_QMLFILE), sourceLine, 1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));
    QCOMPARE(responseBody(m_client).value("sourceLine").toInt(), sourceLine);

    m_client->continueDebugging(QJSDebugClient::In);
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    const QVariantMap body = responseBody(m_client);
    QCOMPARE(body.value("sourceLine").toInt(), actualLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(STEPACTION_QMLFILE));
}

void tst_QQmlDebugJS::stepOut()
{
    //void continueDebugging(StepAction stepAction, int stepCount = 1);
    QFETCH(bool, qmlscene);

    int sourceLine = 37;
    int actualLine = 41;
    QCOMPARE(init(qmlscene, STEPACTION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(STEPACTION_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));
    QCOMPARE(responseBody(m_client).value("sourceLine").toInt(), sourceLine);

    m_client->continueDebugging(QJSDebugClient::Out);
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    const QVariantMap body = responseBody(m_client);
    QCOMPARE(body.value("sourceLine").toInt(), actualLine);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(STEPACTION_QMLFILE));
}

void tst_QQmlDebugJS::continueDebugging()
{
    //void continueDebugging(StepAction stepAction, int stepCount = 1);
    QFETCH(bool, qmlscene);

    int sourceLine1 = 41;
    int sourceLine2 = 38;
    QCOMPARE(init(qmlscene, STEPACTION_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(STEPACTION_QMLFILE), sourceLine1, -1, true);
    m_client->setBreakpoint(QLatin1String(STEPACTION_QMLFILE), sourceLine2, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->continueDebugging(QJSDebugClient::Continue);
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    QCOMPARE(body.value("sourceLine").toInt(), sourceLine2);
    QCOMPARE(QFileInfo(body.value("script").toMap().value("name").toString()).fileName(), QLatin1String(STEPACTION_QMLFILE));
}

void tst_QQmlDebugJS::backtrace()
{
    //void backtrace(int fromFrame = -1, int toFrame = -1, bool bottom = false);
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    QCOMPARE(init(qmlscene, ONCOMPLETED_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(ONCOMPLETED_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->backtrace();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
}

void tst_QQmlDebugJS::getFrameDetails()
{
    //void frame(int number = -1);
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    QCOMPARE(init(qmlscene, ONCOMPLETED_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(ONCOMPLETED_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->frame();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
}

void tst_QQmlDebugJS::getScopeDetails()
{
    //void scope(int number = -1, int frameNumber = -1);
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    QCOMPARE(init(qmlscene, ONCOMPLETED_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(ONCOMPLETED_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->scope();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
}

void tst_QQmlDebugJS::evaluateInGlobalScope()
{
    //void evaluate(QString expr, int frame = -1);
    QCOMPARE(init(true), ConnectSuccess);

    m_client->connect();

    for (int i = 0; i < 10; ++i) {
        // The engine might not be initialized, yet. We just try until it shows up.
        m_client->evaluate(QLatin1String("console.log('Hello World')"));
        if (waitForClientSignal(SIGNAL(result()), 500))
            break;
    }

    //Verify the return value of 'console.log()', which is "undefined"
    QCOMPARE(responseBody(m_client).value("type").toString(), QLatin1String("undefined"));
}

void tst_QQmlDebugJS::evaluateInLocalScope()
{
    //void evaluate(QString expr, bool global = false, bool disableBreak = false, int frame = -1, const QVariantMap &addContext = QVariantMap());
    QFETCH(bool, qmlscene);

    int sourceLine = 34;
    QCOMPARE(init(qmlscene, ONCOMPLETED_QMLFILE), ConnectSuccess);

    m_client->setBreakpoint(QLatin1String(ONCOMPLETED_QMLFILE), sourceLine, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->frame();
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    //Get the frame index
    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    QVariantMap body = value.value("body").toMap();

    int frameIndex = body.value("index").toInt();

    m_client->evaluate(QLatin1String("root.a"), frameIndex);
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    //Verify the value of 'timer.interval'
    jsonString = m_client->response;
    value = m_client->parser.call(QJSValueList() << QJSValue(jsonString)).toVariant().toMap();

    body = value.value("body").toMap();

    QCOMPARE(body.value("value").toInt(),10);
}

void tst_QQmlDebugJS::evaluateInContext()
{
    m_connection = new QQmlDebugConnection();
    m_process = new QQmlDebugProcess(QLibraryInfo::location(QLibraryInfo::BinariesPath)
                                   + "/qmlscene", this);
    m_client = new QJSDebugClient(m_connection);
    QScopedPointer<QQmlEngineDebugClient> engineClient(new QQmlEngineDebugClient(m_connection));
    m_process->start(QStringList() << QLatin1String(BLOCKMODE) << testFile(ONCOMPLETED_QMLFILE));

    QVERIFY(m_process->waitForSessionStart());

    m_connection->connectToHost("127.0.0.1", m_process->debugPort());
    QVERIFY(m_connection->waitForConnected());

    QTRY_COMPARE(m_client->state(), QQmlEngineDebugClient::Enabled);
    QTRY_COMPARE(engineClient->state(), QQmlEngineDebugClient::Enabled);
    m_client->connect();

    // "a" not accessible without extra context
    m_client->evaluate(QLatin1String("a + 10"), -1, -1);
    QVERIFY(waitForClientSignal(SIGNAL(failure())));

    bool success = false;
    engineClient->queryAvailableEngines(&success);
    QVERIFY(success);
    QVERIFY(QQmlDebugTest::waitForSignal(engineClient.data(), SIGNAL(result())));

    QVERIFY(engineClient->engines().count());
    engineClient->queryRootContexts(engineClient->engines()[0].debugId, &success);
    QVERIFY(success);
    QVERIFY(QQmlDebugTest::waitForSignal(engineClient.data(), SIGNAL(result())));

    auto contexts = engineClient->rootContext().contexts;
    QCOMPARE(contexts.count(), 1);
    auto objects = contexts[0].objects;
    QCOMPARE(objects.count(), 1);
    engineClient->queryObjectRecursive(objects[0], &success);
    QVERIFY(success);
    QVERIFY(QQmlDebugTest::waitForSignal(engineClient.data(), SIGNAL(result())));
    auto object = engineClient->object();

    // "a" accessible in context of surrounding object
    m_client->evaluate(QLatin1String("a + 10"), -1, object.debugId);
    QVERIFY(waitForClientSignal(SIGNAL(result())));

    QTRY_COMPARE(responseBody(m_client).value("value").toInt(), 20);
}

void tst_QQmlDebugJS::getScripts()
{
    //void scripts(int types = -1, QList<int> ids = QList<int>(), bool includeSource = false, QVariant filter = QVariant());
    QFETCH(bool, qmlscene);

    QCOMPARE(init(qmlscene), ConnectSuccess);

    m_client->setBreakpoint(QString(TEST_QMLFILE), 35, -1, true);
    m_client->connect();
    QVERIFY(waitForClientSignal(SIGNAL(stopped())));

    m_client->scripts();
    QVERIFY(waitForClientSignal(SIGNAL(result())));
    QString jsonString(m_client->response);
    QVariantMap value = m_client->parser.call(QJSValueList()
                                            << QJSValue(jsonString)).toVariant().toMap();

    QList<QVariant> scripts = value.value("body").toList();

    QCOMPARE(scripts.count(), 1);
    QVERIFY(scripts.first().toMap()[QStringLiteral("name")].toString().endsWith(QStringLiteral("data/test.qml")));
}

void tst_QQmlDebugJS::encodeQmlScope()
{
    QString file(ENCODEQMLSCOPE_QMLFILE);
    QCOMPARE(init(true, file), ConnectSuccess);

    int numFrames = 0;
    int numExpectedScopes = 0;
    int numReceivedScopes = 0;
    bool isStopped = false;
    bool scopesFailed = false;

    QObject::connect(m_client.data(), &QJSDebugClient::failure, this, [&]() {
        qWarning() << "received failure" << m_client->response;
        scopesFailed = true;
        m_process->stop();
        numFrames = 2;
        isStopped = false;
    });

    QObject::connect(m_client.data(), &QJSDebugClient::stopped, this, [&]() {
        m_client->frame();
        isStopped = true;
    });

    QObject::connect(m_client.data(), &QJSDebugClient::result, this, [&]() {
        const QVariantMap value = m_client->parser.call(
                    QJSValueList() << QJSValue(QString(m_client->response))).toVariant().toMap();

        const QMap<QString, QVariant> body = value.value("body").toMap();
        const QString command = value.value("command").toString();

        if (command == QString("scope")) {
            // If the scope commands fail we get a failure() signal above.
            if (++numReceivedScopes == numExpectedScopes) {
                m_client->continueDebugging(QJSDebugClient::Continue);
                isStopped = false;
            }
        } else if (command == QString("frame")) {

            // We want at least a global scope and some kind of local scope here.
            const QList<QVariant> scopes = body.value("scopes").toList();
            if (scopes.length() < 2)
                scopesFailed = true;

            for (const QVariant &scope : scopes) {
                ++numExpectedScopes;
                m_client->scope(scope.toMap().value("index").toInt());
            }

            ++numFrames;
        }
    });

    m_client->setBreakpoint(file, 6);
    m_client->setBreakpoint(file, 8);
    m_client->connect();

    QTRY_COMPARE(numFrames, 2);
    QVERIFY(numExpectedScopes > 3);
    QVERIFY(!scopesFailed);
    QTRY_VERIFY(!isStopped);
    QCOMPARE(numReceivedScopes, numExpectedScopes);
}

void tst_QQmlDebugJS::breakOnAnchor()
{
    QString file(BREAKONANCHOR_QMLFILE);
    QCOMPARE(init(true, file), ConnectSuccess);

    int breaks = 0;
    bool stopped = false;
    QObject::connect(m_client.data(), &QJSDebugClient::stopped, this, [&]() {
        stopped = true;
        ++breaks;
        m_client->evaluate("this", 0, -1);
    });

    QObject::connect(m_client.data(), &QJSDebugClient::result, this, [&]() {
        if (stopped) {
            m_client->continueDebugging(QJSDebugClient::Continue);
            stopped = false;
        }
    });

    QObject::connect(m_client.data(), &QJSDebugClient::failure, this, [&]() {
        qWarning() << "received failure" << m_client->response;
    });

    m_client->setBreakpoint(file, 34);
    m_client->setBreakpoint(file, 37);

    QTRY_COMPARE(m_process->state(), QProcess::Running);

    m_client->connect();

    QTRY_COMPARE(m_process->state(), QProcess::NotRunning);
    QCOMPARE(m_process->exitStatus(), QProcess::NormalExit);

    QCOMPARE(breaks, 2);
}

QList<QQmlDebugClient *> tst_QQmlDebugJS::createClients()
{
    m_client = new QJSDebugClient(m_connection);
    return QList<QQmlDebugClient *>({m_client});
}

void tst_QQmlDebugJS::targetData()
{
    QTest::addColumn<bool>("qmlscene");
    QTest::newRow("custom")   << false;
    QTest::newRow("qmlscene") << true;
}

bool tst_QQmlDebugJS::waitForClientSignal(const char *signal, int timeout)
{
    return QQmlDebugTest::waitForSignal(m_client.data(), signal, timeout);
}

void tst_QQmlDebugJS::checkVersionParameters()
{
    const QVariantMap value = m_client->parser.call(
                QJSValueList() << QJSValue(QString(m_client->response))).toVariant().toMap();
    QCOMPARE(value.value("command").toString(), QString("version"));
    const QVariantMap body = value.value("body").toMap();
    QCOMPARE(body.value("UnpausedEvaluate").toBool(), true);
    QCOMPARE(body.value("ContextEvaluate").toBool(), true);
    QCOMPARE(body.value("ChangeBreakpoint").toBool(), true);
}

QTEST_MAIN(tst_QQmlDebugJS)

#include "tst_qqmldebugjs.moc"

