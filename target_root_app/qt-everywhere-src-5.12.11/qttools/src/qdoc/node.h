/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef NODE_H
#define NODE_H

#include <qdir.h>
#include <qmap.h>
#include <qpair.h>
#include <qstringlist.h>
#include <qvector.h>
#include "codechunk.h"
#include "doc.h"

QT_BEGIN_NAMESPACE

class Node;
class Tree;
class EnumNode;
class ClassNode;
class Aggregate;
class ExampleNode;
class TypedefNode;
class QmlTypeNode;
class QDocDatabase;
class FunctionNode;
class PropertyNode;
class CollectionNode;
class QmlPropertyNode;
class SharedCommentNode;

typedef QList<Node*> NodeList;
typedef QVector<Node*> NodeVector;
typedef QList<PropertyNode*> PropNodeList;
typedef QMap<QString, Node*> NodeMap;
typedef QMultiMap<QString, Node*> NodeMultiMap;
typedef QPair<int, int> NodeTypePair;
typedef QList<NodeTypePair> NodeTypeList;
typedef QMap<QString, CollectionNode*> CNMap;
typedef QMultiMap<QString, CollectionNode*> CNMultiMap;

class Node
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::Node)

public:
    enum NodeType {
        NoType,
        Namespace,
        Class,
        Document,
        Enum,
        Typedef,
        Function,
        Property,
        Variable,
        Group,
        Module,
        QmlType,
        QmlModule,
        QmlPropertyGroup,
        QmlProperty,
        QmlSignal,
        QmlSignalHandler,
        QmlMethod,
        QmlBasicType,
        SharedComment,
        LastType
    };

    enum DocSubtype {
        NoSubtype,
        Attribution,
        Example,
        HeaderFile,
        File,
        Image,
        Page,
        ExternalPage,
        DitaMap,
        LastSubtype
    };

    enum Genus { DontCare, CPP, JS, QML, DOC };

    enum Access { Public, Protected, Private };

    enum Status {
        Obsolete,
        Deprecated,
        Preliminary,
        Active,
        Internal
    }; // don't reorder this enum

    enum ThreadSafeness {
        UnspecifiedSafeness,
        NonReentrant,
        Reentrant,
        ThreadSafe
    };

    enum LinkType {
        StartLink,
        NextLink,
        PreviousLink,
        ContentsLink,
        IndexLink
    };

    enum PageType {
        NoPageType,
        AttributionPage,
        ApiPage,
        ArticlePage,
        ExamplePage,
        HowToPage,
        OverviewPage,
        TutorialPage,
        FAQPage,
        DitaMapPage,
        OnBeyondZebra
    };

    enum FlagValue {
        FlagValueDefault = -1,
        FlagValueFalse = 0,
        FlagValueTrue = 1
    };

    virtual ~Node();

    QString plainName() const;
    QString plainFullName(const Node* relative = 0) const;
    QString plainSignature() const;
    QString fullName(const Node* relative=0) const;
    virtual QString signature(bool ,  bool ) const { return plainName(); }

    const QString& fileNameBase() const { return fileNameBase_; }
    bool hasFileNameBase() const { return !fileNameBase_.isEmpty(); }
    void setFileNameBase(const QString& t) { fileNameBase_ = t; }
    Node::Genus genus() const { return (Genus) genus_; }
    void setGenus(Genus t) { genus_ = (unsigned char) t; }

    void setAccess(Access t) { access_ = (unsigned char) t; }
    void setLocation(const Location& t);
    void setDoc(const Doc& doc, bool replace = false);
    void setStatus(Status t) {
        if (status_ == (unsigned char) Obsolete && t == Deprecated)
            return;
        status_ = (unsigned char) t;
    }
    void setThreadSafeness(ThreadSafeness t) { safeness_ = (unsigned char) t; }
    void setSince(const QString &since);
    virtual void setRelates(Aggregate* pseudoParent);
    virtual void setRelates(const QString &name);
    void setPhysicalModuleName(const QString &name) { physicalModuleName_ = name; }
    void setUrl(const QString& url) { url_ = url; }
    void setTemplateStuff(const QString &t) { templateStuff_ = t; }
    void setReconstitutedBrief(const QString &t) { reconstitutedBrief_ = t; }
    void setPageType(PageType t) { pageType_ = (unsigned char) t; }
    void setPageType(const QString& t);
    void setParent(Aggregate* n) { parent_ = n; }
    void setIndexNodeFlag(bool isIndexNode = true) { indexNodeFlag_ = isIndexNode; }
    virtual void setOutputFileName(const QString& ) { }

    bool isQmlNode() const { return genus() == QML; }
    bool isJsNode() const { return genus() == JS; }
    bool isCppNode() const { return genus() == CPP; }

    virtual bool isAggregate() const = 0;
    virtual bool isCollectionNode() const { return false; }
    virtual bool isDocumentNode() const { return false; }
    virtual bool isGroup() const { return false; }
    virtual bool isModule() const { return false; }
    virtual bool isQmlModule() const { return false; }
    virtual bool isJsModule() const { return false; }
    virtual bool isQmlType() const { return false; }
    virtual bool isJsType() const { return false; }
    virtual bool isQmlBasicType() const { return false; }
    virtual bool isJsBasicType() const { return false; }
    virtual bool isEnumType() const { return false; }
    virtual bool isTypedef() const { return false; }
    virtual bool isTypeAlias() const { return false; }
    virtual bool isExample() const { return false; }
    virtual bool isExampleFile() const { return false; }
    virtual bool isHeaderFile() const { return false; }
    virtual bool isLeaf() const { return false; }
    virtual bool isReimplemented() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isNamespace() const { return false; }
    virtual bool isClass() const { return false; }
    virtual bool isQtQuickNode() const { return false; }
    virtual bool isAbstract() const { return false; }
    virtual bool isProperty() const { return false; }
    virtual bool isVariable() const { return false; }
    virtual bool isQmlProperty() const { return false; }
    virtual bool isJsProperty() const { return false; }
    virtual bool isQmlPropertyGroup() const { return false; }
    virtual bool isJsPropertyGroup() const { return false; }
    virtual bool isQmlSignal() const { return false; }
    virtual bool isJsSignal() const { return false; }
    virtual bool isQmlSignalHandler() const { return false; }
    virtual bool isJsSignalHandler() const { return false; }
    virtual bool isQmlMethod() const { return false; }
    virtual bool isJsMethod() const { return false; }
    virtual bool isAttached() const { return false; }
    virtual bool isAlias() const { return false; }
    virtual bool isWrapper() const;
    virtual bool isReadOnly() const { return false; }
    virtual bool isDefault() const { return false; }
    virtual bool isExternalPage() const { return false; }
    virtual bool isImplicit() const { return false; }
    virtual bool isSharedCommentNode() const { return false; }
    virtual bool isMacro() const { return false; }
    virtual bool isStatic() const { return false; }

    virtual void addMember(Node* ) { }
    virtual bool hasMembers() const { return false; }
    virtual bool hasNamespaces() const { return false; }
    virtual bool hasClasses() const { return false; }
    virtual void setAbstract(bool ) { }
    virtual void setWrapper() { }
    virtual QString title() const { return name(); }
    virtual QString fullTitle() const { return name(); }
    virtual QString subTitle() const { return QString(); }
    virtual void setTitle(const QString& ) { }
    virtual void setSubTitle(const QString& ) { }
    virtual QmlPropertyNode* hasQmlProperty(const QString& ) const { return 0; }
    virtual QmlPropertyNode* hasQmlProperty(const QString&, bool ) const { return 0; }
    virtual void getMemberNamespaces(NodeMap& ) { }
    virtual void getMemberClasses(NodeMap& ) const { }
    virtual bool isInternal() const;
    virtual void setDataType(const QString& ) { }
    virtual void setReadOnly(bool ) { }
    virtual Node* disambiguate(NodeType , DocSubtype ) { return this; }
    virtual bool wasSeen() const { return false; }
    virtual void appendGroupName(const QString& ) { }
    virtual QString element() const { return QString(); }
    virtual Tree* tree() const;
    virtual void findChildren(const QString& , NodeList& nodes) const { nodes.clear(); }
    virtual void setNoAutoList(bool ) { }
    virtual bool docMustBeGenerated() const { return false; }
    bool isIndexNode() const { return indexNodeFlag_; }
    NodeType type() const { return (NodeType) nodeType_; }
    virtual DocSubtype docSubtype() const { return NoSubtype; }
    bool match(const NodeTypeList& types) const;
    Aggregate* parent() const { return parent_; }
    const Node* root() const;
    Aggregate* relates() const { return relatesTo_; }
    const QString& name() const { return name_; }
    QString physicalModuleName() const;
    QString url() const { return url_; }
    virtual QString nameForLists() const { return name_; }
    virtual QString outputFileName() const { return QString(); }
    virtual QString obsoleteLink() const { return QString(); }
    virtual void setObsoleteLink(const QString& ) { };
    virtual void setQtVariable(const QString& ) { }
    virtual QString qtVariable() const { return QString(); }
    virtual bool hasTag(const QString& ) const { return false; }
    virtual void setOverloadFlag(bool ) { }

    const QMap<LinkType, QPair<QString,QString> >& links() const { return linkMap_; }
    void setLink(LinkType linkType, const QString &link, const QString &desc);

    Access access() const { return (Access) access_; }
    bool isPublic() const { return (Access) access_ == Public; }
    bool isPrivate() const { return (Access) access_ == Private; }
    QString accessString() const;
    const Location& declLocation() const { return declLocation_; }
    const Location& defLocation() const { return defLocation_; }
    const Location& location() const { return (defLocation_.isEmpty() ? declLocation_ : defLocation_); }
    const Doc& doc() const { return doc_; }
    bool hasDoc() const { return !doc_.isEmpty(); }
    Status status() const { return (Status) status_; }
    Status inheritedStatus() const;
    bool isObsolete() const { return (status_ == (unsigned char) Obsolete); }
    ThreadSafeness threadSafeness() const;
    ThreadSafeness inheritedThreadSafeness() const;
    QString since() const { return since_; }
    QString templateStuff() const { return templateStuff_; }
    const QString& reconstitutedBrief() const { return reconstitutedBrief_; }
    PageType pageType() const { return (PageType) pageType_; }
    QString pageTypeString() const;
    QString nodeTypeString() const;
    QString nodeSubtypeString() const;
    virtual void addPageKeywords(const QString& ) { }

    void clearRelated() { relatesTo_ = 0; }

    bool isSharingComment() const { return (sharedCommentNode_ != 0); }
    bool hasSharedDoc() const;
    void setSharedCommentNode(SharedCommentNode* t);

    //QString guid() const;
    QString extractClassName(const QString &string) const;
    virtual QString qmlTypeName() const { return name_; }
    virtual QString qmlFullBaseName() const { return QString(); }
    virtual QString logicalModuleName() const { return QString(); }
    virtual QString logicalModuleVersion() const { return QString(); }
    virtual QString logicalModuleIdentifier() const { return QString(); }
    virtual void setLogicalModuleInfo(const QString& ) { }
    virtual void setLogicalModuleInfo(const QStringList& ) { }
    virtual CollectionNode* logicalModule() const { return 0; }
    virtual void setQmlModule(CollectionNode* ) { }
    virtual ClassNode* classNode() { return 0; }
    virtual void setClassNode(ClassNode* ) { }
    virtual const Node* applyModuleName(const Node* ) const { return 0; }
    virtual QString idNumber() { return "0"; }
    QmlTypeNode* qmlTypeNode();
    ClassNode* declarativeCppNode();
    const QString& outputSubdirectory() const { return outSubDir_; }
    virtual void setOutputSubdirectory(const QString& t) { outSubDir_ = t; }
    QString fullDocumentName() const;
    static QString cleanId(const QString &str);

    static FlagValue toFlagValue(bool b);
    static bool fromFlagValue(FlagValue fv, bool defaultValue);

    static QString pageTypeString(unsigned char t);
    static QString nodeTypeString(unsigned char t);
    static QString nodeSubtypeString(unsigned char t);
    static int incPropertyGroupCount();
    static void clearPropertyGroupCount();
    static void initialize();
    static NodeType goal(const QString& t) { return goals_.value(t); }

