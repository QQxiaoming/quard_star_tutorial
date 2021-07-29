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

#include "qv4regexpobject_p.h"
#include "qv4objectproto_p.h"
#include "qv4regexp_p.h"
#include "qv4stringobject_p.h"
#include <private/qv4mm_p.h>
#include "qv4scopedvalue_p.h"
#include "qv4jscall_p.h"
#include "qv4symbol_p.h"

#include "private/qlocale_tools_p.h"

#include <QtCore/QDebug>
#include <QtCore/qregexp.h>
#include <cassert>
#include <typeinfo>
#include <iostream>
#include "qv4alloca_p.h"

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT QString qt_regexp_toCanonical(const QString &, QRegExp::PatternSyntax);

using namespace QV4;

DEFINE_OBJECT_VTABLE(RegExpObject);

void Heap::RegExpObject::init()
{
    Object::init();
    Scope scope(internalClass->engine);
    Scoped<QV4::RegExpObject> o(scope, this);
    value.set(scope.engine, QV4::RegExp::create(scope.engine, QString(), CompiledData::RegExp::RegExp_NoFlags));
    o->initProperties();
}

void Heap::RegExpObject::init(QV4::RegExp *value)
{
    Object::init();
    Scope scope(internalClass->engine);
    this->value.set(scope.engine, value->d());
    Scoped<QV4::RegExpObject> o(scope, this);
    o->initProperties();
}

// Converts a QRegExp to a JS RegExp.
// The conversion is not 100% exact since ECMA regexp and QRegExp
// have different semantics/flags, but we try to do our best.
void Heap::RegExpObject::init(const QRegExp &re)
{
    Object::init();

    // Convert the pattern to a ECMAScript pattern.
    QString pattern = QT_PREPEND_NAMESPACE(qt_regexp_toCanonical)(re.pattern(), re.patternSyntax());
    if (re.isMinimal()) {
        QString ecmaPattern;
        int len = pattern.length();
        ecmaPattern.reserve(len);
        int i = 0;
        const QChar *wc = pattern.unicode();
        bool inBracket = false;
        while (i < len) {
            QChar c = wc[i++];
            ecmaPattern += c;
            switch (c.unicode()) {
            case '?':
            case '+':
            case '*':
            case '}':
                if (!inBracket)
                    ecmaPattern += QLatin1Char('?');
                break;
            case '\\':
                if (i < len)
                    ecmaPattern += wc[i++];
                break;
            case '[':
                inBracket = true;
                break;
            case ']':
                inBracket = false;
                break;
            default:
                break;
            }
        }
        pattern = ecmaPattern;
    }

    Scope scope(internalClass->engine);
    Scoped<QV4::RegExpObject> o(scope, this);

    uint flags = (re.caseSensitivity() == Qt::CaseInsensitive ? CompiledData::RegExp::RegExp_IgnoreCase : CompiledData::RegExp::RegExp_NoFlags);
    o->d()->value.set(scope.engine, QV4::RegExp::create(scope.engine, pattern, flags));

    o->initProperties();
}

void RegExpObject::initProperties()
{
    setProperty(Index_LastIndex, Value::fromInt32(0));

    Q_ASSERT(value());
}

// Converts a JS RegExp to a QRegExp.
// The conversion is not 100% exact since ECMA regexp and QRegExp
// have different semantics/flags, but we try to do our best.
QRegExp RegExpObject::toQRegExp() const
{
    Qt::CaseSensitivity caseSensitivity = (value()->flags & CompiledData::RegExp::RegExp_IgnoreCase) ? Qt::CaseInsensitive : Qt::CaseSensitive;
    return QRegExp(*value()->pattern, caseSensitivity, QRegExp::RegExp2);
}

QString RegExpObject::toString() const
{
    QString p = *value()->pattern;
    if (p.isEmpty()) {
        p = QStringLiteral("(?:)");
    } else {
        // escape certain parts, see ch. 15.10.4
        p.replace('/', QLatin1String("\\/"));
    }
    return p;
}

QString RegExpObject::source() const
{
    Scope scope(engine());
    ScopedValue s(scope, get(scope.engine->id_source()));
    return s->toQString();
}

