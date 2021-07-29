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

#include "qv4numberobject_p.h"
#include "qv4runtime_p.h"
#include "qv4string_p.h"

#include <QtCore/qnumeric.h>
#include <QtCore/qmath.h>
#include <QtCore/QDebug>
#include <cassert>
#include <limits>

using namespace QV4;

DEFINE_OBJECT_VTABLE(NumberCtor);
DEFINE_OBJECT_VTABLE(NumberObject);

struct NumberLocaleHolder : public NumberLocale
{
    NumberLocaleHolder() {}
};

Q_GLOBAL_STATIC(NumberLocaleHolder, numberLocaleHolder)

NumberLocale::NumberLocale() : QLocale(QLocale::C),
    // -128 means shortest string that can accurately represent the number.
    defaultDoublePrecision(0xffffff80)
{
    setNumberOptions(QLocale::OmitGroupSeparator |
                     QLocale::OmitLeadingZeroInExponent |
                     QLocale::IncludeTrailingZeroesAfterDot);
}

const NumberLocale *NumberLocale::instance()
{
    return numberLocaleHolder();
}

void Heap::NumberCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Number"));
}

ReturnedValue NumberCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *argv, int argc, const Value *newTarget)
{
    auto v4 = f->engine();
    double dbl = argc ? argv[0].toNumber() : 0.;

    ReturnedValue o = Encode(f->engine()->newNumberObject(dbl));
    if (!newTarget)
        return o;
    Scope scope(v4);
    ScopedObject obj(scope, o);
    obj->setProtoFromNewTarget(newTarget);
    return obj->asReturnedValue();
}

ReturnedValue NumberCtor::virtualCall(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    double dbl = argc ? argv[0].toNumber() : 0.;
    return Encode(dbl);
}

void NumberPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedObject o(scope);
    ctor->defineReadonlyProperty(engine->id_prototype(), (o = this));
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(1));

    ctor->defineReadonlyProperty(QStringLiteral("NaN"), Value::fromDouble(qt_qnan()));
    ctor->defineReadonlyProperty(QStringLiteral("NEGATIVE_INFINITY"), Value::fromDouble(-qInf()));
    ctor->defineReadonlyProperty(QStringLiteral("POSITIVE_INFINITY"), Value::fromDouble(qInf()));
    ctor->defineReadonlyProperty(QStringLiteral("MAX_VALUE"), Value::fromDouble(1.7976931348623158e+308));
    ctor->defineReadonlyProperty(QStringLiteral("EPSILON"), Value::fromDouble(std::numeric_limits<double>::epsilon()));
    ctor->defineReadonlyProperty(QStringLiteral("MAX_SAFE_INTEGER"), Value::fromDouble(9007199254740991));
    ctor->defineReadonlyProperty(QStringLiteral("MIN_SAFE_INTEGER"), Value::fromDouble(-9007199254740991));

QT_WARNING_PUSH
QT_WARNING_DISABLE_INTEL(239)
    ctor->defineReadonlyProperty(QStringLiteral("MIN_VALUE"), Value::fromDouble(5e-324));
QT_WARNING_POP

    ctor->defineDefaultProperty(QStringLiteral("isFinite"), method_isFinite, 1);
    ctor->defineDefaultProperty(QStringLiteral("isInteger"), method_isInteger, 1);
    ctor->defineDefaultProperty(QStringLiteral("isSafeInteger"), method_isSafeInteger, 1);
    ctor->defineDefaultProperty(QStringLiteral("isNaN"), method_isNaN, 1);

    defineDefaultProperty(QStringLiteral("constructor"), (o = ctor));
    defineDefaultProperty(engine->id_toString(), method_toString, 1);
    defineDefaultProperty(engine->id_toLocaleString(), method_toLocaleString);
    defineDefaultProperty(engine->id_valueOf(), method_valueOf);
    defineDefaultProperty(QStringLiteral("toFixed"), method_toFixed, 1);
    defineDefaultProperty(QStringLiteral("toExponential"), method_toExponential, 1);
    defineDefaultProperty(QStringLiteral("toPrecision"), method_toPrecision, 1);
}

inline ReturnedValue thisNumberValue(ExecutionEngine *v4, const Value *thisObject)
{
    if (thisObject->isNumber())
        return thisObject->asReturnedValue();
    const NumberObject *n = thisObject->as<NumberObject>();
    if (!n) {
        v4->throwTypeError();
        return Encode::undefined();
    }
    return Encode(n->value());
}

inline double thisNumber(ExecutionEngine *engine, const Value *thisObject)
{
    if (thisObject->isNumber())
        return thisObject->asDouble();
    const NumberObject *n = thisObject->as<NumberObject>();
    if (!n) {
        engine->throwTypeError();
        return 0;
    }
    return n->value();
}

ReturnedValue NumberPrototype::method_isFinite(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (!argc || !argv[0].isNumber())
        return Encode(false);

    double v = argv[0].toNumber();
    return Encode(!std::isnan(v) && !qt_is_inf(v));
}