/*!
  Returns \c true if the node \a n1 is less than node \a n2. The
  comparison is performed by comparing properties of the nodes
  in order of increasing complexity.
*/
    static bool nodeNameLessThan(const Node *first, const Node *second);


protected:
    Node(NodeType type, Aggregate* parent, const QString& name);
    void removeRelates();

private:

    unsigned char nodeType_;
    unsigned char genus_;
    unsigned char access_;
    unsigned char safeness_;
    unsigned char pageType_;
    unsigned char status_;
    bool indexNodeFlag_;

    Aggregate* parent_;
    Aggregate* relatesTo_;
    SharedCommentNode *sharedCommentNode_;
    QString name_;
    Location declLocation_;
    Location defLocation_;
    Doc doc_;
    QMap<LinkType, QPair<QString, QString> > linkMap_;
    QString fileNameBase_;
    QString physicalModuleName_;
    QString url_;
    QString since_;
    QString templateStuff_;
    QString reconstitutedBrief_;
    //mutable QString uuid_;
    QString outSubDir_;
    static QStringMap operators_;
    static int propertyGroupCount_;
    static QMap<QString,Node::NodeType> goals_;
};
Q_DECLARE_TYPEINFO(Node::DocSubtype, Q_PRIMITIVE_TYPE);

class Aggregate : public Node
{
public:
    virtual ~Aggregate();