ReturnedValue RegExpObject::builtinExec(ExecutionEngine *engine, const String *str)
{
    QString s = str->toQString();

    Scope scope(engine);
    int offset = (global() || sticky()) ? lastIndex() : 0;
    if (offset < 0 || offset > s.length()) {
        setLastIndex(0);
        RETURN_RESULT(Encode::null());
    }

    Q_ALLOCA_VAR(uint, matchOffsets, value()->captureCount() * 2 * sizeof(uint));
    const uint result = Scoped<RegExp>(scope, value())->match(s, offset, matchOffsets);

    RegExpCtor *regExpCtor = static_cast<RegExpCtor *>(scope.engine->regExpCtor());
    regExpCtor->d()->clearLastMatch();

    if (result == JSC::Yarr::offsetNoMatch) {
        if (global() || sticky())
            setLastIndex(0);
        RETURN_RESULT(Encode::null());
    }

    Q_ASSERT(result <= uint(std::numeric_limits<int>::max()));

    // fill in result data
    ScopedArrayObject array(scope, scope.engine->newArrayObject(scope.engine->internalClasses(EngineBase::Class_RegExpExecArray)));
    int len = value()->captureCount();
    array->arrayReserve(len);
    ScopedValue v(scope);
    int strlen = s.length();
    for (int i = 0; i < len; ++i) {
        int start = matchOffsets[i * 2];
        int end = matchOffsets[i * 2 + 1];
        if (end > strlen)
            end = strlen;
        v = (start != -1) ? scope.engine->memoryManager->alloc<ComplexString>(str->d(), start, end - start)->asReturnedValue() : Encode::undefined();
        array->arrayPut(i, v);
    }
    array->setArrayLengthUnchecked(len);
    array->setProperty(Index_ArrayIndex, Value::fromInt32(int(result)));
    array->setProperty(Index_ArrayInput, *str);

    RegExpCtor::Data *dd = regExpCtor->d();
    dd->lastMatch.set(scope.engine, array);
    dd->lastInput.set(scope.engine, str->d());
    dd->lastMatchStart = matchOffsets[0];
    dd->lastMatchEnd = matchOffsets[1];

    if (global() || sticky())
        setLastIndex(matchOffsets[1]);

    return array.asReturnedValue();
}

DEFINE_OBJECT_VTABLE(RegExpCtor);

void Heap::RegExpCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("RegExp"));
    clearLastMatch();
}

void Heap::RegExpCtor::clearLastMatch()
{
    lastMatch.set(internalClass->engine, Value::nullValue());
    lastInput.set(internalClass->engine, internalClass->engine->id_empty()->d());
    lastMatchStart = 0;
    lastMatchEnd = 0;
}

static bool isRegExp(ExecutionEngine *e, const QV4::Value *arg)
{
    const QV4::Object *o = arg->objectValue();
    if (!o)
        return false;

    QV4::Value isRegExp = QV4::Value::fromReturnedValue(o->get(e->symbol_match()));
    if (!isRegExp.isUndefined())
        return isRegExp.toBoolean();
    const RegExpObject *re = o->as<RegExpObject>();
    return re ? true : false;
}

uint parseFlags(Scope &scope, const QV4::Value *f)
{
    uint flags = CompiledData::RegExp::RegExp_NoFlags;
    if (!f->isUndefined()) {
        ScopedString s(scope, f->toString(scope.engine));
        if (scope.hasException())
            return flags;
        QString str = s->toQString();
        for (int i = 0; i < str.length(); ++i) {
            if (str.at(i) == QLatin1Char('g') && !(flags & CompiledData::RegExp::RegExp_Global)) {
                flags |= CompiledData::RegExp::RegExp_Global;
            } else if (str.at(i) == QLatin1Char('i') && !(flags & CompiledData::RegExp::RegExp_IgnoreCase)) {
                flags |= CompiledData::RegExp::RegExp_IgnoreCase;
            } else if (str.at(i) == QLatin1Char('m') && !(flags & CompiledData::RegExp::RegExp_Multiline)) {
                flags |= CompiledData::RegExp::RegExp_Multiline;
            } else if (str.at(i) == QLatin1Char('u') && !(flags & CompiledData::RegExp::RegExp_Unicode)) {
                flags |= CompiledData::RegExp::RegExp_Unicode;
            } else if (str.at(i) == QLatin1Char('y') && !(flags & CompiledData::RegExp::RegExp_Sticky)) {
                flags |= CompiledData::RegExp::RegExp_Sticky;
            } else {
                scope.engine->throwSyntaxError(QStringLiteral("Invalid flags supplied to RegExp constructor"));
                return flags;
            }
        }
    }
    return flags;
}

