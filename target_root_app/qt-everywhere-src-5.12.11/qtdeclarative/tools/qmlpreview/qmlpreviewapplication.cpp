/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qmlpreviewapplication.h"

#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QCommandLineParser>
#include <QtCore/QTemporaryFile>
#include <QtCore/QUrl>

QmlPreviewApplication::QmlPreviewApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv),
    m_verbose(false),
    m_connectionAttempts(0)
{
    m_connection.reset(new QQmlDebugConnection);
    m_qmlPreviewClient.reset(new QQmlPreviewClient(m_connection.data()));
    m_connectTimer.setInterval(1000);

    m_loadTimer.setInterval(100);
    m_loadTimer.setSingleShot(true);
    connect(&m_loadTimer, &QTimer::timeout, this, [this]() {
        m_qmlPreviewClient->triggerLoad(QUrl());
    });

    connect(&m_connectTimer, &QTimer::timeout, this, &QmlPreviewApplication::tryToConnect);
    connect(m_connection.data(), &QQmlDebugConnection::connected, &m_connectTimer, &QTimer::stop);

    connect(m_qmlPreviewClient.data(), &QQmlPreviewClient::error,
            this, &QmlPreviewApplication::logError);
    connect(m_qmlPreviewClient.data(), &QQmlPreviewClient::request,
            this, &QmlPreviewApplication::serveRequest);

    connect(&m_watcher, &QmlPreviewFileSystemWatcher::fileChanged,
            this, &QmlPreviewApplication::sendFile);
    connect(&m_watcher, &QmlPreviewFileSystemWatcher::directoryChanged,
            this, &QmlPreviewApplication::sendDirectory);
}

QmlPreviewApplication::~QmlPreviewApplication()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        logStatus("Terminating process ...");
        m_process->disconnect();
        m_process->terminate();
        if (!m_process->waitForFinished(1000)) {
            logStatus("Killing process ...");
            m_process->kill();
        }
    }
}

void QmlPreviewApplication::parseArguments()
{
    setApplicationName(QLatin1String("qmlpreview"));
    setApplicationVersion(QLatin1String(qVersion()));

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);

    parser.setApplicationDescription(QChar::LineFeed + tr(
        "The QML Preview tool watches QML and JavaScript files on disk and updates\n"
        "the application live with any changes. The application to be previewed\n"
        "has to enable QML debugging. See the Qt Creator documentation on how to do\n"
        "this for different Qt versions."));

    QCommandLineOption verbose(QStringList() << QLatin1String("verbose"),
                               tr("Print debugging output."));
    parser.addOption(verbose);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(QLatin1String("program"),
                                 tr("The program to be started and previewed."),
                                 QLatin1String("[program]"));
    parser.addPositionalArgument(QLatin1String("parameters"),
                                 tr("Parameters for the program to be started."),
                                 QLatin1String("[parameters...]"));

    parser.process(*this);

    QTemporaryFile file;
    if (file.open())
        m_socketFile = file.fileName();

    if (parser.isSet(verbose))
        m_verbose = true;

    m_programArguments = parser.positionalArguments();
    if (!m_programArguments.isEmpty())
        m_programPath = m_programArguments.takeFirst();

    if (m_programPath.isEmpty()) {
        logError(tr("You have to specify a program to start."));
        parser.showHelp(2);
    }
}

int QmlPreviewApplication::exec()
{
    QTimer::singleShot(0, this, &QmlPreviewApplication::run);
    return QCoreApplication::exec();
}

void QmlPreviewApplication::run()
{
    logStatus(QString("Listening on %1 ...").arg(m_socketFile));
    m_connection->startLocalServer(m_socketFile);
    m_process.reset(new QProcess(this));
    QStringList arguments;
    arguments << QString("-qmljsdebugger=file:%1,block,services:QmlPreview").arg(m_socketFile);
    arguments << m_programArguments;

    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process.data(), &QIODevice::readyRead,
            this, &QmlPreviewApplication::processHasOutput);
    connect(m_process.data(), static_cast<void(QProcess::*)(int)>(&QProcess::finished),
            this, [this](int){ processFinished(); });
    logStatus(QString("Starting '%1 %2' ...").arg(m_programPath, arguments.join(QLatin1Char(' '))));
    m_process->start(m_programPath, arguments);
    if (!m_process->waitForStarted()) {
        logError(QString("Could not run '%1': %2").arg(m_programPath, m_process->errorString()));
        exit(1);
    }
    m_connectTimer.start();
}

void QmlPreviewApplication::tryToConnect()
{
    Q_ASSERT(!m_connection->isConnected());
    ++m_connectionAttempts;

    if (m_verbose && !(m_connectionAttempts % 5)) {// print every 5 seconds
        logError(QString("No connection received on %1 for %2 seconds ...")
                 .arg(m_socketFile).arg(m_connectionAttempts));
    }
}

void QmlPreviewApplication::processHasOutput()
{
    Q_ASSERT(m_process);
    while (m_process->bytesAvailable()) {
        QTextStream out(stderr);
        out << m_process->readAll();
    }
}

void QmlPreviewApplication::processFinished()
{
    Q_ASSERT(m_process);
    int exitCode = 0;
    if (m_process->exitStatus() == QProcess::NormalExit) {
        logStatus(QString("Process exited (%1).").arg(m_process->exitCode()));
    } else {
        logError("Process crashed!");
        exitCode = 3;
    }
    exit(exitCode);
}

void QmlPreviewApplication::logError(const QString &error)
{
    QTextStream err(stderr);
    err << "Error: " << error << endl;
}

void QmlPreviewApplication::logStatus(const QString &status)
{
    if (!m_verbose)
        return;
    QTextStream err(stderr);
    err << status << endl;
}

void QmlPreviewApplication::serveRequest(const QString &path)
{
    QFileInfo info(path);

    if (info.isDir()) {
        m_qmlPreviewClient->sendDirectory(path, QDir(path).entryList());
        m_watcher.addDirectory(path);
    } else {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            m_qmlPreviewClient->sendFile(path, file.readAll());
            m_watcher.addFile(path);
        } else {
            logStatus(QString("Could not open file %1 for reading: %2").arg(path)
                      .arg(file.errorString()));
            m_qmlPreviewClient->sendError(path);
        }
    }
}

bool QmlPreviewApplication::sendFile(const QString &path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        m_qmlPreviewClient->sendFile(path, file.readAll());
        // Defer the Load, because files tend to change multiple times in a row.
        m_loadTimer.start();
        return true;
    }
    logStatus(QString("Could not open file %1 for reading: %2").arg(path).arg(file.errorString()));
    return false;
}

void QmlPreviewApplication::sendDirectory(const QString &path)
{
    m_qmlPreviewClient->sendDirectory(path, QDir(path).entryList());
    m_loadTimer.start();
}