    Node* findChildNode(const QString& name, Node::Genus genus, int findFlags = 0) const;
    Node* findChildNode(const QString& name, NodeType type);
    void findChildren(const QString& name, NodeList& nodes) const override;
    FunctionNode* findFunctionNode(const QString& name, const QString& params) const;
    FunctionNode* findFunctionNode(const FunctionNode* clone) const;
    void addInclude(const QString &include);
    void setIncludes(const QStringList &includes);
    void normalizeOverloads();
    void makeUndocumentedChildrenInternal();
    void deleteChildren();
    void removeFromRelated();

    bool isAggregate() const override { return true; }
    bool isLeaf() const override { return false; }
    const EnumNode* findEnumNodeForValue(const QString &enumValue) const;
    const NodeList & childNodes() const { return children_; }
    const NodeList & relatedNodes() const { return related_; }

    int count() const { return children_.size(); }
    NodeList overloads(const QString &funcName) const;
    const QStringList& includes() const { return includes_; }

    QStringList primaryKeys();
    QStringList secondaryKeys();
    const QStringList& pageKeywords() const { return pageKeywds; }
    void addPageKeywords(const QString& t) override { pageKeywds << t; }
    void setOutputFileName(const QString& f) override { outputFileName_ = f; }
    QString outputFileName() const override { return outputFileName_; }
    QmlPropertyNode* hasQmlProperty(const QString& ) const override;
    QmlPropertyNode* hasQmlProperty(const QString&, bool attached) const override;
    virtual QmlTypeNode* qmlBaseNode() const { return 0; }
    void addChild(Node* child, const QString& title);
    const QStringList& groupNames() const { return groupNames_; }
    void appendGroupName(const QString& t) override { groupNames_.append(t); }
    void printChildren(const QString& title);
    void addChild(Node* child);
    void removeChild(Node* child);
    void setOutputSubdirectory(const QString& t) override;
    const NodeMap& primaryFunctionMap() { return primaryFunctionMap_; }
    const QMap<QString, NodeList>& secondaryFunctionMap() { return secondaryFunctionMap_; }
    bool noAutoList() const { return noAutoList_; }
    void setNoAutoList(bool b)  override { noAutoList_ = b; }
protected:
    Aggregate(NodeType type, Aggregate* parent, const QString& name);

private:
    friend class Node;

    static bool isSameSignature(const FunctionNode* f1, const FunctionNode* f2);
    void removeRelated(Node* pseudoChild);
    void addRelated(Node* pseudoChild);

    QString outputFileName_;
    QStringList pageKeywds;
    QStringList includes_;
    QStringList groupNames_;
    NodeList children_;
    NodeList enumChildren_;
    NodeList related_;
    NodeMap childMap_;
    NodeMap primaryFunctionMap_;
    QMap<QString, NodeList> secondaryFunctionMap_;
    bool noAutoList_;
};

class LeafNode : public Node
{
public:
    LeafNode();
    virtual ~LeafNode() { }

    bool isAggregate() const override { return false; }
    bool isLeaf() const override { return true; }

protected:
    LeafNode(NodeType type, Aggregate* parent, const QString& name);
    LeafNode(Aggregate* parent, NodeType type, const QString& name);
};

class NamespaceNode : public Aggregate
{
public:
    NamespaceNode(Aggregate* parent, const QString& name);
    virtual ~NamespaceNode() { }
    bool isNamespace() const override { return true; }
    Tree* tree() const override { return (parent() ? parent()->tree() : tree_); }
    bool wasSeen() const override { return seen_; }

    void markSeen() { seen_ = true; }
    void markNotSeen() { seen_ = false; }
    void setTree(Tree* t) { tree_ = t; }
    const NodeList& orphans() const { return orphans_; }
    void addOrphan(Node* child) { orphans_.append(child); }
    QString whereDocumented() const { return whereDocumented_; }
    void setWhereDocumented(const QString &t) { whereDocumented_ = t; }
    bool isDocumentedHere() const;
    bool hasDocumentedChildren() const;
    void reportDocumentedChildrenInUndocumentedNamespace() const;
    bool docMustBeGenerated() const override;
    void setDocumented() { documented_ = true; }
    bool wasDocumented() const { return documented_; }
    void setDocNode(NamespaceNode* ns) { docNode_ = ns; }
    NamespaceNode* docNode() const { return docNode_; }

