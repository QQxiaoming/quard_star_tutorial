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

#include <qobjectdefs.h>
#include "generator.h"
#include "sections.h"
#include "config.h"
#include <qdebug.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

//Aggregate *Sections::aggregate_ = 0;

static bool sectionsInitialized = false;
QVector<Section> Sections::stdSummarySections_(7, Section(Section::Summary, Section::Active));
QVector<Section> Sections::stdDetailsSections_(7, Section(Section::Details, Section::Active));
QVector<Section> Sections::stdCppClassSummarySections_(18, Section(Section::Summary, Section::Active));
QVector<Section> Sections::stdCppClassDetailsSections_(6, Section(Section::Details, Section::Active));
QVector<Section> Sections::sinceSections_(15, Section(Section::Details, Section::Active));
QVector<Section> Sections::allMembers_(1, Section(Section::AllMembers, Section::Active));
QVector<Section> Sections::stdQmlTypeSummarySections_(7, Section(Section::Summary, Section::Active));
QVector<Section> Sections::stdQmlTypeDetailsSections_(7, Section(Section::Details, Section::Active));

/*!
  \class Section
  \brief A class for containing the elements of one documentation section
 */

/*!
  The constructor used when the \a style and \a status must
  be provided.
 */
Section::Section(Style style, Status status) : style_(style), status_(status), aggregate_(0)
{
    //members_.reserve(100);
    //obsoleteMembers_.reserve(50);
    //reimplementedMembers_.reserve(50);
}

/*!
  The destructor must delete the members of collections
  when the members are allocated on the heap.
 */
Section::~Section()
{
    clear();
}

/*!
  A Section is now an element in a static vector, so we
  don't have to repeatedly construct and destroy them. But
  we do need to clear them before each call to build the
  sections for a C++ or QML entity.
 */
void Section::clear()
{
    memberMap_.clear();
    obsoleteMemberMap_.clear();
    reimplementedMemberMap_.clear();
    if (!classMapList_.isEmpty()) {
        for (int i = 0; i < classMapList_.size(); i++) {
            ClassMap* cm = classMapList_[i];
            classMapList_[i] = 0;
            delete cm;
        }
        classMapList_.clear();
    }
    keys_.clear();
    obsoleteKeys_.clear();
    members_.clear();
    obsoleteMembers_.clear();
    reimplementedMembers_.clear();
    inheritedMembers_.clear();
    if (!classKeysNodesList_.isEmpty()) {
        for (int i = 0; i < classKeysNodesList_.size(); i++) {
            ClassKeysNodes* ckn = classKeysNodesList_[i];
            classKeysNodesList_[i] = 0;
            delete ckn;
        }
        classKeysNodesList_.clear();
    }
    aggregate_ = 0;
}

/*!
  Construct a name for the \a node that can be used for sorting
  a set of nodes into equivalence classes. If \a name is provided,
  start with that name. Itherwise start with the name in \a node.
 */
QString Section::sortName(const Node *node, const QString* name)
{
    QString nodeName;
    if (name != 0)
        nodeName = *name;
    else
        nodeName = node->name();
    int numDigits = 0;
    for (int i = nodeName.size() - 1; i > 0; --i) {
        if (nodeName.at(i).digitValue() == -1)
            break;
        ++numDigits;
    }

    // we want 'qint8' to appear before 'qint16'
    if (numDigits > 0) {
        for (int i = 0; i < 4 - numDigits; ++i)
            nodeName.insert(nodeName.size()-numDigits-1, QLatin1Char('0'));
    }

    if (node->isFunction()) {
        const FunctionNode *func = static_cast<const FunctionNode *>(node);
        QString sortNo;
        if (func->isSomeCtor())
            sortNo = QLatin1String("C");
        else if (func->isDtor())
            sortNo = QLatin1String("D");
        else if (nodeName.startsWith(QLatin1String("operator"))
                 && nodeName.length() > 8
                 && !nodeName[8].isLetterOrNumber())
            sortNo = QLatin1String("F");
        else
            sortNo = QLatin1String("E");
        return sortNo + nodeName + QLatin1Char(' ') + QString::number(func->overloadNumber(), 36);
    }

    if (node->isClass())
        return QLatin1Char('A') + nodeName;

    if (node->isProperty() || node->isVariable())
        return QLatin1Char('E') + nodeName;

    if (node->isQmlMethod() || node->isQmlSignal() || node->isQmlSignalHandler())
         return QLatin1Char('E') + nodeName;

    return QLatin1Char('B') + nodeName;
}

