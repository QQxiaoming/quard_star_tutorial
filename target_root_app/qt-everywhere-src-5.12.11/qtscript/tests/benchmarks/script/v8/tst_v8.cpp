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

#include <qtest.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptvalue.h>

static QString readFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return QString();
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    return stream.readAll();
}

class tst_V8 : public QObject
{
    Q_OBJECT

public:
    tst_V8();
    virtual ~tst_V8();

public slots:
    void init();
    void cleanup();

private slots:
    void benchmark_data();
    void benchmark();

private:
    QDir testsDir;
};

tst_V8::tst_V8()
{
    testsDir = QDir(":/tests");
    if (!testsDir.exists())
        qWarning("*** no tests/ dir!");
}

tst_V8::~tst_V8()
{
}

void tst_V8::init()
{
}

void tst_V8::cleanup()
{
}

void tst_V8::benchmark_data()
{
    QTest::addColumn<QString>("testName");
    const QFileInfoList testFileInfos = testsDir.entryInfoList(QStringList() << "*.js", QDir::Files);
    for (const QFileInfo &tfi : testFileInfos) {
        QString name = tfi.baseName();
        if (name == QString::fromLatin1("base")) {
            // base.js contains the benchmark library, it's not a test.
            continue;
        }
        QTest::newRow(name.toLatin1().constData()) << name;
    }
}

void tst_V8::benchmark()
{
    QFETCH(QString, testName);

    QString baseDotJsContents = readFile(testsDir.filePath("base.js"));
    QVERIFY(!baseDotJsContents.isEmpty());

    QString testContents = readFile(testsDir.filePath(testName + ".js"));
    QVERIFY(!testContents.isEmpty());

    QScriptEngine engine;
    engine.evaluate(baseDotJsContents);
    QVERIFY(!engine.hasUncaughtException());
    engine.evaluate(testContents);
    QVERIFY(!engine.hasUncaughtException());

    QBENCHMARK {
        engine.evaluate("BenchmarkSuite.RunSuites({})");
    }
    QVERIFY(!engine.hasUncaughtException());
}

QTEST_MAIN(tst_V8)
#include "tst_v8.moc"