 private:
    bool                seen_;
    bool                documented_;
    Tree*               tree_;
    QString             whereDocumented_;
    NamespaceNode*      docNode_;
    NodeList            orphans_;
};

struct RelatedClass
{
    RelatedClass() { }
    // constructor for resolved base class
    RelatedClass(Node::Access access, ClassNode* node)
        : access_(access), node_(node) { }
    // constructor for unresolved base class
    RelatedClass(Node::Access access, const QStringList& path, const QString& signature)
        : access_(access), node_(0), path_(path), signature_(signature) { }
    QString accessString() const;
    bool isPrivate() const { return (access_ == Node::Private); }

    Node::Access        access_;
    ClassNode*          node_;
    QStringList         path_;
    QString             signature_;
};

struct UsingClause
{
    UsingClause() { }
    UsingClause(const QString& signature) : node_(0), signature_(signature) { }
    const QString& signature() const { return signature_; }
    const Node* node() { return node_; }
    void setNode(const Node* n) { node_ = n; }

    const Node* node_;
    QString     signature_;
};

class ClassNode : public Aggregate
{
public:
    ClassNode(Aggregate* parent, const QString& name);
    virtual ~ClassNode() { }
    bool isClass() const override { return true; }
    bool isWrapper() const override { return wrapper_; }
    QString obsoleteLink() const override { return obsoleteLink_; }
    void setObsoleteLink(const QString& t) override { obsoleteLink_ = t; }
    void setWrapper() override { wrapper_ = true; }

    void addResolvedBaseClass(Access access, ClassNode* node);
    void addDerivedClass(Access access, ClassNode* node);
    void addUnresolvedBaseClass(Access access, const QStringList& path, const QString& signature);
    void addUnresolvedUsingClause(const QString& signature);
    void fixBaseClasses();
    void fixPropertyUsingBaseClasses(PropertyNode* pn);

    QList<RelatedClass>& baseClasses() { return bases_; }
    QList<RelatedClass>& derivedClasses() { return derived_; }
    QList<RelatedClass>& ignoredBaseClasses() { return ignoredBases_; }
    QList<UsingClause>& usingClauses() { return usingClauses_; }

    const QList<RelatedClass> &baseClasses() const { return bases_; }
    const QList<RelatedClass> &derivedClasses() const { return derived_; }
    const QList<RelatedClass> &ignoredBaseClasses() const { return ignoredBases_; }
    const QList<UsingClause>& usingClauses() const { return usingClauses_; }

    QmlTypeNode* qmlElement() { return qmlelement; }
    void setQmlElement(QmlTypeNode* qcn) { qmlelement = qcn; }
    bool isAbstract() const override { return abstract_; }
    void setAbstract(bool b) override { abstract_ = b; }
    PropertyNode* findPropertyNode(const QString& name);
    QmlTypeNode* findQmlBaseNode();
    FunctionNode* findOverriddenFunction(const FunctionNode* fn);

private:
    QList<RelatedClass> bases_;
    QList<RelatedClass> derived_;
    QList<RelatedClass> ignoredBases_;
    QList<UsingClause> usingClauses_;
    bool abstract_;
    bool wrapper_;
    QString obsoleteLink_;
    QmlTypeNode* qmlelement;
};

class DocumentNode : public Aggregate
{
public:

    DocumentNode(Aggregate* parent,
             const QString& name,
             DocSubtype docSubtype,
             PageType ptype);
    virtual ~DocumentNode() { }

    bool isDocumentNode() const override { return true; }
    void setTitle(const QString &title) override;
    void setSubTitle(const QString &subTitle) override { subtitle_ = subTitle; }

    DocSubtype docSubtype() const override { return nodeSubtype_; }
    QString title() const override { return title_; }
    QString fullTitle() const override;
    QString subTitle() const override;
    virtual QString imageFileName() const { return QString(); }
    QString nameForLists() const override { return title(); }
    virtual void setImageFileName(const QString& ) { }

    bool isHeaderFile() const override { return (docSubtype() == Node::HeaderFile); }
    bool isExample() const override { return (docSubtype() == Node::Example); }
    bool isExampleFile() const override { return (parent() && parent()->isExample()); }
    bool isExternalPage() const override { return nodeSubtype_ == ExternalPage; }

protected:
    DocSubtype nodeSubtype_;
    QString title_;
    QString subtitle_;
};

class ExampleNode : public DocumentNode
{
public:
    ExampleNode(Aggregate* parent, const QString& name)
        : DocumentNode(parent, name, Node::Example, Node::ExamplePage) { }
    virtual ~ExampleNode() { }
    QString imageFileName() const override { return imageFileName_; }
    void setImageFileName(const QString& ifn) override { imageFileName_ = ifn; }

private:
    QString imageFileName_;
};

struct ImportRec {
    QString name_;      // module name
    QString version_;   // <major> . <minor>
    QString importId_;  // "as" name
    QString importUri_; // subdirectory of module directory

    ImportRec(const QString& name,
              const QString& version,
              const QString& importId,
              const QString& importUri)
    : name_(name), version_(version), importId_(importId), importUri_(importUri) { }
    QString& name() { return name_; }
    QString& version() { return version_; }
    QString& importId() { return importId_; }
    QString& importUri() { return importUri_; }
    bool isEmpty() const { return name_.isEmpty(); }
};

typedef QList<ImportRec> ImportList;