ReturnedValue RegExpCtor::virtualCallAsConstructor(const FunctionObject *fo, const Value *argv, int argc, const Value *newTarget)
{
    Scope scope(fo);

    bool patternIsRegExp = argc ? ::isRegExp(scope.engine, argv) : false;

    if (newTarget == fo) {
        if (patternIsRegExp && (argc < 2 || argv[1].isUndefined())) {
            const Object *pattern = static_cast<const Object *>(argv);
            ScopedValue patternConstructor(scope, pattern->get(scope.engine->id_constructor()));
            if (patternConstructor->sameValue(*newTarget))
                return pattern->asReturnedValue();
        }
    }

    ScopedValue p(scope, argc ? argv[0] : Value::undefinedValue());
    ScopedValue f(scope, argc > 1 ? argv[1] : Value::undefinedValue());
    Scoped<RegExpObject> re(scope, p);
    QString pattern;
    uint flags = CompiledData::RegExp::RegExp_NoFlags;

    if (re) {
        if (f->isUndefined()) {
            Scoped<RegExp> regexp(scope, re->value());
            return Encode(scope.engine->newRegExpObject(regexp));
        }
        pattern = *re->value()->pattern;
        flags = parseFlags(scope, f);
    } else if (patternIsRegExp) {
        const Object *po = static_cast<const Object *>(argv);
        p = po->get(scope.engine->id_source());
        if (!p->isUndefined())
            pattern = p->toQString();
        if (scope.hasException())
            return Encode::undefined();
        if (f->isUndefined())
            f = po->get(scope.engine->id_flags());
        flags = parseFlags(scope, f);
    } else {
        if (!p->isUndefined())
            pattern = p->toQString();
        if (scope.hasException())
            return Encode::undefined();
        flags = parseFlags(scope, f);
    }
    if (scope.hasException())
        return Encode::undefined();

    Scoped<RegExp> regexp(scope, RegExp::create(scope.engine, pattern, flags));
    if (!regexp->isValid()) {
        return scope.engine->throwSyntaxError(QStringLiteral("Invalid regular expression"));
    }

    ReturnedValue o = Encode(scope.engine->newRegExpObject(regexp));

    if (!newTarget)
        return o;
    ScopedObject obj(scope, o);
    obj->setProtoFromNewTarget(newTarget);
    return obj->asReturnedValue();
}

ReturnedValue RegExpCtor::virtualCall(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    return virtualCallAsConstructor(f, argv, argc, f);
}