/*!
  Inserts the \a node into this section if it is appropriate
  for this section.
 */
void Section::insert(Node *node)
{
    bool irrelevant = false;
    bool inherited = false;
    if (!node->relates()) {
        Aggregate* p = node->parent();
        if (p->isQmlPropertyGroup())
            p = p->parent();
        if (!p->isNamespace() && p != aggregate_) {
            if ((!p->isQmlType() && !p->isJsType()) || !p->isAbstract())
                inherited = true;
        }
    }

    if (node->isPrivate() || node->isInternal()) {
        irrelevant = true;
    }
    else if (node->isFunction()) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        irrelevant = (inherited && (func->isSomeCtor() || func->isDtor()));
    }
    else if (node->isClass() || node->isEnumType() || node->isTypedef() || node->isVariable()) {
        irrelevant = (inherited && style_ != AllMembers);
        if (!irrelevant && style_ == Details && node->isTypedef()) {
            const TypedefNode* tdn = static_cast<const TypedefNode*>(node);
            if (tdn->associatedEnum())
                irrelevant = true;
        }
    }

    if (!irrelevant) {
        QString key = sortName(node);
        if (node->isObsolete()) {
            obsoleteMemberMap_.insertMulti(key, node);
        } else {
            if (!inherited)
                memberMap_.insertMulti(key, node);
            else if (style_ == AllMembers) {
                if (!memberMap_.contains(key))
                    memberMap_.insertMulti(key, node);
            }
            if (inherited && (node->parent()->isClass() || node->parent()->isNamespace())) {
                if (inheritedMembers_.isEmpty() || inheritedMembers_.last().first != node->parent()) {
                    QPair<Aggregate *, int> p(node->parent(), 0);
                    inheritedMembers_.append(p);
                }
                inheritedMembers_.last().second++;
            }
        }
    }
}

/*!
  Returns \c true if the \a node is a reimplemented member
  function of the current class. If true, the \a node is
  inserted into the reimplemented member map. The test is
  performed only when the section status is \e Active. True
  is returned only if \a node is inserted into the map.
  That is, false is returned if the \a node is already in
  the map.
 */
bool Section::insertReimplementedMember(Node* node)
{
    if (!node->isPrivate() && (node->relates() == 0)) {
        const FunctionNode* fn = static_cast<const FunctionNode*>(node);
        if (!fn->reimplementedFrom().isEmpty() && (status_ == Active)) {
            if (fn->parent() == aggregate_) {
                QString key = sortName(fn);
                if (!reimplementedMemberMap_.contains(key)) {
                    reimplementedMemberMap_.insert(key, node);
                    return true;
                }
            }
        }
    }
    return false;
}

/*!
  Allocate a new ClassMap on the heap for the \a aggregate
  node, append it to the list of class maps, and return a
  pointer to the new class map.
 */
ClassMap *Section::newClassMap(const Aggregate* aggregate)
{
    ClassMap *classMap = new ClassMap;
    classMap->first = static_cast<const QmlTypeNode*>(aggregate);
    classMapList_.append(classMap);
    return classMap;
}

/*!
  Add node \a n to the \a classMap and to the member map.
 */
void Section::add(ClassMap *classMap, Node *n)
{
    if (n->isQmlPropertyGroup() || n->isJsPropertyGroup()) {
        const QmlPropertyGroupNode* pg = static_cast<const QmlPropertyGroupNode*>(n);
        NodeList::ConstIterator p = pg->childNodes().constBegin();
        while (p != pg->childNodes().constEnd()) {
            if ((*p)->isQmlProperty() || (*p)->isJsProperty()) {
                QString key = (*p)->name();
                key = sortName(*p, &key);
                memberMap_.insert(key,*p);
                classMap->second.insert(key,*p);
            }
            ++p;
        }
    }
    else {
        QString key = n->name();
        key = sortName(n, &key);
        memberMap_.insert(key, n);
        classMap->second.insert(key, n);
    }
}

/*!
  If this section is not empty, convert its maps to sequential
  structures for better traversal during doc generation.
 */