class QmlTypeNode : public Aggregate
{
public:
    QmlTypeNode(Aggregate* parent, const QString& name);
    virtual ~QmlTypeNode();
    bool isQmlType() const override { return isQmlNode(); }
    bool isJsType() const override { return isJsNode(); }
    bool isQtQuickNode() const override {
        return (logicalModuleName() == QLatin1String("QtQuick"));
    }
    ClassNode* classNode() override { return cnode_; }
    void setClassNode(ClassNode* cn) override { cnode_ = cn; }
    bool isAbstract() const override { return abstract_; }
    bool isWrapper() const override { return wrapper_; }
    void setAbstract(bool b) override { abstract_ = b; }
    void setWrapper() override { wrapper_ = true; }
    bool isInternal() const override { return (status() == Internal); }
    QString qmlFullBaseName() const override;
    QString obsoleteLink() const override { return obsoleteLink_; }
    void setObsoleteLink(const QString& t) override { obsoleteLink_ = t; }
    QString logicalModuleName() const override;
    QString logicalModuleVersion() const override;
    QString logicalModuleIdentifier() const override;
    CollectionNode* logicalModule() const override { return logicalModule_; }
    void setQmlModule(CollectionNode* t) override { logicalModule_ = t; }

    const ImportList& importList() const { return importList_; }
    void setImportList(const ImportList& il) { importList_ = il; }
    const QString& qmlBaseName() const { return qmlBaseName_; }
    void setQmlBaseName(const QString& name) { qmlBaseName_ = name; }
    bool qmlBaseNodeNotSet() const { return (qmlBaseNode_ == 0); }
    QmlTypeNode* qmlBaseNode() const override { return qmlBaseNode_; }
    void setQmlBaseNode(QmlTypeNode* b) { qmlBaseNode_ = b; }
    void requireCppClass() { cnodeRequired_ = true; }
    bool cppClassRequired() const { return cnodeRequired_; }
    static void addInheritedBy(const Node *base, Node* sub);
    static void subclasses(const Node *base, NodeList& subs);
    static void terminate();
    bool inherits(Aggregate* type);

public:
    static bool qmlOnly;
    static QMultiMap<const Node*, Node*> inheritedBy;

private:
    bool abstract_;
    bool cnodeRequired_;
    bool wrapper_;
    ClassNode*    cnode_;
    QString             qmlBaseName_;
    QString             obsoleteLink_;
    CollectionNode*     logicalModule_;
    QmlTypeNode*       qmlBaseNode_;
    ImportList          importList_;
};

class QmlBasicTypeNode : public Aggregate
{
public:
    QmlBasicTypeNode(Aggregate* parent,
                     const QString& name);
    virtual ~QmlBasicTypeNode() { }
    bool isQmlBasicType() const override { return (genus() == Node::QML); }
    bool isJsBasicType() const override { return (genus() == Node::JS); }
};

class QmlPropertyGroupNode : public Aggregate
{
public:
    QmlPropertyGroupNode(QmlTypeNode* parent, const QString& name);
    virtual ~QmlPropertyGroupNode() { }
    bool isQtQuickNode() const override { return parent()->isQtQuickNode(); }
    QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    QString logicalModuleName() const override {
        return parent()->logicalModuleName();
    }
    QString logicalModuleVersion() const override {
        return parent()->logicalModuleVersion();
    }
    QString logicalModuleIdentifier() const override {
        return parent()->logicalModuleIdentifier();
    }
    QString idNumber() override;
    bool isQmlPropertyGroup() const override { return genus() == Node::QML; }
    bool isJsPropertyGroup() const override { return genus() == Node::JS; }
    QString element() const override { return parent()->name(); }

 private:
    int     idNumber_;
};

class QmlPropertyNode : public LeafNode
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::QmlPropertyNode)

public:
    QmlPropertyNode(Aggregate *parent,
                    const QString& name,
                    const QString& type,
                    bool attached);
    virtual ~QmlPropertyNode() { }

    void setDataType(const QString& dataType) override { type_ = dataType; }
    void setStored(bool stored) { stored_ = toFlagValue(stored); }
    void setDesignable(bool designable) { designable_ = toFlagValue(designable); }
    void setReadOnly(bool ro) override { readOnly_ = toFlagValue(ro); }
    void setDefault() { isdefault_ = true; }

    const QString &dataType() const { return type_; }
    QString qualifiedDataType() const { return type_; }
    bool isReadOnlySet() const { return (readOnly_ != FlagValueDefault); }
    bool isStored() const { return fromFlagValue(stored_,true); }
    bool isDesignable() const { return fromFlagValue(designable_,false); }
    bool isWritable();
    bool isQmlProperty() const override { return genus() == QML; }
    bool isJsProperty() const override { return genus() == JS; }
    bool isDefault() const override { return isdefault_; }
    bool isReadOnly() const override { return fromFlagValue(readOnly_,false); }
    bool isAlias() const override { return isAlias_; }
    bool isAttached() const override { return attached_; }
    bool isQtQuickNode() const override { return parent()->isQtQuickNode(); }
    QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    QString logicalModuleName() const override {
        return parent()->logicalModuleName();
    }
    QString logicalModuleVersion() const override {
        return parent()->logicalModuleVersion();
    }
    QString logicalModuleIdentifier() const override {
        return parent()->logicalModuleIdentifier();
    }
    QString element() const override;

 private:
    PropertyNode* findCorrespondingCppProperty();

private:
    QString type_;
    FlagValue   stored_;
    FlagValue   designable_;
    bool    isAlias_;
    bool    isdefault_;
    bool    attached_;
    FlagValue   readOnly_;
};

class EnumItem
{
public:
    EnumItem() { }
    EnumItem(const QString& name, const QString& value)
        : name_(name), value_(value) { }

    const QString& name() const { return name_; }
    const QString& value() const { return value_; }

private:
    QString name_;
    QString value_;
};

class EnumNode : public LeafNode
{
public:
    EnumNode(Aggregate* parent, const QString& name);
    virtual ~EnumNode() { }

    void addItem(const EnumItem& item);
    void setFlagsType(TypedefNode* typedeff);
    bool hasItem(const QString &name) const { return names_.contains(name); }
    bool isEnumType() const override { return true; }

