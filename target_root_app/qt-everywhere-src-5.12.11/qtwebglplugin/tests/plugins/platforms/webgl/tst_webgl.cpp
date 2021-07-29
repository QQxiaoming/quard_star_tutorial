/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt WebGL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qregularexpression.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qopengl.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtWebSockets/qwebsocket.h>

#include "parameters.h"

#include <memory>

#define PORT 29836
#define PORTSTRING QT_STRINGIFY(PORT)

class tst_WebGL : public QObject
{
    Q_OBJECT

    struct GLObject {};

    struct Buffer : GLObject {};
    struct Shader : GLObject
    {
        Shader(GLuint type = GL_INVALID_VALUE) : type(type) {}
        GLuint type;
        QString source;
        bool compiled = false;
    };

    struct Program : GLObject
    {
        QMap<GLuint, Shader *> attached;
        bool linked = false;
    };

    struct Texture : GLObject {};

    struct Context
    {
        Context(int winId = -1) : winId(winId) {}

        int winId;
        Buffer *arrayBuffer = nullptr;
        Buffer *elementArrayBuffer = nullptr;
        Program *program = nullptr;
    };

    QList<Context> contexts;
    QList<Buffer> buffers;
    QList<Program> programs;
    QList<Shader> shaders;
    QList<Texture> textures;
    Context *currentContext = nullptr;

    QNetworkAccessManager manager;
    QWebSocket webSocket;
    QStringList functions;
    QProcess process;
    qintptr websocketPort;

    void connectToQmlScene();
    void sendMouseEvent(Qt::MouseButtons buttons, quint32 x, quint32 y, int winId);
    void sendMouseClick(quint32 x, quint32 y, int winId);

    template <typename Struct>
    Struct *pointer(const QVariant &id, QList<Struct> &container)
    {
        const auto handle = id.toInt();
        if (handle > 0 && handle <= container.size())
            return &container[handle - 1];
        return nullptr;
    }

    bool findSwapBuffers(const QSignalSpy &spy);

signals:
    void command(const QString &name, const QVariantList &parameters);
    void queryCommand(const QString &name, int id, const QVariantList &parameters);

public slots:
    void parseTextMessage(const QString &text);
    void parseBinaryMessage(const QByteArray &data);

private slots:
    void initTestCase();

    void init();
    void cleanup();

    void checkFunctionCount_data();
    void checkFunctionCount();

    void waitForSwapBuffers_data();
    void waitForSwapBuffers();

    void reload_data();
    void reload();

    void update_data();
    void update();
};

void tst_WebGL::connectToQmlScene()
{
    const QJsonDocument connectMessage {
        QJsonObject {
            { QLatin1String("type"), QLatin1String("connect") },
            { QLatin1String("width"), 1920 },
            { QLatin1String("height"), 1080 },
            { QLatin1String("physicalWidth"), 531.3 },
            { QLatin1String("physicalHeight"), 298.9 }
        }
    };

    QSignalSpy connected(&webSocket, &QWebSocket::connected);
    webSocket.open(QUrl(QString::fromLatin1("ws://localhost:%1").arg(websocketPort)));
    QVERIFY(connected.wait());
    webSocket.sendTextMessage(connectMessage.toJson());
    QVERIFY(webSocket.state() == QAbstractSocket::ConnectedState);
}

void tst_WebGL::sendMouseEvent(Qt::MouseButtons buttons, quint32 x, quint32 y, const int winId)
{
    const QJsonDocument message {
        QJsonObject {
            { QLatin1String("type"), QLatin1String("mouse") },
            { QLatin1String("buttons"), int(buttons) },
            { QLatin1String("layerX"), int(x) },
            { QLatin1String("layerY"), int(y) },
            { QLatin1String("clientX"), int(x) },
            { QLatin1String("clientY"), int(y) },
            { QLatin1String("time"), QDateTime::currentDateTime().toMSecsSinceEpoch() },
            { QLatin1String("name"), winId }
        }
    };
    webSocket.sendTextMessage(message.toJson());
}

void tst_WebGL::sendMouseClick(quint32 x, quint32 y, int winId)
{
    sendMouseEvent(Qt::LeftButton, x, y, winId);
    sendMouseEvent(Qt::NoButton, x, y, winId);
}

bool tst_WebGL::findSwapBuffers(const QSignalSpy &spy)
{
    return std::find_if(spy.cbegin(), spy.cend(), [](const QList<QVariant> &list) {
        // Our connect message changed the scene's size, forcing a swapBuffers() call.
        return list.first() == QLatin1String("swapBuffers");
    }) != spy.cend();
}

