/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qqmldebugprocess_p.h"
#include "debugutil_p.h"
#include "qqmlpreviewblacklist.h"

#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>
#include <QtCore/qthread.h>
#include <QtCore/qlibraryinfo.h>
#include <QtNetwork/qhostaddress.h>

#include <private/qqmldebugconnection_p.h>
#include <private/qqmlpreviewclient_p.h>

class tst_QQmlPreview : public QQmlDebugTest
{
    Q_OBJECT

private:
    ConnectResult startQmlProcess(const QString &qmlFile);
    void serveRequest(const QString &path);
    QList<QQmlDebugClient *> createClients() override;
    void verifyProcessOutputContains(const QString &string) const;

    QPointer<QQmlPreviewClient> m_client;

    QStringList m_files;
    QStringList m_filesNotFound;
    QStringList m_directories;
    QStringList m_serviceErrors;
    QQmlPreviewClient::FpsInfo m_frameStats;

private slots:
    void cleanup() final;

    void connect();
    void load();
    void rerun();
    void blacklist();
    void error();
    void zoom();
    void fps();
    void language();
};

QQmlDebugTest::ConnectResult tst_QQmlPreview::startQmlProcess(const QString &qmlFile)
{
    return QQmlDebugTest::connect(QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qml",
                                  QStringLiteral("QmlPreview"), testFile(qmlFile), true);
}

void tst_QQmlPreview::serveRequest(const QString &path)
{
    QFileInfo info(path);

    if (info.isDir()) {
        m_directories.append(path);
        m_client->sendDirectory(path, QDir(path).entryList());
    } else {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            m_files.append(path);
            m_client->sendFile(path, file.readAll());
        } else {
            m_filesNotFound.append(path);
            m_client->sendError(path);
        }
    }
}

QList<QQmlDebugClient *> tst_QQmlPreview::createClients()
{
    m_client = new QQmlPreviewClient(m_connection);

    QObject::connect(m_client.data(), &QQmlPreviewClient::request, this, &tst_QQmlPreview::serveRequest);
    QObject::connect(m_client.data(), &QQmlPreviewClient::error, this, [this](const QString &error) {
        m_serviceErrors.append(error);
    });
    QObject::connect(m_client.data(), &QQmlPreviewClient::fps,
                     this, [this](const QQmlPreviewClient::FpsInfo &info) {
        m_frameStats = info;
    });

    return QList<QQmlDebugClient *>({m_client});
}

void tst_QQmlPreview::verifyProcessOutputContains(const QString &string) const
{
    QTRY_VERIFY_WITH_TIMEOUT(m_process->output().contains(string), 30000);
}

void checkFiles(const QStringList &files)
{
    QVERIFY(!files.contains("/etc/localtime"));
    QVERIFY(!files.contains("/etc/timezome"));
    QVERIFY(!files.contains(":/qgradient/webgradients.binaryjson"));
}

void tst_QQmlPreview::cleanup()
{
    // Use a separate function so that we don't return early from cleanup() on failure.
    checkFiles(m_files);

    QQmlDebugTest::cleanup();
    if (QTest::currentTestFailed()) {
        qDebug() << "Files loaded:" << m_files;
        qDebug() << "Files not loaded:" << m_filesNotFound;
        qDebug() << "Directories loaded:" << m_directories;
        qDebug() << "Errors reported:" << m_serviceErrors;
    }

    m_directories.clear();
    m_files.clear();
    m_filesNotFound.clear();
    m_serviceErrors.clear();
    m_frameStats = QQmlPreviewClient::FpsInfo();
}

void tst_QQmlPreview::connect()
{
    const QString file("window.qml");
    QCOMPARE(startQmlProcess(file), ConnectSuccess);
    QVERIFY(m_client);
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::Enabled);
    m_client->triggerLoad(testFileUrl(file));
    QTRY_VERIFY(m_files.contains(testFile(file)));
    verifyProcessOutputContains(file);
    m_process->stop();
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::NotConnected);
    QVERIFY(m_serviceErrors.isEmpty());
}

