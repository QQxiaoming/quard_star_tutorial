INCLUDEPATH += $$PWD
INCLUDEPATH += $$OUT_PWD

!qmldevtools_build {
SOURCES += \
    $$PWD/qv4context.cpp \
    $$PWD/qv4persistent.cpp \
    $$PWD/qv4lookup.cpp \
    $$PWD/qv4identifier.cpp \
    $$PWD/qv4identifiertable.cpp \
    $$PWD/qv4managed.cpp \
    $$PWD/qv4internalclass.cpp \
    $$PWD/qv4sparsearray.cpp \
    $$PWD/qv4atomics.cpp \
    $$PWD/qv4arraydata.cpp \
    $$PWD/qv4arrayobject.cpp \
    $$PWD/qv4arrayiterator.cpp \
    $$PWD/qv4argumentsobject.cpp \
    $$PWD/qv4booleanobject.cpp \
    $$PWD/qv4dateobject.cpp \
    $$PWD/qv4errorobject.cpp \
    $$PWD/qv4function.cpp \
    $$PWD/qv4functionobject.cpp \
    $$PWD/qv4generatorobject.cpp \
    $$PWD/qv4globalobject.cpp \
    $$PWD/qv4iterator.cpp \
    $$PWD/qv4jsonobject.cpp \
    $$PWD/qv4mathobject.cpp \
    $$PWD/qv4memberdata.cpp \
    $$PWD/qv4numberobject.cpp \
    $$PWD/qv4object.cpp \
    $$PWD/qv4objectproto.cpp \
    $$PWD/qv4propertykey.cpp \
    $$PWD/qv4proxy.cpp \
    $$PWD/qv4qmlcontext.cpp \
    $$PWD/qv4reflect.cpp \
    $$PWD/qv4regexpobject.cpp \
    $$PWD/qv4stackframe.cpp \
    $$PWD/qv4stringiterator.cpp \
    $$PWD/qv4stringobject.cpp \
    $$PWD/qv4variantobject.cpp \
    $$PWD/qv4objectiterator.cpp \
    $$PWD/qv4regexp.cpp \
    $$PWD/qv4runtimecodegen.cpp \
    $$PWD/qv4serialize.cpp \
    $$PWD/qv4script.cpp \
    $$PWD/qv4symbol.cpp \
    $$PWD/qv4setobject.cpp \
    $$PWD/qv4setiterator.cpp \
    $$PWD/qv4include.cpp \
    $$PWD/qv4qobjectwrapper.cpp \
    $$PWD/qv4arraybuffer.cpp \
    $$PWD/qv4typedarray.cpp \
    $$PWD/qv4dataview.cpp \
    $$PWD/qv4vme_moth.cpp \
    $$PWD/qv4mapobject.cpp \
    $$PWD/qv4mapiterator.cpp \
    $$PWD/qv4estable.cpp \
    $$PWD/qv4module.cpp \
    $$PWD/qv4promiseobject.cpp

qtConfig(qml-debug): SOURCES += $$PWD/qv4profiling.cpp

HEADERS += \
    $$PWD/qv4global_p.h \
    $$PWD/qv4alloca_p.h \
    $$PWD/qv4engine_p.h \
    $$PWD/qv4enginebase_p.h \
    $$PWD/qv4context_p.h \
    $$PWD/qv4math_p.h \
    $$PWD/qv4persistent_p.h \
    $$PWD/qv4debugging_p.h \
    $$PWD/qv4lookup_p.h \
    $$PWD/qv4identifier_p.h \
    $$PWD/qv4identifiertable_p.h \
    $$PWD/qv4managed_p.h \
    $$PWD/qv4internalclass_p.h \
    $$PWD/qv4jscall_p.h \
    $$PWD/qv4sparsearray_p.h \
    $$PWD/qv4atomics_p.h \
    $$PWD/qv4arraydata_p.h \
    $$PWD/qv4arrayobject_p.h \
    $$PWD/qv4arrayiterator_p.h \
    $$PWD/qv4argumentsobject_p.h \
    $$PWD/qv4booleanobject_p.h \
    $$PWD/qv4dateobject_p.h \
    $$PWD/qv4errorobject_p.h \
    $$PWD/qv4function_p.h \
    $$PWD/qv4functionobject_p.h \
    $$PWD/qv4generatorobject_p.h \
    $$PWD/qv4globalobject_p.h \
    $$PWD/qv4iterator_p.h \
    $$PWD/qv4jsonobject_p.h \
    $$PWD/qv4mathobject_p.h \
    $$PWD/qv4memberdata_p.h \
    $$PWD/qv4numberobject_p.h \
    $$PWD/qv4object_p.h \
    $$PWD/qv4objectproto_p.h \
    $$PWD/qv4propertykey_p.h \
    $$PWD/qv4proxy_p.h \
    $$PWD/qv4qmlcontext_p.h \
    $$PWD/qv4reflect_p.h \
    $$PWD/qv4regexpobject_p.h \
    $$PWD/qv4runtimecodegen_p.h \
    $$PWD/qv4stackframe_p.h \
    $$PWD/qv4stringiterator_p.h \
    $$PWD/qv4stringobject_p.h \
    $$PWD/qv4variantobject_p.h \
    $$PWD/qv4property_p.h \
    $$PWD/qv4objectiterator_p.h \
    $$PWD/qv4regexp_p.h \
    $$PWD/qv4serialize_p.h \
    $$PWD/qv4script_p.h \
    $$PWD/qv4symbol_p.h \
    $$PWD/qv4setobject_p.h \
    $$PWD/qv4setiterator_p.h \
    $$PWD/qv4scopedvalue_p.h \
    $$PWD/qv4executableallocator_p.h \
    $$PWD/qv4include_p.h \
    $$PWD/qv4qobjectwrapper_p.h \
    $$PWD/qv4profiling_p.h \
    $$PWD/qv4arraybuffer_p.h \
    $$PWD/qv4typedarray_p.h \
    $$PWD/qv4dataview_p.h \
    $$PWD/qv4vme_moth_p.h \
    $$PWD/qv4mapobject_p.h \
    $$PWD/qv4mapiterator_p.h \
    $$PWD/qv4estable_p.h \
    $$PWD/qv4vtable_p.h \
    $$PWD/qv4module_p.h \
    $$PWD/qv4promiseobject_p.h

qtConfig(qml-sequence-object) {
    HEADERS += \
        $$PWD/qv4sequenceobject_p.h

    SOURCES += \
        $$PWD/qv4sequenceobject.cpp
}

}


HEADERS += \
    $$PWD/qv4runtime_p.h \
    $$PWD/qv4runtimeapi_p.h \
    $$PWD/qv4value_p.h \
    $$PWD/qv4string_p.h \
    $$PWD/qv4util_p.h \
    $$PWD/qv4value_p.h

SOURCES += \
    $$PWD/qv4engine.cpp \
    $$PWD/qv4runtime.cpp \
    $$PWD/qv4string.cpp \
    $$PWD/qv4value.cpp \
    $$PWD/qv4executableallocator.cpp

valgrind {
    DEFINES += V4_USE_VALGRIND
}

heaptrack {
    DEFINES += V4_USE_HEAPTRACK
}