void Section::reduce()
{
    if (!isEmpty()) {
        keys_ = memberMap_.keys();
        obsoleteKeys_ = obsoleteMemberMap_.keys();
        members_ = memberMap_.values().toVector();
        obsoleteMembers_ = obsoleteMemberMap_.values().toVector();
        reimplementedMembers_ = reimplementedMemberMap_.values().toVector();
        for (int i = 0; i < classMapList_.size(); i++) {
            ClassMap* cm = classMapList_[i];
            ClassKeysNodes* ckn = new ClassKeysNodes;
            ckn->first = cm->first;
            ckn->second.second = cm->second.values().toVector();
            ckn->second.first = cm->second.keys();
            classKeysNodesList_.append(ckn);
        }
    }
}

/*!
  \class Sections
  \brief A class for creating vectors of collections for documentation

  Each element in a vector is an instance of Section, which
  contains all the elements that will be documented in one
  section of a reference documentation page.
 */

/*!
  This constructor builds the vectors of sections based on the
  type of the \a aggregate node.
 */
Sections::Sections(Aggregate *aggregate) : aggregate_(aggregate)
{
    initSections();
    initAggregate(allMembers_, aggregate_);
    switch (aggregate_->type()) {
    case Node::Class:
        initAggregate(stdCppClassSummarySections_, aggregate_);
        initAggregate(stdCppClassDetailsSections_, aggregate_);
        buildStdCppClassRefPageSections();
        break;
    case Node::QmlType:
    case Node::QmlBasicType:
        initAggregate(stdQmlTypeSummarySections_, aggregate_);
        initAggregate(stdQmlTypeDetailsSections_, aggregate_);
        buildStdQmlTypeRefPageSections();
        break;
    default:
        initAggregate(stdSummarySections_, aggregate_);
        initAggregate(stdDetailsSections_, aggregate_);
        buildStdRefPageSections();
        break;
    }
}

/*!
  This constructor builds a vector of sections from the \e since
  node map, \a nsmap
 */
Sections::Sections(const NodeMultiMap& nsmap) : aggregate_(0)
{
    initSections();
    if (nsmap.isEmpty())
        return;
    SectionVector &sections = sinceSections();
    NodeMultiMap::const_iterator n = nsmap.constBegin();
    while (n != nsmap.constEnd()) {
        Node* node = n.value();
        switch (node->type()) {
        case Node::QmlType:
            sections[SinceQmlTypes].appendMember(node);
            break;
        case Node::Namespace:
            sections[SinceNamespaces].appendMember(node);
            break;
        case Node::Class:
            sections[SinceClasses].appendMember(node);
            break;
        case Node::Enum:
            sections[SinceEnumTypes].appendMember(node);
            break;
        case Node::Typedef:
            sections[SinceTypedefs].appendMember(node);
            break;
        case Node::Function: {
            const FunctionNode* fn = static_cast<const FunctionNode*>(node);
            if (fn->isMacro())
                sections[SinceMacros].appendMember(node);
            else {
                Node* p = fn->parent();
                if (p) {
                    if (p->type() == Node::Class)
                        sections[SinceMemberFunctions].appendMember(node);
                    else if (p->type() == Node::Namespace) {
                        if (p->name().isEmpty())
                            sections[SinceGlobalFunctions].appendMember(node);
                        else
                            sections[SinceNamespaceFunctions].appendMember(node);
                    }
                    else
                        sections[SinceGlobalFunctions].appendMember(node);
                }
                else
                    sections[SinceGlobalFunctions].appendMember(node);
            }
            break;
        }
        case Node::Property:
            sections[SinceProperties].appendMember(node);
            break;
        case Node::Variable:
            sections[SinceVariables].appendMember(node);
            break;
        case Node::QmlProperty:
            sections[SinceQmlProperties].appendMember(node);
            break;
        case Node::QmlSignal:
            sections[SinceQmlSignals].appendMember(node);
            break;
        case Node::QmlSignalHandler:
            sections[SinceQmlSignalHandlers].appendMember(node);
            break;
        case Node::QmlMethod:
            sections[SinceQmlMethods].appendMember(node);
            break;
        default:
            break;
        }
        ++n;
    }
}

/*!
  The behavior of the destructor depends on the type of the
  Aggregate node that was passed to the constructor. If the
  constructor was passed a multimap, the destruction is a
  bit different because there was no Aggregate node.
 */