void tst_WebGL::parseTextMessage(const QString &text)
{
    const auto document = QJsonDocument::fromJson(text.toUtf8());
    if (document["type"].toString() == "connect") {
        const auto supportedFunctions = document["supportedFunctions"].toArray();
        functions.clear();
        for (const auto &function : supportedFunctions)
            functions.append(function.toString());
    } else if (document["type"] == "create_canvas") {
        const QJsonDocument defaultValuesMessage {
            QJsonObject {
                { QLatin1String("type"), QLatin1String("default_context_parameters") },
                { QString::number(GL_EXTENSIONS),
                            QLatin1String("GL_OES_element_index_uint "
                                          "GL_OES_standard_derivatives "
                                          "GL_OES_depth_texture GL_OES_packed_depth_stencil") },
                { QString::number(GL_BLEND), false },
                { QString::number(GL_DEPTH_TEST), false },
                { QString::number(GL_MAX_TEXTURE_SIZE), 512 },
                { QString::number(GL_MAX_VERTEX_ATTRIBS), 16},
                { QString::number(GL_RENDERER), "Test WebGL"},
                { QString::number(GL_SCISSOR_TEST), false },
                { QString::number(GL_STENCIL_TEST), false },
                { QString::number(GL_UNPACK_ALIGNMENT), 4 },
                { QString::number(GL_VENDOR), "Qt" },
                { QString::number(GL_VERSION), "WebGL 1.0" },
                { QString::number(GL_VIEWPORT), QJsonArray{ 0, 0, 640, 480 } },
                { QLatin1String("name"), document["winId"] }
            },
        };
        webSocket.sendTextMessage(defaultValuesMessage.toJson());
        contexts.append(Context(document["winId"].toInt()));
    }
}

