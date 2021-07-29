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

/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include <qhash.h>
#include <qregexp.h>
#include <qxmlstream.h>
#include "codemarker.h"
#include "config.h"
#include "generator.h"

QT_BEGIN_NAMESPACE

class HelpProjectWriter;

class HtmlGenerator : public Generator
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::HtmlGenerator)

public:

public:
    HtmlGenerator();
    ~HtmlGenerator();

    void initializeGenerator(const Config& config) override;
    void terminateGenerator() override;
    QString format() override;
    void generateDocs() override;
    void generateManifestFiles();

    QString protectEnc(const QString &string);
    static QString protect(const QString &string, const QString &encoding = "ISO-8859-1");

protected:
    void generateQAPage() override;
    QString generateLinksToLinksPage(const QString& module, CodeMarker* marker);
    QString generateLinksToBrokenLinksPage(CodeMarker* marker, int& count);
    virtual int generateAtom(const Atom *atom,
                             const Node *relative,
                             CodeMarker *marker) override;
    void generateCppReferencePage(Node* node, CodeMarker* marker) override;
    void generateQmlTypePage(QmlTypeNode* qcn, CodeMarker* marker) override;
    void generateQmlBasicTypePage(QmlBasicTypeNode* qbtn, CodeMarker* marker) override;
    void generateDocumentNode(DocumentNode* dn, CodeMarker* marker) override;
    void generateCollectionNode(CollectionNode* cn, CodeMarker* marker) override;
    QString fileExtension() const override;
    virtual QString refForNode(const Node *node);
    virtual QString linkForNode(const Node *node, const Node *relative);

    void generateManifestFile(const QString &manifest, const QString &element);
    void readManifestMetaContent(const Config &config);
    void generateKeywordAnchors(const Node* node);
    void generateAssociatedPropertyNotes(const FunctionNode* fn);

    QString getLink(const Atom *atom, const Node *relative, const Node **node);
    QString getAutoLink(const Atom *atom, const Node *relative, const Node **node);

private:
    enum SubTitleSize { SmallSubTitle, LargeSubTitle };
    enum ExtractionMarkType {
        BriefMark,
        DetailedDescriptionMark,
        MemberMark,
        EndMark
    };

    struct ManifestMetaFilter
    {
        QSet<QString> names;
        QSet<QString> attributes;
        QSet<QString> tags;
    };

    const QPair<QString,QString> anchorForNode(const Node *node);
    void generateNavigationBar(const QString &title,
                               const Node *node,
                               CodeMarker *marker,
                               const QString &buildversion,
                               bool tableItems = false);
    void generateHeader(const QString& title,
                        const Node *node = 0,
                        CodeMarker *marker = 0);
    void generateTitle(const QString& title,
                       const Text &subTitle,
                       SubTitleSize subTitleSize,
                       const Node *relative,
                       CodeMarker *marker);
    void generateFooter(const Node *node = 0);
    void generateRequisites(Aggregate *inner,
                            CodeMarker *marker);
    void generateQmlRequisites(QmlTypeNode *qcn,
                            CodeMarker *marker);
    void generateBrief(const Node *node,
                       CodeMarker *marker,
                       const Node *relative = 0, bool addLink=true);
    void generateIncludes(const Aggregate *inner, CodeMarker *marker);
    void generateTableOfContents(const Node *node,
                                 CodeMarker *marker,
                                 QVector<Section>* sections = 0);
    void generateSidebar();
    QString generateAllMembersFile(const Section &section, CodeMarker *marker);
    QString generateAllQmlMembersFile(const Sections &sections, CodeMarker* marker);
    QString generateObsoleteMembersFile(const Sections &sections, CodeMarker *marker);
    QString generateObsoleteQmlMembersFile(const Sections &sections, CodeMarker *marker);
    void generateClassHierarchy(const Node *relative, NodeMap &classMap);
    void generateAnnotatedList(const Node* relative, CodeMarker* marker, const NodeMultiMap& nodeMap);
    void generateAnnotatedLists(const Node* relative, CodeMarker* marker, const NodeMultiMap& nodeMap);
    void generateAnnotatedList(const Node* relative, CodeMarker* marker, const NodeList& nodes);
    void generateCompactList(ListType listType,
                             const Node *relative,
                             const NodeMultiMap &classMap,
                             bool includeAlphabet,
                             QString commonPrefix);
    void generateFunctionIndex(const Node *relative);
    void generateLegaleseList(const Node *relative, CodeMarker *marker);
    void generateList(const Node* relative, CodeMarker* marker, const QString& selector);
    void generateSectionList(const Section& section,
                             const Node *relative,
                             CodeMarker *marker,
                             Section::Status = Section::Active);
    void generateQmlSummary(const Section& section,
                            const Node *relative,
                            CodeMarker *marker);
    void generateQmlItem(const Node *node,
                         const Node *relative,
                         CodeMarker *marker,
                         bool summary);
    void generateDetailedQmlMember(Node *node,
                                   const Aggregate *relative,
                                   CodeMarker *marker);
    void generateQmlInherits(QmlTypeNode* qcn, CodeMarker* marker) override;
    void generateQmlInstantiates(QmlTypeNode* qcn, CodeMarker* marker);
    void generateInstantiatedBy(ClassNode* cn, CodeMarker* marker);

    void generateSection(const NodeVector& nv, const Node *relative, CodeMarker *marker);
    void generateSynopsis(const Node *node,
                          const Node *relative,
                          CodeMarker *marker,
                          Section::Style style,
                          bool alignNames = false,
                          const QString* prefix = 0);
    void generateSectionInheritedList(const Section& section, const Node *relative);
    QString highlightedCode(const QString& markedCode,
                            const Node* relative,
                            bool alignNames = false,
                            Node::Genus genus = Node::DontCare);

    void generateFullName(const Node *apparentNode, const Node *relative, const Node *actualNode = 0);
    void generateDetailedMember(const Node *node,
                                const Aggregate *relative,
                                CodeMarker *marker);
    void generateLink(const Atom *atom, CodeMarker *marker);

    inline bool hasBrief(const Node *node);
    QString registerRef(const QString& ref);
    QString fileBase(const Node *node) const override;
    QString fileName(const Node *node);
    static int hOffset(const Node *node);
    static bool isThreeColumnEnumValueTable(const Atom *atom);
