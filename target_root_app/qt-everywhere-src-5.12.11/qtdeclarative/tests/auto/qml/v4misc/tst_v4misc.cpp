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

#include <qtest.h>
#include <private/qv4instr_moth_p.h>
#include <private/qv4script_p.h>

class tst_v4misc: public QObject
{
    Q_OBJECT

private slots:
    void tdzOptimizations_data();
    void tdzOptimizations();

    void parserMisc_data();
    void parserMisc();

    void subClassing_data();
    void subClassing();

    void nestingDepth();
};

void tst_v4misc::tdzOptimizations_data()
{
    QTest::addColumn<QString>("scriptToCompile");

    QTest::newRow("access-after-let") << QString("let x; x = 10;");
    QTest::newRow("access-after-const") << QString("const x = 10; print(x);");
    QTest::newRow("access-after-let") << QString("for (let x of y) print(x);");
}

void tst_v4misc::tdzOptimizations()
{
    QFETCH(QString, scriptToCompile);

    QV4::ExecutionEngine v4;
    QV4::Script script(&v4, nullptr, /*parse as binding*/false, scriptToCompile);
    script.parse();
    QVERIFY(!v4.hasException);

    const auto function = script.compilationUnit->unitData()->functionAt(0);
    const auto *code = function->code();
    const auto len = function->codeSize;
    const char *end = code + len;

    const auto decodeInstruction = [&code]() {
        QV4::Moth::Instr::Type type = QV4::Moth::Instr::Type(static_cast<uchar>(*code));
    dispatch:
        switch (type) {
            case QV4::Moth::Instr::Type::Nop:
                ++code;
                type = QV4::Moth::Instr::Type(static_cast<uchar>(*code));
                goto dispatch;
            case QV4::Moth::Instr::Type::Nop_Wide: /* wide prefix */
                ++code;
                type = QV4::Moth::Instr::Type(0x100 | static_cast<uchar>(*code));
                goto dispatch;

#define CASE_AND_GOTO_INSTRUCTION(name, nargs, ...) \
      case QV4::Moth::Instr::Type::name: \
            MOTH_ADJUST_CODE(qint8, nargs); \
            break;

#define CASE_AND_GOTO_WIDE_INSTRUCTION(name, nargs, ...) \
      case QV4::Moth::Instr::Type::name##_Wide: \
            MOTH_ADJUST_CODE(int, nargs); \
            type = QV4::Moth::Instr::Type::name; \
            break;

#define MOTH_DECODE_WITHOUT_ARGS(instr) \
     INSTR_##instr(CASE_AND_GOTO) \
     INSTR_##instr(CASE_AND_GOTO_WIDE)

            FOR_EACH_MOTH_INSTR(MOTH_DECODE_WITHOUT_ARGS)
        }
        return type;
    };

    while (code < end) {
        QV4::Moth::Instr::Type type = decodeInstruction();
        QVERIFY(type != QV4::Moth::Instr::Type::DeadTemporalZoneCheck);
    }

}

void tst_v4misc::parserMisc_data()
{
    QTest::addColumn<QString>("error");

    QTest::newRow("8[++i][+++i]") << QString("ReferenceError: Prefix ++ operator applied to value that is not a reference.");
    QTest::newRow("`a${1++}`") << QString("ReferenceError: Invalid left-hand side expression in postfix operation");
    QTest::newRow("for (var f in ++!binaryMathg) ;") << QString("ReferenceError: Prefix ++ operator applied to value that is not a reference.");
    QTest::newRow("for (va() in obj) {}") << QString("ReferenceError: Invalid left-hand side expression for 'in' expression");
    QTest::newRow("[1]=7[A=8=9]") << QString("ReferenceError: left-hand side of assignment operator is not an lvalue");
    QTest::newRow("var asmvalsLen = asmvals{{{{{ngth}}}}};") << QString("SyntaxError: Expected token `;'");
    QTest::newRow("T||9[---L6i]") << QString("ReferenceError: Prefix ++ operator applied to value that is not a reference.");
    QTest::newRow("a?b:[---Hi]") << QString("ReferenceError: Prefix ++ operator applied to value that is not a reference.");
    QTest::newRow("[``]=1") << QString("ReferenceError: Binding target is not a reference.");
}

void tst_v4misc::parserMisc()
{
    QFETCH(QString, error);

    QJSEngine engine;
    QJSValue result = engine.evaluate(QString::fromUtf8(QTest::currentDataTag()));
    QVERIFY(result.isError());
    QCOMPARE(result.toString(), error);
}

void tst_v4misc::subClassing_data()
{
    QTest::addColumn<QString>("script");

    QString code(
                "class Foo extends %1 {"
                "    constructor() { super(); this.reset(); }"
                "    reset() { }"
                "}"
                "new Foo();");


    QTest::newRow("Array") << code.arg("Array");
    QTest::newRow("Boolean") << code.arg("Boolean");
    QTest::newRow("Date") << code.arg("Date");
    QTest::newRow("Function") << code.arg("Function");
    QTest::newRow("Number") << code.arg("Number");
    QTest::newRow("Map") << code.arg("Map");
    QTest::newRow("Promise") << QString(
            "class Foo extends Promise {"
            "    constructor() { super(Function()); this.reset(); }"
            "    reset() { }"
            "}"
            "new Foo();");
    QTest::newRow("RegExp") << code.arg("RegExp");
    QTest::newRow("Set") << code.arg("Set");
    QTest::newRow("String") << code.arg("String");
    QTest::newRow("WeakMap") << code.arg("WeakMap");
    QTest::newRow("WeakSet") << code.arg("WeakSet");
}

void tst_v4misc::subClassing()
{
    QFETCH(QString, script);

    QJSEngine engine;
    QJSValue result = engine.evaluate(script);
    QVERIFY(!result.isError());
}

void tst_v4misc::nestingDepth()
{
    { // left recursive
        QString s(40000, '`');

        QJSEngine engine;
        QJSValue result = engine.evaluate(s);
        QVERIFY(result.isError());
        QCOMPARE(result.toString(), "SyntaxError: Maximum statement or expression depth exceeded");
    }

    { // right recursive
        QString s(200000, '-');
        s += "\nd";

        QJSEngine engine;
        QJSValue result = engine.evaluate(s);
        QVERIFY(result.isError());
        QCOMPARE(result.toString(), "SyntaxError: Maximum statement or expression depth exceeded");
    }
}

QTEST_MAIN(tst_v4misc);

#include "tst_v4misc.moc"
