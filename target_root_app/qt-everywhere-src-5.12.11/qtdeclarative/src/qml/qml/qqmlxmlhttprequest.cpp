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

#include "qqmlxmlhttprequest_p.h"

#include <private/qv8engine_p.h>

#include "qqmlengine.h"
#include "qqmlengine_p.h"
#include <private/qqmlrefcount_p.h>
#include "qqmlengine_p.h"
#include "qqmlexpression_p.h"
#include "qqmlglobal_p.h"
#include <private/qv4domerrors_p.h>
#include <private/qv4engine_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4jscall_p.h>

#include <QtCore/qobject.h>
#include <QtQml/qjsvalue.h>
#include <QtQml/qjsengine.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtCore/qtextcodec.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/qstack.h>
#include <QtCore/qdebug.h>
#include <QtCore/qbuffer.h>

#include <private/qv4objectproto_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4arraybuffer_p.h>
#include <private/qv4jsonobject_p.h>

using namespace QV4;

#if QT_CONFIG(xmlstreamreader) && QT_CONFIG(qml_network)

#define V4THROW_REFERENCE(string) \
    do { \
        ScopedObject error(scope, scope.engine->newReferenceErrorObject(QStringLiteral(string))); \
        return scope.engine->throwError(error); \
    } while (false)

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(xhrDump, QML_XHR_DUMP);

struct QQmlXMLHttpRequestData {
    QQmlXMLHttpRequestData();
    ~QQmlXMLHttpRequestData();

    PersistentValue nodeFunction;

    PersistentValue nodePrototype;
    PersistentValue elementPrototype;
    PersistentValue attrPrototype;
    PersistentValue characterDataPrototype;
    PersistentValue textPrototype;
    PersistentValue cdataPrototype;
    PersistentValue documentPrototype;
};

static inline QQmlXMLHttpRequestData *xhrdata(ExecutionEngine *v4)
{
    return (QQmlXMLHttpRequestData *)v4->v8Engine->xmlHttpRequestData();
}

QQmlXMLHttpRequestData::QQmlXMLHttpRequestData()
{
}

QQmlXMLHttpRequestData::~QQmlXMLHttpRequestData()
{
}

namespace QV4 {

class DocumentImpl;
class NodeImpl
{
public:
    NodeImpl() : type(Element), document(nullptr), parent(nullptr) {}
    virtual ~NodeImpl() {
        qDeleteAll(children);
        qDeleteAll(attributes);
    }

    // These numbers are copied from the Node IDL definition
    enum Type {
        Attr = 2,
        CDATA = 4,
        Comment = 8,
        Document = 9,
        DocumentFragment = 11,
        DocumentType = 10,
        Element = 1,
        Entity = 6,
        EntityReference = 5,
        Notation = 12,
        ProcessingInstruction = 7,
        Text = 3
    };
    Type type;

    QString namespaceUri;
    QString name;

    QString data;

    void addref();
    void release();

    DocumentImpl *document;
    NodeImpl *parent;

    QList<NodeImpl *> children;
    QList<NodeImpl *> attributes;
};

class DocumentImpl : public QQmlRefCount, public NodeImpl
{
public:
    DocumentImpl() : root(nullptr) { type = Document; }
    virtual ~DocumentImpl() {
        delete root;
    }

    QString version;
    QString encoding;
    bool isStandalone;

    NodeImpl *root;

    void addref() { QQmlRefCount::addref(); }
    void release() { QQmlRefCount::release(); }
};

namespace Heap {

struct NamedNodeMap : Object {
    void init(NodeImpl *data, const QList<NodeImpl *> &list);
    void destroy() {
        delete listPtr;
        if (d)
            d->release();
        Object::destroy();
    }
    QList<NodeImpl *> &list() {
        if (listPtr == nullptr)
            listPtr = new QList<NodeImpl *>;
        return *listPtr;
    }

    QList<NodeImpl *> *listPtr; // Only used in NamedNodeMap
    NodeImpl *d;
};

struct NodeList : Object {
    void init(NodeImpl *data);
    void destroy() {
        if (d)
            d->release();
        Object::destroy();
    }
    NodeImpl *d;
};

struct NodePrototype : Object {
    void init();
};

struct Node : Object {
    void init(NodeImpl *data);
    void destroy() {
        if (d)
            d->release();
        Object::destroy();
    }
    NodeImpl *d;
};

}

class NamedNodeMap : public Object
{
public:
    V4_OBJECT2(NamedNodeMap, Object)
    V4_NEEDS_DESTROY

    // C++ API
    static ReturnedValue create(ExecutionEngine *, NodeImpl *, const QList<NodeImpl *> &);

    // JS API
    static ReturnedValue virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty);
};

void Heap::NamedNodeMap::init(NodeImpl *data, const QList<NodeImpl *> &list)
{
    Object::init();
    d = data;
    this->list() = list;
    if (d)
        d->addref();
}

DEFINE_OBJECT_VTABLE(NamedNodeMap);

class NodeList : public Object
{
public:
    V4_OBJECT2(NodeList, Object)
    V4_NEEDS_DESTROY

    // JS API
    static ReturnedValue virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty);

    // C++ API
    static ReturnedValue create(ExecutionEngine *, NodeImpl *);

};

void Heap::NodeList::init(NodeImpl *data)
{
    Object::init();
    d = data;
    if (d)
        d->addref();
}

DEFINE_OBJECT_VTABLE(NodeList);

class NodePrototype : public Object
{
public:
    V4_OBJECT2(NodePrototype, Object)

    static void initClass(ExecutionEngine *engine);