Sections::~Sections()
{
    if (aggregate_) {
        switch (aggregate_->type()) {
        case Node::Class:
            clear(stdCppClassSummarySections());
            clear(stdCppClassDetailsSections());
            allMembersSection().clear();
            break;
        case Node::QmlType:
        case Node::QmlBasicType:
            clear(stdQmlTypeSummarySections());
            clear(stdQmlTypeDetailsSections());
            allMembersSection().clear();
            break;
        default:
            clear(stdSummarySections());
            clear(stdDetailsSections());
            allMembersSection().clear();
            break;
        }
        aggregate_ = 0;
    }
    else {
        clear(sinceSections());
    }
}

/*!
  Initialize the Aggregate in each Section of vector \a v with \a aggregate.
 */
void Sections::initAggregate(SectionVector &v, Aggregate *aggregate)
{
    for (int i = 0; i< v.size(); ++i)
        v[i].setAggregate(aggregate);
}

/*!
  This function is called once to initialize all the instances
  of QVector<Section>. The vectors have already been constructed
  with the correct number of Section entries in each. Each Section
  entry has already been constructed with the correct values of
  Style and Status for the vector it is in. This function adds the
  correct text strings to each section in each vector.
 */
void Sections::initSections()
{
    if (sectionsInitialized)
        return;
    sectionsInitialized = true;

    allMembers_[0].init("member", "members");
    {
        QVector<Section> &v = stdCppClassSummarySections_;
        v[0].init("Public Types", "public type", "public types");
        v[1].init("Properties", "property", "properties");
        v[2].init("Public Functions", "public function", "public functions");
        v[3].init("Public Slots", "public slot", "public slots");
        v[4].init("Signals", "signal", "signals");
        v[5].init("Public Variables", "public variable", "public variables");
        v[6].init("Static Public Members", "static public member", "static public members");
        v[7].init("Protected Types", "protected type", "protected types");
        v[8].init("Protected Functions", "protected function", "protected functions");
        v[9].init("Protected Slots", "protected slot", "protected slots");
        v[10].init("Protected Variables", "protected type", "protected variables");
        v[11].init("Static Protected Members", "static protected member", "static protected members");
        v[12].init("Private Types", "private type", "private types");
        v[13].init("Private Functions", "private function", "private functions");
        v[14].init("Private Slots", "private slot", "private slots");
        v[15].init("Static Private Members", "static private member", "static private members");
        v[16].init("Related Non-Members", "related non-member", "related non-members");
        v[17].init("Macros", "macro", "macros");
    }

    {
        QVector<Section> &v = stdCppClassDetailsSections_;
        v[0].init("Member Type Documentation", "types", "member", "members");
        v[1].init("Property Documentation", "prop", "member", "members");
        v[2].init("Member Function Documentation", "func", "member", "members");
        v[3].init("Member Variable Documentation", "vars", "member", "members");
        v[4].init("Related Non-Members", "relnonmem", "member", "members");
        v[5].init("Macro Documentation", "macros", "member", "members");
    }

    {
        QVector<Section> &v = stdSummarySections_;
        v[0].init("Namespaces", "namespace", "namespaces");
        v[1].init("Classes", "class", "classes");
        v[2].init("Types", "type", "types");
        v[3].init("Variables", "variable", "variables");
        v[4].init("Static Variables", "static variable", "static variables");
        v[5].init("Functions", "function", "functions");
        v[6].init("Macros", "macro", "macros");
    }

    {
        QVector<Section> &v = stdDetailsSections_;
        v[0].init("Namespaces", "nmspace", "namespace", "namespaces");
        v[1].init("Classes", "classes", "class", "classes");
        v[2].init("Type Documentation", "types", "type", "types");
        v[3].init("Variable Documentation", "vars", "variable", "variables");
        v[4].init("Static Variables", QString(), "static variable", "static variables");
        v[5].init("Function Documentation", "func", "function", "functions");
        v[6].init("Macro Documentation", "macros", "macro", "macros");
    }

    {
        QVector<Section> &v = sinceSections_;
        v[SinceNamespaces].init("    New Namespaces");
        v[SinceClasses].init("    New Classes");
        v[SinceMemberFunctions].init("    New Member Functions");
        v[SinceNamespaceFunctions].init("    New Functions in Namespaces");
        v[SinceGlobalFunctions].init("    New Global Functions");
        v[SinceMacros].init("    New Macros");
        v[SinceEnumTypes].init("    New Enum Types");
        v[SinceTypedefs].init("    New Typedefs");
        v[SinceProperties].init("    New Properties");
        v[SinceVariables].init("    New Variables");
        v[SinceQmlTypes].init("    New QML Types");
        v[SinceQmlProperties].init("    New QML Properties");
        v[SinceQmlSignals].init("    New QML Signals");
        v[SinceQmlSignalHandlers].init("    New QML Signal Handlers");
        v[SinceQmlMethods].init("    New QML Methods");
    }

    {
        QVector<Section> &v = stdQmlTypeSummarySections_;
        v[0].init("Properties", "property", "properties");
        v[1].init("Attached Properties", "attached property", "attached properties");
        v[2].init("Signals", "signal", "signals");
        v[3].init("Signal Handlers", "signal handler", "signal handlers");
        v[4].init("Attached Signals", "attached signal", "attached signals");
        v[5].init("Methods", "method", "methods");
        v[6].init("Attached Methods", "attached method", "attached methods");
    }

    {
        QVector<Section> &v = stdQmlTypeDetailsSections_;
        v[0].init("Property Documentation","qmlprop","member","members");
        v[1].init("Attached Property Documentation","qmlattprop", "member","members");
        v[2].init("Signal Documentation","qmlsig","signal","signals");
        v[3].init("Signal Handler Documentation","qmlsighan","signal handler","signal handlers");
        v[4].init("Attached Signal Documentation","qmlattsig", "signal","signals");
        v[5].init("Method Documentation","qmlmeth","member","members");
        v[6].init("Attached Method Documentation","qmlattmeth", "member","members");
    }
}