    const QList<EnumItem>& items() const { return items_; }
    Access itemAccess(const QString& name) const;
    const TypedefNode* flagsType() const { return flagsType_; }
    QString itemValue(const QString &name) const;

private:
    QList<EnumItem> items_;
    QSet<QString> names_;
    const TypedefNode* flagsType_;
};

class TypedefNode : public LeafNode
{
public:
    TypedefNode(Aggregate* parent, const QString& name);
    virtual ~TypedefNode() { }

    virtual bool isTypedef() const { return true; }
    const EnumNode* associatedEnum() const { return associatedEnum_; }

private:
    void setAssociatedEnum(const EnumNode* t);

    friend class EnumNode;

    const EnumNode* associatedEnum_;
};

class TypeAliasNode : public TypedefNode
{
 public:
    TypeAliasNode(Aggregate* parent, const QString& name, const QString& aliasedType);
    virtual ~TypeAliasNode() { }

    virtual bool isTypeAlias() const { return true; }
    QString aliasedType() { return aliasedType_; }

 private:
    QString aliasedType_;
};

inline void EnumNode::setFlagsType(TypedefNode* t)
{
    flagsType_ = t;
    t->setAssociatedEnum(this);
}

class Parameter
{
public:
    Parameter() {}
    Parameter(const QString& dataType,
              const QString& name = QString(),
              const QString& defaultValue = QString());
    Parameter(const Parameter& p);

    Parameter& operator=(const Parameter& p);

    void setName(const QString& name) { name_ = name; }

    bool hasType() const { return dataType_.length() > 0; }
    const QString& dataType() const { return dataType_; }
    const QString& name() const { return name_; }
    const QString& defaultValue() const { return defaultValue_; }
    void setDefaultValue(const QString& defaultValue) { defaultValue_ = defaultValue; }

    QString reconstruct(bool value = false) const;

 public:
    QString dataType_;
    QString name_;
    QString defaultValue_;
};

class SharedCommentNode : public LeafNode
{
 public:
    SharedCommentNode(Node *n)
        : LeafNode(Node::SharedComment, n->parent(), QString()) {
        collective_.reserve(1); n->setSharedCommentNode(this);
    }
    virtual ~SharedCommentNode() { collective_.clear(); }

    bool isSharedCommentNode() const override { return true; }
    void append(Node* n) { collective_.append(n); }
    const QVector<Node*>& collective() const { return collective_; }
    void setOverloadFlag(bool b) override;
    void setRelates(Aggregate* pseudoParent) override;
    void setRelates(const QString &name) override;

 private:
    QVector<Node*> collective_;
};

//friend class QTypeInfo<Parameter>;
//Q_DECLARE_TYPEINFO(Parameter, Q_MOVABLE_TYPE);

class FunctionNode : public LeafNode
{
public:
    enum Virtualness { NonVirtual, NormalVirtual, PureVirtual };

    enum Metaness {
        Plain,
        Signal,
        Slot,
        Ctor,
        Dtor,
        CCtor,                  // copy constructor
        MCtor,                  // move-copy constructor
        MacroWithParams,
        MacroWithoutParams,
        Native,
        CAssign,                // copy-assignment operator
        MAssign                 // move-assignment operator
    };

    FunctionNode(Aggregate* parent, const QString &name);
    FunctionNode(NodeType type, Aggregate* parent, const QString &name, bool attached);
    virtual ~FunctionNode() { }

    void setReturnType(const QString& t) { returnType_ = t; }
    void setParentPath(const QStringList& p) { parentPath_ = p; }
    void setMetaness(Metaness t) { metaness_ = t; }
    void setMetaness(const QString& t);
    void setVirtualness(const QString& t);
    void setVirtualness(Virtualness v) { virtualness_ = v; }
    void setVirtual() { virtualness_ = NormalVirtual; }
    void setConst(bool b) { const_ = b; }
    void setStatic(bool b) { static_ = b; }
    unsigned char overloadNumber() const { return overloadNumber_; }
    void setOverloadFlag(bool b) override { overload_ = b; }
    void setOverloadNumber(unsigned char n) { overloadNumber_ = n; }
    void setReimplemented(bool b);
    void addParameter(const Parameter& parameter);
    inline void setParameters(const QVector<Parameter>& parameters);
    void setParameters(const QString &t);
    void borrowParameterNames(const FunctionNode* source);
    void setReimplementedFrom(const QString &path) { reimplementedFrom_ = path; }