// This function gets called inside a QTRY_* that is subject to
// a QEXPECT_FAIL, which treats QCOMPARE and QVERIFY
// specially, so we have to avoid using those.
void tst_WebGL::parseBinaryMessage(const QByteArray &data)
{
    const QSet<QString> commandsNeedingResponse {
        QLatin1String("swapBuffers"),
        QLatin1String("checkFramebufferStatus"),
        QLatin1String("createProgram"),
        QLatin1String("createShader"),
        QLatin1String("genBuffers"),
        QLatin1String("genFramebuffers"),
        QLatin1String("genRenderbuffers"),
        QLatin1String("genTextures"),
        QLatin1String("getAttachedShaders"),
        QLatin1String("getAttribLocation"),
        QLatin1String("getBooleanv"),
        QLatin1String("getError"),
        QLatin1String("getFramebufferAttachmentParameteriv"),
        QLatin1String("getIntegerv"),
        QLatin1String("getParameter"),
        QLatin1String("getProgramInfoLog"),
        QLatin1String("getProgramiv"),
        QLatin1String("getRenderbufferParameteriv"),
        QLatin1String("getShaderiv"),
        QLatin1String("getShaderPrecisionFormat"),
        QLatin1String("getString"),
        QLatin1String("getTexParameterfv"),
        QLatin1String("getTexParameteriv"),
        QLatin1String("getUniformfv"),
        QLatin1String("getUniformLocation"),
        QLatin1String("getUniformiv"),
        QLatin1String("getVertexAttribfv"),
        QLatin1String("getVertexAttribiv"),
        QLatin1String("getShaderSource"),
        QLatin1String("getShaderInfoLog"),
        QLatin1String("isRenderbuffer")
    };

    quint32 offset = 0;
    QString function;
    int id = -1;
    QDataStream stream(data);
    {
        quint8 functionIndex;
        stream >> functionIndex;
        offset += sizeof(functionIndex);
        function = functions[functionIndex];
        if (commandsNeedingResponse.contains(function)) {
            stream >> id;
            offset += sizeof(id);
        }
    }
    const auto parameters = Parameters::read(data, stream, offset);
    {
        quint32 magic = 0;
        stream >> magic;
        offset += sizeof(magic);
        if (magic != 0xbaadf00d) {
            QFAIL(qPrintable(QStringLiteral("Magic token is 0x%1 not 0x%2")
                             .arg(magic, 0, 16).arg(0xbaadf00d, 0, 16)));
        }
    }

    if (int(offset) != data.size())
        QFAIL(qPrintable(QStringLiteral("Offset is %1 not %2").arg(offset).arg(data.size())));

    if (id == -1) {
        emit command(function, parameters);

        if (function == "attachShader") {
            const auto shader = pointer(parameters[1], shaders);
            if (!shader) QFAIL("Null pointer");
            auto  program = pointer(parameters[0], programs);
            if (!program) QFAIL("Null pointer");
            program->attached[shader->type] = shader;
        } else if (function == "bindBuffer") {
            auto buffer = pointer(parameters[1], buffers);
            if (parameters[0].toUInt() == GL_ARRAY_BUFFER)
                currentContext->arrayBuffer = buffer;
            else if (parameters[0].toUInt() == GL_ELEMENT_ARRAY_BUFFER)
                currentContext->elementArrayBuffer = buffer;
            else
                QTest::qFail("Unsupported buffer type", __FILE__, __LINE__);
        } else if (function == "compileShader") {
            auto shader = pointer(parameters[0], shaders);
            if (!shader) QFAIL("Null pointer");
            shader->compiled = true;
        } else if (function == "linkProgram") {
            auto program = pointer(parameters[0], programs);
            if (!program) QFAIL("Null pointer");
            program->linked = true;
        } else if (function == "shaderSource") {
            auto shader = pointer(parameters[0], shaders);
            if (!shader) QFAIL("Null pointer");
            shader->source = parameters[1].toString();
        } else if (function == "makeCurrent") {
            currentContext = pointer(parameters[3].toInt(), contexts);
        } else if (function == "useProgram") {
            currentContext->program = pointer(parameters[0], programs);
        }
    } else {
        emit queryCommand(function, id, parameters);

        QJsonValue retval;
        static QMap<QString, int> nextIds;

        if (function == "createProgram") {
            programs.append(Program{});
            retval = programs.size();
        } else if (function == "createShader") {
            shaders.append(Shader(parameters[0].toUInt()));
            retval = shaders.size();
        } else if (function == "genBuffers") {
            QJsonArray array;
            for (int i = 0, count = parameters.first().toInt(); i < count; ++i) {
                buffers.append(Buffer{});
                array.append(buffers.size());
            }
            retval = array;
        } else if (function == "genTextures") {
            QJsonArray array;
            for (int i = 0, count = parameters.first().toInt(); i < count; ++i) {
                textures.append(Texture{});
                array.append(textures.size());
            }
            retval = array;
        } else if (function == "getError") {
            retval = "";
        } else if (function == "getProgramiv") {
            const auto program = pointer(parameters[0], programs);
            retval = program ? program->linked : false;
        } else if (function == "getShaderiv") {
            const auto shader = pointer(parameters[0], shaders);
            retval = shader ? shader->compiled : false;
        } else if (function == "getUniformLocation") {
            const auto shaders = currentContext->program->attached[GL_VERTEX_SHADER]->source
                    + currentContext->program->attached[GL_FRAGMENT_SHADER] ->source;
            const QRegularExpression rx(R"rx(uniform +(?:(?:high|medium|low)p +)?\w+ +(\w+) *;)rx");
            auto m = rx.globalMatch(shaders);
            for (int i = 0; m.hasNext() && retval.isNull(); ++i) {
                if (m.next().captured(1) == parameters[1].toString())
                    retval = i;
            }
        } else if (function == "linkProgram") {
            auto program = pointer(parameters[0], programs);
            if (program)
                program->linked = true;
        } else if (function == "swapBuffers") {
            // do nothing
        } else {
            QFAIL("Function not handled");
        }
        const QJsonDocument answer {
            QJsonObject {
                { QLatin1String("type"), QLatin1String("gl_response") },
                { QLatin1String("id"), id },
                { QLatin1String("value"), retval }
            }
        };
        webSocket.sendTextMessage(answer.toJson());
    }
}

void tst_WebGL::initTestCase()
{
    connect(&webSocket, &QWebSocket::binaryMessageReceived, this, &tst_WebGL::parseBinaryMessage);
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &tst_WebGL::parseTextMessage);
}