/*!
  Reset each Section in vector \a v to its initialized state.
 */
void Sections::clear(QVector<Section> &v)
{
    for (int i = 0; i < v.size(); ++i)
        v[i].clear();
}

/*!
  Linearize the maps in each Section in \a v.
 */
void Sections::reduce(QVector<Section> &v)
{
    for (int i = 0; i < v.size(); ++i)
        v[i].reduce();
}

/*!
  This is a private helper function for buildStdRefPageSections().
 */
void Sections::stdRefPageSwitch(SectionVector &v, Node *n)
{
    switch (n->type()) {
    case Node::Namespace:
        v[StdNamespaces].insert(n);
        return;
    case Node::Class:
        v[StdClasses].insert(n);
        return;
    case Node::Enum:
    case Node::Typedef:
        v[StdTypes].insert(n);
        return;
    case Node::Function:
        {
            FunctionNode *func = static_cast<FunctionNode *>(n);
            if (func->isMacro())
                v[StdMacros].insert(n);
            else
                v[StdFunctions].insert(n);
        }
        return;
    case Node::Variable:
        {
            const VariableNode* var = static_cast<const VariableNode*>(n);
            if (!var->doc().isEmpty()) {
                if (var->isStatic())
                    v[StdStaticVariables].insert(n);
                else
                    v[StdVariables].insert(n);
            }
        }
        return;
    case Node::SharedComment:
        {
            SharedCommentNode *scn = static_cast<SharedCommentNode *>(n);
            if (!scn->doc().isEmpty())
                v[StdFunctions].insert(scn);
        }
        return;
    default:
        return;
    }
}

/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is not a C++ class or a QML type.
 */
void Sections::buildStdRefPageSections()
{
    const NamespaceNode* ns = 0;
    bool documentAll = true;
    NodeList nodeList = aggregate_->childNodes();
    nodeList += aggregate_->relatedNodes();
    if (aggregate_->isNamespace()) {
        ns = static_cast<const NamespaceNode*>(aggregate_);
        if (!ns->hasDoc())
            documentAll = false;
    }
    NodeList::ConstIterator c = nodeList.constBegin();
    while (c != nodeList.constEnd()) {
        Node *n = *c;
        if (documentAll || n->hasDoc()) {
            stdRefPageSwitch(stdSummarySections(), n);
            stdRefPageSwitch(stdDetailsSections(), n);
        }
        ++c;
    }
    if (ns && !ns->orphans().isEmpty()) {
        NodeList::ConstIterator c = ns->orphans().constBegin();
        while (c != ns->orphans().constEnd()) {
            Node *n = *c;
            if (documentAll || n->hasDoc())
                stdRefPageSwitch(stdSummarySections(), n);
            ++c;
        }
    }
    reduce(stdSummarySections());
    reduce(stdDetailsSections());
    allMembersSection().reduce();
}