    const QString& returnType() const { return returnType_; }
    QString metaness() const;
    QString virtualness() const;
    bool isConst() const { return const_; }
    bool isStatic() const override { return static_; }
    bool isOverload() const { return overload_; }
    bool isReimplemented() const override { return reimplemented_; }
    bool isFunction() const override { return true; }
    bool isSomeCtor() const { return isCtor() || isCCtor() || isMCtor(); }
    bool isMacroWithParams() const { return (metaness_ == MacroWithParams); }
    bool isMacroWithoutParams() const { return (metaness_ == MacroWithoutParams); }
    bool isMacro() const override {
        return (isMacroWithParams() || isMacroWithoutParams());
    }
    bool isSignal() const { return (metaness_ == Signal); }
    bool isSlot() const { return (metaness_ == Slot); }
    bool isCtor() const { return (metaness_ == Ctor); }
    bool isDtor() const { return (metaness_ == Dtor); }
    bool isCCtor() const { return (metaness_ == CCtor); }
    bool isMCtor() const { return (metaness_ == MCtor); }
    bool isCAssign() const { return (metaness_ == CAssign); }
    bool isMAssign() const { return (metaness_ == MAssign); }
    bool isSpecialMemberFunction() const {
        return (isDtor() || isCCtor() || isMCtor() || isCAssign() || isMAssign());
    }
    bool isNonvirtual() const { return (virtualness_ == NonVirtual); }
    bool isVirtual() const { return (virtualness_ == NormalVirtual); }
    bool isPureVirtual() const { return (virtualness_ == PureVirtual); }
    bool isQmlSignal() const override {
        return (type() == Node::QmlSignal) && (genus() == Node::QML);
    }
    bool isJsSignal() const override {
        return (type() == Node::QmlSignal) && (genus() == Node::JS);
    }
    bool isQmlSignalHandler() const override {
        return (type() == Node::QmlSignalHandler) && (genus() == Node::QML);
    }
    bool isJsSignalHandler() const override {
        return (type() == Node::QmlSignalHandler) && (genus() == Node::JS);
    }
    bool isQmlMethod() const override {
        return (type() == Node::QmlMethod) && (genus() == Node::QML);
    }
    bool isJsMethod() const override {
        return (type() == Node::QmlMethod) && (genus() == Node::JS);
    }
    QVector<Parameter> &parameters() { return parameters_; }
    const QVector<Parameter>& parameters() const { return parameters_; }
    void clearParams() { parameters_.clear(); }
    QStringList parameterNames() const;
    QString rawParameters(bool names = false, bool values = false) const;
    const QString& reimplementedFrom() const { return reimplementedFrom_; }
    const PropNodeList& associatedProperties() const { return associatedProperties_; }
    const QStringList& parentPath() const { return parentPath_; }
    bool hasAssociatedProperties() const { return !associatedProperties_.isEmpty(); }
    bool hasOneAssociatedProperty() const { return (associatedProperties_.size() == 1); }
    PropertyNode* firstAssociatedProperty() const { return associatedProperties_[0]; }
    bool hasActiveAssociatedProperty() const;

    QStringList reconstructParameters(bool values = false) const;
    QString signature(bool values, bool noReturnType = false) const override;
    QString element() const override { return parent()->name(); }
    bool isAttached() const override { return attached_; }
    bool isQtQuickNode() const override { return parent()->isQtQuickNode(); }
    QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    QString logicalModuleName() const override {
        return parent()->logicalModuleName();
    }
    QString logicalModuleVersion() const override {
        return parent()->logicalModuleVersion();
    }
    QString logicalModuleIdentifier() const override {
        return parent()->logicalModuleIdentifier();
    }
    bool isPrivateSignal() const { return privateSignal_; }
    void setPrivateSignal() { privateSignal_ = true; }

    void debug() const;

    bool isDeleted() const { return isDeleted_; }
    void setIsDeleted(bool b) { isDeleted_ = b; }

    bool isDefaulted() const { return isDefaulted_; }
    void setIsDefaulted(bool b) { isDefaulted_ = b; }

    void setFinal(bool b) { isFinal_ = b; }
    bool isFinal() const { return isFinal_; }

    void setOverride(bool b) { isOverride_ = b; }
    bool isOverride() const { return isOverride_; }

    void setImplicit(bool b) { isImplicit_ = b; }
    bool isImplicit() const override { return isImplicit_; }

    void setRef(bool b) { isRef_ = b; }
    bool isRef() const { return isRef_; }

    void setRefRef(bool b) { isRefRef_ = b; }
    bool isRefRef() const { return isRefRef_; }

    void setInvokable(bool b) { isInvokable_ = b; }
    bool isInvokable() const { return isInvokable_; }

    bool hasTag(const QString& t) const override { return (tag_ == t); }
    void setTag(const QString& t) { tag_ = t; }
    const QString &tag() const { return tag_; }
    bool compare(const FunctionNode *fn) const;
    bool isIgnored() const;

private:
    void addAssociatedProperty(PropertyNode* property);

    friend class Aggregate;
    friend class PropertyNode;

    QString     returnType_;
    QStringList parentPath_;
    Metaness    metaness_;
    Virtualness virtualness_;
    bool const_ : 1;
    bool static_ : 1;
    bool reimplemented_: 1;
    bool attached_: 1;
    bool privateSignal_: 1;
    bool overload_ : 1;
    bool isDeleted_ : 1;
    bool isDefaulted_ : 1;
    bool isFinal_ : 1;
    bool isOverride_ : 1;
    bool isImplicit_ : 1;
    bool isRef_ : 1;
    bool isRefRef_ : 1;
    bool isInvokable_ : 1;
    unsigned char overloadNumber_;
    QVector<Parameter> parameters_;
    QString             reimplementedFrom_;
    PropNodeList        associatedProperties_;
    QString             tag_;
};

class PropertyNode : public LeafNode
{
public:
    enum FunctionRole { Getter, Setter, Resetter, Notifier };
    enum { NumFunctionRoles = Notifier + 1 };

    PropertyNode(Aggregate* parent, const QString& name);
    virtual ~PropertyNode() { }

    void setDataType(const QString& dataType) override { type_ = dataType; }
    bool isProperty() const override { return true; }
    void addFunction(FunctionNode* function, FunctionRole role);
    void addSignal(FunctionNode* function, FunctionRole role);
    void setStored(bool stored) { stored_ = toFlagValue(stored); }
    void setDesignable(bool designable) { designable_ = toFlagValue(designable); }
    void setScriptable(bool scriptable) { scriptable_ = toFlagValue(scriptable); }
    void setWritable(bool writable) { writable_ = toFlagValue(writable); }
    void setUser(bool user) { user_ = toFlagValue(user); }
    void setOverriddenFrom(const PropertyNode* baseProperty);
    void setRuntimeDesFunc(const QString& rdf) { runtimeDesFunc_ = rdf; }
    void setRuntimeScrFunc(const QString& scrf) { runtimeScrFunc_ = scrf; }
    void setConstant() { const_ = true; }
    void setFinal() { final_ = true; }
    void setRevision(int revision) { revision_ = revision; }

