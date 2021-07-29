/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include <QtCore/qdir.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qprocess.h>

#include <QtTest/qtest.h>

class tst_qtattributionsscanner : public QObject
{
    Q_OBJECT
public:
    tst_qtattributionsscanner();

private slots:
    void test_data();
    void test();

private:
    void readExpectedFile(const QString &baseDir, const QString &fileName, QByteArray *content);

    QString m_cmd;
    QString m_basePath;
};


tst_qtattributionsscanner::tst_qtattributionsscanner()
{
    QString binPath = QLibraryInfo::location(QLibraryInfo::BinariesPath);
    m_cmd = binPath + QLatin1String("/qtattributionsscanner");
    m_basePath = QFINDTESTDATA("testdata");
}


void tst_qtattributionsscanner::test_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("stdout_file");
    QTest::addColumn<QString>("stderr_file");

    QTest::newRow("good")
            << QStringLiteral("good")
            << QStringLiteral("good/expected.json")
            << QStringLiteral("good/expected.error");
    QTest::newRow("warnings (incomplete)")
            << QStringLiteral("warnings/incomplete")
            << QStringLiteral("warnings/incomplete/expected.json")
            << QStringLiteral("warnings/incomplete/expected.error");
    QTest::newRow("warnings (unknown attribute)")
            << QStringLiteral("warnings/unknown")
            << QStringLiteral("warnings/unknown/expected.json")
            << QStringLiteral("warnings/unknown/expected.error");
    QTest::newRow("singlefile")
            << QStringLiteral("good/minimal/qt_attribution_test.json")
            << QStringLiteral("good/minimal/expected.json")
            << QStringLiteral("good/minimal/expected.error");
}

void tst_qtattributionsscanner::readExpectedFile(const QString &baseDir, const QString &fileName, QByteArray *content)
{
    QFile file(QDir(m_basePath).absoluteFilePath(fileName));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Could not open " + file.fileName().toLocal8Bit());
    *content = file.readAll();
    content->replace("%{PWD}", baseDir.toUtf8());
}

void tst_qtattributionsscanner::test()
{
    QFETCH(QString, input);
    QFETCH(QString, stdout_file);
    QFETCH(QString, stderr_file);

    QString dir = QDir(m_basePath).absoluteFilePath(input);
    if (QFileInfo(dir).isFile())
        dir = QFileInfo(dir).absolutePath();

    QProcess proc;
    QString command = m_cmd + " " + dir + " --output-format json";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_ATTRIBUTIONSSCANNER_TEST", "1");
    proc.setProcessEnvironment(env);
    proc.start(command, QIODevice::ReadWrite | QIODevice::Text);

    QVERIFY2(proc.waitForStarted(), qPrintable(command + QLatin1String(" :") + proc.errorString()));
    QVERIFY2(proc.waitForFinished(30000), qPrintable(command));

    QVERIFY2(proc.exitStatus() == QProcess::NormalExit,
             "\"qtattributionsscanner " + m_cmd.toLatin1() + "\" crashed");
    QVERIFY2(!proc.exitCode(),
             "\"qtattributionsscanner " + m_cmd.toLatin1() + "\" exited with code " +
             QByteArray::number(proc.exitCode()));

    { // compare error output
        QByteArray stdErr = proc.readAllStandardError();
        stdErr.replace(QDir::separator(), "/");

        QByteArray expectedErrorOutput;
        readExpectedFile(dir, stderr_file, &expectedErrorOutput);

        QCOMPARE(stdErr, expectedErrorOutput);
    }

    { // compare json output
        QByteArray stdOut = proc.readAllStandardOutput();

        QJsonParseError jsonError;
        QJsonDocument actualJson = QJsonDocument::fromJson(stdOut, &jsonError);
        QVERIFY2(!actualJson.isNull(), "Invalid output: " + jsonError.errorString().toLatin1());

        QByteArray expectedOutput;
        readExpectedFile(dir, stdout_file, &expectedOutput);
        QJsonDocument expectedJson = QJsonDocument::fromJson(expectedOutput);

        if (!QTest::qCompare(actualJson, expectedJson, "actualJson", "expectedJson", __FILE__, __LINE__)) {
            qWarning() << "Actual (actualJson)    :" << actualJson;
            qWarning() << "Expected (expectedJson):" << expectedJson;
            return;
        }
    }
}

QTEST_MAIN(tst_qtattributionsscanner)
#include "tst_qtattributionsscanner.moc"