/*!
  Inserts the node \a n in one of the entries in the vector \a v
  depending on the node's type, access attribute, and a few other
  attributes if the node is a signal, slot, or function.
 */
void Sections::distributeNodeInSummaryVector(SectionVector &sv, Node *n)
{
    if (n->isSharedCommentNode())
        return;
    if (n->isFunction()) {
        FunctionNode *fn = static_cast<FunctionNode*>(n);
        if (fn->hasAssociatedProperties() && !fn->hasActiveAssociatedProperty())
            return;
        else if (fn->isIgnored())
            return;
        if (fn->isSlot()) {
            if (fn->isPublic())
                sv[PublicSlots].insert(fn);
            else if (fn->isPrivate())
                sv[PrivateSlots].insert(fn);
            else
                sv[ProtectedSlots].insert(fn);
        }
        else if (fn->isSignal()) {
            if (fn->isPublic())
                sv[Signals].insert(fn);
        }
        else if (fn->isPublic()) {
            if (fn->isStatic())
                sv[StaticPublicMembers].insert(fn);
            else if (!sv[PublicFunctions].insertReimplementedMember(fn))
                sv[PublicFunctions].insert(fn);
        }
        else if (fn->isPrivate()) {
            if (fn->isStatic())
                sv[StaticPrivateMembers].insert(fn);
            else if (!sv[PrivateFunctions].insertReimplementedMember(fn))
                sv[PrivateFunctions].insert(fn);
        }
        else { // protected
            if (fn->isStatic())
                sv[StaticProtectedMembers].insert(fn);
            else if (!sv[ProtectedFunctions].insertReimplementedMember(fn))
                sv[ProtectedFunctions].insert(fn);
        }
        return;
    }
    if (n->isVariable()) {
        if (n->isStatic()) {
            if (n->isPublic())
                sv[StaticPublicMembers].insert(n);
            else if (n->isPrivate())
                sv[StaticPrivateMembers].insert(n);
            else
                sv[StaticProtectedMembers].insert(n);
        }
        else {
            if (n->isPublic())
                sv[PublicVariables].insert(n);
            else if (!n->isPrivate())
                sv[ProtectedVariables].insert(n);
        }
        return;
    }
    /*
      Getting this far means the node is either a property
      or some kind of type, like an enum or a typedef.
    */
    if (n->isTypedef() && (n->name() == QLatin1String("QtGadgetHelper")))
        return;
    if (n->isProperty())
        sv[Properties].insert(n);
    else if (n->isPublic())
        sv[PublicTypes].insert(n);
    else if (n->isPrivate())
        sv[PrivateTypes].insert(n);
    else
        sv[ProtectedTypes].insert(n);
}

/*!
  Inserts the node \a n in one of the entries in the vector \a v
  depending on the node's type, access attribute, and a few other
  attributes if the node is a signal, slot, or function.
 */
void Sections::distributeNodeInDetailsVector(SectionVector &dv, Node *n)
{
    if (n->isSharingComment())
        return;
    if (n->isFunction()) {
        FunctionNode *fn = static_cast<FunctionNode*>(n);
        if (fn->isIgnored())
            return;
        if (!fn->isSharingComment()) {
            if (!fn->hasAssociatedProperties() || !fn->doc().isEmpty())
                dv[DetailsMemberFunctions].insert(fn);
        }
        return;
    }
    if (n->isEnumType() || n->isTypedef()) {
        if (n->name() != QLatin1String("QtGadgetHelper"))
            dv[DetailsMemberTypes].insert(n);
        return;
    }
    if (n->isProperty())
        dv[DetailsProperties].insert(n);
    else if (n->isVariable() && !n->doc().isEmpty())
        dv[DetailsMemberVariables].insert(n);
    else if (n->isSharedCommentNode() && !n->doc().isEmpty())
        dv[DetailsMemberFunctions].insert(n);
}

