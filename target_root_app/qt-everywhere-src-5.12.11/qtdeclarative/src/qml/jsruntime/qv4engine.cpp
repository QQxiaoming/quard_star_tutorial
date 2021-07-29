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
#include <qv4engine_p.h>

#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qqmljsast_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qv4compiler_p.h>
#include <private/qv4compilercontext_p.h>
#include <private/qv4codegen_p.h>

#include <QtCore/QTextStream>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#ifndef V4_BOOTSTRAP

#include <qv4qmlcontext_p.h>
#include <qv4value_p.h>
#include <qv4object_p.h>
#include <qv4objectproto_p.h>
#include <qv4objectiterator_p.h>
#include <qv4setiterator_p.h>
#include <qv4mapiterator_p.h>
#include <qv4arrayiterator_p.h>
#include <qv4arrayobject_p.h>
#include <qv4booleanobject_p.h>
#include <qv4globalobject_p.h>
#include <qv4errorobject_p.h>
#include <qv4functionobject_p.h>
#include "qv4function_p.h"
#include <qv4mathobject_p.h>
#include <qv4numberobject_p.h>
#include <qv4regexpobject_p.h>
#include <qv4regexp_p.h>
#include "qv4symbol_p.h"
#include "qv4setobject_p.h"
#include "qv4mapobject_p.h"
#include <qv4variantobject_p.h>
#include <qv4runtime_p.h>
#include <private/qv4mm_p.h>
#include <qv4argumentsobject_p.h>
#include <qv4dateobject_p.h>
#include <qv4jsonobject_p.h>
#include <qv4stringobject_p.h>
#include <qv4identifiertable_p.h>
#include "qv4debugging_p.h"
#include "qv4profiling_p.h"
#include "qv4executableallocator_p.h"
#include "qv4iterator_p.h"
#include "qv4stringiterator_p.h"
#include "qv4generatorobject_p.h"
#include "qv4reflect_p.h"
#include "qv4proxy_p.h"
#include "qv4stackframe_p.h"
#include "qv4atomics_p.h"

#if QT_CONFIG(qml_sequence_object)
#include "qv4sequenceobject_p.h"
#endif

#include "qv4qobjectwrapper_p.h"
#include "qv4memberdata_p.h"
#include "qv4arraybuffer_p.h"
#include "qv4dataview_p.h"
#include "qv4promiseobject_p.h"
#include "qv4typedarray_p.h"
#include <private/qv8engine_p.h>
#include <private/qjsvalue_p.h>
#include <private/qqmltypewrapper_p.h>
#include <private/qqmlvaluetypewrapper_p.h>
#include <private/qqmlvaluetype_p.h>
#include <private/qqmllistwrapper_p.h>
#include <private/qqmllist_p.h>
#include <private/qqmltypeloader_p.h>
#if QT_CONFIG(qml_locale)
#include <private/qqmllocale_p.h>
#endif
#include <qqmlfile.h>

#if USE(PTHREADS)
#  include <pthread.h>
#if !defined(Q_OS_INTEGRITY)
#  include <sys/resource.h>
#endif
#if HAVE(PTHREAD_NP_H)
#  include <pthread_np.h>
#endif
#endif

#ifdef V4_USE_VALGRIND
#include <valgrind/memcheck.h>
#endif

#endif // #ifndef V4_BOOTSTRAP

QT_BEGIN_NAMESPACE

using namespace QV4;

#ifndef V4_BOOTSTRAP

static QBasicAtomicInt engineSerial = Q_BASIC_ATOMIC_INITIALIZER(1);

ReturnedValue throwTypeError(const FunctionObject *b, const QV4::Value *, const QV4::Value *, int)
{
    return b->engine()->throwTypeError();
}

qint32 ExecutionEngine::maxCallDepth = -1;

ExecutionEngine::ExecutionEngine(QJSEngine *jsEngine)
    : executableAllocator(new QV4::ExecutableAllocator)
    , regExpAllocator(new QV4::ExecutableAllocator)
    , bumperPointerAllocator(new WTF::BumpPointerAllocator)
    , jsStack(new WTF::PageAllocation)
    , gcStack(new WTF::PageAllocation)
    , globalCode(nullptr)
    , v8Engine(nullptr)
    , publicEngine(jsEngine)
    , m_engineId(engineSerial.fetchAndAddOrdered(1))
    , regExpCache(nullptr)
    , m_multiplyWrappedQObjects(nullptr)
#if defined(V4_ENABLE_JIT) && !defined(V4_BOOTSTRAP)
    , m_canAllocateExecutableMemory(OSAllocator::canAllocateExecutableMemory())