void tst_WebGL::init()
{
    QFETCH(QString, scene);

    contexts.clear();
    buffers.clear();
    programs.clear();
    shaders.clear();
    textures.clear();
    currentContext = nullptr;

    const auto tryToConnect = [=](quint16 port = PORT) {
        QTcpSocket socket;
        socket.connectToHost("localhost", port);
        QTRY_LOOP_IMPL(socket.state() == QTcpSocket::ConnectedState ||
                       socket.state() == QTcpSocket::UnconnectedState, 1000, 50);
        return socket.state() == QTcpSocket::ConnectedState;
    };

    QVERIFY2(!tryToConnect(), "An application is listening on port " PORTSTRING);

    QString executableName = QLatin1String("qmlscene");
#if defined(Q_OS_WIN)
    executableName += QString::fromLatin1(".exe");
#endif

    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setProgram(QLibraryInfo::location(QLibraryInfo::BinariesPath) + QChar('/')
                       + executableName);
    process.setArguments(QStringList { QDir::toNativeSeparators(scene) });
    process.setEnvironment(QProcess::systemEnvironment()
                           << "QT_QPA_PLATFORM=webgl:port=" PORTSTRING);
    process.start();
    process.waitForStarted();
    QVERIFY(process.isOpen());
#if defined(QT_DEBUG)
    connect(&process, &QProcess::readyReadStandardOutput, [=]() {
        while (process.bytesAvailable())
            qDebug() << process.pid() << process.readLine();
    });
#endif // defined(QT_DEBUG)
    QTRY_VERIFY(tryToConnect());

    auto reply = manager.get(QNetworkRequest(QUrl("http://localhost:" PORTSTRING "/webqt.js")));
    QSignalSpy replyFinishedSpy(reply, &QNetworkReply::finished);
    QTRY_VERIFY(!replyFinishedSpy.isEmpty());
    reply->readLine();
    const auto portString = reply->readLine().trimmed();
    QVERIFY(portString.size());
    const QRegularExpression rx("var port = (\\d+);");
    const auto match = rx.match(portString);
    QVERIFY(!match.captured(1).isEmpty());
    QVERIFY(match.captured(1).toUInt() <= std::numeric_limits<quint16>::max());
    websocketPort = match.captured(1).toInt();

    QVERIFY(websocketPort != 0);

    connectToQmlScene();
}

void tst_WebGL::cleanup()
{
    webSocket.close();

    process.kill();
    process.waitForFinished();
}

void tst_WebGL::checkFunctionCount_data()
{
    QTest::addColumn<QString>("scene"); // Fetched in tst_WebGL::init
    QTest::newRow("Basic scene") << QFINDTESTDATA("basic_scene.qml");
}

void tst_WebGL::checkFunctionCount()
{
    QCOMPARE(functions.size(), 147);
}

void tst_WebGL::waitForSwapBuffers_data()
{
    QTest::addColumn<QString>("scene"); // Fetched in tst_WebGL::init
    QTest::newRow("Basic scene") << QFINDTESTDATA("basic_scene.qml");
}

void tst_WebGL::waitForSwapBuffers()
{
    QSignalSpy spy(this, &tst_WebGL::queryCommand);
    QTRY_VERIFY(findSwapBuffers(spy));
}

void tst_WebGL::reload_data()
{
    QTest::addColumn<QString>("scene"); // Fetched in tst_WebGL::init
    QTest::newRow("Basic scene") << QFINDTESTDATA("basic_scene.qml");
    QTest::newRow("Colors") << QFINDTESTDATA("colors.qml");;
}

void tst_WebGL::reload()
{
    for (int i = 0; i < 2; ++i) {
        QSignalSpy spy(this, &tst_WebGL::queryCommand);
        QTRY_VERIFY(findSwapBuffers(spy));
        QVERIFY(!QTest::currentTestFailed());
        webSocket.close();
        connectToQmlScene();
        QVERIFY(!QTest::currentTestFailed());
    }
}

void tst_WebGL::update_data()
{
    QTest::addColumn<QString>("scene"); // Fetched in tst_WebGL::init
    QTest::newRow("Colors") << QFINDTESTDATA("colors.qml");
    QTest::newRow("Launcher") << QFINDTESTDATA("launcher.qml");
}

void tst_WebGL::update()
{
    {
        QSignalSpy spy(this, &tst_WebGL::queryCommand);
        QTRY_VERIFY(findSwapBuffers(spy));
        QVERIFY(!QTest::currentTestFailed());
    }
    sendMouseClick(0, 0, currentContext->winId);
    {
        QSignalSpy spy(this, &tst_WebGL::queryCommand);
        QTRY_VERIFY(findSwapBuffers(spy));
    }
}

QTEST_MAIN(tst_WebGL)

#include "tst_webgl.moc"
