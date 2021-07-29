/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <qv4symbol_p.h>
#include <qv4functionobject_p.h>
#include <qv4identifiertable_p.h>

using namespace QV4;

DEFINE_OBJECT_VTABLE(SymbolCtor);
DEFINE_MANAGED_VTABLE(Symbol);
DEFINE_OBJECT_VTABLE(SymbolObject);

void Heap::Symbol::init(const QString &s)
{
    Q_ASSERT(s.at(0) == QLatin1Char('@'));
    identifier = PropertyKey::fromStringOrSymbol(this);
    QString desc(s);
    text = desc.data_ptr();
    text->ref.ref();
}

void Heap::SymbolCtor::init(QV4::ExecutionContext *scope)
{
    Heap::FunctionObject::init(scope, QStringLiteral("Symbol"));
}

void Heap::SymbolObject::init(const QV4::Symbol *s)
{
    Object::init();
    symbol.set(internalClass->engine, s->d());
}

ReturnedValue QV4::SymbolCtor::virtualCall(const QV4::FunctionObject *f, const QV4::Value *, const QV4::Value *argv, int argc)
{
    Scope scope(f);
    QString desc = QChar::fromLatin1('@');
    if (argc && !argv[0].isUndefined()) {
        ScopedString s(scope, argv[0].toString(scope.engine));
        if (scope.hasException())
            return Encode::undefined();
        desc += s->toQString();
    }
    return Symbol::create(scope.engine, desc)->asReturnedValue();
}

ReturnedValue SymbolCtor::virtualCallAsConstructor(const FunctionObject *f, const Value *, int, const Value *)
{
    return f->engine()->throwTypeError(QStringLiteral("Symbol can't be used together with |new|."));
}

ReturnedValue SymbolCtor::method_for(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    Scope scope(f);
    ScopedValue k(scope, argc ? argv[0]: Value::undefinedValue());
    ScopedString key(scope, k->toString(scope.engine));
    if (scope.hasException())
        return Encode::undefined();
    QString desc = QLatin1Char('@') + key->toQString();
    return scope.engine->identifierTable->insertSymbol(desc)->asReturnedValue();
}

ReturnedValue SymbolCtor::method_keyFor(const FunctionObject *f, const Value *, const Value *argv, int argc)
{
    ExecutionEngine *e = f->engine();
    if (!argc || !argv[0].isSymbol())
        return e->throwTypeError(QLatin1String("Symbol.keyFor: Argument is not a symbol."));
    const Symbol &arg = static_cast<const Symbol &>(argv[0]);
    Heap::Symbol *s = e->identifierTable->symbolForId(arg.propertyKey());
    Q_ASSERT(!s || s == arg.d());
    if (s)
        return e->newString(arg.toQString().mid((1)))->asReturnedValue();
    return Encode::undefined();
}

void SymbolPrototype::init(ExecutionEngine *engine, Object *ctor)
{
    Scope scope(engine);
    ScopedValue v(scope);
    ctor->defineReadonlyProperty(engine->id_prototype(), (v = this));
    ctor->defineReadonlyConfigurableProperty(engine->id_length(), Value::fromInt32(0));

    ctor->defineDefaultProperty(QStringLiteral("for"), SymbolCtor::method_for, 1);
    ctor->defineDefaultProperty(QStringLiteral("keyFor"), SymbolCtor::method_keyFor, 1);
    ctor->defineReadonlyProperty(QStringLiteral("hasInstance"), *engine->symbol_hasInstance());
    ctor->defineReadonlyProperty(QStringLiteral("isConcatSpreadable"), *engine->symbol_isConcatSpreadable());
    ctor->defineReadonlyProperty(QStringLiteral("iterator"), *engine->symbol_iterator());
    ctor->defineReadonlyProperty(QStringLiteral("match"), *engine->symbol_match());
    ctor->defineReadonlyProperty(QStringLiteral("replace"), *engine->symbol_replace());
    ctor->defineReadonlyProperty(QStringLiteral("search"), *engine->symbol_search());
    ctor->defineReadonlyProperty(QStringLiteral("species"), *engine->symbol_species());
    ctor->defineReadonlyProperty(QStringLiteral("split"), *engine->symbol_split());
    ctor->defineReadonlyProperty(QStringLiteral("toPrimitive"), *engine->symbol_toPrimitive());
    ctor->defineReadonlyProperty(QStringLiteral("toStringTag"), *engine->symbol_toStringTag());
    ctor->defineReadonlyProperty(QStringLiteral("unscopables"), *engine->symbol_unscopables());

    defineDefaultProperty(QStringLiteral("constructor"), (v = ctor));
    defineDefaultProperty(QStringLiteral("toString"), method_toString);
    defineDefaultProperty(QStringLiteral("valueOf"), method_valueOf);
    defineDefaultProperty(engine->symbol_toPrimitive(), method_symbolToPrimitive, 1, Attr_ReadOnly_ButConfigurable);

    v = engine->newString(QStringLiteral("Symbol"));
    defineReadonlyConfigurableProperty(engine->symbol_toStringTag(), v);

}

ReturnedValue SymbolPrototype::method_toString(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<Symbol> s(scope, thisObject->as<Symbol>());
    if (!s) {
        if (const SymbolObject *o = thisObject->as<SymbolObject>())
            s = o->d()->symbol;
        else
            return scope.engine->throwTypeError();
    }
    return scope.engine->newString(s->descriptiveString())->asReturnedValue();
}

ReturnedValue SymbolPrototype::method_valueOf(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    Scope scope(f);
    Scoped<Symbol> s(scope, thisObject->as<Symbol>());
    if (!s) {
        if (const SymbolObject *o = thisObject->as<SymbolObject>())
            s = o->d()->symbol;
        else
            return scope.engine->throwTypeError();
    }
    return s->asReturnedValue();
}

ReturnedValue SymbolPrototype::method_symbolToPrimitive(const FunctionObject *f, const Value *thisObject, const Value *, int)
{
    if (thisObject->isSymbol())
        return thisObject->asReturnedValue();
    if (const SymbolObject *o = thisObject->as<SymbolObject>())
        return o->d()->symbol->asReturnedValue();
    return f->engine()->throwTypeError();
}

Heap::Symbol *Symbol::create(ExecutionEngine *e, const QString &s)
{
    Q_ASSERT(s.at(0) == QLatin1Char('@'));
    return e->memoryManager->alloc<Symbol>(s);
}

QString Symbol::descriptiveString() const
{
    return QLatin1String("Symbol(") + toQString().midRef(1) + QLatin1String(")");
}