#ifdef GENERATE_MAC_REFS
    void generateMacRef(const Node *node, CodeMarker *marker);
#endif
    void beginLink(const QString &link, const Node *node, const Node *relative);
    void endLink();
    void generateExtractionMark(const Node *node, ExtractionMarkType markType);
    void reportOrphans(const Aggregate* parent);

    void beginDitamapPage(const Aggregate* node, const QString& fileName);
    void endDitamapPage();
    void writeDitaMap(const DitaMapNode* node);
    void writeDitaRefs(const DitaRefList& ditarefs);
    QXmlStreamWriter& xmlWriter();

    QHash<QString, QString> refMap;
    int codeIndent;
    QString codePrefix;
    QString codeSuffix;
    HelpProjectWriter *helpProjectWriter;
    bool inObsoleteLink;
    QRegExp funcLeftParen;
    QString style;
    QString headerScripts;
    QString headerStyles;
    QString endHeader;
    QString postHeader;
    QString postPostHeader;
    QString prologue;
    QString footer;
    QString address;
    bool pleaseGenerateMacRef;
    bool noNavigationBar;
    QString project;
    QString projectDescription;
    QString projectUrl;
    QString navigationLinks;
    QString navigationSeparator;
    QString manifestDir;
    QString examplesPath;
    QStringList stylesheets;
    QStringList customHeadElements;
    bool obsoleteLinks;
    QStack<QXmlStreamWriter*> xmlWriterStack;
    static int id;
    QList<ManifestMetaFilter> manifestMetaContent;
    QString homepage;
    QString hometitle;
    QString landingpage;
    QString landingtitle;
    QString cppclassespage;
    QString cppclassestitle;
    QString qmltypespage;
    QString qmltypestitle;
    QString buildversion;
    QString qflagsHref_;
    int tocDepth;

public:
    static bool debugging_on;
    static QString divNavTop;
};

// Do not display \brief for QML/JS types, document and collection nodes
inline bool HtmlGenerator::hasBrief(const Node *node)
{
    return !(node->isQmlType()
             || node->isDocumentNode()
             || node->isCollectionNode()
             || node->isJsType());
}

#define HTMLGENERATOR_ADDRESS           "address"
#define HTMLGENERATOR_FOOTER            "footer"
#define HTMLGENERATOR_GENERATEMACREFS   "generatemacrefs" // ### document me
#define HTMLGENERATOR_POSTHEADER        "postheader"
#define HTMLGENERATOR_POSTPOSTHEADER    "postpostheader"
#define HTMLGENERATOR_PROLOGUE          "prologue"
#define HTMLGENERATOR_NONAVIGATIONBAR   "nonavigationbar"
#define HTMLGENERATOR_NAVIGATIONSEPARATOR "navigationseparator"
#define HTMLGENERATOR_NOSUBDIRS         "nosubdirs"
#define HTMLGENERATOR_TOCDEPTH          "tocdepth"


QT_END_NAMESPACE

#endif