void Sections::distributeQmlNodeInDetailsVector(SectionVector &dv, Node *n)
{
    if (n->isSharingComment())
        return;
    if (n->isQmlPropertyGroup() || n->isJsPropertyGroup())
        dv[QmlProperties].insert(n);
    else if (n->isQmlProperty() || n->isJsProperty()) {
        QmlPropertyNode* pn = static_cast<QmlPropertyNode*>(n);
        if (pn->isAttached())
            dv[QmlAttachedProperties].insert(pn);
        else
            dv[QmlProperties].insert(pn);
    } else if (n->isQmlSignal() || n->isJsSignal()) {
        FunctionNode* fn = static_cast<FunctionNode*>(n);
        if (fn->isAttached())
            dv[QmlAttachedSignals].insert(fn);
        else
            dv[QmlSignals].insert(fn);
    } else if (n->isQmlSignalHandler() || n->isJsSignalHandler()) {
        dv[QmlSignalHandlers].insert(n);
    } else if (n->isQmlMethod() || n->isJsMethod()) {
        FunctionNode* fn = static_cast<FunctionNode*>(n);
        if (fn->isAttached())
            dv[QmlAttachedMethods].insert(fn);
        else
            dv[QmlMethods].insert(fn);
    } else if (n->isSharedCommentNode() && n->hasDoc())
        dv[QmlMethods].insert(n);
}

void Sections::distributeQmlNodeInSummaryVector(SectionVector &sv, Node *n)
{
    if (n->isQmlPropertyGroup() || n->isJsPropertyGroup())
        sv[QmlProperties].insert(n);
    else if (n->isQmlProperty() || n->isJsProperty()) {
        QmlPropertyNode* pn = static_cast<QmlPropertyNode*>(n);
        if (pn->isAttached())
            sv[QmlAttachedProperties].insert(pn);
        else
            sv[QmlProperties].insert(pn);
    } else if (n->isQmlSignal() || n->isJsSignal()) {
        FunctionNode* fn = static_cast<FunctionNode*>(n);
        if (fn->isAttached())
            sv[QmlAttachedSignals].insert(fn);
        else
            sv[QmlSignals].insert(fn);
    } else if (n->isQmlSignalHandler() || n->isJsSignalHandler()) {
        sv[QmlSignalHandlers].insert(n);
    } else if (n->isQmlMethod() || n->isJsMethod()) {
        FunctionNode* fn = static_cast<FunctionNode*>(n);
        if (fn->isAttached())
            sv[QmlAttachedMethods].insert(fn);
        else
            sv[QmlMethods].insert(fn);
    }
}

static void pushBaseClasses(QStack<ClassNode*> &stack, ClassNode *cn)
{
    QList<RelatedClass>::ConstIterator r = cn->baseClasses().constBegin();
    while (r != cn->baseClasses().constEnd()) {
        if (r->node_)
            stack.prepend(r->node_);
        ++r;
    }
}

/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is a C++.
 */
void Sections::buildStdCppClassRefPageSections()
{
    SectionVector &sv = stdCppClassSummarySections();
    SectionVector &dv = stdCppClassDetailsSections();
    Section &allMembers = allMembersSection();
    NodeList::ConstIterator r = aggregate_->relatedNodes().constBegin();
    while (r != aggregate_->relatedNodes().constEnd()) {
        Node* n = *r;
        if (n->isFunction()) {
            FunctionNode *func = static_cast<FunctionNode *>(n);
            if (func->isMacro()) {
                sv[Macros].insert(n);
                dv[DetailsMacros].insert(n);
            } else {
                sv[RelatedNonmembers].insert(n);
                dv[DetailsRelatedNonmembers].insert(n);
            }
        } else {
            sv[RelatedNonmembers].insert(n);
            dv[DetailsRelatedNonmembers].insert(n);
        }
        ++r;
    }

    bool documentAll = true;
    if (aggregate_->parent() && !aggregate_->name().isEmpty() && !aggregate_->hasDoc())
        documentAll = false;
    NodeList::ConstIterator c = aggregate_->childNodes().constBegin();
    while (c != aggregate_->childNodes().constEnd()) {
        Node* n = *c;
        if (!n->isPrivate() && !n->isProperty())
            allMembers.insert(n);
        if (!documentAll && !n->hasDoc()) {
            ++c;
            continue;
        }
        distributeNodeInSummaryVector(sv, n);
        distributeNodeInDetailsVector(dv, n);
        ++c;
    }

    QStack<ClassNode*> stack;
    ClassNode* cn = static_cast<ClassNode*>(aggregate_);
    pushBaseClasses(stack, cn);
    while (!stack.isEmpty()) {
        ClassNode *cn = stack.pop();
        c = cn->childNodes().constBegin();
        while (c != cn->childNodes().constEnd()) {
            Node* n = *c;
            if (!n->isPrivate() && !n->isProperty())
                allMembers.insert(n);
            if (!documentAll && !n->hasDoc()) {
                ++c;
                continue;
            }
            distributeNodeInSummaryVector(sv, n);
            ++c;
        }
        pushBaseClasses(stack, cn);
    }
    reduce(sv);
    reduce(dv);
    allMembers.reduce();
}


