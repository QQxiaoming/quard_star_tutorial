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

#ifndef SECTIONS_H
#define SECTIONS_H

#include <qpair.h>
#include "node.h"

QT_BEGIN_NAMESPACE

typedef QMultiMap<QString, Node*> MemberMap; // the string is the member signature
typedef QPair<const QmlTypeNode*, MemberMap> ClassMap;    // the node is the QML type
typedef QList<ClassMap*> ClassMapList;

typedef QPair<QStringList, NodeVector> KeysAndNodes;
typedef QPair<const QmlTypeNode*, KeysAndNodes> ClassKeysNodes;
typedef QList<ClassKeysNodes*> ClassKeysNodesList;

class Section
{
 public:
    enum Style { Summary, Details, AllMembers, Accessors };
    enum Status { Obsolete, Active };

 public:
    Section() : style_(Details), status_(Active), aggregate_(0) { }
    Section(Style style, Status status);
    ~Section();

    void init(const QString& title) {
        title_ = title;
    }
    void init(const QString& singular,
              const QString& plural) {
        singular_ = singular; plural_ = plural;
    }
    void init(const QString& title,
              const QString& singular,
              const QString& plural) {
        title_ = title; divClass_.clear(); singular_= singular; plural_ = plural;
    }
    void init(const QString& title,
              const QString& divClass,
              const QString& singular,
              const QString& plural) {
        title_ = title; divClass_ = divClass; singular_= singular; plural_ = plural;
    }

    void insert(Node *node);
    void insert(const QString& key, Node *node) { memberMap_.insertMulti(key, node); }
    bool insertReimplementedMember(Node* node);

    ClassMap *newClassMap(const Aggregate *aggregate);
    void add(ClassMap *classMap, Node *n);
    void appendMember(Node* node) { members_.append(node); }
    void appendReimplementedMember(Node* node) { reimplementedMembers_.append(node); }

    void clear();
    void reduce();
    bool isEmpty() const {
        return (memberMap_.isEmpty() &&
                inheritedMembers_.isEmpty() &&
                reimplementedMemberMap_.isEmpty() &&
                classMapList_.isEmpty());
    }

    Style style() const { return style_; }
    Status status() const { return status_; }
    const QString &title() const { return title_; }
    const QString &divClass() const { return divClass_; }
    const QString &singular() const { return singular_; }
    const QString &plural() const { return plural_; }
    const QStringList &keys() const { return keys_; }
    const QStringList &keys(Status t) const { return (t == Obsolete ? obsoleteKeys_ : keys_); }
    const NodeVector &members() const { return members_; }
    const NodeVector &reimplementedMembers() const { return reimplementedMembers_; }
    const QList<QPair<Aggregate*, int> > &inheritedMembers() const { return inheritedMembers_; }
    ClassKeysNodesList &classKeysNodesList() { return classKeysNodesList_; }
    const NodeVector &obsoleteMembers() const { return obsoleteMembers_; }
    void appendMembers(const NodeVector &nv) { members_.append(nv); }
    const Aggregate *aggregate() const { return aggregate_; }
    void setAggregate(Aggregate *t) { aggregate_ = t; }

 private:
    QString sortName(const Node *node, const QString* name = 0);

 private:
    Style   style_;
    Status  status_;
    QString title_;
    QString divClass_;
    QString singular_;
    QString plural_;

    Aggregate *aggregate_;
    QStringList keys_;
    QStringList obsoleteKeys_;
    NodeVector members_;
    NodeVector obsoleteMembers_;
    NodeVector reimplementedMembers_;
    QList<QPair<Aggregate *, int> > inheritedMembers_;
    ClassKeysNodesList classKeysNodesList_;

    QMultiMap<QString, Node *> memberMap_;
    QMultiMap<QString, Node *> obsoleteMemberMap_;
    QMultiMap<QString, Node *> reimplementedMemberMap_;
    ClassMapList classMapList_;
};

typedef QVector<Section> SectionVector;
typedef QVector<const Section*> SectionPtrVector;

