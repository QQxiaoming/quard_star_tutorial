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

#include <QtTest/QtTest>
#include <QtCore/qprocess.h>
#include <QtCore/qtemporaryfile.h>
#include <private/qv4global_p.h>

class tst_QV4Assembler : public QObject
{
    Q_OBJECT

private slots:
    void perfMapFile();
    void jitEnabled();
};

void tst_QV4Assembler::perfMapFile()
{
#if !defined(Q_OS_LINUX)
    QSKIP("perf map files are only generated on linux");
#else
    const QString qmljs = QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/qmljs";
    QProcess process;

    QTemporaryFile infile;
    QVERIFY(infile.open());
    infile.write("'use strict'; function foo() { return 42 }; foo();");
    infile.close();

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("QV4_PROFILE_WRITE_PERF_MAP", "1");
    environment.insert("QV4_JIT_CALL_THRESHOLD", "0");

    process.setProcessEnvironment(environment);
    process.start(qmljs, QStringList({infile.fileName()}));
    QVERIFY(process.waitForStarted());
    const qint64 pid = process.processId();
    QVERIFY(pid != 0);
    QVERIFY(process.waitForFinished());
    QCOMPARE(process.exitCode(), 0);

    QFile file(QString::fromLatin1("/tmp/perf-%1.map").arg(pid));
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly));
    QList<QByteArray> functions;
    while (!file.atEnd()) {
        const QByteArray contents = file.readLine();
        QVERIFY(contents.endsWith('\n'));
        QList<QByteArray> fields = contents.split(' ');
        QCOMPARE(fields.length(), 3);
        bool ok = false;
        const qulonglong address = fields[0].toULongLong(&ok, 16);
        QVERIFY(ok);
        QVERIFY(address > 0);
        const ulong size = fields[1].toULong(&ok, 16);
        QVERIFY(ok);
        QVERIFY(size > 0);
        functions.append(fields[2]);
    }
    QVERIFY(functions.contains("foo\n"));
#endif
}

#ifdef V4_ENABLE_JIT
#define JIT_ENABLED 1
#else
#define JIT_ENABLED 0
#endif

void tst_QV4Assembler::jitEnabled()
{
#if defined(Q_OS_IOS) || defined(Q_OS_TVOS)
    /* JIT should be disabled on iOS and tvOS. */
    QCOMPARE(JIT_ENABLED, 0);
#elif defined(Q_OS_WIN) && defined(Q_PROCESSOR_ARM)
    /* JIT should be disabled Windows on ARM/ARM64 for now. */
    QCOMPARE(JIT_ENABLED, 0);
#else
    /* JIT should be enabled on all other architectures/OSes tested in CI. */
    QCOMPARE(JIT_ENABLED, 1);
#endif
}

QTEST_MAIN(tst_QV4Assembler)

#include "tst_qv4assembler.moc"