void RegExpPrototype::init(ExecutionEngine *engine, Object *constructor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ScopedObject ctor(scope, constructor);

    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(2));
    ctor->addSymbolSpecies();

    // Properties deprecated in the spec but required by "the web" :(
    ctor->defineAccessorProperty(QStringLiteral("lastMatch"), method_get_lastMatch_n<0>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$&"), method_get_lastMatch_n<0>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$1"), method_get_lastMatch_n<1>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$2"), method_get_lastMatch_n<2>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$3"), method_get_lastMatch_n<3>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$4"), method_get_lastMatch_n<4>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$5"), method_get_lastMatch_n<5>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$6"), method_get_lastMatch_n<6>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$7"), method_get_lastMatch_n<7>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$8"), method_get_lastMatch_n<8>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$9"), method_get_lastMatch_n<9>, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("lastParen"), method_get_lastParen, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$+"), method_get_lastParen, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("input"), method_get_input, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$_"), method_get_input, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("leftContext"), method_get_leftContext, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$`"), method_get_leftContext, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("rightContext"), method_get_rightContext, nullptr);
    ctor->defineAccessorProperty(QStringLiteral("$'"), method_get_rightContext, nullptr);

    defineDefaultProperty(QStringLiteral("constructor"), (o = ctor));
    defineAccessorProperty(scope.engine->id_flags(), method_get_flags, nullptr);
    defineAccessorProperty(scope.engine->id_global(), method_get_global, nullptr);
    defineAccessorProperty(scope.engine->id_ignoreCase(), method_get_ignoreCase, nullptr);
    defineDefaultProperty(QStringLiteral("exec"), method_exec, 1);
    defineDefaultProperty(engine->symbol_match(), method_match, 1);
    defineAccessorProperty(scope.engine->id_multiline(), method_get_multiline, nullptr);
    defineDefaultProperty(engine->symbol_replace(), method_replace, 2);
    defineDefaultProperty(engine->symbol_search(), method_search, 1);
    defineAccessorProperty(scope.engine->id_source(), method_get_source, nullptr);
    defineDefaultProperty(engine->symbol_split(), method_split, 2);
    defineAccessorProperty(scope.engine->id_sticky(), method_get_sticky, nullptr);
    defineDefaultProperty(QStringLiteral("test"), method_test, 1);
    defineDefaultProperty(engine->id_toString(), method_toString, 0);
    defineAccessorProperty(scope.engine->id_unicode(), method_get_unicode, nullptr);

    // another web extension
    defineDefaultProperty(QStringLiteral("compile"), method_compile, 2);
}

/* used by String.match */
ReturnedValue RegExpPrototype::execFirstMatch(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<RegExpObject> r(scope, thisObject->as<RegExpObject>());
    Q_ASSERT(r && r->global());

    ScopedString str(scope, argc ? argv[0] : Value::undefinedValue());
    Q_ASSERT(str);
    QString s = str->toQString();

    int offset = r->lastIndex();
    if (offset < 0 || offset > s.length()) {
        r->setLastIndex(0);
        RETURN_RESULT(Encode::null());
    }

    Q_ALLOCA_VAR(uint, matchOffsets, r->value()->captureCount() * 2 * sizeof(uint));
    const int result = Scoped<RegExp>(scope, r->value())->match(s, offset, matchOffsets);

    RegExpCtor *regExpCtor = static_cast<RegExpCtor *>(scope.engine->regExpCtor());
    regExpCtor->d()->clearLastMatch();

    if (result == -1) {
        r->setLastIndex(0);
        RETURN_RESULT(Encode::null());
    }

    ReturnedValue retVal = Encode::undefined();
    // return first match
    if (r->value()->captureCount()) {
        int start = matchOffsets[0];
        int end = matchOffsets[1];
        retVal = (start != -1) ? scope.engine->memoryManager->alloc<ComplexString>(str->d(), start, end - start)->asReturnedValue() : Encode::undefined();
    }

    RegExpCtor::Data *dd = regExpCtor->d();
    dd->lastInput.set(scope.engine, str->d());
    dd->lastMatchStart = matchOffsets[0];
    dd->lastMatchEnd = matchOffsets[1];

    r->setLastIndex(matchOffsets[1]);

    return retVal;
}

ReturnedValue RegExpPrototype::exec(ExecutionEngine *engine, const Object *o, const String *s)
{
    Scope scope(engine);
    ScopedString key(scope, scope.engine->newString(QStringLiteral("exec")));
    ScopedFunctionObject exec(scope, o->get(key));
    if (exec) {
        ScopedValue result(scope, exec->call(o, s, 1));
        if (scope.hasException())
            RETURN_UNDEFINED();
        if (!result->isNull() && !result->isObject())
            return scope.engine->throwTypeError();
        return result->asReturnedValue();
    }
    Scoped<RegExpObject> re(scope, o);
    if (!re)
        return scope.engine->throwTypeError();
    return re->builtinExec(engine, s);
}

ReturnedValue RegExpPrototype::method_exec(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<RegExpObject> r(scope, thisObject->as<RegExpObject>());
    if (!r)
        return scope.engine->throwTypeError();

    ScopedValue arg(scope, argc ? argv[0]: Value::undefinedValue());
    ScopedString str(scope, arg->toString(scope.engine));
    if (scope.hasException())
        RETURN_UNDEFINED();

    return  r->builtinExec(scope.engine, str);
}

ReturnedValue RegExpPrototype::method_get_flags(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    ScopedObject o(scope, thisObject);
    if (!o)
        return scope.engine->throwTypeError();

    QString result;
    ScopedValue v(scope);
    v = o->get(scope.engine->id_global());
    if (scope.hasException())
        return Encode::undefined();
    if (v->toBoolean())
        result += QLatin1Char('g');
    v = o->get(scope.engine->id_ignoreCase());
    if (scope.hasException())
        return Encode::undefined();
    if (v->toBoolean())
        result += QLatin1Char('i');
    v = o->get(scope.engine->id_multiline());
    if (scope.hasException())
        return Encode::undefined();
    if (v->toBoolean())
        result += QLatin1Char('m');
    v = o->get(scope.engine->id_unicode());
    if (scope.hasException())
        return Encode::undefined();
    if (v->toBoolean())
        result += QLatin1Char('u');
    v = o->get(scope.engine->id_sticky());
    if (scope.hasException())
        return Encode::undefined();
    if (v->toBoolean())
        result += QLatin1Char('y');
    return scope.engine->newString(result)->asReturnedValue();
}

ReturnedValue RegExpPrototype::method_get_global(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<RegExpObject> re(scope, thisObject);
    if (!re) {
        if (thisObject->sameValue(*scope.engine->regExpPrototype()))
            return Encode::undefined();
        return scope.engine->throwTypeError();
    }

    bool b = re->value()->flags & CompiledData::RegExp::RegExp_Global;
    return Encode(b);
}

ReturnedValue RegExpPrototype::method_get_ignoreCase(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<RegExpObject> re(scope, thisObject);
    if (!re) {
        if (thisObject->sameValue(*scope.engine->regExpPrototype()))
            return Encode::undefined();
        return scope.engine->throwTypeError();
    }

    bool b = re->value()->flags & CompiledData::RegExp::RegExp_IgnoreCase;
    return Encode(b);
}

static int advanceStringIndex(int index, const QString &str, bool unicode)
{
    if (unicode) {
        if (index < str.length() - 1 &&
            str.at(index).isHighSurrogate() &&
            str.at(index + 1).isLowSurrogate())
            ++index;
    }
    ++index;
    return index;
}

static void advanceLastIndexOnEmptyMatch(ExecutionEngine *e, bool unicode, QV4::Object *rx, const String *matchString, const QString &str)
{
    Scope scope(e);
    if (matchString->d()->length() == 0) {
        QV4::ScopedValue v(scope, rx->get(scope.engine->id_lastIndex()));
        int lastIndex = advanceStringIndex(v->toLength(), str, unicode);
        if (!rx->put(scope.engine->id_lastIndex(), QV4::Value::fromInt32(lastIndex)))
            scope.engine->throwTypeError();
    }
}

ReturnedValue RegExpPrototype::method_match(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    ScopedObject rx(scope, thisObject);
    if (!rx)
        return scope.engine->throwTypeError();
    ScopedString s(scope, (argc ? argv[0] : Value::undefinedValue()).toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();
    bool global = ScopedValue(scope, rx->get(scope.engine->id_global()))->toBoolean();

    if (!global)
        return exec(scope.engine, rx, s);

    bool unicode = ScopedValue(scope, rx->get(scope.engine->id_unicode()))->toBoolean();

    rx->put(scope.engine->id_lastIndex(), Value::fromInt32(0));
    ScopedArrayObject a(scope, scope.engine->newArrayObject());
    uint n = 0;

    ScopedValue result(scope);
    ScopedValue match(scope);
    ScopedString matchString(scope);
    ScopedValue v(scope);
    while (1) {
        result = exec(scope.engine, rx, s);
        if (scope.hasException())
            return Encode::undefined();
        if (result->isNull()) {
            if (!n)
                return Encode::null();
            return a->asReturnedValue();
        }
        Q_ASSERT(result->isObject());
        match = static_cast<Object &>(*result).get(PropertyKey::fromArrayIndex(0));
        matchString = match->toString(scope.engine);
        if (scope.hasException())
            return Encode::undefined();
        a->push_back(matchString);
        advanceLastIndexOnEmptyMatch(scope.engine, unicode, rx, matchString, s->toQString());
        ++n;
    }
}

ReturnedValue RegExpPrototype::method_get_multiline(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<RegExpObject> re(scope, thisObject);
    if (!re) {
        if (thisObject->sameValue(*scope.engine->regExpPrototype()))
            return Encode::undefined();
        return scope.engine->throwTypeError();
    }

    bool b = re->value()->flags & CompiledData::RegExp::RegExp_Multiline;
    return Encode(b);
}

ReturnedValue RegExpPrototype::method_replace(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    ScopedObject rx(scope, thisObject);
    if (!rx)
        return scope.engine->throwTypeError();

    ScopedString s(scope, (argc ? argv[0] : Value::undefinedValue()).toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();

    int lengthS = s->toQString().length();

    ScopedString replaceValue(scope);
    ScopedFunctionObject replaceFunction(scope, (argc > 1 ? argv[1] : Value::undefinedValue()));
    bool functionalReplace = !!replaceFunction;
    if (!functionalReplace)
        replaceValue = (argc > 1 ? argv[1] : Value::undefinedValue()).toString(scope.engine);

    ScopedValue v(scope);
    bool global = (v = rx->get(scope.engine->id_global()))->toBoolean();
    bool unicode = false;
    if (global) {
        unicode = (v = rx->get(scope.engine->id_unicode()))->toBoolean();
        if (!rx->put(scope.engine->id_lastIndex(), Value::fromInt32(0)))
            return scope.engine->throwTypeError();
    }

    ScopedArrayObject results(scope, scope.engine->newArrayObject());
    ScopedValue result(scope);
    ScopedValue match(scope);
    ScopedString matchString(scope);
    while (1) {
        result = exec(scope.engine, rx, s);
        if (scope.hasException())
            return Encode::undefined();
        if (result->isNull())
            break;
        results->push_back(result);
        if (!global)
            break;
        match = static_cast<Object &>(*result).get(PropertyKey::fromArrayIndex(0));
        matchString = match->toString(scope.engine);
        if (scope.hasException())
            return Encode::undefined();
        advanceLastIndexOnEmptyMatch(scope.engine, unicode, rx, matchString, s->toQString());
    }
    QString accumulatedResult;
    int nextSourcePosition = 0;
    int resultsLength = results->getLength();
    ScopedObject resultObject(scope);
    for (int i = 0; i < resultsLength; ++i) {
        resultObject = results->get(PropertyKey::fromArrayIndex(i));
        if (scope.hasException())
            return Encode::undefined();

        int nCaptures = resultObject->getLength();
        nCaptures = qMax(nCaptures - 1, 0);
        match = resultObject->get(PropertyKey::fromArrayIndex(0));
        matchString = match->toString(scope.engine);
        if (scope.hasException())
            return Encode::undefined();
        QString m = matchString->toQString();
        int matchLength = m.length();
        v = resultObject->get(scope.engine->id_index());
        int position = v->toInt32();
        position = qMax(qMin(position, lengthS), 0);
        if (scope.hasException())
            return Encode::undefined();

        int n = 1;
        Scope innerScope(scope.engine);
        JSCallData cData(scope, nCaptures + 3);
        while (n <= nCaptures) {
            v = resultObject->get(PropertyKey::fromArrayIndex(n));
            if (!v->isUndefined())
                cData->args[n] = v->toString(scope.engine);
            ++n;
        }
        QString replacement;
        if (functionalReplace) {
            cData->args[0] = matchString;
            cData->args[nCaptures + 1] = Encode(position);
            cData->args[nCaptures + 2] = s;
            ScopedValue replValue(scope, replaceFunction->call(cData));
            if (scope.hasException())
                return Encode::undefined();
            replacement = replValue->toQString();
        } else {
            replacement = RegExp::getSubstitution(matchString->toQString(), s->toQString(), position, cData.args, nCaptures, replaceValue->toQString());
        }
        if (scope.hasException())
            return Encode::undefined();
        if (position >= nextSourcePosition) {
            accumulatedResult += s->toQString().midRef(nextSourcePosition, position - nextSourcePosition) + replacement;
            nextSourcePosition = position + matchLength;
        }
    }
    if (nextSourcePosition < lengthS) {
        accumulatedResult += s->toQString().midRef(nextSourcePosition);
    }
    return scope.engine->newString(accumulatedResult)->asReturnedValue();
}

ReturnedValue RegExpPrototype::method_search(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    ScopedObject rx(scope, thisObject);
    if (!rx)
        return scope.engine->throwTypeError();

    ScopedString s(scope, (argc ? argv[0] : Value::undefinedValue()).toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();

    ScopedValue previousLastIndex(scope, rx->get(scope.engine->id_lastIndex()));
    if (previousLastIndex->toNumber() != 0) {
        if (!rx->put(scope.engine->id_lastIndex(), Value::fromInt32(0)))
            return scope.engine->throwTypeError();
    }

    ScopedValue result(scope, exec(scope.engine, rx, s));
    if (scope.hasException())
        return Encode::undefined();

    ScopedValue currentLastIndex(scope, rx->get(scope.engine->id_lastIndex()));
    if (!currentLastIndex->sameValue(previousLastIndex)) {
        if (!rx->put(scope.engine->id_lastIndex(), previousLastIndex))
            return scope.engine->throwTypeError();
    }

    if (result->isNull())
        return Encode(-1);
    ScopedObject o(scope, result);
    Q_ASSERT(o);
    return o->get(scope.engine->id_index());
}


ReturnedValue RegExpPrototype::method_get_source(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<RegExpObject> re(scope, thisObject);
    if (!re) {
        if (thisObject->sameValue(*scope.engine->regExpPrototype()))
            return scope.engine->newString(QStringLiteral("(?:)"))->asReturnedValue();
        return scope.engine->throwTypeError();
    }

    return scope.engine->newString(re->toString())->asReturnedValue();
}

ReturnedValue RegExpPrototype::method_split(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(f);
    ScopedObject rx(scope, thisObject);
    if (!rx)
        return scope.engine->throwTypeError();

    ScopedString s(scope, (argc ? argv[0] : Value::undefinedValue()).toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();

    ScopedValue flagsValue(scope, rx->get(scope.engine->id_flags()));
    ScopedString flags(scope, flagsValue->toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();
    QString flagsString = flags->toQString();
    if (!flagsString.contains(QLatin1Char('y')))
        flags = scope.engine->newString(flagsString + QLatin1Char('y'));
    bool unicodeMatching = flagsString.contains(QLatin1Char('u'));

    const FunctionObject *C = rx->speciesConstructor(scope, scope.engine->regExpCtor());
    if (!C)
        return Encode::undefined();

    Value *args = scope.alloc(2);
    args[0] = rx;
    args[1] = flags;
    ScopedObject splitter(scope, C->callAsConstructor(args, 2, f));
    if (scope.hasException())
        return Encode::undefined();

    ScopedArrayObject A(scope, scope.engine->newArrayObject());
    uint lengthA = 0;
    uint limit = argc < 2 ? UINT_MAX : argv[1].toUInt32();
    if (limit == 0)
        return A->asReturnedValue();

    QString S = s->toQString();
    int size = S.length();
    if (size == 0) {
        ScopedValue z(scope, exec(scope.engine, splitter, s));
        if (z->isNull())
            A->push_back(s);
        return A->asReturnedValue();
    }

    int p = 0;
    int q = 0;
    ScopedValue v(scope);
    ScopedValue z(scope);
    ScopedObject zz(scope);
    ScopedString t(scope);
    while (q < size) {
        Value qq = Value::fromInt32(q);
        if (!splitter->put(scope.engine->id_lastIndex(), qq))
            return scope.engine->throwTypeError();
        z = exec(scope.engine, splitter, s);
        if (scope.hasException())
            return Encode::undefined();

        if (z->isNull()) {
            q = advanceStringIndex(q, S, unicodeMatching);
            continue;
        }

        v = splitter->get(scope.engine->id_lastIndex());
        int e = qMin(v->toInt32(), size);
        if (e == p) {
            q = advanceStringIndex(q, S, unicodeMatching);
            continue;
        }
        QString T = S.mid(p, q - p);
        t = scope.engine->newString(T);
        A->push_back(t);
        ++lengthA;
        if (lengthA == limit)
            return A->asReturnedValue();
        p = e;
        zz = *z;
        uint numberOfCaptures = qMax(zz->getLength() - 1, 0ll);
        for (uint i = 1; i <= numberOfCaptures; ++i) {
            v = zz->get(PropertyKey::fromArrayIndex(i));
            A->push_back(v);
            ++lengthA;
            if (lengthA == limit)
                return A->asReturnedValue();
        }
        q = p;
    }

    QString T = S.mid(p);
    t = scope.engine->newString(T);
    A->push_back(t);
    return A->asReturnedValue();
}

ReturnedValue RegExpPrototype::method_get_sticky(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<RegExpObject> re(scope, thisObject);
    if (!re) {
        if (thisObject->sameValue(*scope.engine->regExpPrototype()))
            return Encode::undefined();
        return scope.engine->throwTypeError();
    }

    bool b = re->value()->flags & CompiledData::RegExp::RegExp_Sticky;
    return Encode(b);
}

ReturnedValue RegExpPrototype::method_test(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Value res = Value::fromReturnedValue(method_exec(b, thisObject, argv, argc));
    return Encode(!res.isNull());
}

ReturnedValue RegExpPrototype::method_toString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    const Object *r = thisObject->as<Object>();
    if (!r)
        return scope.engine->throwTypeError();

    ScopedValue v(scope);
    v = r->get(scope.engine->id_source());
    ScopedString source(scope, v->toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();
    v = r->get(scope.engine->id_flags());
    ScopedString flags(scope, v->toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();

    QString result = QLatin1Char('/') + source->toQString() + QLatin1Char('/') + flags->toQString();
    return Encode(scope.engine->newString(result));
}

ReturnedValue RegExpPrototype::method_get_unicode(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<RegExpObject> re(scope, thisObject);
    if (!re) {
        if (thisObject->sameValue(*scope.engine->regExpPrototype()))
            return Encode::undefined();
        return scope.engine->throwTypeError();
    }

    bool b = re->value()->flags & CompiledData::RegExp::RegExp_Unicode;
    return Encode(b);
}

ReturnedValue RegExpPrototype::method_compile(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<RegExpObject> r(scope, thisObject->as<RegExpObject>());
    if (!r)
        return scope.engine->throwTypeError();

    Scoped<RegExpObject> re(scope, scope.engine->regExpCtor()->callAsConstructor(argv, argc));

    r->d()->value.set(scope.engine, re->value());
    return Encode::undefined();
}

template <uint index>
ReturnedValue RegExpPrototype::method_get_lastMatch_n(const FunctionObject *b, const Value *, const Value *, int)
{
    Scope scope(b);
    ScopedArrayObject lastMatch(scope, static_cast<RegExpCtor*>(scope.engine->regExpCtor())->lastMatch());
    ScopedValue res(scope, lastMatch ? lastMatch->get(index) : Encode::undefined());
    if (res->isUndefined())
        res = scope.engine->newString();
    return res->asReturnedValue();
}

ReturnedValue RegExpPrototype::method_get_lastParen(const FunctionObject *b, const Value *, const Value *, int)
{
    Scope scope(b);
    ScopedArrayObject lastMatch(scope, static_cast<RegExpCtor*>(scope.engine->regExpCtor())->lastMatch());
    ScopedValue res(scope, lastMatch ? lastMatch->get(lastMatch->getLength() - 1) : Encode::undefined());
    if (res->isUndefined())
        res = scope.engine->newString();
    return res->asReturnedValue();
}

ReturnedValue RegExpPrototype::method_get_input(const FunctionObject *b, const Value *, const Value *, int)
{
    return static_cast<RegExpCtor*>(b->engine()->regExpCtor())->lastInput()->asReturnedValue();
}

ReturnedValue  RegExpPrototype::method_get_leftContext(const FunctionObject *b, const Value *, const Value *, int)
{
    Scope scope(b);
    Scoped<RegExpCtor> regExpCtor(scope, scope.engine->regExpCtor());
    QString lastInput = regExpCtor->lastInput()->toQString();
    return Encode(scope.engine->newString(lastInput.left(regExpCtor->lastMatchStart())));
}

ReturnedValue  RegExpPrototype::method_get_rightContext(const FunctionObject *b, const Value *, const Value *, int)
{
    Scope scope(b);
    Scoped<RegExpCtor> regExpCtor(scope, scope.engine->regExpCtor());
    QString lastInput = regExpCtor->lastInput()->toQString();
    return Encode(scope.engine->newString(lastInput.mid(regExpCtor->lastMatchEnd())));
}

QT_END_NAMESPACE