class Sections
{
 public:
    enum VectorIndex {
        AllMembers = 0,
        PublicTypes = 0,
        DetailsMemberTypes = 0,
        SinceNamespaces = 0,
        StdNamespaces = 0,
        QmlProperties = 0,
        Properties = 1,
        DetailsProperties = 1,
        SinceClasses = 1,
        StdClasses = 1,
        QmlAttachedProperties = 1,
        PublicFunctions = 2,
        DetailsMemberFunctions = 2,
        SinceMemberFunctions = 2,
        StdTypes = 2,
        QmlSignals = 2,
        PublicSlots = 3,
        DetailsMemberVariables = 3,
        SinceNamespaceFunctions = 3,
        StdVariables = 3,
        QmlSignalHandlers = 3,
        Signals = 4,
        SinceGlobalFunctions = 4,
        DetailsRelatedNonmembers = 4,
        StdStaticVariables = 4,
        QmlAttachedSignals = 4,
        PublicVariables = 5,
        SinceMacros = 5,
        DetailsMacros = 5,
        StdFunctions = 5,
        QmlMethods = 5,
        StaticPublicMembers = 6,
        SinceEnumTypes = 6,
        StdMacros = 6,
        QmlAttachedMethods = 6,
        ProtectedTypes = 7,
        SinceTypedefs = 7,
        ProtectedFunctions = 8,
        SinceProperties = 8,
        ProtectedSlots = 9,
        SinceVariables = 9,
        ProtectedVariables = 10,
        SinceQmlTypes = 10,
        StaticProtectedMembers = 11,
        SinceQmlProperties = 11,
        PrivateTypes = 12,
        SinceQmlSignals = 12,
        PrivateFunctions = 13,
        SinceQmlSignalHandlers = 13,
        PrivateSlots = 14,
        SinceQmlMethods = 14,
        StaticPrivateMembers = 15,
        RelatedNonmembers = 16,
        Macros = 17
    };

    Sections(Aggregate *aggregate);
    Sections(const NodeMultiMap& nsmap);
    ~Sections();

    void initSections();
    void clear(SectionVector &v);
    void reduce(SectionVector &v);
    void buildStdRefPageSections();
    void buildStdCppClassRefPageSections();
    void buildStdQmlTypeRefPageSections();

    bool hasObsoleteMembers(SectionPtrVector *summary_spv, SectionPtrVector *details_spv) const;

    static Section &allMembersSection() { return allMembers_[0]; }
    SectionVector &sinceSections() { return sinceSections_; }
    SectionVector &stdSummarySections() { return stdSummarySections_; }
    SectionVector &stdDetailsSections() { return stdDetailsSections_; }
    SectionVector &stdCppClassSummarySections() { return stdCppClassSummarySections_; }
    SectionVector &stdCppClassDetailsSections() { return stdCppClassDetailsSections_; }
    SectionVector &stdQmlTypeSummarySections() { return stdQmlTypeSummarySections_; }
    SectionVector &stdQmlTypeDetailsSections() { return stdQmlTypeDetailsSections_; }

    const SectionVector &stdSummarySections() const { return stdSummarySections_; }
    const SectionVector &stdDetailsSections() const { return stdDetailsSections_; }
    const SectionVector &stdCppClassSummarySections() const { return stdCppClassSummarySections_; }
    const SectionVector &stdCppClassDetailsSections() const { return stdCppClassDetailsSections_; }
    const SectionVector &stdQmlTypeSummarySections() const { return stdQmlTypeSummarySections_; }
    const SectionVector &stdQmlTypeDetailsSections() const { return stdQmlTypeDetailsSections_; }

    Aggregate *aggregate() const { return aggregate_; }

 private:
    void stdRefPageSwitch(SectionVector &v, Node *n);
    void distributeNodeInSummaryVector(SectionVector &sv, Node *n);
    void distributeNodeInDetailsVector(SectionVector &dv, Node *n);
    void distributeQmlNodeInDetailsVector(SectionVector &dv, Node *n);
    void distributeQmlNodeInSummaryVector(SectionVector &sv, Node *n);
    void initAggregate(SectionVector &v, Aggregate *aggregate);

 private:
    Aggregate   *aggregate_;

    static SectionVector stdSummarySections_;
    static SectionVector stdDetailsSections_;
    static SectionVector stdCppClassSummarySections_;
    static SectionVector stdCppClassDetailsSections_;
    static SectionVector stdQmlTypeSummarySections_;
    static SectionVector stdQmlTypeDetailsSections_;
    static SectionVector sinceSections_;
    static SectionVector allMembers_;

};

QT_END_NAMESPACE

#endif