void tst_QQmlPreview::load()
{
    const QString file("qtquick2.qml");
    QCOMPARE(startQmlProcess(file), ConnectSuccess);
    QVERIFY(m_client);
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::Enabled);
    m_client->triggerLoad(testFileUrl(file));
    QTRY_VERIFY(m_files.contains(testFile(file)));
    verifyProcessOutputContains("ms/degrees");

    const QStringList files({"window2.qml", "window1.qml", "window.qml"});
    for (const QString &newFile : files) {
        m_client->triggerLoad(testFileUrl(newFile));
        QTRY_VERIFY(m_files.contains(testFile(newFile)));
        verifyProcessOutputContains(newFile);
    }

    m_process->stop();
    QTRY_COMPARE(m_client->state(), QQmlDebugClient::NotConnected);
    QVERIFY(m_serviceErrors.isEmpty());
}

void tst_QQmlPreview::rerun()
{
    const QString file("window.qml");
    QCOMPARE(startQmlProcess(file), ConnectSuccess);
    QVERIFY(m_client);
    m_client->triggerLoad(testFileUrl(file));
    const QLatin1String message("window.qml");
    verifyProcessOutputContains(message);
    const int pos = m_process->output().lastIndexOf(message) + message.size();
    QVERIFY(pos >= 0);

    m_client->triggerRerun();
    QTRY_VERIFY_WITH_TIMEOUT(m_process->output().indexOf(message, pos) >= pos, 30000);

    m_process->stop();
    QVERIFY(m_serviceErrors.isEmpty());
}

void tst_QQmlPreview::blacklist()
{
    QQmlPreviewBlacklist blacklist;

    QStringList strings({
        "lalala", "lulul", "trakdkd", "suppe", "zack"
    });

    for (const QString &string : strings)
        QVERIFY(!blacklist.isBlacklisted(string));

    for (const QString &string : strings)
        blacklist.blacklist(string);

    for (const QString &string : strings) {
        QVERIFY(blacklist.isBlacklisted(string));
        QVERIFY(!blacklist.isBlacklisted(string.left(string.size() / 2)));
        QVERIFY(!blacklist.isBlacklisted(string + "45"));
        QVERIFY(!blacklist.isBlacklisted(" " + string));
        QVERIFY(blacklist.isBlacklisted(string + "/45"));
    }

    for (auto begin = strings.begin(), it = begin, end = strings.end(); it != end; ++it) {
        std::rotate(begin, it, end);
        QString path = "/" + strings.join('/');
        blacklist.blacklist(path);
        QVERIFY(blacklist.isBlacklisted(path));
        QVERIFY(blacklist.isBlacklisted(path + "/file"));
        QVERIFY(!blacklist.isBlacklisted(path + "more"));
        path.chop(1);
        QVERIFY(!blacklist.isBlacklisted(path));
        std::reverse(begin, end);
    }

    blacklist.clear();
    for (const QString &string : strings)
        QVERIFY(!blacklist.isBlacklisted(string));

    blacklist.blacklist(":/qt-project.org");
    QVERIFY(blacklist.isBlacklisted(":/qt-project.org/QmlRuntime/conf/configuration.qml"));
    QVERIFY(!blacklist.isBlacklisted(":/qt-project.orgQmlRuntime/conf/configuration.qml"));

    QQmlPreviewBlacklist blacklist2;

    blacklist2.blacklist(":/qt-project.org");
    blacklist2.blacklist(":/QtQuick/Controls/Styles");
    blacklist2.blacklist(":/ExtrasImports/QtQuick/Controls/Styles");
    blacklist2.blacklist(QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath));
    blacklist2.blacklist("/home/ulf/.local/share/QtProject/Qml Runtime/configuration.qml");
    blacklist2.blacklist("/usr/share");
    blacklist2.blacklist("/usr/share/QtProject/Qml Runtime/configuration.qml");
    QVERIFY(blacklist2.isBlacklisted(QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath)));
    blacklist2.blacklist("/usr/local/share/QtProject/Qml Runtime/configuration.qml");
    blacklist2.blacklist("qml");
    blacklist2.blacklist(""); // This should not remove all other paths.

    QVERIFY(blacklist2.isBlacklisted(QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath) +
                                     "/QtQuick/Window.2.0"));
    QVERIFY(blacklist2.isBlacklisted(QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath)));
    QVERIFY(blacklist2.isBlacklisted("/usr/share/QtProject/Qml Runtime/configuration.qml"));
    QVERIFY(blacklist2.isBlacklisted("/usr/share/stuff"));
    QVERIFY(blacklist2.isBlacklisted(""));

    QQmlPreviewBlacklist blacklist3;
    blacklist3.blacklist("/usr/share");
    blacklist3.blacklist("/usr");
    blacklist3.blacklist("/usrdings");
    QVERIFY(blacklist3.isBlacklisted("/usrdings"));
    QVERIFY(blacklist3.isBlacklisted("/usr/src"));
    QVERIFY(!blacklist3.isBlacklisted("/opt/share"));
    QVERIFY(!blacklist3.isBlacklisted("/opt"));

    blacklist3.whitelist("/usr/share");
    QVERIFY(blacklist3.isBlacklisted("/usrdings"));
    QVERIFY(!blacklist3.isBlacklisted("/usr"));
    QVERIFY(!blacklist3.isBlacklisted("/usr/share"));
    QVERIFY(!blacklist3.isBlacklisted("/usr/src"));
    QVERIFY(!blacklist3.isBlacklisted("/opt/share"));
    QVERIFY(!blacklist3.isBlacklisted("/opt"));
}

