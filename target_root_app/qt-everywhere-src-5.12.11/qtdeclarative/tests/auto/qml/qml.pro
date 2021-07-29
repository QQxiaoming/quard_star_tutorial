TEMPLATE = subdirs
QT_FOR_CONFIG += qml

METATYPETESTS += \
    qqmlmetatype

PUBLICTESTS += \
    parserstress \
    qjsvalueiterator \
    qjsonbinding \
    qqmlfile \
    qqmlfileselector

PUBLICTESTS += \
    qmlmin \
    qqmlcomponent \
    qqmlconsole \
    qqmlengine \
    qqmlerror \
    qqmlincubator \
    qqmlinfo \
    qqmllistreference \
    qqmllocale \
    qqmlmetaobject \
    qqmlmoduleplugin \
    qqmlnotifier \
    qqmlqt \
    qqmlxmlhttprequest \
    qqmlpromise \
    qtqmlmodules \
    qquickfolderlistmodel \
    qqmlapplicationengine \
    qqmlsettings \
    qqmlstatemachine \
    qmldiskcache

PRIVATETESTS += \
    qqmlcpputils \
    qqmldirparser \
    qmlcachegen

PRIVATETESTS += \
    animation \
    qqmlecmascript \
    qqmlcontext \
    qqmlexpression \
    qqmlglobal \
    qqmllanguage \
    qqmlopenmetaobject \
    qqmlproperty \
    qqmlpropertycache \
    qqmlpropertymap \
    qqmlsqldatabase \
    qqmlvaluetypes \
    qqmlvaluetypeproviders \
    qqmlbinding \
    qqmlchangeset \
    qqmlconnections \
    qqmllistcompositor \
    qqmllistmodel \
    qqmllistmodelworkerscript \
    qqmlitemmodels \
    qqmltypeloader \
    qqmlparser \
    qquickworkerscript \
    qrcqml \
    qqmltimer \
    qqmlinstantiator \
    qqmlenginecleanup \
    qqmltranslation \
    qqmlimport \
    qqmlobjectmodel \
    qv4assembler \
    qv4mm \
    qv4identifiertable \
    qv4regexp \
    ecmascripttests \
    bindingdependencyapi \
    v4misc

qtHaveModule(widgets) {
    PUBLICTESTS += \
        qjsengine \
        qjsvalue \
#        qwidgetsinqml
}

SUBDIRS += $$PUBLICTESTS
SUBDIRS += $$METATYPETESTS
qtConfig(process) {
    qtConfig(qml-debug): SUBDIRS += debugger
    !boot2qt {
        SUBDIRS += qmllint qmlplugindump
    }
}

qtConfig(library) {
    SUBDIRS += qqmlextensionplugin
}

qtConfig(private_tests): \
    SUBDIRS += $$PRIVATETESTS

qtNomakeTools( \
    qmlplugindump \
)