#endif
{
    memoryManager = new QV4::MemoryManager(this);

    if (maxCallDepth == -1) {
        bool ok = false;
        maxCallDepth = qEnvironmentVariableIntValue("QV4_MAX_CALL_DEPTH", &ok);
        if (!ok || maxCallDepth <= 0) {
#if defined(QT_NO_DEBUG) && !defined(__SANITIZE_ADDRESS__) && !QT_HAS_FEATURE(address_sanitizer)
            maxCallDepth = 1234;
#else
            // no (tail call) optimization is done, so there'll be a lot mare stack frames active
            maxCallDepth = 200;
#endif
        }
    }
    Q_ASSERT(maxCallDepth > 0);

    // reserve space for the JS stack
    // we allow it to grow to a bit more than JSStackLimit, as we can overshoot due to ScopedValues
    // allocated outside of JIT'ed methods.
    *jsStack = WTF::PageAllocation::allocate(JSStackLimit + 256*1024, WTF::OSAllocator::JSVMStackPages,
                                             /* writable */ true, /* executable */ false,
                                             /* includesGuardPages */ true);
    jsStackBase = (Value *)jsStack->base();
#ifdef V4_USE_VALGRIND
    VALGRIND_MAKE_MEM_UNDEFINED(jsStackBase, JSStackLimit + 256*1024);
#endif

    jsStackTop = jsStackBase;

    *gcStack = WTF::PageAllocation::allocate(GCStackLimit, WTF::OSAllocator::JSVMStackPages,
                                             /* writable */ true, /* executable */ false,
                                             /* includesGuardPages */ true);

    {
        bool ok = false;
        jitCallCountThreshold = qEnvironmentVariableIntValue("QV4_JIT_CALL_THRESHOLD", &ok);
        if (!ok)
            jitCallCountThreshold = 3;
        if (qEnvironmentVariableIsSet("QV4_FORCE_INTERPRETER"))
            jitCallCountThreshold = std::numeric_limits<int>::max();
    }

    exceptionValue = jsAlloca(1);
    *exceptionValue = Encode::undefined();
    globalObject = static_cast<Object *>(jsAlloca(1));
    jsObjects = jsAlloca(NJSObjects);
    typedArrayPrototype = static_cast<Object *>(jsAlloca(NTypedArrayTypes));
    typedArrayCtors = static_cast<FunctionObject *>(jsAlloca(NTypedArrayTypes));
    jsStrings = jsAlloca(NJSStrings);
    jsSymbols = jsAlloca(NJSSymbols);

    // set up stack limits
    jsStackLimit = jsStackBase + JSStackLimit/sizeof(Value);

    identifierTable = new IdentifierTable(this);

    memset(classes, 0, sizeof(classes));
    classes[Class_Empty] = memoryManager->allocIC<InternalClass>();
    classes[Class_Empty]->init(this);

    classes[Class_MemberData] = classes[Class_Empty]->changeVTable(QV4::MemberData::staticVTable());
    classes[Class_SimpleArrayData] = classes[Class_Empty]->changeVTable(QV4::SimpleArrayData::staticVTable());
    classes[Class_SparseArrayData] = classes[Class_Empty]->changeVTable(QV4::SparseArrayData::staticVTable());
    classes[Class_ExecutionContext] = classes[Class_Empty]->changeVTable(QV4::ExecutionContext::staticVTable());
    classes[Class_CallContext] = classes[Class_Empty]->changeVTable(QV4::CallContext::staticVTable());
    classes[Class_QmlContext] = classes[Class_Empty]->changeVTable(QV4::QmlContext::staticVTable());

    Scope scope(this);
    Scoped<InternalClass> ic(scope);
    ic = classes[Class_Empty]->changeVTable(QV4::Object::staticVTable());
    jsObjects[ObjectProto] = memoryManager->allocObject<ObjectPrototype>(ic->d());
    classes[Class_Object] = ic->changePrototype(objectPrototype()->d());
    classes[Class_QmlContextWrapper] = classes[Class_Object]->changeVTable(QV4::QQmlContextWrapper::staticVTable());

    ic = newInternalClass(QV4::StringObject::staticVTable(), objectPrototype());
    jsObjects[StringProto] = memoryManager->allocObject<StringPrototype>(ic->d(), /*init =*/ false);
    classes[Class_String] = classes[Class_Empty]->changeVTable(QV4::String::staticVTable())->changePrototype(stringPrototype()->d());
    Q_ASSERT(stringPrototype()->d() && classes[Class_String]->prototype);

    jsObjects[SymbolProto] = memoryManager->allocate<SymbolPrototype>();
    classes[Class_Symbol] = classes[EngineBase::Class_Empty]->changeVTable(QV4::Symbol::staticVTable())->changePrototype(symbolPrototype()->d());

    jsStrings[String_Empty] = newIdentifier(QString());
    jsStrings[String_undefined] = newIdentifier(QStringLiteral("undefined"));
    jsStrings[String_null] = newIdentifier(QStringLiteral("null"));
    jsStrings[String_true] = newIdentifier(QStringLiteral("true"));
    jsStrings[String_false] = newIdentifier(QStringLiteral("false"));
    jsStrings[String_boolean] = newIdentifier(QStringLiteral("boolean"));
    jsStrings[String_number] = newIdentifier(QStringLiteral("number"));
    jsStrings[String_string] = newIdentifier(QStringLiteral("string"));
    jsStrings[String_default] = newIdentifier(QStringLiteral("default"));
    jsStrings[String_symbol] = newIdentifier(QStringLiteral("symbol"));
    jsStrings[String_object] = newIdentifier(QStringLiteral("object"));
    jsStrings[String_function] = newIdentifier(QStringLiteral("function"));
    jsStrings[String_length] = newIdentifier(QStringLiteral("length"));
    jsStrings[String_prototype] = newIdentifier(QStringLiteral("prototype"));
    jsStrings[String_constructor] = newIdentifier(QStringLiteral("constructor"));
    jsStrings[String_arguments] = newIdentifier(QStringLiteral("arguments"));
    jsStrings[String_caller] = newIdentifier(QStringLiteral("caller"));
    jsStrings[String_callee] = newIdentifier(QStringLiteral("callee"));
    jsStrings[String_this] = newIdentifier(QStringLiteral("this"));
    jsStrings[String___proto__] = newIdentifier(QStringLiteral("__proto__"));
    jsStrings[String_enumerable] = newIdentifier(QStringLiteral("enumerable"));
    jsStrings[String_configurable] = newIdentifier(QStringLiteral("configurable"));
    jsStrings[String_writable] = newIdentifier(QStringLiteral("writable"));
    jsStrings[String_value] = newIdentifier(QStringLiteral("value"));
    jsStrings[String_get] = newIdentifier(QStringLiteral("get"));
    jsStrings[String_set] = newIdentifier(QStringLiteral("set"));
    jsStrings[String_eval] = newIdentifier(QStringLiteral("eval"));
    jsStrings[String_uintMax] = newIdentifier(QStringLiteral("4294967295"));
    jsStrings[String_name] = newIdentifier(QStringLiteral("name"));
    jsStrings[String_index] = newIdentifier(QStringLiteral("index"));
    jsStrings[String_input] = newIdentifier(QStringLiteral("input"));
    jsStrings[String_toString] = newIdentifier(QStringLiteral("toString"));
    jsStrings[String_toLocaleString] = newIdentifier(QStringLiteral("toLocaleString"));
    jsStrings[String_destroy] = newIdentifier(QStringLiteral("destroy"));
    jsStrings[String_valueOf] = newIdentifier(QStringLiteral("valueOf"));
    jsStrings[String_byteLength] = newIdentifier(QStringLiteral("byteLength"));
    jsStrings[String_byteOffset] = newIdentifier(QStringLiteral("byteOffset"));
    jsStrings[String_buffer] = newIdentifier(QStringLiteral("buffer"));
    jsStrings[String_lastIndex] = newIdentifier(QStringLiteral("lastIndex"));
    jsStrings[String_next] = newIdentifier(QStringLiteral("next"));
    jsStrings[String_done] = newIdentifier(QStringLiteral("done"));
    jsStrings[String_return] = newIdentifier(QStringLiteral("return"));
    jsStrings[String_throw] = newIdentifier(QStringLiteral("throw"));
    jsStrings[String_global] = newIdentifier(QStringLiteral("global"));
    jsStrings[String_ignoreCase] = newIdentifier(QStringLiteral("ignoreCase"));
    jsStrings[String_multiline] = newIdentifier(QStringLiteral("multiline"));
    jsStrings[String_unicode] = newIdentifier(QStringLiteral("unicode"));
    jsStrings[String_sticky] = newIdentifier(QStringLiteral("sticky"));
    jsStrings[String_source] = newIdentifier(QStringLiteral("source"));
    jsStrings[String_flags] = newIdentifier(QStringLiteral("flags"));

    jsSymbols[Symbol_hasInstance] = Symbol::create(this, QStringLiteral("@Symbol.hasInstance"));
    jsSymbols[Symbol_isConcatSpreadable] = Symbol::create(this, QStringLiteral("@Symbol.isConcatSpreadable"));
    jsSymbols[Symbol_iterator] = Symbol::create(this, QStringLiteral("@Symbol.iterator"));
    jsSymbols[Symbol_match] = Symbol::create(this, QStringLiteral("@Symbol.match"));
    jsSymbols[Symbol_replace] = Symbol::create(this, QStringLiteral("@Symbol.replace"));
    jsSymbols[Symbol_search] = Symbol::create(this, QStringLiteral("@Symbol.search"));
    jsSymbols[Symbol_species] = Symbol::create(this, QStringLiteral("@Symbol.species"));
    jsSymbols[Symbol_split] = Symbol::create(this, QStringLiteral("@Symbol.split"));
    jsSymbols[Symbol_toPrimitive] = Symbol::create(this, QStringLiteral("@Symbol.toPrimitive"));
    jsSymbols[Symbol_toStringTag] = Symbol::create(this, QStringLiteral("@Symbol.toStringTag"));
    jsSymbols[Symbol_unscopables] = Symbol::create(this, QStringLiteral("@Symbol.unscopables"));
    jsSymbols[Symbol_revokableProxy] = Symbol::create(this, QStringLiteral("@Proxy.revokableProxy"));

    ic = newInternalClass(ArrayPrototype::staticVTable(), objectPrototype());
    Q_ASSERT(ic->d()->prototype);
    ic = ic->addMember(id_length()->propertyKey(), Attr_NotConfigurable|Attr_NotEnumerable);
    Q_ASSERT(ic->d()->prototype);
    jsObjects[ArrayProto] = memoryManager->allocObject<ArrayPrototype>(ic->d());
    classes[Class_ArrayObject] = ic->changePrototype(arrayPrototype()->d());
    jsObjects[PropertyListProto] = memoryManager->allocate<PropertyListPrototype>();

    Scoped<InternalClass> argsClass(scope);
    argsClass = newInternalClass(ArgumentsObject::staticVTable(), objectPrototype());
    argsClass = argsClass->addMember(id_length()->propertyKey(), Attr_NotEnumerable);
    argsClass = argsClass->addMember(symbol_iterator()->propertyKey(), Attr_Data|Attr_NotEnumerable);
    classes[Class_ArgumentsObject] = argsClass->addMember(id_callee()->propertyKey(), Attr_Data|Attr_NotEnumerable);
    argsClass = newInternalClass(StrictArgumentsObject::staticVTable(), objectPrototype());
    argsClass = argsClass->addMember(id_length()->propertyKey(), Attr_NotEnumerable);
    argsClass = argsClass->addMember(symbol_iterator()->propertyKey(), Attr_Data|Attr_NotEnumerable);
    classes[Class_StrictArgumentsObject] = argsClass->addMember(id_callee()->propertyKey(), Attr_Accessor|Attr_NotConfigurable|Attr_NotEnumerable);

    *static_cast<Value *>(globalObject) = newObject();
    Q_ASSERT(globalObject->d()->vtable());
    initRootContext();

    ic = newInternalClass(QV4::StringObject::staticVTable(), objectPrototype());
    ic = ic->addMember(id_length()->propertyKey(), Attr_ReadOnly);
    classes[Class_StringObject] = ic->changePrototype(stringPrototype()->d());
    Q_ASSERT(classes[Class_StringObject]->verifyIndex(id_length()->propertyKey(), Heap::StringObject::LengthPropertyIndex));

    classes[Class_SymbolObject] = newInternalClass(QV4::SymbolObject::staticVTable(), symbolPrototype());

    jsObjects[NumberProto] = memoryManager->allocate<NumberPrototype>();
    jsObjects[BooleanProto] = memoryManager->allocate<BooleanPrototype>();
    jsObjects[DateProto] = memoryManager->allocate<DatePrototype>();

#if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
    InternalClassEntry *index = nullptr;
#else
    InternalClassEntry _index;
    auto *index = &_index;
#endif
    ic = newInternalClass(QV4::FunctionPrototype::staticVTable(), objectPrototype());
    auto addProtoHasInstance = [&] {
        // Add an invalid prototype slot, so that all function objects have the same layout
        // This helps speed up instanceof operations and other things where we need to query
        // prototype property (as we always know it's location)
        ic = ic->addMember(id_prototype()->propertyKey(), Attr_Invalid, index);
        Q_ASSERT(index->index == Heap::FunctionObject::Index_Prototype);
        // add an invalid @hasInstance slot, so that we can quickly track whether the
        // hasInstance method has been reimplemented. This is required for a fast
        // instanceof implementation
        ic = ic->addMember(symbol_hasInstance()->propertyKey(), Attr_Invalid, index);
        Q_ASSERT(index->index == Heap::FunctionObject::Index_HasInstance);
    };
    addProtoHasInstance();
    jsObjects[FunctionProto] = memoryManager->allocObject<FunctionPrototype>(ic->d());
    ic = newInternalClass(FunctionObject::staticVTable(), functionPrototype());
    addProtoHasInstance();
    classes[Class_FunctionObject] = ic->d();
    ic = ic->addMember(id_name()->propertyKey(), Attr_ReadOnly, index);
    Q_ASSERT(index->index == Heap::ArrowFunction::Index_Name);
    ic = ic->addMember(id_length()->propertyKey(), Attr_ReadOnly_ButConfigurable, index);
    Q_ASSERT(index->index == Heap::ArrowFunction::Index_Length);
    classes[Class_ArrowFunction] = ic->changeVTable(ArrowFunction::staticVTable());
    ic = ic->changeVTable(MemberFunction::staticVTable());
    classes[Class_MemberFunction] = ic->d();
    ic = ic->changeVTable(GeneratorFunction::staticVTable());
    classes[Class_GeneratorFunction] = ic->d();
    ic = ic->changeVTable(MemberGeneratorFunction::staticVTable());
    classes[Class_MemberGeneratorFunction] = ic->d();

    ic = ic->changeMember(id_prototype()->propertyKey(), Attr_NotConfigurable|Attr_NotEnumerable);
    ic = ic->changeVTable(ScriptFunction::staticVTable());
    classes[Class_ScriptFunction] = ic->d();
    ic = ic->changeVTable(ConstructorFunction::staticVTable());
    classes[Class_ConstructorFunction] = ic->d();

    classes[Class_ObjectProto] = classes[Class_Object]->addMember(id_constructor()->propertyKey(), Attr_NotEnumerable, index);
    Q_ASSERT(index->index == Heap::FunctionObject::Index_ProtoConstructor);

    jsObjects[GeneratorProto] = memoryManager->allocObject<GeneratorPrototype>(classes[Class_Object]);
    classes[Class_GeneratorObject] = newInternalClass(QV4::GeneratorObject::staticVTable(), generatorPrototype());

    ScopedString str(scope);
    classes[Class_RegExp] = classes[Class_Empty]->changeVTable(QV4::RegExp::staticVTable());
    ic = newInternalClass(QV4::RegExpObject::staticVTable(), objectPrototype());
    ic = ic->addMember(id_lastIndex()->propertyKey(), Attr_NotEnumerable|Attr_NotConfigurable, index);
    Q_ASSERT(index->index == RegExpObject::Index_LastIndex);
    jsObjects[RegExpProto] = memoryManager->allocObject<RegExpPrototype>(classes[Class_Object]);
    classes[Class_RegExpObject] = ic->changePrototype(regExpPrototype()->d());

    ic = classes[Class_ArrayObject]->addMember(id_index()->propertyKey(), Attr_Data, index);
    Q_ASSERT(index->index == RegExpObject::Index_ArrayIndex);
    classes[Class_RegExpExecArray] = ic->addMember(id_input()->propertyKey(), Attr_Data, index);
    Q_ASSERT(index->index == RegExpObject::Index_ArrayInput);

    ic = newInternalClass(ErrorObject::staticVTable(), nullptr);
    ic = ic->addMember((str = newIdentifier(QStringLiteral("stack")))->propertyKey(), Attr_Accessor|Attr_NotConfigurable|Attr_NotEnumerable, index);
    Q_ASSERT(index->index == ErrorObject::Index_Stack);
    Q_ASSERT(index->setterIndex == ErrorObject::Index_StackSetter);
    ic = ic->addMember((str = newIdentifier(QStringLiteral("fileName")))->propertyKey(), Attr_Data|Attr_NotEnumerable, index);
    Q_ASSERT(index->index == ErrorObject::Index_FileName);
    ic = ic->addMember((str = newIdentifier(QStringLiteral("lineNumber")))->propertyKey(), Attr_Data|Attr_NotEnumerable, index);
    classes[Class_ErrorObject] = ic->d();
    Q_ASSERT(index->index == ErrorObject::Index_LineNumber);
    classes[Class_ErrorObjectWithMessage] = ic->addMember((str = newIdentifier(QStringLiteral("message")))->propertyKey(), Attr_Data|Attr_NotEnumerable, index);
    Q_ASSERT(index->index == ErrorObject::Index_Message);
    ic = newInternalClass(Object::staticVTable(), objectPrototype());
    ic = ic->addMember(id_constructor()->propertyKey(), Attr_Data|Attr_NotEnumerable, index);
    Q_ASSERT(index->index == ErrorPrototype::Index_Constructor);
    ic = ic->addMember((str = newIdentifier(QStringLiteral("message")))->propertyKey(), Attr_Data|Attr_NotEnumerable, index);
    Q_ASSERT(index->index == ErrorPrototype::Index_Message);
    classes[Class_ErrorProto] = ic->addMember(id_name()->propertyKey(), Attr_Data|Attr_NotEnumerable, index);
    Q_ASSERT(index->index == ErrorPrototype::Index_Name);

    classes[Class_ProxyObject] = classes[Class_Empty]->changeVTable(ProxyObject::staticVTable());
    classes[Class_ProxyFunctionObject] = classes[Class_Empty]->changeVTable(ProxyFunctionObject::staticVTable());

    jsObjects[GetStack_Function] = FunctionObject::createBuiltinFunction(this, str = newIdentifier(QStringLiteral("stack")), ErrorObject::method_get_stack, 0);

    jsObjects[ErrorProto] = memoryManager->allocObject<ErrorPrototype>(classes[Class_ErrorProto]);
    ic = classes[Class_ErrorProto]->changePrototype(errorPrototype()->d());
    jsObjects[EvalErrorProto] = memoryManager->allocObject<EvalErrorPrototype>(ic->d());
    jsObjects[RangeErrorProto] = memoryManager->allocObject<RangeErrorPrototype>(ic->d());
    jsObjects[ReferenceErrorProto] = memoryManager->allocObject<ReferenceErrorPrototype>(ic->d());
    jsObjects[SyntaxErrorProto] = memoryManager->allocObject<SyntaxErrorPrototype>(ic->d());
    jsObjects[TypeErrorProto] = memoryManager->allocObject<TypeErrorPrototype>(ic->d());
    jsObjects[URIErrorProto] = memoryManager->allocObject<URIErrorPrototype>(ic->d());

    jsObjects[VariantProto] = memoryManager->allocate<VariantPrototype>();
    Q_ASSERT(variantPrototype()->getPrototypeOf() == objectPrototype()->d());

#if QT_CONFIG(qml_sequence_object)
    ic = newInternalClass(SequencePrototype::staticVTable(), SequencePrototype::defaultPrototype(this));
    jsObjects[SequenceProto] = ScopedValue(scope, memoryManager->allocObject<SequencePrototype>(ic->d()));
#endif

    ExecutionContext *global = rootContext();

    jsObjects[Object_Ctor] = memoryManager->allocate<ObjectCtor>(global);
    jsObjects[String_Ctor] = memoryManager->allocate<StringCtor>(global);
    jsObjects[Symbol_Ctor] = memoryManager->allocate<SymbolCtor>(global);
    jsObjects[Number_Ctor] = memoryManager->allocate<NumberCtor>(global);
    jsObjects[Boolean_Ctor] = memoryManager->allocate<BooleanCtor>(global);
    jsObjects[Array_Ctor] = memoryManager->allocate<ArrayCtor>(global);
    jsObjects[Function_Ctor] = memoryManager->allocate<FunctionCtor>(global);
    jsObjects[GeneratorFunction_Ctor] = memoryManager->allocate<GeneratorFunctionCtor>(global);
    jsObjects[Date_Ctor] = memoryManager->allocate<DateCtor>(global);
    jsObjects[RegExp_Ctor] = memoryManager->allocate<RegExpCtor>(global);
    jsObjects[Error_Ctor] = memoryManager->allocate<ErrorCtor>(global);
    jsObjects[EvalError_Ctor] = memoryManager->allocate<EvalErrorCtor>(global);
    jsObjects[RangeError_Ctor] = memoryManager->allocate<RangeErrorCtor>(global);
    jsObjects[ReferenceError_Ctor] = memoryManager->allocate<ReferenceErrorCtor>(global);
    jsObjects[SyntaxError_Ctor] = memoryManager->allocate<SyntaxErrorCtor>(global);
    jsObjects[TypeError_Ctor] = memoryManager->allocate<TypeErrorCtor>(global);
    jsObjects[URIError_Ctor] = memoryManager->allocate<URIErrorCtor>(global);
    jsObjects[IteratorProto] = memoryManager->allocate<IteratorPrototype>();

    ic = newInternalClass(ForInIteratorPrototype::staticVTable(), iteratorPrototype());
    jsObjects[ForInIteratorProto] = memoryManager->allocObject<ForInIteratorPrototype>(ic);
    ic = newInternalClass(SetIteratorPrototype::staticVTable(), iteratorPrototype());
    jsObjects[MapIteratorProto] = memoryManager->allocObject<MapIteratorPrototype>(ic);
    ic = newInternalClass(SetIteratorPrototype::staticVTable(), iteratorPrototype());
    jsObjects[SetIteratorProto] = memoryManager->allocObject<SetIteratorPrototype>(ic);
    ic = newInternalClass(ArrayIteratorPrototype::staticVTable(), iteratorPrototype());
    jsObjects[ArrayIteratorProto] = memoryManager->allocObject<ArrayIteratorPrototype>(ic);
    ic = newInternalClass(StringIteratorPrototype::staticVTable(), iteratorPrototype());
    jsObjects[StringIteratorProto] = memoryManager->allocObject<StringIteratorPrototype>(ic);

    str = newString(QStringLiteral("get [Symbol.species]"));
    jsObjects[GetSymbolSpecies] = FunctionObject::createBuiltinFunction(this, str, ArrayPrototype::method_get_species, 0);

    static_cast<ObjectPrototype *>(objectPrototype())->init(this, objectCtor());
    static_cast<StringPrototype *>(stringPrototype())->init(this, stringCtor());
    static_cast<SymbolPrototype *>(symbolPrototype())->init(this, symbolCtor());
    static_cast<NumberPrototype *>(numberPrototype())->init(this, numberCtor());
    static_cast<BooleanPrototype *>(booleanPrototype())->init(this, booleanCtor());
    static_cast<ArrayPrototype *>(arrayPrototype())->init(this, arrayCtor());
    static_cast<PropertyListPrototype *>(propertyListPrototype())->init(this);
    static_cast<DatePrototype *>(datePrototype())->init(this, dateCtor());
    static_cast<FunctionPrototype *>(functionPrototype())->init(this, functionCtor());
    static_cast<GeneratorPrototype *>(generatorPrototype())->init(this, generatorFunctionCtor());
    static_cast<RegExpPrototype *>(regExpPrototype())->init(this, regExpCtor());
    static_cast<ErrorPrototype *>(errorPrototype())->init(this, errorCtor());
    static_cast<EvalErrorPrototype *>(evalErrorPrototype())->init(this, evalErrorCtor());
    static_cast<RangeErrorPrototype *>(rangeErrorPrototype())->init(this, rangeErrorCtor());
    static_cast<ReferenceErrorPrototype *>(referenceErrorPrototype())->init(this, referenceErrorCtor());
    static_cast<SyntaxErrorPrototype *>(syntaxErrorPrototype())->init(this, syntaxErrorCtor());
    static_cast<TypeErrorPrototype *>(typeErrorPrototype())->init(this, typeErrorCtor());
    static_cast<URIErrorPrototype *>(uRIErrorPrototype())->init(this, uRIErrorCtor());

    static_cast<IteratorPrototype *>(iteratorPrototype())->init(this);
    static_cast<ForInIteratorPrototype *>(forInIteratorPrototype())->init(this);
    static_cast<MapIteratorPrototype *>(mapIteratorPrototype())->init(this);
    static_cast<SetIteratorPrototype *>(setIteratorPrototype())->init(this);
    static_cast<ArrayIteratorPrototype *>(arrayIteratorPrototype())->init(this);
    static_cast<StringIteratorPrototype *>(stringIteratorPrototype())->init(this);

    static_cast<VariantPrototype *>(variantPrototype())->init();

#if QT_CONFIG(qml_sequence_object)
    sequencePrototype()->cast<SequencePrototype>()->init();
#endif

    jsObjects[WeakMap_Ctor] = memoryManager->allocate<WeakMapCtor>(global);
    jsObjects[WeakMapProto] = memoryManager->allocate<WeakMapPrototype>();
    static_cast<WeakMapPrototype *>(weakMapPrototype())->init(this, weakMapCtor());

    jsObjects[Map_Ctor] = memoryManager->allocate<MapCtor>(global);
    jsObjects[MapProto] = memoryManager->allocate<MapPrototype>();
    static_cast<MapPrototype *>(mapPrototype())->init(this, mapCtor());

    jsObjects[WeakSet_Ctor] = memoryManager->allocate<WeakSetCtor>(global);
    jsObjects[WeakSetProto] = memoryManager->allocate<WeakSetPrototype>();
    static_cast<WeakSetPrototype *>(weakSetPrototype())->init(this, weakSetCtor());

    jsObjects[Set_Ctor] = memoryManager->allocate<SetCtor>(global);
    jsObjects[SetProto] = memoryManager->allocate<SetPrototype>();
    static_cast<SetPrototype *>(setPrototype())->init(this, setCtor());

    //
    // promises
    //

    jsObjects[Promise_Ctor] = memoryManager->allocate<PromiseCtor>(global);
    jsObjects[PromiseProto] = memoryManager->allocate<PromisePrototype>();
    static_cast<PromisePrototype *>(promisePrototype())->init(this, promiseCtor());

    // typed arrays

    jsObjects[SharedArrayBuffer_Ctor] = memoryManager->allocate<SharedArrayBufferCtor>(global);
    jsObjects[SharedArrayBufferProto] = memoryManager->allocate<SharedArrayBufferPrototype>();
    static_cast<SharedArrayBufferPrototype *>(sharedArrayBufferPrototype())->init(this, sharedArrayBufferCtor());

    jsObjects[ArrayBuffer_Ctor] = memoryManager->allocate<ArrayBufferCtor>(global);
    jsObjects[ArrayBufferProto] = memoryManager->allocate<ArrayBufferPrototype>();
    static_cast<ArrayBufferPrototype *>(arrayBufferPrototype())->init(this, arrayBufferCtor());

    jsObjects[DataView_Ctor] = memoryManager->allocate<DataViewCtor>(global);
    jsObjects[DataViewProto] = memoryManager->allocate<DataViewPrototype>();
    static_cast<DataViewPrototype *>(dataViewPrototype())->init(this, dataViewCtor());
    jsObjects[ValueTypeProto] = (Heap::Base *) nullptr;
    jsObjects[SignalHandlerProto] = (Heap::Base *) nullptr;

    jsObjects[IntrinsicTypedArray_Ctor] = memoryManager->allocate<IntrinsicTypedArrayCtor>(global);
    jsObjects[IntrinsicTypedArrayProto] = memoryManager->allocate<IntrinsicTypedArrayPrototype>();
    static_cast<IntrinsicTypedArrayPrototype *>(intrinsicTypedArrayPrototype())
            ->init(this, static_cast<IntrinsicTypedArrayCtor *>(intrinsicTypedArrayCtor()));

    for (int i = 0; i < NTypedArrayTypes; ++i) {
        static_cast<Value &>(typedArrayCtors[i]) = memoryManager->allocate<TypedArrayCtor>(global, Heap::TypedArray::Type(i));
        static_cast<Value &>(typedArrayPrototype[i]) = memoryManager->allocate<TypedArrayPrototype>(Heap::TypedArray::Type(i));
        typedArrayPrototype[i].as<TypedArrayPrototype>()->init(this, static_cast<TypedArrayCtor *>(typedArrayCtors[i].as<Object>()));
    }

    //
    // set up the global object
    //
    rootContext()->d()->activation.set(scope.engine, globalObject->d());
    Q_ASSERT(globalObject->d()->vtable());

    globalObject->defineDefaultProperty(QStringLiteral("Object"), *objectCtor());
    globalObject->defineDefaultProperty(QStringLiteral("String"), *stringCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Symbol"), *symbolCtor());
    FunctionObject *numberObject = numberCtor();
    globalObject->defineDefaultProperty(QStringLiteral("Number"), *numberObject);
    globalObject->defineDefaultProperty(QStringLiteral("Boolean"), *booleanCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Array"), *arrayCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Function"), *functionCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Date"), *dateCtor());
    globalObject->defineDefaultProperty(QStringLiteral("RegExp"), *regExpCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Error"), *errorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("EvalError"), *evalErrorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("RangeError"), *rangeErrorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("ReferenceError"), *referenceErrorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("SyntaxError"), *syntaxErrorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("TypeError"), *typeErrorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("URIError"), *uRIErrorCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Promise"), *promiseCtor());

    globalObject->defineDefaultProperty(QStringLiteral("SharedArrayBuffer"), *sharedArrayBufferCtor());
    globalObject->defineDefaultProperty(QStringLiteral("ArrayBuffer"), *arrayBufferCtor());
    globalObject->defineDefaultProperty(QStringLiteral("DataView"), *dataViewCtor());
    globalObject->defineDefaultProperty(QStringLiteral("WeakSet"), *weakSetCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Set"), *setCtor());
    globalObject->defineDefaultProperty(QStringLiteral("WeakMap"), *weakMapCtor());
    globalObject->defineDefaultProperty(QStringLiteral("Map"), *mapCtor());

    for (int i = 0; i < NTypedArrayTypes; ++i)
        globalObject->defineDefaultProperty((str = typedArrayCtors[i].as<FunctionObject>()->name()), typedArrayCtors[i]);
    ScopedObject o(scope);
    globalObject->defineDefaultProperty(QStringLiteral("Atomics"), (o = memoryManager->allocate<Atomics>()));
    globalObject->defineDefaultProperty(QStringLiteral("Math"), (o = memoryManager->allocate<MathObject>()));
    globalObject->defineDefaultProperty(QStringLiteral("JSON"), (o = memoryManager->allocate<JsonObject>()));
    globalObject->defineDefaultProperty(QStringLiteral("Reflect"), (o = memoryManager->allocate<Reflect>()));
    globalObject->defineDefaultProperty(QStringLiteral("Proxy"), (o = memoryManager->allocate<Proxy>(rootContext())));

    globalObject->defineReadonlyProperty(QStringLiteral("undefined"), Value::undefinedValue());
    globalObject->defineReadonlyProperty(QStringLiteral("NaN"), Value::fromDouble(std::numeric_limits<double>::quiet_NaN()));
    globalObject->defineReadonlyProperty(QStringLiteral("Infinity"), Value::fromDouble(Q_INFINITY));


    jsObjects[Eval_Function] = memoryManager->allocate<EvalFunction>(global);
    globalObject->defineDefaultProperty(QStringLiteral("eval"), *evalFunction());

    // ES6: 20.1.2.12 &  20.1.2.13:
    // parseInt and parseFloat must be the same FunctionObject on the global &
    // Number object.
    {
        QString piString(QStringLiteral("parseInt"));
        QString pfString(QStringLiteral("parseFloat"));
        Scope scope(this);
        ScopedString pi(scope, newIdentifier(piString));
        ScopedString pf(scope, newIdentifier(pfString));
        ScopedFunctionObject parseIntFn(scope, FunctionObject::createBuiltinFunction(this, pi, GlobalFunctions::method_parseInt, 2));
        ScopedFunctionObject parseFloatFn(scope, FunctionObject::createBuiltinFunction(this, pf, GlobalFunctions::method_parseFloat, 1));
        globalObject->defineDefaultProperty(piString, parseIntFn);
        globalObject->defineDefaultProperty(pfString, parseFloatFn);
        numberObject->defineDefaultProperty(piString, parseIntFn);
        numberObject->defineDefaultProperty(pfString, parseFloatFn);
    }

    globalObject->defineDefaultProperty(QStringLiteral("isNaN"), GlobalFunctions::method_isNaN, 1);
    globalObject->defineDefaultProperty(QStringLiteral("isFinite"), GlobalFunctions::method_isFinite, 1);
    globalObject->defineDefaultProperty(QStringLiteral("decodeURI"), GlobalFunctions::method_decodeURI, 1);
    globalObject->defineDefaultProperty(QStringLiteral("decodeURIComponent"), GlobalFunctions::method_decodeURIComponent, 1);
    globalObject->defineDefaultProperty(QStringLiteral("encodeURI"), GlobalFunctions::method_encodeURI, 1);
    globalObject->defineDefaultProperty(QStringLiteral("encodeURIComponent"), GlobalFunctions::method_encodeURIComponent, 1);
    globalObject->defineDefaultProperty(QStringLiteral("escape"), GlobalFunctions::method_escape, 1);
    globalObject->defineDefaultProperty(QStringLiteral("unescape"), GlobalFunctions::method_unescape, 1);

    ScopedFunctionObject t(scope, memoryManager->allocate<FunctionObject>(rootContext(), nullptr, ::throwTypeError));
    t->defineReadonlyProperty(id_length(), Value::fromInt32(0));
    t->setInternalClass(t->internalClass()->frozen());
    jsObjects[ThrowerObject] = t;

    ScopedProperty pd(scope);
    pd->value = thrower();
    pd->set = thrower();
    functionPrototype()->insertMember(id_caller(), pd, Attr_Accessor|Attr_ReadOnly_ButConfigurable);
    functionPrototype()->insertMember(id_arguments(), pd, Attr_Accessor|Attr_ReadOnly_ButConfigurable);
}

ExecutionEngine::~ExecutionEngine()
{
    modules.clear();
    delete m_multiplyWrappedQObjects;
    m_multiplyWrappedQObjects = nullptr;
    delete identifierTable;
    delete memoryManager;

    while (!compilationUnits.isEmpty())
        (*compilationUnits.begin())->unlink();

    delete bumperPointerAllocator;
    delete regExpCache;
    delete regExpAllocator;
    delete executableAllocator;
    jsStack->deallocate();
    delete jsStack;
    gcStack->deallocate();
    delete gcStack;
}

ExecutionContext *ExecutionEngine::currentContext() const
{
    return static_cast<ExecutionContext *>(&currentStackFrame->jsFrame->context);
}

#if QT_CONFIG(qml_debug)
void ExecutionEngine::setDebugger(Debugging::Debugger *debugger)
{
    Q_ASSERT(!m_debugger);
    m_debugger.reset(debugger);
}

void ExecutionEngine::setProfiler(Profiling::Profiler *profiler)
{
    Q_ASSERT(!m_profiler);
    m_profiler.reset(profiler);
}
#endif // QT_CONFIG(qml_debug)

void ExecutionEngine::initRootContext()
{
    Scope scope(this);
    Scoped<ExecutionContext> r(scope, memoryManager->allocManaged<ExecutionContext>(sizeof(ExecutionContext::Data)));
    r->d_unchecked()->init(Heap::ExecutionContext::Type_GlobalContext);
    r->d()->activation.set(this, globalObject->d());
    jsObjects[RootContext] = r;
    jsObjects[ScriptContext] = r;
    jsObjects[IntegerNull] = Encode((int)0);
}

Heap::InternalClass *ExecutionEngine::newClass(Heap::InternalClass *other)
{
    Heap::InternalClass *ic = memoryManager->allocIC<InternalClass>();
    ic->init(other);
    return ic;
}

Heap::InternalClass *ExecutionEngine::newInternalClass(const VTable *vtable, Object *prototype)
{
    Scope scope(this);
    Scoped<InternalClass> ic(scope, internalClasses(Class_Empty)->changeVTable(vtable));
    return ic->changePrototype(prototype ? prototype->d() : nullptr);
}

Heap::Object *ExecutionEngine::newObject()
{
    return memoryManager->allocate<Object>();
}

Heap::Object *ExecutionEngine::newObject(Heap::InternalClass *internalClass)
{
    return memoryManager->allocObject<Object>(internalClass);
}

Heap::String *ExecutionEngine::newString(const QString &s)
{
    return memoryManager->allocWithStringData<String>(s.length() * sizeof(QChar), s);
}

Heap::String *ExecutionEngine::newIdentifier(const QString &text)
{
    Scope scope(this);
    ScopedString s(scope, memoryManager->allocWithStringData<String>(text.length() * sizeof(QChar), text));
    s->toPropertyKey();
    return s->d();
}

Heap::Object *ExecutionEngine::newStringObject(const String *string)
{
    return memoryManager->allocate<StringObject>(string);
}

Heap::Object *ExecutionEngine::newSymbolObject(const Symbol *symbol)
{
    return memoryManager->allocObject<SymbolObject>(classes[Class_SymbolObject], symbol);
}

Heap::Object *ExecutionEngine::newNumberObject(double value)
{
    return memoryManager->allocate<NumberObject>(value);
}

Heap::Object *ExecutionEngine::newBooleanObject(bool b)
{
    return memoryManager->allocate<BooleanObject>(b);
}

Heap::ArrayObject *ExecutionEngine::newArrayObject(int count)
{
    Scope scope(this);
    ScopedArrayObject object(scope, memoryManager->allocate<ArrayObject>());

    if (count) {
        if (count < 0x1000)
            object->arrayReserve(count);
        object->setArrayLengthUnchecked(count);
    }
    return object->d();
}

Heap::ArrayObject *ExecutionEngine::newArrayObject(const Value *values, int length)
{
    Scope scope(this);
    ScopedArrayObject a(scope, memoryManager->allocate<ArrayObject>());

    if (length) {
        size_t size = sizeof(Heap::ArrayData) + (length-1)*sizeof(Value);
        Heap::SimpleArrayData *d = scope.engine->memoryManager->allocManaged<SimpleArrayData>(size);
        d->init();
        d->type = Heap::ArrayData::Simple;
        d->offset = 0;
        d->values.alloc = length;
        d->values.size = length;
        // this doesn't require a write barrier, things will be ok, when the new array data gets inserted into
        // the parent object
        memcpy(&d->values.values, values, length*sizeof(Value));
        a->d()->arrayData.set(this, d);
        a->setArrayLengthUnchecked(length);
    }
    return a->d();
}

Heap::ArrayObject *ExecutionEngine::newArrayObject(const QStringList &list)
{
    return memoryManager->allocate<ArrayObject>(list);
}

Heap::ArrayObject *ExecutionEngine::newArrayObject(Heap::InternalClass *internalClass)
{
    return memoryManager->allocObject<ArrayObject>(internalClass);
}

Heap::ArrayBuffer *ExecutionEngine::newArrayBuffer(const QByteArray &array)
{
    return memoryManager->allocate<ArrayBuffer>(array);
}

Heap::ArrayBuffer *ExecutionEngine::newArrayBuffer(size_t length)
{
    return memoryManager->allocate<ArrayBuffer>(length);
}


Heap::DateObject *ExecutionEngine::newDateObject(const Value &value)
{
    return memoryManager->allocate<DateObject>(value);
}

Heap::DateObject *ExecutionEngine::newDateObject(const QDateTime &dt)
{
    Scope scope(this);
    Scoped<DateObject> object(scope, memoryManager->allocate<DateObject>(dt));
    return object->d();
}

Heap::DateObject *ExecutionEngine::newDateObjectFromTime(const QTime &t)
{
    Scope scope(this);
    Scoped<DateObject> object(scope, memoryManager->allocate<DateObject>(t));
    return object->d();
}

Heap::RegExpObject *ExecutionEngine::newRegExpObject(const QString &pattern, int flags)
{
    Scope scope(this);
    Scoped<RegExp> re(scope, RegExp::create(this, pattern, static_cast<CompiledData::RegExp::Flags>(flags)));
    return newRegExpObject(re);
}

Heap::RegExpObject *ExecutionEngine::newRegExpObject(RegExp *re)
{
    return memoryManager->allocate<RegExpObject>(re);
}

Heap::RegExpObject *ExecutionEngine::newRegExpObject(const QRegExp &re)
{
    return memoryManager->allocate<RegExpObject>(re);
}

Heap::Object *ExecutionEngine::newErrorObject(const Value &value)
{
    return ErrorObject::create<ErrorObject>(this, value, errorCtor());
}

Heap::Object *ExecutionEngine::newErrorObject(const QString &message)
{
    return ErrorObject::create<ErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newSyntaxErrorObject(const QString &message)
{
    return ErrorObject::create<SyntaxErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newSyntaxErrorObject(const QString &message, const QString &fileName, int line, int column)
{
    return ErrorObject::create<SyntaxErrorObject>(this, message, fileName, line, column);
}


Heap::Object *ExecutionEngine::newReferenceErrorObject(const QString &message)
{
    return ErrorObject::create<ReferenceErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newReferenceErrorObject(const QString &message, const QString &fileName, int line, int column)
{
    return ErrorObject::create<ReferenceErrorObject>(this, message, fileName, line, column);
}


Heap::Object *ExecutionEngine::newTypeErrorObject(const QString &message)
{
    return ErrorObject::create<TypeErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newRangeErrorObject(const QString &message)
{
    return ErrorObject::create<RangeErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newURIErrorObject(const Value &message)
{
    return ErrorObject::create<URIErrorObject>(this, message, uRIErrorCtor());
}

Heap::PromiseObject *ExecutionEngine::newPromiseObject()
{
    if (!m_reactionHandler) {
        m_reactionHandler.reset(new Promise::ReactionHandler);
    }

    Scope scope(this);
    Scoped<PromiseObject> object(scope, memoryManager->allocate<PromiseObject>(this));
    return object->d();
}

Heap::Object *ExecutionEngine::newPromiseObject(const QV4::FunctionObject *thisObject, const QV4::PromiseCapability *capability)
{
    if (!m_reactionHandler) {
        m_reactionHandler.reset(new Promise::ReactionHandler);
    }

    Scope scope(this);
    Scoped<CapabilitiesExecutorWrapper> executor(scope,  memoryManager->allocate<CapabilitiesExecutorWrapper>());
    executor->d()->capabilities.set(this, capability->d());
    executor->insertMember(id_length(), Primitive::fromInt32(2), Attr_NotWritable|Attr_NotEnumerable);

    ScopedObject object(scope, thisObject->callAsConstructor(executor, 1));
    return object->d();
}

Promise::ReactionHandler *ExecutionEngine::getPromiseReactionHandler()
{
    Q_ASSERT(m_reactionHandler);
    return m_reactionHandler.data();
}

Heap::Object *ExecutionEngine::newURIErrorObject(const QString &message)
{
    return ErrorObject::create<URIErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newEvalErrorObject(const QString &message)
{
    return ErrorObject::create<EvalErrorObject>(this, message);
}

Heap::Object *ExecutionEngine::newVariantObject(const QVariant &v)
{
    return memoryManager->allocate<VariantObject>(v);
}

Heap::Object *ExecutionEngine::newForInIteratorObject(Object *o)
{
    Scope scope(this);
    ScopedObject obj(scope, memoryManager->allocate<ForInIteratorObject>(o));
    return obj->d();
}

Heap::Object *ExecutionEngine::newMapIteratorObject(Object *o)
{
    return memoryManager->allocate<MapIteratorObject>(o->d(), this);
}

Heap::Object *ExecutionEngine::newSetIteratorObject(Object *o)
{
    return memoryManager->allocate<SetIteratorObject>(o->d(), this);
}

Heap::Object *ExecutionEngine::newArrayIteratorObject(Object *o)
{
    return memoryManager->allocate<ArrayIteratorObject>(o->d(), this);
}

Heap::QmlContext *ExecutionEngine::qmlContext() const
{
    if (!currentStackFrame)
        return nullptr;
    Heap::ExecutionContext *ctx = currentContext()->d();

    if (ctx->type != Heap::ExecutionContext::Type_QmlContext && !ctx->outer)
        return nullptr;

    while (ctx->outer && ctx->outer->type != Heap::ExecutionContext::Type_GlobalContext)
        ctx = ctx->outer;

    Q_ASSERT(ctx);
    if (ctx->type != Heap::ExecutionContext::Type_QmlContext)
        return nullptr;

    return static_cast<Heap::QmlContext *>(ctx);
}

QObject *ExecutionEngine::qmlScopeObject() const
{
    Heap::QmlContext *ctx = qmlContext();
    if (!ctx)
        return nullptr;

    return ctx->qml()->scopeObject;
}

QQmlContextData *ExecutionEngine::callingQmlContext() const
{
    Heap::QmlContext *ctx = qmlContext();
    if (!ctx)
        return nullptr;

    return ctx->qml()->context->contextData();
}

StackTrace ExecutionEngine::stackTrace(int frameLimit) const
{
    Scope scope(const_cast<ExecutionEngine *>(this));
    ScopedString name(scope);
    StackTrace stack;

    CppStackFrame *f = currentStackFrame;
    while (f && frameLimit) {
        QV4::StackFrame frame;
        frame.source = f->source();
        frame.function = f->function();
        frame.line = qAbs(f->lineNumber());
        frame.column = -1;
        stack.append(frame);
        if (f->isTailCalling) {
            QV4::StackFrame frame;
            frame.function = QStringLiteral("[elided tail calls]");
            stack.append(frame);
        }
        --frameLimit;
        f = f->parent;
    }

    return stack;
}

/* Helper and "C" linkage exported function to format a GDBMI stacktrace for
 * invocation by a debugger.
 * Sample GDB invocation: print qt_v4StackTrace((void*)0x7fffffffb290)
 * Sample CDB invocation: .call Qt5Qmld!qt_v4StackTrace(0x7fffffffb290) ; gh
 * Note: The helper is there to suppress MSVC warning 4190 about anything
 * with UDT return types in a "C" linkage function. */

static inline char *v4StackTrace(const ExecutionContext *context)
{
    QString result;
    QTextStream str(&result);
    str << "stack=[";
    if (context && context->engine()) {
        const QVector<StackFrame> stackTrace = context->engine()->stackTrace(20);
        for (int i = 0; i < stackTrace.size(); ++i) {
            if (i)
                str << ',';
            const QUrl url(stackTrace.at(i).source);
            const QString fileName = url.isLocalFile() ? url.toLocalFile() : url.toString();
            str << "frame={level=\"" << i << "\",func=\"" << stackTrace.at(i).function
                << "\",file=\"" << fileName << "\",fullname=\"" << fileName
                << "\",line=\"" << stackTrace.at(i).line << "\",language=\"js\"}";
        }
    }
    str << ']';
    return qstrdup(result.toLocal8Bit().constData());
}

extern "C" Q_QML_EXPORT char *qt_v4StackTrace(void *executionContext)
{
    return v4StackTrace(reinterpret_cast<const ExecutionContext *>(executionContext));
}

QUrl ExecutionEngine::resolvedUrl(const QString &file)
{
    QUrl src(file);
    if (!src.isRelative())
        return src;

    QUrl base;
    CppStackFrame *f = currentStackFrame;
    while (f) {
        if (f->v4Function) {
            base = f->v4Function->finalUrl();
            break;
        }
        f = f->parent;
    }

    if (base.isEmpty() && globalCode)
        base = globalCode->finalUrl();

    if (base.isEmpty())
        return src;

    return base.resolved(src);
}

void ExecutionEngine::markObjects(MarkStack *markStack)
{
    for (int i = 0; i < NClasses; ++i)
        if (classes[i]) {
            classes[i]->mark(markStack);
            if (markStack->top >= markStack->limit)
                markStack->drain();
        }
    markStack->drain();

    identifierTable->markObjects(markStack);

    for (auto compilationUnit: compilationUnits) {
        compilationUnit->markObjects(markStack);
        markStack->drain();
    }
}

ReturnedValue ExecutionEngine::throwError(const Value &value)
{
    // we can get in here with an exception already set, as the runtime
    // doesn't check after every operation that can throw.
    // in this case preserve the first exception to give correct error
    // information
    if (hasException)
        return Encode::undefined();

    hasException = true;
    *exceptionValue = value;
    QV4::Scope scope(this);
    QV4::Scoped<ErrorObject> error(scope, value);
    if (!!error)
        exceptionStackTrace = *error->d()->stackTrace;
    else
        exceptionStackTrace = stackTrace();

    if (QV4::Debugging::Debugger *debug = debugger())
        debug->aboutToThrow();

    return Encode::undefined();
}

ReturnedValue ExecutionEngine::catchException(StackTrace *trace)
{
    Q_ASSERT(hasException);
    if (trace)
        *trace = exceptionStackTrace;
    exceptionStackTrace.clear();
    hasException = false;
    ReturnedValue res = exceptionValue->asReturnedValue();
    *exceptionValue = Value::emptyValue();
    return res;
}

ReturnedValue ExecutionEngine::throwError(const QString &message)
{
    Scope scope(this);
    ScopedValue v(scope, newString(message));
    v = newErrorObject(v);
    return throwError(v);
}

ReturnedValue ExecutionEngine::throwSyntaxError(const QString &message, const QString &fileName, int line, int column)
{
    Scope scope(this);
    ScopedObject error(scope, newSyntaxErrorObject(message, fileName, line, column));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwSyntaxError(const QString &message)
{
    Scope scope(this);
    ScopedObject error(scope, newSyntaxErrorObject(message));
    return throwError(error);
}


ReturnedValue ExecutionEngine::throwTypeError()
{
    Scope scope(this);
    ScopedObject error(scope, newTypeErrorObject(QStringLiteral("Type error")));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwTypeError(const QString &message)
{
    Scope scope(this);
    ScopedObject error(scope, newTypeErrorObject(message));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwReferenceError(const QString &name)
{
    Scope scope(this);
    QString msg = name + QLatin1String(" is not defined");
    ScopedObject error(scope, newReferenceErrorObject(msg));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwReferenceError(const Value &value)
{
    Scope scope(this);
    ScopedString s(scope, value.toString(this));
    QString msg = s->toQString() + QLatin1String(" is not defined");
    ScopedObject error(scope, newReferenceErrorObject(msg));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwReferenceError(const QString &message, const QString &fileName, int line, int column)
{
    Scope scope(this);
    QString msg = message;
    ScopedObject error(scope, newReferenceErrorObject(msg, fileName, line, column));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwRangeError(const QString &message)
{
    Scope scope(this);
    ScopedObject error(scope, newRangeErrorObject(message));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwRangeError(const Value &value)
{
    Scope scope(this);
    ScopedString s(scope, value.toString(this));
    QString msg = s->toQString() + QLatin1String(" out of range");
    ScopedObject error(scope, newRangeErrorObject(msg));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwURIError(const Value &msg)
{
    Scope scope(this);
    ScopedObject error(scope, newURIErrorObject(msg));
    return throwError(error);
}

ReturnedValue ExecutionEngine::throwUnimplemented(const QString &message)
{
    Scope scope(this);
    ScopedValue v(scope, newString(QLatin1String("Unimplemented ") + message));
    v = newErrorObject(v);
    return throwError(v);
}


QQmlError ExecutionEngine::catchExceptionAsQmlError()
{
    QV4::StackTrace trace;
    QV4::Scope scope(this);
    QV4::ScopedValue exception(scope, catchException(&trace));
    QQmlError error;
    if (!trace.isEmpty()) {
        QV4::StackFrame frame = trace.constFirst();
        error.setUrl(QUrl(frame.source));
        error.setLine(frame.line);
        error.setColumn(frame.column);
    }
    QV4::Scoped<QV4::ErrorObject> errorObj(scope, exception);
    error.setDescription(exception->toQStringNoThrow());
    return error;
}

// Variant conversion code

typedef QSet<QV4::Heap::Object *> V4ObjectSet;
static QVariant toVariant(QV4::ExecutionEngine *e, const QV4::Value &value, int typeHint, bool createJSValueForObjects, V4ObjectSet *visitedObjects);
static QObject *qtObjectFromJS(QV4::ExecutionEngine *engine, const QV4::Value &value);
static QVariant objectToVariant(QV4::ExecutionEngine *e, const QV4::Object *o, V4ObjectSet *visitedObjects = nullptr);
static bool convertToNativeQObject(QV4::ExecutionEngine *e, const QV4::Value &value,
                            const QByteArray &targetType,
                            void **result);
static QV4::ReturnedValue variantListToJS(QV4::ExecutionEngine *v4, const QVariantList &lst);
static QV4::ReturnedValue variantMapToJS(QV4::ExecutionEngine *v4, const QVariantMap &vmap);
static QV4::ReturnedValue variantToJS(QV4::ExecutionEngine *v4, const QVariant &value)
{
    return v4->metaTypeToJS(value.userType(), value.constData());
}


QVariant ExecutionEngine::toVariant(const Value &value, int typeHint, bool createJSValueForObjects)
{
    return ::toVariant(this, value, typeHint, createJSValueForObjects, nullptr);
}


static QVariant toVariant(QV4::ExecutionEngine *e, const QV4::Value &value, int typeHint, bool createJSValueForObjects, V4ObjectSet *visitedObjects)
{
    Q_ASSERT (!value.isEmpty());
    QV4::Scope scope(e);

    if (const QV4::VariantObject *v = value.as<QV4::VariantObject>())
        return v->d()->data();

    if (typeHint == QVariant::Bool)
        return QVariant(value.toBoolean());

    if (typeHint == QMetaType::QJsonValue)
        return QVariant::fromValue(QV4::JsonObject::toJsonValue(value));

    if (typeHint == qMetaTypeId<QJSValue>())
        return QVariant::fromValue(QJSValue(e, value.asReturnedValue()));

    if (value.as<QV4::Object>()) {
        QV4::ScopedObject object(scope, value);
        if (typeHint == QMetaType::QJsonObject
                   && !value.as<ArrayObject>() && !value.as<FunctionObject>()) {
            return QVariant::fromValue(QV4::JsonObject::toJsonObject(object));
        } else if (QV4::QObjectWrapper *wrapper = object->as<QV4::QObjectWrapper>()) {
            return qVariantFromValue<QObject *>(wrapper->object());
        } else if (object->as<QV4::QQmlContextWrapper>()) {
            return QVariant();
        } else if (QV4::QQmlTypeWrapper *w = object->as<QV4::QQmlTypeWrapper>()) {
            return w->toVariant();
        } else if (QV4::QQmlValueTypeWrapper *v = object->as<QV4::QQmlValueTypeWrapper>()) {
            return v->toVariant();
        } else if (QV4::QmlListWrapper *l = object->as<QV4::QmlListWrapper>()) {
            return l->toVariant();
#if QT_CONFIG(qml_sequence_object)
        } else if (object->isListType()) {
            return QV4::SequencePrototype::toVariant(object);
#endif
        }
    }

    if (value.as<ArrayObject>()) {
        QV4::ScopedArrayObject a(scope, value);
        if (typeHint == qMetaTypeId<QList<QObject *> >()) {
            QList<QObject *> list;
            uint length = a->getLength();
            QV4::Scoped<QV4::QObjectWrapper> qobjectWrapper(scope);
            for (uint ii = 0; ii < length; ++ii) {
                qobjectWrapper = a->get(ii);
                if (!!qobjectWrapper) {
                    list << qobjectWrapper->object();
                } else {
                    list << 0;
                }
            }

            return qVariantFromValue<QList<QObject*> >(list);
        } else if (typeHint == QMetaType::QJsonArray) {
            return QVariant::fromValue(QV4::JsonObject::toJsonArray(a));
        }

#if QT_CONFIG(qml_sequence_object)
        bool succeeded = false;
        QVariant retn = QV4::SequencePrototype::toVariant(value, typeHint, &succeeded);
        if (succeeded)
            return retn;
#endif
    }

    if (value.isUndefined())
        return QVariant();
    if (value.isNull())
        return QVariant::fromValue(nullptr);
    if (value.isBoolean())
        return value.booleanValue();
    if (value.isInteger())
        return value.integerValue();
    if (value.isNumber())
        return value.asDouble();
    if (String *s = value.stringValue()) {
        const QString &str = s->toQString();
        // QChars are stored as a strings
        if (typeHint == QVariant::Char && str.size() == 1)
            return str.at(0);
        return str;
    }
#if QT_CONFIG(qml_locale)
    if (const QV4::QQmlLocaleData *ld = value.as<QV4::QQmlLocaleData>())
        return *ld->d()->locale;
#endif
    if (const QV4::DateObject *d = value.as<DateObject>())
        return d->toQDateTime();
    if (const ArrayBuffer *d = value.as<ArrayBuffer>())
        return d->asByteArray();
    // NOTE: since we convert QTime to JS Date, round trip will change the variant type (to QDateTime)!

    QV4::ScopedObject o(scope, value);
    Q_ASSERT(o);

    if (QV4::RegExpObject *re = o->as<QV4::RegExpObject>())
        return re->toQRegExp();

    if (createJSValueForObjects)
        return QVariant::fromValue(QJSValue(scope.engine, o->asReturnedValue()));

    return objectToVariant(e, o, visitedObjects);
}

static QVariant objectToVariant(QV4::ExecutionEngine *e, const QV4::Object *o, V4ObjectSet *visitedObjects)
{
    Q_ASSERT(o);

    V4ObjectSet recursionGuardSet;
    if (!visitedObjects) {
        visitedObjects = &recursionGuardSet;
    } else if (visitedObjects->contains(o->d())) {
        // Avoid recursion.
        // For compatibility with QVariant{List,Map} conversion, we return an
        // empty object (and no error is thrown).
        if (o->as<ArrayObject>())
            return QVariantList();
        return QVariantMap();
    }
    visitedObjects->insert(o->d());

    QVariant result;

    if (o->as<ArrayObject>()) {
        QV4::Scope scope(e);
        QV4::ScopedArrayObject a(scope, o->asReturnedValue());
        QV4::ScopedValue v(scope);
        QVariantList list;

        int length = a->getLength();
        for (int ii = 0; ii < length; ++ii) {
            v = a->get(ii);
            list << ::toVariant(e, v, -1, /*createJSValueForObjects*/false, visitedObjects);
        }

        result = list;
    } else if (!o->as<FunctionObject>()) {
        QVariantMap map;
        QV4::Scope scope(e);
        QV4::ObjectIterator it(scope, o, QV4::ObjectIterator::EnumerableOnly);
        QV4::ScopedValue name(scope);
        QV4::ScopedValue val(scope);
        while (1) {
            name = it.nextPropertyNameAsString(val);
            if (name->isNull())
                break;

            QString key = name->toQStringNoThrow();
            map.insert(key, ::toVariant(e, val, /*type hint*/-1, /*createJSValueForObjects*/false, visitedObjects));
        }

        result = map;
    }

    visitedObjects->remove(o->d());
    return result;
}

static QV4::ReturnedValue arrayFromVariantList(QV4::ExecutionEngine *e, const QVariantList &list)
{
    QV4::Scope scope(e);
    QV4::ScopedArrayObject a(scope, e->newArrayObject());
    int len = list.count();
    a->arrayReserve(len);
    QV4::ScopedValue v(scope);
    for (int ii = 0; ii < len; ++ii)
        a->arrayPut(ii, (v = scope.engine->fromVariant(list.at(ii))));

    a->setArrayLengthUnchecked(len);
    return a.asReturnedValue();
}

static QV4::ReturnedValue objectFromVariantMap(QV4::ExecutionEngine *e, const QVariantMap &map)
{
    QV4::Scope scope(e);
    QV4::ScopedObject o(scope, e->newObject());
    QV4::ScopedString s(scope);
    QV4::ScopedValue v(scope);
    for (QVariantMap::const_iterator iter = map.begin(), cend = map.end(); iter != cend; ++iter) {
        s = e->newString(iter.key());
        o->put(s, (v = e->fromVariant(iter.value())));
    }
    return o.asReturnedValue();
}

Q_CORE_EXPORT QString qt_regexp_toCanonical(const QString &, QRegExp::PatternSyntax);

QV4::ReturnedValue QV4::ExecutionEngine::fromVariant(const QVariant &variant)
{
    int type = variant.userType();
    const void *ptr = variant.constData();

    if (type < QMetaType::User) {
        switch (QMetaType::Type(type)) {
            case QMetaType::UnknownType:
            case QMetaType::Void:
                return QV4::Encode::undefined();
            case QMetaType::Nullptr:
            case QMetaType::VoidStar:
                return QV4::Encode::null();
            case QMetaType::Bool:
                return QV4::Encode(*reinterpret_cast<const bool*>(ptr));
            case QMetaType::Int:
                return QV4::Encode(*reinterpret_cast<const int*>(ptr));
            case QMetaType::UInt:
                return QV4::Encode(*reinterpret_cast<const uint*>(ptr));
            case QMetaType::LongLong:
                return QV4::Encode((double)*reinterpret_cast<const qlonglong*>(ptr));
            case QMetaType::ULongLong:
                return QV4::Encode((double)*reinterpret_cast<const qulonglong*>(ptr));
            case QMetaType::Double:
                return QV4::Encode(*reinterpret_cast<const double*>(ptr));
            case QMetaType::QString:
                return newString(*reinterpret_cast<const QString*>(ptr))->asReturnedValue();
            case QMetaType::QByteArray:
                return newArrayBuffer(*reinterpret_cast<const QByteArray*>(ptr))->asReturnedValue();
            case QMetaType::Float:
                return QV4::Encode(*reinterpret_cast<const float*>(ptr));
            case QMetaType::Short:
                return QV4::Encode((int)*reinterpret_cast<const short*>(ptr));
            case QMetaType::UShort:
                return QV4::Encode((int)*reinterpret_cast<const unsigned short*>(ptr));
            case QMetaType::Char:
                return QV4::Encode((int)*reinterpret_cast<const char*>(ptr));
            case QMetaType::UChar:
                return QV4::Encode((int)*reinterpret_cast<const unsigned char*>(ptr));
            case QMetaType::QChar:
                return newString(*reinterpret_cast<const QChar *>(ptr))->asReturnedValue();
            case QMetaType::QDateTime:
                return QV4::Encode(newDateObject(*reinterpret_cast<const QDateTime *>(ptr)));
            case QMetaType::QDate:
                return QV4::Encode(newDateObject(QDateTime(*reinterpret_cast<const QDate *>(ptr), QTime(0, 0, 0), Qt::UTC)));
            case QMetaType::QTime:
                return QV4::Encode(newDateObjectFromTime(*reinterpret_cast<const QTime *>(ptr)));
            case QMetaType::QRegExp:
                return QV4::Encode(newRegExpObject(*reinterpret_cast<const QRegExp *>(ptr)));
            case QMetaType::QObjectStar:
                return QV4::QObjectWrapper::wrap(this, *reinterpret_cast<QObject* const *>(ptr));
#if QT_CONFIG(qml_sequence_object)
            case QMetaType::QStringList:
                {
                bool succeeded = false;
                QV4::Scope scope(this);
                QV4::ScopedValue retn(scope, QV4::SequencePrototype::fromVariant(this, variant, &succeeded));
                if (succeeded)
                    return retn->asReturnedValue();
                return QV4::Encode(newArrayObject(*reinterpret_cast<const QStringList *>(ptr)));
                }
#endif
            case QMetaType::QVariantList:
                return arrayFromVariantList(this, *reinterpret_cast<const QVariantList *>(ptr));
            case QMetaType::QVariantMap:
                return objectFromVariantMap(this, *reinterpret_cast<const QVariantMap *>(ptr));
            case QMetaType::QJsonValue:
                return QV4::JsonObject::fromJsonValue(this, *reinterpret_cast<const QJsonValue *>(ptr));
            case QMetaType::QJsonObject:
                return QV4::JsonObject::fromJsonObject(this, *reinterpret_cast<const QJsonObject *>(ptr));
            case QMetaType::QJsonArray:
                return QV4::JsonObject::fromJsonArray(this, *reinterpret_cast<const QJsonArray *>(ptr));
#if QT_CONFIG(qml_locale)
            case QMetaType::QLocale:
                return QQmlLocale::wrap(this, *reinterpret_cast<const QLocale*>(ptr));
#endif
            case QMetaType::QPixmap:
            case QMetaType::QImage:
                // Scarce value types
                return QV4::Encode(newVariantObject(variant));
            default:
                break;
        }

        if (const QMetaObject *vtmo = QQmlValueTypeFactory::metaObjectForMetaType(type))
            return QV4::QQmlValueTypeWrapper::create(this, variant, vtmo, type);
    } else {
        QV4::Scope scope(this);
        if (type == qMetaTypeId<QQmlListReference>()) {
            typedef QQmlListReferencePrivate QDLRP;
            QDLRP *p = QDLRP::get((QQmlListReference*)const_cast<void *>(ptr));
            if (p->object) {
                return QV4::QmlListWrapper::create(scope.engine, p->property, p->propertyType);
            } else {
                return QV4::Encode::null();
            }
        } else if (type == qMetaTypeId<QJSValue>()) {
            const QJSValue *value = reinterpret_cast<const QJSValue *>(ptr);
            return QJSValuePrivate::convertedToValue(this, *value);
        } else if (type == qMetaTypeId<QList<QObject *> >()) {
            // XXX Can this be made more by using Array as a prototype and implementing
            // directly against QList<QObject*>?
            const QList<QObject *> &list = *(const QList<QObject *>*)ptr;
            QV4::ScopedArrayObject a(scope, newArrayObject());
            a->arrayReserve(list.count());
            QV4::ScopedValue v(scope);
            for (int ii = 0; ii < list.count(); ++ii)
                a->arrayPut(ii, (v = QV4::QObjectWrapper::wrap(this, list.at(ii))));
            a->setArrayLengthUnchecked(list.count());
            return a.asReturnedValue();
        } else if (QMetaType::typeFlags(type) & QMetaType::PointerToQObject) {
            return QV4::QObjectWrapper::wrap(this, *reinterpret_cast<QObject* const *>(ptr));
        }

        bool objOk;
        QObject *obj = QQmlMetaType::toQObject(variant, &objOk);
        if (objOk)
            return QV4::QObjectWrapper::wrap(this, obj);

#if QT_CONFIG(qml_sequence_object)
        bool succeeded = false;
        QV4::ScopedValue retn(scope, QV4::SequencePrototype::fromVariant(this, variant, &succeeded));
        if (succeeded)
            return retn->asReturnedValue();
#endif

        if (const QMetaObject *vtmo = QQmlValueTypeFactory::metaObjectForMetaType(type))
            return QV4::QQmlValueTypeWrapper::create(this, variant, vtmo, type);
    }

    // XXX TODO: To be compatible, we still need to handle:
    //    + QObjectList
    //    + QList<int>

    return QV4::Encode(newVariantObject(variant));
}

QVariantMap ExecutionEngine::variantMapFromJS(const Object *o)
{
    return objectToVariant(this, o).toMap();
}


// Converts a QVariantList to JS.
// The result is a new Array object with length equal to the length
// of the QVariantList, and the elements being the QVariantList's
// elements converted to JS, recursively.
static QV4::ReturnedValue variantListToJS(QV4::ExecutionEngine *v4, const QVariantList &lst)
{
    QV4::Scope scope(v4);
    QV4::ScopedArrayObject a(scope, v4->newArrayObject());
    a->arrayReserve(lst.size());
    QV4::ScopedValue v(scope);
    for (int i = 0; i < lst.size(); i++)
        a->arrayPut(i, (v = variantToJS(v4, lst.at(i))));
    a->setArrayLengthUnchecked(lst.size());
    return a.asReturnedValue();
}

// Converts a QVariantMap to JS.
// The result is a new Object object with property names being
// the keys of the QVariantMap, and values being the values of
// the QVariantMap converted to JS, recursively.
static QV4::ReturnedValue variantMapToJS(QV4::ExecutionEngine *v4, const QVariantMap &vmap)
{
    QV4::Scope scope(v4);
    QV4::ScopedObject o(scope, v4->newObject());
    QV4::ScopedString s(scope);
    QV4::ScopedPropertyKey key(scope);
    QV4::ScopedValue v(scope);
    for (QVariantMap::const_iterator it = vmap.constBegin(), cend = vmap.constEnd(); it != cend; ++it) {
        s = v4->newIdentifier(it.key());
        key = s->propertyKey();
        v = variantToJS(v4, it.value());
        uint idx = key->asArrayIndex();
        if (idx < UINT_MAX)
            o->arraySet(idx, v);
        else
            o->insertMember(s, v);
    }
    return o.asReturnedValue();
}

// Converts the meta-type defined by the given type and data to JS.
// Returns the value if conversion succeeded, an empty handle otherwise.
QV4::ReturnedValue ExecutionEngine::metaTypeToJS(int type, const void *data)
{
    Q_ASSERT(data != nullptr);

    // check if it's one of the types we know
    switch (QMetaType::Type(type)) {
    case QMetaType::UnknownType:
    case QMetaType::Void:
        return QV4::Encode::undefined();
    case QMetaType::Nullptr:
    case QMetaType::VoidStar:
        return QV4::Encode::null();
    case QMetaType::Bool:
        return QV4::Encode(*reinterpret_cast<const bool*>(data));
    case QMetaType::Int:
        return QV4::Encode(*reinterpret_cast<const int*>(data));
    case QMetaType::UInt:
        return QV4::Encode(*reinterpret_cast<const uint*>(data));
    case QMetaType::LongLong:
        return QV4::Encode(double(*reinterpret_cast<const qlonglong*>(data)));
    case QMetaType::ULongLong:
#if defined(Q_OS_WIN) && defined(_MSC_FULL_VER) && _MSC_FULL_VER <= 12008804
#pragma message("** NOTE: You need the Visual Studio Processor Pack to compile support for 64bit unsigned integers.")
        return QV4::Encode(double((qlonglong)*reinterpret_cast<const qulonglong*>(data)));
#elif defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
        return QV4::Encode(double((qlonglong)*reinterpret_cast<const qulonglong*>(data)));
#else
        return QV4::Encode(double(*reinterpret_cast<const qulonglong*>(data)));
#endif
    case QMetaType::Double:
        return QV4::Encode(*reinterpret_cast<const double*>(data));
    case QMetaType::QString:
        return newString(*reinterpret_cast<const QString*>(data))->asReturnedValue();
    case QMetaType::QByteArray:
        return newArrayBuffer(*reinterpret_cast<const QByteArray*>(data))->asReturnedValue();
    case QMetaType::Float:
        return QV4::Encode(*reinterpret_cast<const float*>(data));
    case QMetaType::Short:
        return QV4::Encode((int)*reinterpret_cast<const short*>(data));
    case QMetaType::UShort:
        return QV4::Encode((int)*reinterpret_cast<const unsigned short*>(data));
    case QMetaType::Char:
        return QV4::Encode((int)*reinterpret_cast<const char*>(data));
    case QMetaType::UChar:
        return QV4::Encode((int)*reinterpret_cast<const unsigned char*>(data));
    case QMetaType::QChar:
        return QV4::Encode((int)(*reinterpret_cast<const QChar*>(data)).unicode());
    case QMetaType::QStringList:
        return QV4::Encode(newArrayObject(*reinterpret_cast<const QStringList *>(data)));
    case QMetaType::QVariantList:
        return variantListToJS(this, *reinterpret_cast<const QVariantList *>(data));
    case QMetaType::QVariantMap:
        return variantMapToJS(this, *reinterpret_cast<const QVariantMap *>(data));
    case QMetaType::QDateTime:
        return QV4::Encode(newDateObject(*reinterpret_cast<const QDateTime *>(data)));
    case QMetaType::QDate:
        return QV4::Encode(newDateObject(QDateTime(*reinterpret_cast<const QDate *>(data))));
    case QMetaType::QRegExp:
        return QV4::Encode(newRegExpObject(*reinterpret_cast<const QRegExp *>(data)));
    case QMetaType::QObjectStar:
        return QV4::QObjectWrapper::wrap(this, *reinterpret_cast<QObject* const *>(data));
    case QMetaType::QVariant:
        return variantToJS(this, *reinterpret_cast<const QVariant*>(data));
    case QMetaType::QJsonValue:
        return QV4::JsonObject::fromJsonValue(this, *reinterpret_cast<const QJsonValue *>(data));
    case QMetaType::QJsonObject:
        return QV4::JsonObject::fromJsonObject(this, *reinterpret_cast<const QJsonObject *>(data));
    case QMetaType::QJsonArray:
        return QV4::JsonObject::fromJsonArray(this, *reinterpret_cast<const QJsonArray *>(data));
    default:
        if (type == qMetaTypeId<QJSValue>()) {
            return QJSValuePrivate::convertedToValue(this, *reinterpret_cast<const QJSValue*>(data));
        } else {
            QByteArray typeName = QMetaType::typeName(type);
            if (typeName.endsWith('*') && !*reinterpret_cast<void* const *>(data)) {
                return QV4::Encode::null();
            }
            QMetaType mt(type);
            if (mt.flags() & QMetaType::IsGadget) {
                Q_ASSERT(mt.metaObject());
                return QV4::QQmlValueTypeWrapper::create(this, QVariant(type, data), mt.metaObject(), type);
            }
            // Fall back to wrapping in a QVariant.
            return QV4::Encode(newVariantObject(QVariant(type, data)));
        }
    }
    Q_UNREACHABLE();
    return 0;
}

ReturnedValue ExecutionEngine::global()
{
    return globalObject->asReturnedValue();
}

QQmlRefPointer<CompiledData::CompilationUnit> ExecutionEngine::compileModule(const QUrl &url)
{
    QQmlMetaType::CachedUnitLookupError cacheError = QQmlMetaType::CachedUnitLookupError::NoError;
    if (const QV4::CompiledData::Unit *cachedUnit = QQmlMetaType::findCachedCompilationUnit(url, &cacheError)) {
        QQmlRefPointer<QV4::CompiledData::CompilationUnit> jsUnit;
        jsUnit.adopt(new QV4::CompiledData::CompilationUnit(cachedUnit, url.fileName(), url.toString()));
        return jsUnit;
    }

    QFile f(QQmlFile::urlToLocalFileOrQrc(url));
    if (!f.open(QIODevice::ReadOnly)) {
        throwError(QStringLiteral("Could not open module %1 for reading").arg(url.toString()));
        return nullptr;
    }

    const QDateTime timeStamp = QFileInfo(f).lastModified();

    const QString sourceCode = QString::fromUtf8(f.readAll());
    f.close();

    return compileModule(url, sourceCode, timeStamp);
}


QQmlRefPointer<CompiledData::CompilationUnit> ExecutionEngine::compileModule(const QUrl &url, const QString &sourceCode, const QDateTime &sourceTimeStamp)
{
    QList<QQmlJS::DiagnosticMessage> diagnostics;
    auto unit = compileModule(/*debugMode*/debugger() != nullptr, url.toString(), sourceCode, sourceTimeStamp, &diagnostics);
    for (const QQmlJS::DiagnosticMessage &m : diagnostics) {
        if (m.isError()) {
            throwSyntaxError(m.message, url.toString(), m.loc.startLine, m.loc.startColumn);
            return nullptr;
        } else {
            qWarning() << url << ':' << m.loc.startLine << ':' << m.loc.startColumn
                      << ": warning: " << m.message;
        }
    }
    return unit;
}

#endif // ifndef V4_BOOTSTRAP

QQmlRefPointer<CompiledData::CompilationUnit> ExecutionEngine::compileModule(bool debugMode, const QString &url, const QString &sourceCode,
                                                                             const QDateTime &sourceTimeStamp, QList<QQmlJS::DiagnosticMessage> *diagnostics)
{
    QQmlJS::Engine ee;
    QQmlJS::Lexer lexer(&ee);
    lexer.setCode(sourceCode, /*line*/1, /*qml mode*/false);
    QQmlJS::Parser parser(&ee);

    const bool parsed = parser.parseModule();

    if (diagnostics)
        *diagnostics = parser.diagnosticMessages();

    if (!parsed)
        return nullptr;

    QQmlJS::AST::ESModule *moduleNode = QQmlJS::AST::cast<QQmlJS::AST::ESModule*>(parser.rootNode());
    if (!moduleNode) {
        // if parsing was successful, and we have no module, then
        // the file was empty.
        if (diagnostics)
            diagnostics->clear();
        return nullptr;
    }

    using namespace QV4::Compiler;
    Compiler::Module compilerModule(debugMode);
    compilerModule.unitFlags |= CompiledData::Unit::IsESModule;
    compilerModule.sourceTimeStamp = sourceTimeStamp;
    JSUnitGenerator jsGenerator(&compilerModule);
    Codegen cg(&jsGenerator, /*strictMode*/true);
    cg.generateFromModule(url, url, sourceCode, moduleNode, &compilerModule);
    auto errors = cg.errors();
    if (diagnostics)
        *diagnostics << errors;

    if (!errors.isEmpty())
        return nullptr;

    return cg.generateCompilationUnit();
}

#ifndef V4_BOOTSTRAP

void ExecutionEngine::injectModule(const QQmlRefPointer<CompiledData::CompilationUnit> &moduleUnit)
{
    // Injection can happen from the QML type loader thread for example, but instantiation and
    // evaluation must be limited to the ExecutionEngine's thread.
    QMutexLocker moduleGuard(&moduleMutex);
    modules.insert(moduleUnit->finalUrl(), moduleUnit);
}

QQmlRefPointer<CompiledData::CompilationUnit> ExecutionEngine::moduleForUrl(const QUrl &_url, const CompiledData::CompilationUnit *referrer) const
{
    QUrl url = QQmlTypeLoader::normalize(_url);
    if (referrer)
        url = referrer->finalUrl().resolved(url);

    QMutexLocker moduleGuard(&moduleMutex);
    auto existingModule = modules.find(url);
    if (existingModule == modules.end())
        return nullptr;
    return *existingModule;
}

QQmlRefPointer<CompiledData::CompilationUnit> ExecutionEngine::loadModule(const QUrl &_url, const CompiledData::CompilationUnit *referrer)
{
    QUrl url = QQmlTypeLoader::normalize(_url);
    if (referrer)
        url = referrer->finalUrl().resolved(url);

    QMutexLocker moduleGuard(&moduleMutex);
    auto existingModule = modules.find(url);
    if (existingModule != modules.end())
        return *existingModule;

    moduleGuard.unlock();

    auto newModule = compileModule(url);
    if (newModule) {
        moduleGuard.relock();
        modules.insert(url, newModule);
    }

    return newModule;
}

// Converts a JS value to a meta-type.
// data must point to a place that can store a value of the given type.
// Returns true if conversion succeeded, false otherwise.
bool ExecutionEngine::metaTypeFromJS(const Value *value, int type, void *data)
{
    // check if it's one of the types we know
    switch (QMetaType::Type(type)) {
    case QMetaType::Bool:
        *reinterpret_cast<bool*>(data) = value->toBoolean();
        return true;
    case QMetaType::Int:
        *reinterpret_cast<int*>(data) = value->toInt32();
        return true;
    case QMetaType::UInt:
        *reinterpret_cast<uint*>(data) = value->toUInt32();
        return true;
    case QMetaType::LongLong:
        *reinterpret_cast<qlonglong*>(data) = qlonglong(value->toInteger());
        return true;
    case QMetaType::ULongLong:
        *reinterpret_cast<qulonglong*>(data) = qulonglong(value->toInteger());
        return true;
    case QMetaType::Double:
        *reinterpret_cast<double*>(data) = value->toNumber();
        return true;
    case QMetaType::QString:
        if (value->isUndefined() || value->isNull())
            *reinterpret_cast<QString*>(data) = QString();
        else
            *reinterpret_cast<QString*>(data) = value->toQString();
        return true;
    case QMetaType::QByteArray:
        if (const ArrayBuffer *ab = value->as<ArrayBuffer>())
            *reinterpret_cast<QByteArray*>(data) = ab->asByteArray();
        else
            *reinterpret_cast<QByteArray*>(data) = QByteArray();
        return true;
    case QMetaType::Float:
        *reinterpret_cast<float*>(data) = value->toNumber();
        return true;
    case QMetaType::Short:
        *reinterpret_cast<short*>(data) = short(value->toInt32());
        return true;
    case QMetaType::UShort:
        *reinterpret_cast<unsigned short*>(data) = value->toUInt16();
        return true;
    case QMetaType::Char:
        *reinterpret_cast<char*>(data) = char(value->toInt32());
        return true;
    case QMetaType::UChar:
        *reinterpret_cast<unsigned char*>(data) = (unsigned char)(value->toInt32());
        return true;
    case QMetaType::QChar:
        if (String *s = value->stringValue()) {
            QString str = s->toQString();
            *reinterpret_cast<QChar*>(data) = str.isEmpty() ? QChar() : str.at(0);
        } else {
            *reinterpret_cast<QChar*>(data) = QChar(ushort(value->toUInt16()));
        }
        return true;
    case QMetaType::QDateTime:
        if (const QV4::DateObject *d = value->as<DateObject>()) {
            *reinterpret_cast<QDateTime *>(data) = d->toQDateTime();
            return true;
        } break;
    case QMetaType::QDate:
        if (const QV4::DateObject *d = value->as<DateObject>()) {
            *reinterpret_cast<QDate *>(data) = d->toQDateTime().date();
            return true;
        } break;
    case QMetaType::QRegExp:
        if (const QV4::RegExpObject *r = value->as<QV4::RegExpObject>()) {
            *reinterpret_cast<QRegExp *>(data) = r->toQRegExp();
            return true;
        } break;
    case QMetaType::QObjectStar: {
        const QV4::QObjectWrapper *qobjectWrapper = value->as<QV4::QObjectWrapper>();
        if (qobjectWrapper || value->isNull()) {
            *reinterpret_cast<QObject* *>(data) = qtObjectFromJS(this, *value);
            return true;
        } break;
    }
    case QMetaType::QStringList: {
        const QV4::ArrayObject *a = value->as<QV4::ArrayObject>();
        if (a) {
            *reinterpret_cast<QStringList *>(data) = a->toQStringList();
            return true;
        }
        break;
    }
    case QMetaType::QVariantList: {
        const QV4::ArrayObject *a = value->as<QV4::ArrayObject>();
        if (a) {
            *reinterpret_cast<QVariantList *>(data) = toVariant(*a, /*typeHint*/-1, /*createJSValueForObjects*/false).toList();
            return true;
        }
        break;
    }
    case QMetaType::QVariantMap: {
        const QV4::Object *o = value->as<QV4::Object>();
        if (o) {
            *reinterpret_cast<QVariantMap *>(data) = variantMapFromJS(o);
            return true;
        }
        break;
    }
    case QMetaType::QVariant:
        *reinterpret_cast<QVariant*>(data) = toVariant(*value, /*typeHint*/-1, /*createJSValueForObjects*/false);
        return true;
    case QMetaType::QJsonValue:
        *reinterpret_cast<QJsonValue *>(data) = QV4::JsonObject::toJsonValue(*value);
        return true;
    case QMetaType::QJsonObject: {
        *reinterpret_cast<QJsonObject *>(data) = QV4::JsonObject::toJsonObject(value->as<Object>());
        return true;
    }
    case QMetaType::QJsonArray: {
        const QV4::ArrayObject *a = value->as<ArrayObject>();
        if (a) {
            *reinterpret_cast<QJsonArray *>(data) = JsonObject::toJsonArray(a);
            return true;
        }
        break;
    }
    default:
    ;
    }

    {
        const QQmlValueTypeWrapper *vtw = value->as<QQmlValueTypeWrapper>();
        if (vtw && vtw->typeId() == type) {
            return vtw->toGadget(data);
        }
    }

#if 0
    if (isQtVariant(value)) {
        const QVariant &var = variantValue(value);
        // ### Enable once constructInPlace() is in qt master.
        if (var.userType() == type) {
            QMetaType::constructInPlace(type, data, var.constData());
            return true;
        }
        if (var.canConvert(type)) {
            QVariant vv = var;
            vv.convert(type);
            Q_ASSERT(vv.userType() == type);
            QMetaType::constructInPlace(type, data, vv.constData());
            return true;
        }

    }
#endif

    // Try to use magic; for compatibility with qjsvalue_cast.

    QByteArray name = QMetaType::typeName(type);
    if (convertToNativeQObject(this, *value, name, reinterpret_cast<void* *>(data)))
        return true;
    if (value->as<QV4::VariantObject>() && name.endsWith('*')) {
        int valueType = QMetaType::type(name.left(name.size()-1));
        QVariant &var = value->as<QV4::VariantObject>()->d()->data();
        if (valueType == var.userType()) {
            // We have T t, T* is requested, so return &t.
            *reinterpret_cast<void* *>(data) = var.data();
            return true;
        } else if (Object *o = value->objectValue()) {
            // Look in the prototype chain.
            QV4::Scope scope(this);
            QV4::ScopedObject proto(scope, o->getPrototypeOf());
            while (proto) {
                bool canCast = false;
                if (QV4::VariantObject *vo = proto->as<QV4::VariantObject>()) {
                    const QVariant &v = vo->d()->data();
                    canCast = (type == v.userType()) || (valueType && (valueType == v.userType()));
                }
                else if (proto->as<QV4::QObjectWrapper>()) {
                    QByteArray className = name.left(name.size()-1);
                    QV4::ScopedObject p(scope, proto.getPointer());
                    if (QObject *qobject = qtObjectFromJS(this, p))
                        canCast = qobject->qt_metacast(className) != nullptr;
                }
                if (canCast) {
                    QByteArray varTypeName = QMetaType::typeName(var.userType());
                    if (varTypeName.endsWith('*'))
                        *reinterpret_cast<void* *>(data) = *reinterpret_cast<void* *>(var.data());
                    else
                        *reinterpret_cast<void* *>(data) = var.data();
                    return true;
                }
                proto = proto->getPrototypeOf();
            }
        }
    } else if (value->isNull() && name.endsWith('*')) {
        *reinterpret_cast<void* *>(data) = nullptr;
        return true;
    } else if (type == qMetaTypeId<QJSValue>()) {
        *reinterpret_cast<QJSValue*>(data) = QJSValue(this, value->asReturnedValue());
        return true;
    }

    return false;
}

static bool convertToNativeQObject(QV4::ExecutionEngine *e, const QV4::Value &value, const QByteArray &targetType, void **result)
{
    if (!targetType.endsWith('*'))
        return false;
    if (QObject *qobject = qtObjectFromJS(e, value)) {
        int start = targetType.startsWith("const ") ? 6 : 0;
        QByteArray className = targetType.mid(start, targetType.size()-start-1);
        if (void *instance = qobject->qt_metacast(className)) {
            *result = instance;
            return true;
        }
    }
    return false;
}

static QObject *qtObjectFromJS(QV4::ExecutionEngine *engine, const QV4::Value &value)
{
    if (!value.isObject())
        return nullptr;

    QV4::Scope scope(engine);
    QV4::Scoped<QV4::VariantObject> v(scope, value);

    if (v) {
        QVariant variant = v->d()->data();
        int type = variant.userType();
        if (type == QMetaType::QObjectStar)
            return *reinterpret_cast<QObject* const *>(variant.constData());
    }
    QV4::Scoped<QV4::QObjectWrapper> wrapper(scope, value);
    if (!wrapper)
        return nullptr;
    return wrapper->object();
}

#endif // ifndef V4_BOOTSTRAP

QT_END_NAMESPACE