void tst_QQmlPreview::error()
{
    QCOMPARE(startQmlProcess("window.qml"), ConnectSuccess);
    QVERIFY(m_client);
    m_client->triggerLoad(testFileUrl("broken.qml"));
    QTRY_COMPARE_WITH_TIMEOUT(m_serviceErrors.count(), 1, 10000);
    QVERIFY(m_serviceErrors.first().contains("broken.qml:32 Expected token `}'"));
}

static float parseZoomFactor(const QString &output)
{
    const QString prefix("zoom ");
    const int start = output.lastIndexOf(prefix) + prefix.length();
    if (start < 0)
        return -1;
    const int end = output.indexOf('\n', start);
    if (end < 0)
        return -1;
    bool ok = false;
    const float zoomFactor = output.mid(start, end - start).toFloat(&ok);
    if (!ok)
        return -1;
    return zoomFactor;
}

static void verifyZoomFactor(const QQmlDebugProcess *process, float factor)
{
    QTRY_VERIFY_WITH_TIMEOUT(qFuzzyCompare(parseZoomFactor(process->output()), factor), 30000);
}

void tst_QQmlPreview::zoom()
{
    const QString file("zoom.qml");
    QCOMPARE(startQmlProcess(file), ConnectSuccess);
    QVERIFY(m_client);
    m_client->triggerLoad(testFileUrl(file));
    QTRY_VERIFY(m_files.contains(testFile(file)));
    float baseZoomFactor = -1;
    QTRY_VERIFY_WITH_TIMEOUT((baseZoomFactor = parseZoomFactor(m_process->output())) > 0, 30000);

    for (auto testZoomFactor : {2.0f, 1.5f, 0.5f}) {
        m_client->triggerZoom(testZoomFactor);
        verifyZoomFactor(m_process, testZoomFactor);
    }

    m_client->triggerZoom(-1.0f);
    verifyZoomFactor(m_process, baseZoomFactor);
    m_process->stop();
    QVERIFY(m_serviceErrors.isEmpty());
}

void tst_QQmlPreview::fps()
{
    const QString file("qtquick2.qml");
    QCOMPARE(startQmlProcess(file), ConnectSuccess);
    QVERIFY(m_client);
    m_client->triggerLoad(testFileUrl(file));
    if (QGuiApplication::platformName() != "offscreen") {
        QTRY_VERIFY_WITH_TIMEOUT(m_frameStats.numSyncs > 10, 10000);
        QVERIFY(m_frameStats.minSync <= m_frameStats.maxSync);
        QVERIFY(m_frameStats.totalSync / m_frameStats.numSyncs >= m_frameStats.minSync - 1);
        QVERIFY(m_frameStats.totalSync / m_frameStats.numSyncs <= m_frameStats.maxSync);

        QVERIFY(m_frameStats.numRenders > 0);
        QVERIFY(m_frameStats.minRender <= m_frameStats.maxRender);
        QVERIFY(m_frameStats.totalRender / m_frameStats.numRenders >= m_frameStats.minRender - 1);
        QVERIFY(m_frameStats.totalRender / m_frameStats.numRenders <= m_frameStats.maxRender);
    } else {
        QSKIP("offscreen rendering doesn't produce any frames");
    }
}

void tst_QQmlPreview::language()
{
    QCOMPARE(startQmlProcess("window.qml"), ConnectSuccess);
    QVERIFY(m_client);
    m_client->triggerLanguage(dataDirectoryUrl(), "fr_FR");
    QTRY_VERIFY_WITH_TIMEOUT(m_files.contains(testFile("i18n/qml_fr_FR.qm")), 30000);
}

QTEST_MAIN(tst_QQmlPreview)

#include "tst_qqmlpreview.moc"