/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is a QML type.
 */
void Sections::buildStdQmlTypeRefPageSections()
{
    ClassMap* classMap = 0;
    SectionVector &sv = stdQmlTypeSummarySections();
    SectionVector &dv = stdQmlTypeDetailsSections();
    Section &allMembers = allMembersSection();

    const Aggregate* qcn = aggregate_;
    while (true) {
        if (!qcn->isAbstract() || !classMap)
            classMap = allMembers.newClassMap(qcn);
        NodeList::ConstIterator c = qcn->childNodes().constBegin();
        while (c != qcn->childNodes().constEnd()) {
            Node *n = *c;
            if (n->isInternal()) {
                ++c;
                continue;
            }
            allMembers.add(classMap, n);
            distributeQmlNodeInSummaryVector(sv, n);
            distributeQmlNodeInDetailsVector(dv, n);
            ++c;
        }
        if (qcn->qmlBaseNode() == qcn) {
            qDebug() << "qdoc internal error: circular type definition."
                     << "QML type" << qcn->name()
                     << "can't be its own base type";
            qcn = 0;
            break;
        }
        qcn = static_cast<QmlTypeNode*>(qcn->qmlBaseNode());
        if (qcn == 0)
            break;
        if (!qcn->isAbstract())
            break;
    }

    while (qcn != 0) {
        if (!qcn->isAbstract() || !classMap)
            classMap = allMembers.newClassMap(qcn);
        NodeList::ConstIterator c = qcn->childNodes().constBegin();
        while (c != qcn->childNodes().constEnd()) {
            Node *n = *c;
            if (n->isInternal()) {
                ++c;
                continue;
            }
            allMembers.add(classMap, n);
            ++c;
        }
        if (qcn->qmlBaseNode() == qcn) {
            qDebug() << "qdoc internal error: circular type definition."
                     << "QML type" << qcn->name()
                     << "can't be its own base type";
            qcn = 0;
            break;
        }
        qcn = static_cast<QmlTypeNode*>(qcn->qmlBaseNode());
    }
    reduce(sv);
    reduce(dv);
    allMembers.reduce();
}

/*!
  Returns true if any sections in this object contain obsolete
  members. If it returns false, then \a summary_spv and \a details_spv
  have not been modified. Otherwise, both vectors will contain pointers
  to the sections that contain obsolete members.
 */
bool Sections::hasObsoleteMembers(SectionPtrVector *summary_spv, SectionPtrVector *details_spv) const
{
    const SectionVector *sv = 0;
    if (aggregate_->isClass())
        sv = &stdCppClassSummarySections();
    else if (aggregate_->isQmlType() || aggregate_->isQmlBasicType())
        sv = &stdQmlTypeSummarySections();
    else
        sv = &stdSummarySections();
    SectionVector::ConstIterator s = sv->constBegin();
    while (s != sv->constEnd()) {
        if (!s->obsoleteMembers().isEmpty())
            summary_spv->append(&(*s));
        ++s;
    }
    if (aggregate_->isClass())
        sv = &stdCppClassDetailsSections();
    else if (aggregate_->isQmlType() || aggregate_->isQmlBasicType())
        sv = &stdQmlTypeDetailsSections();
    else
        sv = &stdDetailsSections();
    s = sv->constBegin();
    while (s != sv->constEnd()) {
        if (!s->obsoleteMembers().isEmpty())
            details_spv->append(&(*s));
        ++s;
    }
    return !summary_spv->isEmpty();
}


QT_END_NAMESPACE