    const QString &dataType() const { return type_; }
    QString qualifiedDataType() const;
    NodeList functions() const;
    const NodeList &functions(FunctionRole role) const { return functions_[(int)role]; }
    const NodeList &getters() const { return functions(Getter); }
    const NodeList &setters() const { return functions(Setter); }
    const NodeList &resetters() const { return functions(Resetter); }
    const NodeList &notifiers() const { return functions(Notifier); }
    FunctionRole role(const FunctionNode* fn) const;
    bool isStored() const { return fromFlagValue(stored_, storedDefault()); }
    bool isDesignable() const { return fromFlagValue(designable_, designableDefault()); }
    bool isScriptable() const { return fromFlagValue(scriptable_, scriptableDefault()); }
    const QString& runtimeDesignabilityFunction() const { return runtimeDesFunc_; }
    const QString& runtimeScriptabilityFunction() const { return runtimeScrFunc_; }
    bool isWritable() const { return fromFlagValue(writable_, writableDefault()); }
    bool isUser() const { return fromFlagValue(user_, userDefault()); }
    bool isConstant() const { return const_; }
    bool isFinal() const { return final_; }
    const PropertyNode* overriddenFrom() const { return overrides_; }

    bool storedDefault() const { return true; }
    bool userDefault() const { return false; }
    bool designableDefault() const { return !setters().isEmpty(); }
    bool scriptableDefault() const { return true; }
    bool writableDefault() const { return !setters().isEmpty(); }

private:
    QString type_;
    QString runtimeDesFunc_;
    QString runtimeScrFunc_;
    NodeList functions_[NumFunctionRoles];
    FlagValue stored_;
    FlagValue designable_;
    FlagValue scriptable_;
    FlagValue writable_;
    FlagValue user_;
    bool const_;
    bool final_;
    int revision_;
    const PropertyNode* overrides_;
};

inline void FunctionNode::setParameters(const QVector<Parameter> &p)
{
    parameters_ = p;
}

inline void PropertyNode::addFunction(FunctionNode* function, FunctionRole role)
{
    functions_[(int)role].append(function);
    function->addAssociatedProperty(this);
}

inline void PropertyNode::addSignal(FunctionNode* function, FunctionRole role)
{
    functions_[(int)role].append(function);
    function->addAssociatedProperty(this);
}

inline NodeList PropertyNode::functions() const
{
    NodeList list;
    for (int i = 0; i < NumFunctionRoles; ++i)
        list += functions_[i];
    return list;
}

class VariableNode : public LeafNode
{
public:
    VariableNode(Aggregate* parent, const QString &name);
    virtual ~VariableNode() { }

    void setLeftType(const QString &leftType) { leftType_ = leftType; }
    void setRightType(const QString &rightType) { rightType_ = rightType; }
    void setStatic(bool b) { static_ = b; }

    const QString &leftType() const { return leftType_; }
    const QString &rightType() const { return rightType_; }
    QString dataType() const { return leftType_ + rightType_; }
    bool isStatic() const override { return static_; }
    virtual bool isVariable() const override { return true; }

private:
    QString leftType_;
    QString rightType_;
    bool static_;
};

inline VariableNode::VariableNode(Aggregate* parent, const QString &name)
    : LeafNode(Variable, parent, name), static_(false)
{
    setGenus(Node::CPP);
}

class DitaMapNode : public DocumentNode
{
public:
    DitaMapNode(Aggregate* parent, const QString& name)
        : DocumentNode(parent, name, Node::Page, Node::DitaMapPage) { }
    virtual ~DitaMapNode() { }

    const DitaRefList& map() const { return doc().ditamap(); }
};

class CollectionNode : public LeafNode
{
 public:
 CollectionNode(NodeType type,
                Aggregate* parent,
                const QString& name,
                Genus genus)
     : LeafNode(type, parent, name), seen_(false), noAutoList_(false)
    {
        setPageType(Node::OverviewPage);
        setGenus(genus);
    }
    virtual ~CollectionNode() { }

    bool isCollectionNode() const override { return true; }
    bool isGroup() const override { return genus() == Node::DOC; }
    bool isModule() const override { return genus() == Node::CPP; }
    bool isQmlModule() const override { return genus() == Node::QML; }
    bool isJsModule() const override { return genus() == Node::JS; }
    QString qtVariable() const override { return qtVariable_; }
    void setQtVariable(const QString& v) override { qtVariable_ = v; }
    void addMember(Node* node) override;
    bool hasMembers() const override;
    bool hasNamespaces() const override;
    bool hasClasses() const override;
    void getMemberNamespaces(NodeMap& out) override;
    void getMemberClasses(NodeMap& out) const override;
    bool wasSeen() const override { return seen_; }
    QString title() const override { return title_; }
    QString subTitle() const override { return subtitle_; }
    QString fullTitle() const override { return title_; }
    QString nameForLists() const override { return title_; }
    void setTitle(const QString &title) override;
    void setSubTitle(const QString &subTitle) override { subtitle_ = subTitle; }

    QString logicalModuleName() const override { return logicalModuleName_; }
    QString logicalModuleVersion() const override {
        return logicalModuleVersionMajor_ + QLatin1Char('.') + logicalModuleVersionMinor_;
    }
    QString logicalModuleIdentifier() const override {
        return logicalModuleName_ + logicalModuleVersionMajor_;
    }
    void setLogicalModuleInfo(const QString& arg) override;
    void setLogicalModuleInfo(const QStringList& info) override;

    const NodeList& members() const { return members_; }
    void printMembers(const QString& title);

    void markSeen() { seen_ = true; }
    void markNotSeen() { seen_ = false; }

    bool noAutoList() const { return noAutoList_; }
    void setNoAutoList(bool b)  override { noAutoList_ = b; }

    const QStringList& groupNames() const { return groupNames_; }
    void appendGroupName(const QString& t) override { groupNames_.append(t); }

 private:
    bool        seen_;
    bool        noAutoList_;
    QString     title_;
    QString     subtitle_;
    NodeList    members_;
    QStringList groupNames_;
    QString     logicalModuleName_;
    QString     logicalModuleVersionMajor_;
    QString     logicalModuleVersionMinor_;
    QString     qtVariable_;
};

QT_END_NAMESPACE

#endif