ReturnedValue NumberPrototype::method_isInteger(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (!argc)
        return Encode(false);

    const Value &v = argv[0];
    if (!v.isNumber())
        return Encode(false);

    double dv = v.toNumber();
    if (std::isnan(dv) || qt_is_inf(dv))
        return Encode(false);

    double iv = v.toInteger();
    return Encode(dv == iv);
}

ReturnedValue NumberPrototype::method_isSafeInteger(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (!argc)
        return Encode(false);

    const Value &v = argv[0];
    if (!v.isNumber())
        return Encode(false);

    double dv = v.toNumber();
    if (std::isnan(dv) || qt_is_inf(dv))
        return Encode(false);

    double iv = v.toInteger();
    return Encode(dv == iv && std::fabs(iv) <= (1LL << 53) - 1);
}

ReturnedValue NumberPrototype::method_isNaN(const FunctionObject *, const Value *, const Value *argv, int argc)
{
    if (!argc || !argv[0].isNumber())
        return Encode(false);

    double v = argv[0].toNumber();
    // cast to bool explicitly as std::isnan() may give us ::isnan(), which
    // sometimes returns an int and we don't want the Encode(int) overload.
    return Encode(bool(std::isnan(v)));
}

ReturnedValue NumberPrototype::method_toString(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    double num = thisNumber(v4, thisObject);
    if (v4->hasException)
        return QV4::Encode::undefined();

    if (argc && !argv[0].isUndefined()) {
        int radix = argv[0].toInt32();
        if (radix < 2 || radix > 36) {
            return v4->throwError(QStringLiteral("Number.prototype.toString: %0 is not a valid radix").arg(radix));
        }

        QString str;
        RuntimeHelpers::numberToString(&str, num, radix);
        return Encode(v4->newString(str));
    }

    return Encode(Value::fromDouble(num).toString(v4));
}

ReturnedValue NumberPrototype::method_toLocaleString(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    ScopedValue v(scope, thisNumberValue(b->engine(), thisObject));
    return Encode(v->toString(scope.engine));
}

ReturnedValue NumberPrototype::method_valueOf(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    return thisNumberValue(b->engine(), thisObject);
}

ReturnedValue NumberPrototype::method_toFixed(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    double v = thisNumber(v4, thisObject);
    if (v4->hasException)
        return QV4::Encode::undefined();

    double fdigits = 0;

    if (argc > 0)
        fdigits = argv[0].toInteger();

    if (std::isnan(fdigits))
        fdigits = 0;

    if (fdigits < 0 || fdigits > 100)
        return v4->throwRangeError(*thisObject);

    QString str;
    if (std::isnan(v))
        str = QStringLiteral("NaN");
    else if (qt_is_inf(v))
        str = QString::fromLatin1(v < 0 ? "-Infinity" : "Infinity");
    else if (v < 1.e21)
        str = NumberLocale::instance()->toString(v, 'f', int(fdigits));
    else {
        return Encode(RuntimeHelpers::stringFromNumber(v4, v));
    }
    return Encode(v4->newString(str));
}

ReturnedValue NumberPrototype::method_toExponential(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    ExecutionEngine *v4 = b->engine();
    double d = thisNumber(v4, thisObject);
    if (v4->hasException)
        return QV4::Encode::undefined();

    bool defaultDigits = !argc || argv[0].isUndefined();
    int fdigits = !defaultDigits ? argv[0].toInteger() : NumberLocale::instance()->defaultDoublePrecision;
    if (v4->hasException)
        return QV4::Encode::undefined();

    if (std::isnan(d))
        return Encode(v4->newString(QLatin1String("NaN")));

    if (qIsInf(d))
        return Encode(v4->newString(QLatin1String(d < 0 ? "-Infinity" : "Infinity")));

    if (!defaultDigits && (fdigits < 0 || fdigits > 100)) {
        Scope scope(v4);
        ScopedString error(scope, v4->newString(QStringLiteral("Number.prototype.toExponential: fractionDigits out of range")));
        return v4->throwRangeError(error);
    }

    QString result = NumberLocale::instance()->toString(d, 'e', fdigits);
    return Encode(v4->newString(result));
}

ReturnedValue NumberPrototype::method_toPrecision(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    ScopedValue v(scope, thisNumberValue(scope.engine, thisObject));
    if (scope.engine->hasException)
        return QV4::Encode::undefined();
    double d = v->asDouble();

    if (!argc || argv[0].isUndefined())
        return Encode(v->toString(scope.engine));

    int precision = argv[0].toInt32();
    if (scope.engine->hasException)
        return QV4::Encode::undefined();

    if (std::isnan(d))
        return Encode(scope.engine->newString(QLatin1String("NaN")));

    if (qIsInf(d))
        return Encode(scope.engine->newString(QLatin1String(d < 0 ? "-Infinity" : "Infinity")));

    if (precision < 1 || precision > 100) {
        ScopedString error(scope, scope.engine->newString(QStringLiteral("Number.prototype.toPrecision: precision out of range")));
        return scope.engine->throwRangeError(error);
    }

    QString result = NumberLocale::instance()->toString(d, 'g', precision);
    return Encode(scope.engine->newString(result));
}