    // JS API
    static ReturnedValue method_get_nodeName(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_nodeValue(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_nodeType(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_namespaceUri(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    static ReturnedValue method_get_parentNode(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_childNodes(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_firstChild(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_lastChild(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_previousSibling(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_nextSibling(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_attributes(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    //static ReturnedValue ownerDocument(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    //static ReturnedValue namespaceURI(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    //static ReturnedValue prefix(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    //static ReturnedValue localName(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    //static ReturnedValue baseURI(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    //static ReturnedValue textContent(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    static ReturnedValue getProto(ExecutionEngine *v4);

};

void Heap::NodePrototype::init()
{
    Object::init();
    Scope scope(internalClass->engine);
    ScopedObject o(scope, this);

    o->defineAccessorProperty(QStringLiteral("nodeName"), QV4::NodePrototype::method_get_nodeName, nullptr);
    o->defineAccessorProperty(QStringLiteral("nodeValue"), QV4::NodePrototype::method_get_nodeValue, nullptr);
    o->defineAccessorProperty(QStringLiteral("nodeType"), QV4::NodePrototype::method_get_nodeType, nullptr);
    o->defineAccessorProperty(QStringLiteral("namespaceUri"), QV4::NodePrototype::method_get_namespaceUri, nullptr);

    o->defineAccessorProperty(QStringLiteral("parentNode"), QV4::NodePrototype::method_get_parentNode, nullptr);
    o->defineAccessorProperty(QStringLiteral("childNodes"), QV4::NodePrototype::method_get_childNodes, nullptr);
    o->defineAccessorProperty(QStringLiteral("firstChild"), QV4::NodePrototype::method_get_firstChild, nullptr);
    o->defineAccessorProperty(QStringLiteral("lastChild"), QV4::NodePrototype::method_get_lastChild, nullptr);
    o->defineAccessorProperty(QStringLiteral("previousSibling"), QV4::NodePrototype::method_get_previousSibling, nullptr);
    o->defineAccessorProperty(QStringLiteral("nextSibling"), QV4::NodePrototype::method_get_nextSibling, nullptr);
    o->defineAccessorProperty(QStringLiteral("attributes"), QV4::NodePrototype::method_get_attributes, nullptr);
}


DEFINE_OBJECT_VTABLE(NodePrototype);

struct Node : public Object
{
    V4_OBJECT2(Node, Object)
    V4_NEEDS_DESTROY

    // C++ API
    static ReturnedValue create(ExecutionEngine *v4, NodeImpl *);

    bool isNull() const;
};

void Heap::Node::init(NodeImpl *data)
{
    Object::init();
    d = data;
    if (d)
        d->addref();
}

DEFINE_OBJECT_VTABLE(Node);

class Element : public Node
{
public:
    // C++ API
    static ReturnedValue prototype(ExecutionEngine *);
};

class Attr : public Node
{
public:
    // JS API
    static ReturnedValue method_name(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
//    static void specified(CallContext *);
    static ReturnedValue method_value(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_ownerElement(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
//    static void schemaTypeInfo(CallContext *);
//    static void isId(CallContext *c);

    // C++ API
    static ReturnedValue prototype(ExecutionEngine *);
};

class CharacterData : public Node
{
public:
    // JS API
    static ReturnedValue method_length(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    // C++ API
    static ReturnedValue prototype(ExecutionEngine *v4);
};

class Text : public CharacterData
{
public:
    // JS API
    static ReturnedValue method_isElementContentWhitespace(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_wholeText(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    // C++ API
    static ReturnedValue prototype(ExecutionEngine *);
};

class CDATA : public Text
{
public:
    // C++ API
    static ReturnedValue prototype(ExecutionEngine *v4);
};

class Document : public Node
{
public:
    // JS API
    static ReturnedValue method_xmlVersion(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_xmlEncoding(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_xmlStandalone(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_documentElement(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    // C++ API
    static ReturnedValue prototype(ExecutionEngine *);
    static ReturnedValue load(ExecutionEngine *engine, const QByteArray &data);
};

}

void NodeImpl::addref()
{
    document->addref();
}

void NodeImpl::release()
{
    document->release();
}

ReturnedValue NodePrototype::method_get_nodeName(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    QString name;
    switch (r->d()->d->type) {
    case NodeImpl::Document:
        name = QStringLiteral("#document");
        break;
    case NodeImpl::CDATA:
        name = QStringLiteral("#cdata-section");
        break;
    case NodeImpl::Text:
        name = QStringLiteral("#text");
        break;
    default:
        name = r->d()->d->name;
        break;
    }
    return Encode(scope.engine->newString(name));
}

ReturnedValue NodePrototype::method_get_nodeValue(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (r->d()->d->type == NodeImpl::Document ||
        r->d()->d->type == NodeImpl::DocumentFragment ||
        r->d()->d->type == NodeImpl::DocumentType ||
        r->d()->d->type == NodeImpl::Element ||
        r->d()->d->type == NodeImpl::Entity ||
        r->d()->d->type == NodeImpl::EntityReference ||
        r->d()->d->type == NodeImpl::Notation)
        RETURN_RESULT(Encode::null());

    return Encode(scope.engine->newString(r->d()->d->data));
}

ReturnedValue NodePrototype::method_get_nodeType(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    return Encode(r->d()->d->type);
}

ReturnedValue NodePrototype::method_get_namespaceUri(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    return Encode(scope.engine->newString(r->d()->d->namespaceUri));
}

ReturnedValue NodePrototype::method_get_parentNode(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (r->d()->d->parent)
        return Node::create(scope.engine, r->d()->d->parent);
    else
        return Encode::null();
}

ReturnedValue NodePrototype::method_get_childNodes(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    return NodeList::create(scope.engine, r->d()->d);
}

ReturnedValue NodePrototype::method_get_firstChild(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (r->d()->d->children.isEmpty())
        return Encode::null();
    else
        return Node::create(scope.engine, r->d()->d->children.constFirst());
}

ReturnedValue NodePrototype::method_get_lastChild(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (r->d()->d->children.isEmpty())
        return Encode::null();
    else
        return Node::create(scope.engine, r->d()->d->children.constLast());
}

ReturnedValue NodePrototype::method_get_previousSibling(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (!r->d()->d->parent)
        RETURN_RESULT(Encode::null());

    for (int ii = 0; ii < r->d()->d->parent->children.count(); ++ii) {
        if (r->d()->d->parent->children.at(ii) == r->d()->d) {
            if (ii == 0)
                return Encode::null();
            else
                return Node::create(scope.engine, r->d()->d->parent->children.at(ii - 1));
        }
    }

    return Encode::null();
}

ReturnedValue NodePrototype::method_get_nextSibling(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (!r->d()->d->parent)
        RETURN_RESULT(Encode::null());

    for (int ii = 0; ii < r->d()->d->parent->children.count(); ++ii) {
        if (r->d()->d->parent->children.at(ii) == r->d()->d) {
            if ((ii + 1) == r->d()->d->parent->children.count())
                return Encode::null();
            else
                return Node::create(scope.engine, r->d()->d->parent->children.at(ii + 1));
        }
    }

    return Encode::null();
}

ReturnedValue NodePrototype::method_get_attributes(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        THROW_TYPE_ERROR();

    if (r->d()->d->type != NodeImpl::Element)
        return Encode::null();
    else
        return NamedNodeMap::create(scope.engine, r->d()->d, r->d()->d->attributes);
}

ReturnedValue NodePrototype::getProto(ExecutionEngine *v4)
{
    Scope scope(v4);
    QQmlXMLHttpRequestData *d = xhrdata(v4);
    if (d->nodePrototype.isUndefined()) {
        ScopedObject p(scope, v4->memoryManager->allocate<NodePrototype>());
        d->nodePrototype.set(v4, p);
        v4->v8Engine->freezeObject(p);
    }
    return d->nodePrototype.value();
}

ReturnedValue Node::create(ExecutionEngine *v4, NodeImpl *data)
{
    Scope scope(v4);

    Scoped<Node> instance(scope, v4->memoryManager->allocate<Node>(data));
    ScopedObject p(scope);

    switch (data->type) {
    case NodeImpl::Attr:
        instance->setPrototypeUnchecked((p = Attr::prototype(v4)));
        break;
    case NodeImpl::Comment:
    case NodeImpl::Document:
    case NodeImpl::DocumentFragment:
    case NodeImpl::DocumentType:
    case NodeImpl::Entity:
    case NodeImpl::EntityReference:
    case NodeImpl::Notation:
    case NodeImpl::ProcessingInstruction:
        return Encode::undefined();
    case NodeImpl::CDATA:
        instance->setPrototypeUnchecked((p = CDATA::prototype(v4)));
        break;
    case NodeImpl::Text:
        instance->setPrototypeUnchecked((p = Text::prototype(v4)));
        break;
    case NodeImpl::Element:
        instance->setPrototypeUnchecked((p = Element::prototype(v4)));
        break;
    }

    return instance.asReturnedValue();
}

ReturnedValue Element::prototype(ExecutionEngine *engine)
{
    QQmlXMLHttpRequestData *d = xhrdata(engine);
    if (d->elementPrototype.isUndefined()) {
        Scope scope(engine);
        ScopedObject p(scope, engine->newObject());
        ScopedObject pp(scope);
        p->setPrototypeUnchecked((pp = NodePrototype::getProto(engine)));
        p->defineAccessorProperty(QStringLiteral("tagName"), NodePrototype::method_get_nodeName, nullptr);
        d->elementPrototype.set(engine, p);
        engine->v8Engine->freezeObject(p);
    }
    return d->elementPrototype.value();
}

ReturnedValue Attr::prototype(ExecutionEngine *engine)
{
    QQmlXMLHttpRequestData *d = xhrdata(engine);
    if (d->attrPrototype.isUndefined()) {
        Scope scope(engine);
        ScopedObject p(scope, engine->newObject());
        ScopedObject pp(scope);
        p->setPrototypeUnchecked((pp = NodePrototype::getProto(engine)));
        p->defineAccessorProperty(QStringLiteral("name"), method_name, nullptr);
        p->defineAccessorProperty(QStringLiteral("value"), method_value, nullptr);
        p->defineAccessorProperty(QStringLiteral("ownerElement"), method_ownerElement, nullptr);
        d->attrPrototype.set(engine, p);
        engine->v8Engine->freezeObject(p);
    }
    return d->attrPrototype.value();
}

ReturnedValue Attr::method_name(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        RETURN_UNDEFINED();

    return Encode(scope.engine->newString(r->d()->d->name));
}

ReturnedValue Attr::method_value(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        RETURN_UNDEFINED();

    return Encode(scope.engine->newString(r->d()->d->data));
}

ReturnedValue Attr::method_ownerElement(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        RETURN_UNDEFINED();

    return Node::create(scope.engine, r->d()->d->parent);
}

ReturnedValue CharacterData::method_length(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        RETURN_UNDEFINED();

    return Encode(r->d()->d->data.length());
}

ReturnedValue CharacterData::prototype(ExecutionEngine *v4)
{
    QQmlXMLHttpRequestData *d = xhrdata(v4);
    if (d->characterDataPrototype.isUndefined()) {
        Scope scope(v4);
        ScopedObject p(scope, v4->newObject());
        ScopedObject pp(scope);
        p->setPrototypeUnchecked((pp = NodePrototype::getProto(v4)));
        p->defineAccessorProperty(QStringLiteral("data"), NodePrototype::method_get_nodeValue, nullptr);
        p->defineAccessorProperty(QStringLiteral("length"), method_length, nullptr);
        d->characterDataPrototype.set(v4, p);
        v4->v8Engine->freezeObject(p);
    }
    return d->characterDataPrototype.value();
}

ReturnedValue Text::method_isElementContentWhitespace(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        RETURN_UNDEFINED();

    return Encode(QStringRef(&r->d()->d->data).trimmed().isEmpty());
}

ReturnedValue Text::method_wholeText(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    QV4::Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r)
        RETURN_UNDEFINED();

    return Encode(scope.engine->newString(r->d()->d->data));
}

ReturnedValue Text::prototype(ExecutionEngine *v4)
{
    QQmlXMLHttpRequestData *d = xhrdata(v4);
    if (d->textPrototype.isUndefined()) {
        Scope scope(v4);
        ScopedObject p(scope, v4->newObject());
        ScopedObject pp(scope);
        p->setPrototypeUnchecked((pp = CharacterData::prototype(v4)));
        p->defineAccessorProperty(QStringLiteral("isElementContentWhitespace"), method_isElementContentWhitespace, nullptr);
        p->defineAccessorProperty(QStringLiteral("wholeText"), method_wholeText, nullptr);
        d->textPrototype.set(v4, p);
        v4->v8Engine->freezeObject(p);
    }
    return d->textPrototype.value();
}

ReturnedValue CDATA::prototype(ExecutionEngine *v4)
{
    // ### why not just use TextProto???
    QQmlXMLHttpRequestData *d = xhrdata(v4);
    if (d->cdataPrototype.isUndefined()) {
        Scope scope(v4);
        ScopedObject p(scope, v4->newObject());
        ScopedObject pp(scope);
        p->setPrototypeUnchecked((pp = Text::prototype(v4)));
        d->cdataPrototype.set(v4, p);
        v4->v8Engine->freezeObject(p);
    }
    return d->cdataPrototype.value();
}

ReturnedValue Document::prototype(ExecutionEngine *v4)
{
    QQmlXMLHttpRequestData *d = xhrdata(v4);
    if (d->documentPrototype.isUndefined()) {
        Scope scope(v4);
        ScopedObject p(scope, v4->newObject());
        ScopedObject pp(scope);
        p->setPrototypeUnchecked((pp = NodePrototype::getProto(v4)));
        p->defineAccessorProperty(QStringLiteral("xmlVersion"), method_xmlVersion, nullptr);
        p->defineAccessorProperty(QStringLiteral("xmlEncoding"), method_xmlEncoding, nullptr);
        p->defineAccessorProperty(QStringLiteral("xmlStandalone"), method_xmlStandalone, nullptr);
        p->defineAccessorProperty(QStringLiteral("documentElement"), method_documentElement, nullptr);
        d->documentPrototype.set(v4, p);
        v4->v8Engine->freezeObject(p);
    }
    return d->documentPrototype.value();
}

ReturnedValue Document::load(ExecutionEngine *v4, const QByteArray &data)
{
    Scope scope(v4);

    DocumentImpl *document = nullptr;
    QStack<NodeImpl *> nodeStack;

    QXmlStreamReader reader(data);

    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::NoToken:
            break;
        case QXmlStreamReader::Invalid:
            break;
        case QXmlStreamReader::StartDocument:
            Q_ASSERT(!document);
            document = new DocumentImpl;
            document->document = document;
            document->version = reader.documentVersion().toString();
            document->encoding = reader.documentEncoding().toString();
            document->isStandalone = reader.isStandaloneDocument();
            break;
        case QXmlStreamReader::EndDocument:
            break;
        case QXmlStreamReader::StartElement:
        {
            Q_ASSERT(document);
            NodeImpl *node = new NodeImpl;
            node->document = document;
            node->namespaceUri = reader.namespaceUri().toString();
            node->name = reader.name().toString();
            if (nodeStack.isEmpty()) {
                document->root = node;
            } else {
                node->parent = nodeStack.top();
                node->parent->children.append(node);
            }
            nodeStack.append(node);

            const auto attributes = reader.attributes();
            for (const QXmlStreamAttribute &a : attributes) {
                NodeImpl *attr = new NodeImpl;
                attr->document = document;
                attr->type = NodeImpl::Attr;
                attr->namespaceUri = a.namespaceUri().toString();
                attr->name = a.name().toString();
                attr->data = a.value().toString();
                attr->parent = node;
                node->attributes.append(attr);
            }
        }
            break;
        case QXmlStreamReader::EndElement:
            nodeStack.pop();
            break;
        case QXmlStreamReader::Characters:
        {
            NodeImpl *node = new NodeImpl;
            node->document = document;
            node->type = reader.isCDATA()?NodeImpl::CDATA:NodeImpl::Text;
            node->parent = nodeStack.top();
            node->parent->children.append(node);
            node->data = reader.text().toString();
        }
            break;
        case QXmlStreamReader::Comment:
            break;
        case QXmlStreamReader::DTD:
            break;
        case QXmlStreamReader::EntityReference:
            break;
        case QXmlStreamReader::ProcessingInstruction:
            break;
        }
    }

    if (!document || reader.hasError()) {
        if (document)
            document->release();
        return Encode::null();
    }

    ScopedObject instance(scope, v4->memoryManager->allocate<Node>(document));
    document->release(); // the GC should own the NodeImpl via Node now
    ScopedObject p(scope);
    instance->setPrototypeUnchecked((p = Document::prototype(v4)));
    return instance.asReturnedValue();
}

bool Node::isNull() const
{
    return d()->d == nullptr;
}

ReturnedValue NamedNodeMap::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    Q_ASSERT(m->as<NamedNodeMap>());

    const NamedNodeMap *r = static_cast<const NamedNodeMap *>(m);
    QV4::ExecutionEngine *v4 = r->engine();

    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();

        if ((int)index < r->d()->list().count()) {
            if (hasProperty)
                *hasProperty = true;
            return Node::create(v4, r->d()->list().at(index));
        }
        if (hasProperty)
            *hasProperty = false;
        return Encode::undefined();
    }

    if (id.isSymbol())
        return Object::virtualGet(m, id, receiver, hasProperty);

    if (id == v4->id_length()->propertyKey())
        return Value::fromInt32(r->d()->list().count()).asReturnedValue();

    QString str = id.toQString();
    for (int ii = 0; ii < r->d()->list().count(); ++ii) {
        if (r->d()->list().at(ii)->name == str) {
            if (hasProperty)
                *hasProperty = true;
            return Node::create(v4, r->d()->list().at(ii));
        }
    }

    if (hasProperty)
        *hasProperty = false;
    return Encode::undefined();
}

ReturnedValue NamedNodeMap::create(ExecutionEngine *v4, NodeImpl *data, const QList<NodeImpl *> &list)
{
    return (v4->memoryManager->allocate<NamedNodeMap>(data, list))->asReturnedValue();
}

ReturnedValue NodeList::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    Q_ASSERT(m->as<NodeList>());
    const NodeList *r = static_cast<const NodeList *>(m);
    QV4::ExecutionEngine *v4 = r->engine();

    if (id.isArrayIndex()) {
        uint index = id.asArrayIndex();
        if ((int)index < r->d()->d->children.count()) {
            if (hasProperty)
                *hasProperty = true;
            return Node::create(v4, r->d()->d->children.at(index));
        }
        if (hasProperty)
            *hasProperty = false;
        return Encode::undefined();
    }

    if (id == v4->id_length()->propertyKey())
        return Value::fromInt32(r->d()->d->children.count()).asReturnedValue();
    return Object::virtualGet(m, id, receiver, hasProperty);
}

ReturnedValue NodeList::create(ExecutionEngine *v4, NodeImpl *data)
{
    return (v4->memoryManager->allocate<NodeList>(data))->asReturnedValue();
}

ReturnedValue Document::method_documentElement(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r || r->d()->d->type != NodeImpl::Document)
        RETURN_UNDEFINED();

    return Node::create(scope.engine, static_cast<DocumentImpl *>(r->d()->d)->root);
}

ReturnedValue Document::method_xmlStandalone(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r || r->d()->d->type != NodeImpl::Document)
        RETURN_UNDEFINED();

    return Encode(static_cast<DocumentImpl *>(r->d()->d)->isStandalone);
}

ReturnedValue Document::method_xmlVersion(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r || r->d()->d->type != NodeImpl::Document)
        RETURN_UNDEFINED();

    return Encode(scope.engine->newString(static_cast<DocumentImpl *>(r->d()->d)->version));
}

ReturnedValue Document::method_xmlEncoding(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<Node> r(scope, thisObject->as<Node>());
    if (!r || r->d()->d->type != NodeImpl::Document)
        RETURN_UNDEFINED();

    return Encode(scope.engine->newString(static_cast<DocumentImpl *>(r->d()->d)->encoding));
}

class QQmlXMLHttpRequest : public QObject
{
    Q_OBJECT
public:
    enum LoadType {
        AsynchronousLoad,
        SynchronousLoad
    };
    enum State { Unsent = 0,
                 Opened = 1, HeadersReceived = 2,
                 Loading = 3, Done = 4 };

    QQmlXMLHttpRequest(QNetworkAccessManager *manager, QV4::ExecutionEngine *v4);
    virtual ~QQmlXMLHttpRequest();

    bool sendFlag() const;
    bool errorFlag() const;
    quint32 readyState() const;
    int replyStatus() const;
    QString replyStatusText() const;

    ReturnedValue open(Object *thisObject, const QString &, const QUrl &, LoadType);
    ReturnedValue send(Object *thisObject, QQmlContextData *context, const QByteArray &);
    ReturnedValue abort(Object *thisObject);

    void addHeader(const QString &, const QString &);
    QString header(const QString &name) const;
    QString headers() const;

    QString responseBody();
    const QByteArray & rawResponseBody() const;
    bool receivedXml() const;

    const QString & responseType() const;
    void setResponseType(const QString &);

    QV4::ReturnedValue jsonResponseBody(QV4::ExecutionEngine*);
    QV4::ReturnedValue xmlResponseBody(QV4::ExecutionEngine*);
private slots:
    void readyRead();
    void error(QNetworkReply::NetworkError);
    void finished();

private:
    void requestFromUrl(const QUrl &url);

    State m_state;
    bool m_errorFlag;
    bool m_sendFlag;
    QString m_method;
    QUrl m_url;
    QByteArray m_responseEntityBody;
    QByteArray m_data;
    int m_redirectCount;

    typedef QPair<QByteArray, QByteArray> HeaderPair;
    typedef QList<HeaderPair> HeadersList;
    HeadersList m_headersList;
    void fillHeadersList();

    bool m_gotXml;
    QByteArray m_mime;
    QByteArray m_charset;
    QTextCodec *m_textCodec;
#if QT_CONFIG(textcodec)
    QTextCodec* findTextCodec() const;
#endif
    void readEncoding();

    PersistentValue m_thisObject;
    QQmlContextDataRef m_qmlContext;
    bool m_wasConstructedWithQmlContext = true;

    void dispatchCallbackNow(Object *thisObj);
    static void dispatchCallbackNow(Object *thisObj, bool done, bool error);
    void dispatchCallbackSafely();

    int m_status;
    QString m_statusText;
    QNetworkRequest m_request;
    QStringList m_addedHeaders;
    QPointer<QNetworkReply> m_network;
    void destroyNetwork();

    QNetworkAccessManager *m_nam;
    QNetworkAccessManager *networkAccessManager() { return m_nam; }

    QString m_responseType;
    QV4::PersistentValue m_parsedDocument;
};

QQmlXMLHttpRequest::QQmlXMLHttpRequest(QNetworkAccessManager *manager, QV4::ExecutionEngine *v4)
    : m_state(Unsent), m_errorFlag(false), m_sendFlag(false)
    , m_redirectCount(0), m_gotXml(false), m_textCodec(nullptr), m_network(nullptr), m_nam(manager)
    , m_responseType()
    , m_parsedDocument()
{
    m_wasConstructedWithQmlContext = v4->callingQmlContext() != nullptr;
}

QQmlXMLHttpRequest::~QQmlXMLHttpRequest()
{
    destroyNetwork();
}

bool QQmlXMLHttpRequest::sendFlag() const
{
    return m_sendFlag;
}

bool QQmlXMLHttpRequest::errorFlag() const
{
    return m_errorFlag;
}

quint32 QQmlXMLHttpRequest::readyState() const
{
    return m_state;
}

int QQmlXMLHttpRequest::replyStatus() const
{
    return m_status;
}

QString QQmlXMLHttpRequest::replyStatusText() const
{
    return m_statusText;
}

ReturnedValue QQmlXMLHttpRequest::open(Object *thisObject, const QString &method, const QUrl &url, LoadType loadType)
{
    destroyNetwork();
    m_sendFlag = false;
    m_errorFlag = false;
    m_responseEntityBody = QByteArray();
    m_method = method;
    m_url = url;
    m_request.setAttribute(QNetworkRequest::SynchronousRequestAttribute, loadType == SynchronousLoad);
    m_state = Opened;
    m_addedHeaders.clear();
    dispatchCallbackNow(thisObject);
    return Encode::undefined();
}

void QQmlXMLHttpRequest::addHeader(const QString &name, const QString &value)
{
    QByteArray utfname = name.toUtf8();

    if (m_addedHeaders.contains(name, Qt::CaseInsensitive)) {
        m_request.setRawHeader(utfname, m_request.rawHeader(utfname) + ',' + value.toUtf8());
    } else {
        m_request.setRawHeader(utfname, value.toUtf8());
        m_addedHeaders.append(name);
    }
}

QString QQmlXMLHttpRequest::header(const QString &name) const
{
    if (!m_headersList.isEmpty()) {
        const QByteArray utfname = name.toLower().toUtf8();
        for (const HeaderPair &header : m_headersList) {
            if (header.first == utfname)
                return QString::fromUtf8(header.second);
        }
    }
    return QString();
}

QString QQmlXMLHttpRequest::headers() const
{
    QString ret;

    for (const HeaderPair &header : m_headersList) {
        if (ret.length())
            ret.append(QLatin1String("\r\n"));
        ret += QString::fromUtf8(header.first) + QLatin1String(": ")
             + QString::fromUtf8(header.second);
    }
    return ret;
}

void QQmlXMLHttpRequest::fillHeadersList()
{
    const QList<QByteArray> headerList = m_network->rawHeaderList();

    m_headersList.clear();
    for (const QByteArray &header : headerList) {
        HeaderPair pair (header.toLower(), m_network->rawHeader(header));
        if (pair.first == "set-cookie" ||
            pair.first == "set-cookie2")
            continue;

        m_headersList << pair;
    }
}

void QQmlXMLHttpRequest::requestFromUrl(const QUrl &url)
{
    QNetworkRequest request = m_request;
    request.setUrl(url);
    if(m_method == QLatin1String("POST") ||
       m_method == QLatin1String("PUT")) {
        QVariant var = request.header(QNetworkRequest::ContentTypeHeader);
        if (var.isValid()) {
            QString str = var.toString();
            int charsetIdx = str.indexOf(QLatin1String("charset="));
            if (charsetIdx == -1) {
                // No charset - append
                if (!str.isEmpty()) str.append(QLatin1Char(';'));
                str.append(QLatin1String("charset=UTF-8"));
            } else {
                charsetIdx += 8;
                int n = 0;
                int semiColon = str.indexOf(QLatin1Char(';'), charsetIdx);
                if (semiColon == -1) {
                    n = str.length() - charsetIdx;
                } else {
                    n = semiColon - charsetIdx;
                }

                str.replace(charsetIdx, n, QLatin1String("UTF-8"));
            }
            request.setHeader(QNetworkRequest::ContentTypeHeader, str);
        } else {
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                              QLatin1String("text/plain;charset=UTF-8"));
        }
    }

    if (xhrDump()) {
        qWarning().nospace() << "XMLHttpRequest: " << qPrintable(m_method) << ' ' << qPrintable(url.toString());
        if (!m_data.isEmpty()) {
            qWarning().nospace() << "                "
                                 << qPrintable(QString::fromUtf8(m_data));
        }
    }

    if (m_method == QLatin1String("GET")) {
        m_network = networkAccessManager()->get(request);
    } else if (m_method == QLatin1String("HEAD")) {
        m_network = networkAccessManager()->head(request);
    } else if (m_method == QLatin1String("POST")) {
        m_network = networkAccessManager()->post(request, m_data);
    } else if (m_method == QLatin1String("PUT")) {
        m_network = networkAccessManager()->put(request, m_data);
    } else if (m_method == QLatin1String("DELETE")) {
        m_network = networkAccessManager()->deleteResource(request);
    } else if ((m_method == QLatin1String("OPTIONS")) ||
               m_method == QLatin1String("PROPFIND") ||
               m_method == QLatin1String("PATCH")) {
        QBuffer *buffer = new QBuffer;
        buffer->setData(m_data);
        buffer->open(QIODevice::ReadOnly);
        m_network = networkAccessManager()->sendCustomRequest(request, QByteArray(m_method.toUtf8().constData()), buffer);
        buffer->setParent(m_network);
    }

    if (m_request.attribute(QNetworkRequest::SynchronousRequestAttribute).toBool()) {
        if (m_network->bytesAvailable() > 0)
            readyRead();

        QNetworkReply::NetworkError networkError = m_network->error();
        if (networkError != QNetworkReply::NoError) {
            error(networkError);
        } else {
            finished();
        }
    } else {
        QObject::connect(m_network, SIGNAL(readyRead()),
                         this, SLOT(readyRead()));
        QObject::connect(m_network, SIGNAL(error(QNetworkReply::NetworkError)),
                         this, SLOT(error(QNetworkReply::NetworkError)));
        QObject::connect(m_network, SIGNAL(finished()),
                         this, SLOT(finished()));
    }
}

ReturnedValue QQmlXMLHttpRequest::send(Object *thisObject, QQmlContextData *context, const QByteArray &data)
{
    m_errorFlag = false;
    m_sendFlag = true;
    m_redirectCount = 0;
    m_data = data;

    m_thisObject = thisObject;
    m_qmlContext = context;

    requestFromUrl(m_url);

    return Encode::undefined();
}

ReturnedValue QQmlXMLHttpRequest::abort(Object *thisObject)
{
    destroyNetwork();
    m_responseEntityBody = QByteArray();
    m_errorFlag = true;
    m_request = QNetworkRequest();

    if (!(m_state == Unsent ||
          (m_state == Opened && !m_sendFlag) ||
          m_state == Done)) {

        m_state = Done;
        m_sendFlag = false;
        dispatchCallbackNow(thisObject);
    }

    m_state = Unsent;

    return Encode::undefined();
}

void QQmlXMLHttpRequest::readyRead()
{
    m_status =
        m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    m_statusText =
        QString::fromUtf8(m_network->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray());

    // ### We assume if this is called the headers are now available
    if (m_state < HeadersReceived) {
        m_state = HeadersReceived;
        fillHeadersList ();
        dispatchCallbackSafely();
    }

    bool wasEmpty = m_responseEntityBody.isEmpty();
    m_responseEntityBody.append(m_network->readAll());
    if (wasEmpty && !m_responseEntityBody.isEmpty())
        m_state = Loading;

    dispatchCallbackSafely();
}

static const char *errorToString(QNetworkReply::NetworkError error)
{
    int idx = QNetworkReply::staticMetaObject.indexOfEnumerator("NetworkError");
    if (idx == -1) return "EnumLookupFailed";

    QMetaEnum e = QNetworkReply::staticMetaObject.enumerator(idx);

    const char *name = e.valueToKey(error);
    if (!name) return "EnumLookupFailed";
    else return name;
}

void QQmlXMLHttpRequest::error(QNetworkReply::NetworkError error)
{
    m_status =
        m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    m_statusText =
        QString::fromUtf8(m_network->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray());

    m_request = QNetworkRequest();
    m_data.clear();
    destroyNetwork();

    if (xhrDump()) {
        qWarning().nospace() << "XMLHttpRequest: ERROR " << qPrintable(m_url.toString());
        qWarning().nospace() << "    " << error << ' ' << errorToString(error) << ' ' << m_statusText;
    }

    if (error == QNetworkReply::ContentAccessDenied ||
        error == QNetworkReply::ContentOperationNotPermittedError ||
        error == QNetworkReply::ContentNotFoundError ||
        error == QNetworkReply::AuthenticationRequiredError ||
        error == QNetworkReply::ContentReSendError ||
        error == QNetworkReply::UnknownContentError ||
        error == QNetworkReply::ProtocolInvalidOperationError ||
        error == QNetworkReply::InternalServerError ||
        error == QNetworkReply::OperationNotImplementedError ||
        error == QNetworkReply::ServiceUnavailableError ||
        error == QNetworkReply::UnknownServerError) {
        m_state = Loading;
        dispatchCallbackSafely();
    } else {
        m_errorFlag = true;
        m_responseEntityBody = QByteArray();
    }

    m_state = Done;
    dispatchCallbackSafely();
}

#define XMLHTTPREQUEST_MAXIMUM_REDIRECT_RECURSION 15
void QQmlXMLHttpRequest::finished()
{
    m_redirectCount++;
    if (m_redirectCount < XMLHTTPREQUEST_MAXIMUM_REDIRECT_RECURSION) {
        QVariant redirect = m_network->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (redirect.isValid()) {
            QUrl url = m_network->url().resolved(redirect.toUrl());
            if (url.scheme() != QLatin1String("file")) {
                // See http://www.ietf.org/rfc/rfc2616.txt, section 10.3.4 "303 See Other":
                // Result of 303 redirection should be a new "GET" request.
                const QVariant code = m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                if (code.isValid() && code.toInt() == 303 && m_method != QLatin1String("GET"))
                    m_method = QStringLiteral("GET");
                destroyNetwork();
                requestFromUrl(url);
                return;
            }
        }
    }

    m_status =
        m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    m_statusText =
        QString::fromUtf8(m_network->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray());

    if (m_state < HeadersReceived) {
        m_state = HeadersReceived;
        fillHeadersList ();
        dispatchCallbackSafely();
    }
    m_responseEntityBody.append(m_network->readAll());
    readEncoding();

    if (xhrDump()) {
        qWarning().nospace() << "XMLHttpRequest: RESPONSE " << qPrintable(m_url.toString());
        if (!m_responseEntityBody.isEmpty()) {
            qWarning().nospace() << "                "
                                 << qPrintable(QString::fromUtf8(m_responseEntityBody));
        }
    }

    m_data.clear();
    destroyNetwork();
    if (m_state < Loading) {
        m_state = Loading;
        dispatchCallbackSafely();
    }
    m_state = Done;

    dispatchCallbackSafely();

    m_thisObject.clear();
    m_qmlContext.setContextData(nullptr);
}


void QQmlXMLHttpRequest::readEncoding()
{
    for (const HeaderPair &header : qAsConst(m_headersList)) {
        if (header.first == "content-type") {
            int separatorIdx = header.second.indexOf(';');
            if (separatorIdx == -1) {
                m_mime = header.second;
            } else {
                m_mime = header.second.mid(0, separatorIdx);
                int charsetIdx = header.second.indexOf("charset=");
                if (charsetIdx != -1) {
                    charsetIdx += 8;
                    separatorIdx = header.second.indexOf(';', charsetIdx);
                    m_charset = header.second.mid(charsetIdx, separatorIdx >= 0 ? separatorIdx : header.second.length());
                }
            }
            break;
        }
    }

    if (m_mime.isEmpty() || m_mime == "text/xml" || m_mime == "application/xml" || m_mime.endsWith("+xml"))
        m_gotXml = true;
}

bool QQmlXMLHttpRequest::receivedXml() const
{
    return m_gotXml;
}

const QString & QQmlXMLHttpRequest::responseType() const
{
    return m_responseType;
}

void QQmlXMLHttpRequest::setResponseType(const QString &responseType)
{
    m_responseType = responseType;
}

QV4::ReturnedValue QQmlXMLHttpRequest::jsonResponseBody(QV4::ExecutionEngine* engine)
{
    if (m_parsedDocument.isEmpty()) {
        Scope scope(engine);

        QJsonParseError error;
        const QString& jtext = responseBody();
        JsonParser parser(scope.engine, jtext.constData(), jtext.length());
        ScopedValue jsonObject(scope, parser.parse(&error));
        if (error.error != QJsonParseError::NoError)
            return engine->throwSyntaxError(QStringLiteral("JSON.parse: Parse error"));

        m_parsedDocument.set(scope.engine, jsonObject);
    }

    return m_parsedDocument.value();
}

QV4::ReturnedValue QQmlXMLHttpRequest::xmlResponseBody(QV4::ExecutionEngine* engine)
{
    if (m_parsedDocument.isEmpty()) {
        m_parsedDocument.set(engine, Document::load(engine, rawResponseBody()));
    }

    return m_parsedDocument.value();
}

#if QT_CONFIG(textcodec)
QTextCodec* QQmlXMLHttpRequest::findTextCodec() const
{
    QTextCodec *codec = nullptr;

    if (!m_charset.isEmpty())
        codec = QTextCodec::codecForName(m_charset);

    if (!codec && m_gotXml) {
        QXmlStreamReader reader(m_responseEntityBody);
        reader.readNext();
        codec = QTextCodec::codecForName(reader.documentEncoding().toString().toUtf8());
    }

    if (!codec && m_mime == "text/html")
        codec = QTextCodec::codecForHtml(m_responseEntityBody, nullptr);

    if (!codec)
        codec = QTextCodec::codecForUtfText(m_responseEntityBody, nullptr);

    if (!codec)
        codec = QTextCodec::codecForName("UTF-8");
    return codec;
}
#endif


QString QQmlXMLHttpRequest::responseBody()
{
#if QT_CONFIG(textcodec)
    if (!m_textCodec)
        m_textCodec = findTextCodec();
    if (m_textCodec)
        return m_textCodec->toUnicode(m_responseEntityBody);
#endif

    return QString::fromUtf8(m_responseEntityBody);
}

const QByteArray &QQmlXMLHttpRequest::rawResponseBody() const
{
    return m_responseEntityBody;
}

void QQmlXMLHttpRequest::dispatchCallbackNow(Object *thisObj)
{
    dispatchCallbackNow(thisObj, m_state == Done, m_errorFlag);
}

void QQmlXMLHttpRequest::dispatchCallbackNow(Object *thisObj, bool done, bool error)
{
    Q_ASSERT(thisObj);

    const auto dispatch = [thisObj](const QString &eventName) {
        QV4::Scope scope(thisObj->engine());
        ScopedString s(scope, scope.engine->newString(eventName));
        ScopedFunctionObject callback(scope, thisObj->get(s));
        // not an error, but no event handler to call.
        if (!callback)
            return;

        QV4::JSCallData jsCallData(scope);
        callback->call(jsCallData);

        if (scope.engine->hasException) {
            QQmlError error = scope.engine->catchExceptionAsQmlError();
            QQmlEnginePrivate *qmlEnginePrivate = scope.engine->qmlEngine() ? QQmlEnginePrivate::get(scope.engine->qmlEngine()) : nullptr;
            QQmlEnginePrivate::warning(qmlEnginePrivate, error);
        }
    };

    dispatch(QStringLiteral("onreadystatechange"));
    if (done) {
        if (error)
            dispatch(QStringLiteral("onerror"));
        else
            dispatch(QStringLiteral("onload"));
        dispatch(QStringLiteral("onloadend"));
    }
}

void QQmlXMLHttpRequest::dispatchCallbackSafely()
{
    if (m_wasConstructedWithQmlContext && !m_qmlContext.contextData())
        // if the calling context object is no longer valid, then it has been
        // deleted explicitly (e.g., by a Loader deleting the itemContext when
        // the source is changed).  We do nothing in this case, as the evaluation
        // cannot succeed.
        return;

    dispatchCallbackNow(m_thisObject.as<Object>());
}

void QQmlXMLHttpRequest::destroyNetwork()
{
    if (m_network) {
        m_network->disconnect();
        m_network->deleteLater();
        m_network = nullptr;
    }
}

namespace QV4 {
namespace Heap {

struct QQmlXMLHttpRequestWrapper : Object {
    void init(QQmlXMLHttpRequest *request) {
        Object::init();
        this->request = request;
    }

    void destroy() {
        delete request;
        Object::destroy();
    }
    QQmlXMLHttpRequest *request;
};

#define QQmlXMLHttpRequestCtorMembers(class, Member) \
    Member(class, Pointer, Object *, proto)

DECLARE_HEAP_OBJECT(QQmlXMLHttpRequestCtor, FunctionObject) {
    DECLARE_MARKOBJECTS(QQmlXMLHttpRequestCtor);
    void init(ExecutionEngine *engine);
};

}

struct QQmlXMLHttpRequestWrapper : public Object
{
    V4_OBJECT2(QQmlXMLHttpRequestWrapper, Object)
    V4_NEEDS_DESTROY
};

struct QQmlXMLHttpRequestCtor : public FunctionObject
{
    V4_OBJECT2(QQmlXMLHttpRequestCtor, FunctionObject)

    static ReturnedValue virtualCallAsConstructor(const FunctionObject *f, const Value *, int, const Value *)
    {
        Scope scope(f->engine());
        const QQmlXMLHttpRequestCtor *ctor = static_cast<const QQmlXMLHttpRequestCtor *>(f);

        QQmlXMLHttpRequest *r = new QQmlXMLHttpRequest(scope.engine->v8Engine->networkAccessManager(), scope.engine);
        Scoped<QQmlXMLHttpRequestWrapper> w(scope, scope.engine->memoryManager->allocate<QQmlXMLHttpRequestWrapper>(r));
        ScopedObject proto(scope, ctor->d()->proto);
        w->setPrototypeUnchecked(proto);
        return w.asReturnedValue();
    }

    static ReturnedValue virtualCall(const FunctionObject *, const Value *, const Value *, int) {
        return Encode::undefined();
    }

    void setupProto();

    static ReturnedValue method_open(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_setRequestHeader(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_send(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_abort(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_getResponseHeader(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_getAllResponseHeaders(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);

    static ReturnedValue method_get_readyState(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_status(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_statusText(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_responseText(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_responseXML(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_response(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_get_responseType(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
    static ReturnedValue method_set_responseType(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc);
};

}

DEFINE_OBJECT_VTABLE(QQmlXMLHttpRequestWrapper);

void Heap::QQmlXMLHttpRequestCtor::init(ExecutionEngine *engine)
{
    Heap::FunctionObject::init(engine->rootContext(), QStringLiteral("XMLHttpRequest"));
    Scope scope(engine);
    Scoped<QV4::QQmlXMLHttpRequestCtor> ctor(scope, this);

    ctor->defineReadonlyProperty(QStringLiteral("UNSENT"), Value::fromInt32(0));
    ctor->defineReadonlyProperty(QStringLiteral("OPENED"), Value::fromInt32(1));
    ctor->defineReadonlyProperty(QStringLiteral("HEADERS_RECEIVED"), Value::fromInt32(2));
    ctor->defineReadonlyProperty(QStringLiteral("LOADING"), Value::fromInt32(3));
    ctor->defineReadonlyProperty(QStringLiteral("DONE"), Value::fromInt32(4));
    if (!ctor->d()->proto)
        ctor->setupProto();
    ScopedString s(scope, engine->id_prototype());
    ctor->defineDefaultProperty(s, ScopedObject(scope, ctor->d()->proto));
}

DEFINE_OBJECT_VTABLE(QQmlXMLHttpRequestCtor);

void QQmlXMLHttpRequestCtor::setupProto()
{
    ExecutionEngine *v4 = engine();
    Scope scope(v4);
    ScopedObject p(scope, v4->newObject());
    d()->proto.set(scope.engine, p->d());

    // Methods
    p->defineDefaultProperty(QStringLiteral("open"), method_open);
    p->defineDefaultProperty(QStringLiteral("setRequestHeader"), method_setRequestHeader);
    p->defineDefaultProperty(QStringLiteral("send"), method_send);
    p->defineDefaultProperty(QStringLiteral("abort"), method_abort);
    p->defineDefaultProperty(QStringLiteral("getResponseHeader"), method_getResponseHeader);
    p->defineDefaultProperty(QStringLiteral("getAllResponseHeaders"), method_getAllResponseHeaders);

    // Read-only properties
    p->defineAccessorProperty(QStringLiteral("readyState"), method_get_readyState, nullptr);
    p->defineAccessorProperty(QStringLiteral("status"),method_get_status, nullptr);
    p->defineAccessorProperty(QStringLiteral("statusText"),method_get_statusText, nullptr);
    p->defineAccessorProperty(QStringLiteral("responseText"),method_get_responseText, nullptr);
    p->defineAccessorProperty(QStringLiteral("responseXML"),method_get_responseXML, nullptr);
    p->defineAccessorProperty(QStringLiteral("response"),method_get_response, nullptr);

    // Read-write properties
    p->defineAccessorProperty(QStringLiteral("responseType"), method_get_responseType, method_set_responseType);

    // State values
    p->defineReadonlyProperty(QStringLiteral("UNSENT"), Value::fromInt32(0));
    p->defineReadonlyProperty(QStringLiteral("OPENED"), Value::fromInt32(1));
    p->defineReadonlyProperty(QStringLiteral("HEADERS_RECEIVED"), Value::fromInt32(2));
    p->defineReadonlyProperty(QStringLiteral("LOADING"), Value::fromInt32(3));
    p->defineReadonlyProperty(QStringLiteral("DONE"), Value::fromInt32(4));
}


// XMLHttpRequest methods
ReturnedValue QQmlXMLHttpRequestCtor::method_open(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (argc < 2 || argc > 5)
        THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Incorrect argument count");

    // Argument 0 - Method
    QString method = argv[0].toQStringNoThrow().toUpper();
    if (method != QLatin1String("GET") &&
        method != QLatin1String("PUT") &&
        method != QLatin1String("HEAD") &&
        method != QLatin1String("POST") &&
        method != QLatin1String("DELETE") &&
        method != QLatin1String("OPTIONS") &&
        method != QLatin1String("PROPFIND") &&
        method != QLatin1String("PATCH"))
        THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Unsupported HTTP method type");

    // Argument 1 - URL
    QUrl url = QUrl(argv[1].toQStringNoThrow());

    if (url.isRelative()) {
        QQmlContextData *qmlContextData = scope.engine->callingQmlContext();
        if (qmlContextData)
            url = qmlContextData->resolvedUrl(url);
        else
            url = scope.engine->resolvedUrl(url.url());
    }

    bool async = true;
    // Argument 2 - async (optional)
    if (argc > 2) {
        async = argv[2].booleanValue();
    }

    // Argument 3/4 - user/pass (optional)
    QString username, password;
    if (argc > 3)
        username = argv[3].toQStringNoThrow();
    if (argc > 4)
        password = argv[4].toQStringNoThrow();

    // Clear the fragment (if any)
    url.setFragment(QString());

    // Set username/password
    if (!username.isNull()) url.setUserName(username);
    if (!password.isNull()) url.setPassword(password);

    return r->open(w, method, url, async ? QQmlXMLHttpRequest::AsynchronousLoad : QQmlXMLHttpRequest::SynchronousLoad);
}

ReturnedValue QQmlXMLHttpRequestCtor::method_setRequestHeader(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (argc != 2)
        THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Incorrect argument count");

    if (r->readyState() != QQmlXMLHttpRequest::Opened || r->sendFlag())
        THROW_DOM(DOMEXCEPTION_INVALID_STATE_ERR, "Invalid state");

    QString name = argv[0].toQStringNoThrow();
    QString value = argv[1].toQStringNoThrow();

    // ### Check that name and value are well formed

    QString nameUpper = name.toUpper();
    if (nameUpper == QLatin1String("ACCEPT-CHARSET") ||
        nameUpper == QLatin1String("ACCEPT-ENCODING") ||
        nameUpper == QLatin1String("CONNECTION") ||
        nameUpper == QLatin1String("CONTENT-LENGTH") ||
        nameUpper == QLatin1String("COOKIE") ||
        nameUpper == QLatin1String("COOKIE2") ||
        nameUpper == QLatin1String("CONTENT-TRANSFER-ENCODING") ||
        nameUpper == QLatin1String("DATE") ||
        nameUpper == QLatin1String("EXPECT") ||
        nameUpper == QLatin1String("HOST") ||
        nameUpper == QLatin1String("KEEP-ALIVE") ||
        nameUpper == QLatin1String("REFERER") ||
        nameUpper == QLatin1String("TE") ||
        nameUpper == QLatin1String("TRAILER") ||
        nameUpper == QLatin1String("TRANSFER-ENCODING") ||
        nameUpper == QLatin1String("UPGRADE") ||
        nameUpper == QLatin1String("USER-AGENT") ||
        nameUpper == QLatin1String("VIA") ||
        nameUpper.startsWith(QLatin1String("PROXY-")) ||
        nameUpper.startsWith(QLatin1String("SEC-")))
        RETURN_UNDEFINED();

    r->addHeader(name, value);

    RETURN_UNDEFINED();
}

ReturnedValue QQmlXMLHttpRequestCtor::method_send(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (r->readyState() != QQmlXMLHttpRequest::Opened ||
        r->sendFlag())
        THROW_DOM(DOMEXCEPTION_INVALID_STATE_ERR, "Invalid state");

    QByteArray data;
    if (argc > 0) {
        if (const ArrayBuffer *buffer = argv[0].as<ArrayBuffer>()) {
            data = buffer->asByteArray();
        } else {
            data = argv[0].toQStringNoThrow().toUtf8();
        }
    }

    return r->send(w, scope.engine->callingQmlContext(), data);
}

ReturnedValue QQmlXMLHttpRequestCtor::method_abort(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    return r->abort(w);
}

ReturnedValue QQmlXMLHttpRequestCtor::method_getResponseHeader(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (argc != 1)
        THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Incorrect argument count");

    if (r->readyState() != QQmlXMLHttpRequest::Loading &&
        r->readyState() != QQmlXMLHttpRequest::Done &&
        r->readyState() != QQmlXMLHttpRequest::HeadersReceived)
        THROW_DOM(DOMEXCEPTION_INVALID_STATE_ERR, "Invalid state");

    return Encode(scope.engine->newString(r->header(argv[0].toQStringNoThrow())));
}

ReturnedValue QQmlXMLHttpRequestCtor::method_getAllResponseHeaders(const FunctionObject *b, const Value *thisObject, const Value *, int argc)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (argc != 0)
        THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Incorrect argument count");

    if (r->readyState() != QQmlXMLHttpRequest::Loading &&
        r->readyState() != QQmlXMLHttpRequest::Done &&
        r->readyState() != QQmlXMLHttpRequest::HeadersReceived)
        THROW_DOM(DOMEXCEPTION_INVALID_STATE_ERR, "Invalid state");

    return Encode(scope.engine->newString(r->headers()));
}

// XMLHttpRequest properties
ReturnedValue QQmlXMLHttpRequestCtor::method_get_readyState(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    return Encode(r->readyState());
}

ReturnedValue QQmlXMLHttpRequestCtor::method_get_status(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (r->readyState() == QQmlXMLHttpRequest::Unsent ||
        r->readyState() == QQmlXMLHttpRequest::Opened)
        THROW_DOM(DOMEXCEPTION_INVALID_STATE_ERR, "Invalid state");

    if (r->errorFlag())
        return Encode(0);
    else
        return Encode(r->replyStatus());
}

ReturnedValue QQmlXMLHttpRequestCtor::method_get_statusText(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (r->readyState() == QQmlXMLHttpRequest::Unsent ||
        r->readyState() == QQmlXMLHttpRequest::Opened)
        THROW_DOM(DOMEXCEPTION_INVALID_STATE_ERR, "Invalid state");

    if (r->errorFlag())
        return Encode(scope.engine->newString(QString()));
    else
        return Encode(scope.engine->newString(r->replyStatusText()));
}

ReturnedValue QQmlXMLHttpRequestCtor::method_get_responseText(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (r->readyState() != QQmlXMLHttpRequest::Loading &&
        r->readyState() != QQmlXMLHttpRequest::Done)
        return Encode(scope.engine->newString(QString()));
    else
        return Encode(scope.engine->newString(r->responseBody()));
}

ReturnedValue QQmlXMLHttpRequestCtor::method_get_responseXML(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (!r->receivedXml() ||
        (r->readyState() != QQmlXMLHttpRequest::Loading &&
         r->readyState() != QQmlXMLHttpRequest::Done)) {
        return Encode::null();
    } else {
        if (r->responseType().isEmpty())
            r->setResponseType(QLatin1String("document"));
        return r->xmlResponseBody(scope.engine);
    }
}

ReturnedValue QQmlXMLHttpRequestCtor::method_get_response(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (r->readyState() != QQmlXMLHttpRequest::Loading &&
            r->readyState() != QQmlXMLHttpRequest::Done)
        RETURN_RESULT(scope.engine->newString(QString()));

    const QString& responseType = r->responseType();
    if (responseType.compare(QLatin1String("text"), Qt::CaseInsensitive) == 0 || responseType.isEmpty()) {
        RETURN_RESULT(scope.engine->newString(r->responseBody()));
    } else if (responseType.compare(QLatin1String("arraybuffer"), Qt::CaseInsensitive) == 0) {
        RETURN_RESULT(scope.engine->newArrayBuffer(r->rawResponseBody()));
    } else if (responseType.compare(QLatin1String("json"), Qt::CaseInsensitive) == 0) {
        RETURN_RESULT(r->jsonResponseBody(scope.engine));
    } else if (responseType.compare(QLatin1String("document"), Qt::CaseInsensitive) == 0) {
        RETURN_RESULT(r->xmlResponseBody(scope.engine));
    } else {
        RETURN_RESULT(scope.engine->newString(QString()));
    }
}


ReturnedValue QQmlXMLHttpRequestCtor::method_get_responseType(const FunctionObject *b, const Value *thisObject, const Value *, int)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;
    return Encode(scope.engine->newString(r->responseType()));
}

ReturnedValue QQmlXMLHttpRequestCtor::method_set_responseType(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    Scoped<QQmlXMLHttpRequestWrapper> w(scope, thisObject->as<QQmlXMLHttpRequestWrapper>());
    if (!w)
        V4THROW_REFERENCE("Not an XMLHttpRequest object");
    QQmlXMLHttpRequest *r = w->d()->request;

    if (argc < 1)
        THROW_DOM(DOMEXCEPTION_SYNTAX_ERR, "Incorrect argument count");

    // Argument 0 - response type
    r->setResponseType(argv[0].toQStringNoThrow());

    return Encode::undefined();
}

void qt_rem_qmlxmlhttprequest(ExecutionEngine * /* engine */, void *d)
{
    QQmlXMLHttpRequestData *data = (QQmlXMLHttpRequestData *)d;
    delete data;
}

void *qt_add_qmlxmlhttprequest(ExecutionEngine *v4)
{
    Scope scope(v4);

    Scoped<QQmlXMLHttpRequestCtor> ctor(scope, v4->memoryManager->allocate<QQmlXMLHttpRequestCtor>(v4));
    ScopedString s(scope, v4->newString(QStringLiteral("XMLHttpRequest")));
    v4->globalObject->defineReadonlyProperty(s, ctor);

    QQmlXMLHttpRequestData *data = new QQmlXMLHttpRequestData;
    return data;
}

QT_END_NAMESPACE

#endif // xmlstreamreader && qml_network

#include <qqmlxmlhttprequest.moc>
