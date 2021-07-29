/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "codemarker.h"
#include "generator.h"
#include "webxmlgenerator.h"
#include "node.h"
#include "separator.h"
#include "tree.h"
#include "qdocdatabase.h"
#include "qdocindexfiles.h"
#include "helpprojectwriter.h"

#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

void WebXMLGenerator::initializeGenerator(const Config &config)
{
    HtmlGenerator::initializeGenerator(config);
}

void WebXMLGenerator::terminateGenerator()
{
    Generator::terminateGenerator();
}

QString WebXMLGenerator::format()
{
    return "WebXML";
}

QString WebXMLGenerator::fileExtension() const
{
    // As this is meant to be an intermediate format,
    // use .html for internal references. The name of
    // the output file is set separately in
    // beginSubPage() calls.
    return "html";
}

int WebXMLGenerator::generateAtom(const Atom * /* atom, */,
                                  const Node * /* relative */,
                                  CodeMarker * /* marker */)
{
    return 0;
}

void WebXMLGenerator::generateCppReferencePage(Node *node, CodeMarker *marker)
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    beginSubPage(node, Generator::fileName(node, "webxml"));
    writer.writeStartDocument();
    writer.writeStartElement("WebXML");
    writer.writeStartElement("document");

    generateIndexSections(writer, node, marker);

    writer.writeEndElement(); // document
    writer.writeEndElement(); // WebXML
    writer.writeEndDocument();

    out() << data;
    endSubPage();
}

void WebXMLGenerator::generateDocumentNode(DocumentNode *dn, CodeMarker *marker)
{
    generateCppReferencePage(dn, marker);
}

void WebXMLGenerator::generateIndexSections(QXmlStreamWriter &writer,
                                            Node *node, CodeMarker *marker)
{
    // TODO: Here, filter non-aggregate nodes based on their type if output
    // contains items that Shiboken cannot (and does not need to) handle.
    if (QDocIndexFiles::qdocIndexFiles()->generateIndexSection(writer, node)) {

        // Add documentation to this node if it exists.
        writer.writeStartElement("description");
        writer.writeAttribute("path", node->doc().location().filePath());
        writer.writeAttribute("line", QString::number(node->doc().location().lineNo()));
        writer.writeAttribute("column", QString::number(node->doc().location().columnNo()));

        if (node->isDocumentNode())
            generateRelations(writer, node);

        if (node->isModule()) {
            writer.writeStartElement("generatedlist");
            writer.writeAttribute("contents", "classesbymodule");
            CollectionNode *cnn = static_cast<CollectionNode *>(node);

            if (cnn->hasNamespaces()) {
                writer.writeStartElement("section");
                writer.writeStartElement("heading");
                writer.writeAttribute("level", "1");
                writer.writeCharacters("Namespaces");
                writer.writeEndElement(); // heading
                NodeMap namespaces;
                cnn->getMemberNamespaces(namespaces);
                generateAnnotatedList(writer, node, namespaces);
                writer.writeEndElement(); // section
            }
            if (cnn->hasClasses()) {
                writer.writeStartElement("section");
                writer.writeStartElement("heading");
                writer.writeAttribute("level", "1");
                writer.writeCharacters("Classes");
                writer.writeEndElement(); // heading
                NodeMap classes;
                cnn->getMemberClasses(classes);
                generateAnnotatedList(writer, node, classes);
                writer.writeEndElement(); // section
            }
            writer.writeEndElement(); // generatedlist
        }

        inLink = inContents = inSectionHeading = hasQuotingInformation = false;
        numTableRows = 0;

        const Atom *atom = node->doc().body().firstAtom();
        while (atom)
            atom = addAtomElements(writer, atom, node, marker);

        QList<Text> alsoList = node->doc().alsoList();
        supplementAlsoList(node, alsoList);

        if (!alsoList.isEmpty()) {
            writer.writeStartElement("see-also");
            for (int i = 0; i < alsoList.size(); ++i) {
                const Atom *atom = alsoList.at(i).firstAtom();
                while (atom)
                    atom = addAtomElements(writer, atom, node, marker);
            }
            writer.writeEndElement(); // see-also
        }
        writer.writeEndElement(); // description

        if (node->isAggregate()) {
            for (auto child : static_cast<Aggregate *>(node)->childNodes())
                generateIndexSections(writer, child, marker);
            for (auto related : static_cast<Aggregate *>(node)->relatedNodes())
                generateIndexSections(writer, related, marker);
        }
        writer.writeEndElement();
    }
}

void WebXMLGenerator::generateDocumentation(Node *node)
{
    // Don't generate nodes that are already processed, or if they're not supposed to
    // generate output, ie. external, index or images nodes.
    if (!node->url().isNull() ||
         node->isExternalPage() ||
         node->isIndexNode() ||
         node->docSubtype() == Node::Image) {
        return;
    }

    if (node->isInternal() && !showInternal_)
        return;

    CodeMarker *marker = CodeMarker::markerForFileName(node->location().filePath());
    if (node->parent()) {
        if (node->isNamespace() || node->isClass())
            generateCppReferencePage(static_cast<Aggregate*>(node), marker);
        else if (node->isDocumentNode())
            generateDocumentNode(static_cast<DocumentNode *>(node), marker);
        else if (node->isCollectionNode() && node->wasSeen()) {
            // see remarks in base class impl.
            qdb_->mergeCollections(static_cast<CollectionNode *>(node));
            generateCppReferencePage(node, marker);
        }
        // else if TODO: anything else?
    }

    if (node->isAggregate()) {
        Aggregate* aggregate = static_cast<Aggregate*>(node);
        for (auto c : aggregate->childNodes()) {
            if ((c->isAggregate() || c->isCollectionNode()) && !c->isPrivate())
                generateDocumentation(c);
        }
    }
}

const Atom *WebXMLGenerator::addAtomElements(QXmlStreamWriter &writer,
                                             const Atom *atom, const Node *relative, CodeMarker *marker)
{
    bool keepQuoting = false;

    switch (atom->type()) {
    case Atom::AutoLink:
        if (!inLink && !inSectionHeading) {
            const Node *node = nullptr;
            QString link = getLink(atom, relative, &node);
            if (node) {
                startLink(writer, atom, node, link);
                if (inLink) {
                    writer.writeCharacters(atom->string());
                    writer.writeEndElement(); // link
                    inLink = false;
                }
            } else {
                writer.writeCharacters(atom->string());
            }
        } else {
            writer.writeCharacters(atom->string());
        }
        break;
    case Atom::BaseName:
        break;
    case Atom::BriefLeft:

        writer.writeStartElement("brief");
        switch (relative->type()) {
        case Node::Property:
            writer.writeCharacters("This property");
            break;
        case Node::Variable:
            writer.writeCharacters("This variable");
            break;
        default:
            break;
        }
        if (relative->type() == Node::Property || relative->type() == Node::Variable) {
            QString str;
            const Atom *a = atom->next();
            while (a != nullptr && a->type() != Atom::BriefRight) {
                if (a->type() == Atom::String || a->type() == Atom::AutoLink)
                    str += a->string();
                a = a->next();
            }
            str[0] = str[0].toLower();
            if (str.endsWith('.'))
                str.chop(1);

            const QVector<QStringRef> words = str.splitRef(' ');
            if (!words.isEmpty()) {
                const QStringRef &first(words.at(0));
                if (!(first == "contains" || first == "specifies"
                      || first == "describes" || first == "defines"
                      || first == "holds" || first == "determines"))
                    writer.writeCharacters(" holds ");
                else
                    writer.writeCharacters(" ");
            }
        }
        break;

    case Atom::BriefRight:
        if (relative->type() == Node::Property || relative->type() == Node::Variable)
            writer.writeCharacters(".");

        writer.writeEndElement(); // brief
        break;

    case Atom::C:
        writer.writeStartElement("teletype");
        if (inLink)
            writer.writeAttribute("type", "normal");
        else
            writer.writeAttribute("type", "highlighted");

        writer.writeCharacters(plainCode(atom->string()));
        writer.writeEndElement(); // teletype
        break;

    case Atom::Code:
        if (!hasQuotingInformation)
            writer.writeTextElement("code", trimmedTrailing(plainCode(atom->string()), QString(), QString()));
        else
            keepQuoting = true;
        break;

#ifdef QDOC_QML
    case Atom::Qml:
        if (!hasQuotingInformation)
            writer.writeTextElement("qml", trimmedTrailing(plainCode(atom->string()), QString(), QString()));
        else
            keepQuoting = true;
#endif
    case Atom::CodeBad:
        writer.writeTextElement("badcode", trimmedTrailing(plainCode(atom->string()), QString(), QString()));
        break;

    case Atom::CodeNew:
        writer.writeTextElement("para", "you can rewrite it as");
        writer.writeTextElement("newcode", trimmedTrailing(plainCode(atom->string()), QString(), QString()));
        break;

    case Atom::CodeOld:
        writer.writeTextElement("para", "For example, if you have code like");
        writer.writeTextElement("oldcode", trimmedTrailing(plainCode(atom->string()), QString(), QString()));
        break;

    case Atom::CodeQuoteArgument:
        if (quoting_) {
            if (quoteCommand == "dots") {
                writer.writeAttribute("indent", atom->string());
                writer.writeCharacters("...");
            } else {
                writer.writeCharacters(atom->string());
            }
            writer.writeEndElement(); // code
            keepQuoting = true;
        }
        break;

    case Atom::CodeQuoteCommand:
        if (quoting_) {
            quoteCommand = atom->string();
            writer.writeStartElement(quoteCommand);
        }
        break;

    case Atom::FootnoteLeft:
        writer.writeStartElement("footnote");
        break;

    case Atom::FootnoteRight:
        writer.writeEndElement(); // footnote
        break;

    case Atom::FormatEndif:
        writer.writeEndElement(); // raw
        break;
    case Atom::FormatIf:
        writer.writeStartElement("raw");
        writer.writeAttribute("format", atom->string());
        break;
    case Atom::FormattingLeft:
    {
        if (atom->string() == ATOM_FORMATTING_BOLD)
            writer.writeStartElement("bold");
        else if (atom->string() == ATOM_FORMATTING_ITALIC)
            writer.writeStartElement("italic");
        else if (atom->string() == ATOM_FORMATTING_UNDERLINE)
            writer.writeStartElement("underline");
        else if (atom->string() == ATOM_FORMATTING_SUBSCRIPT)
            writer.writeStartElement("subscript");
        else if (atom->string() == ATOM_FORMATTING_SUPERSCRIPT)
            writer.writeStartElement("superscript");
        else if (atom->string() == ATOM_FORMATTING_TELETYPE)
            writer.writeStartElement("teletype");
        else if (atom->string() == ATOM_FORMATTING_PARAMETER)
            writer.writeStartElement("argument");
        else if (atom->string() == ATOM_FORMATTING_INDEX)
            writer.writeStartElement("index");
    }
        break;

    case Atom::FormattingRight:
    {
        if (atom->string() == ATOM_FORMATTING_BOLD)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_ITALIC)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_UNDERLINE)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_SUBSCRIPT)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_SUPERSCRIPT)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_TELETYPE)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_PARAMETER)
            writer.writeEndElement();
        else if (atom->string() == ATOM_FORMATTING_INDEX)
            writer.writeEndElement();
    }
        if (inLink) {
            writer.writeEndElement(); // link
            inLink = false;
        }
        break;

    case Atom::GeneratedList:
        writer.writeStartElement("generatedlist");
        writer.writeAttribute("contents", atom->string());
        writer.writeEndElement();
        break;
    case Atom::Image:
        writer.writeStartElement("image");
        writer.writeAttribute("href", imageFileName(relative, atom->string()));
        writer.writeEndElement();
        break;

    case Atom::InlineImage:
        writer.writeStartElement("inlineimage");
        writer.writeAttribute("href", imageFileName(relative, atom->string()));
        writer.writeEndElement();
        break;

    case Atom::ImageText:
        break;

    case Atom::ImportantLeft:
        writer.writeStartElement("para");
        writer.writeTextElement("bold", "Important:");
        writer.writeCharacters(" ");
        break;

    case Atom::ImportantRight:
        writer.writeEndElement(); // para
        break;

    case Atom::LegaleseLeft:
        writer.writeStartElement("legalese");
        break;

    case Atom::LegaleseRight:
        writer.writeEndElement(); // legalese
        break;

    case Atom::Link:
    case Atom::LinkNode:
        if (!inLink) {
            const Node *node = nullptr;
            QString link = getLink(atom, relative, &node);
            if (node)
                startLink(writer, atom, node, link);
        }
        break;

    case Atom::ListLeft:
        writer.writeStartElement("list");

        if (atom->string() == ATOM_LIST_BULLET)
            writer.writeAttribute("type", "bullet");
        else if (atom->string() == ATOM_LIST_TAG)
            writer.writeAttribute("type", "definition");
        else if (atom->string() == ATOM_LIST_VALUE) {
            if (relative->type() == Node::Enum)
                writer.writeAttribute("type", "enum");
            else
                writer.writeAttribute("type", "definition");
        }
        else {
            writer.writeAttribute("type", "ordered");
            if (atom->string() == ATOM_LIST_UPPERALPHA)
                writer.writeAttribute("start", "A");
            else if (atom->string() == ATOM_LIST_LOWERALPHA)
                writer.writeAttribute("start", "a");
            else if (atom->string() == ATOM_LIST_UPPERROMAN)
                writer.writeAttribute("start", "I");
            else if (atom->string() == ATOM_LIST_LOWERROMAN)
                writer.writeAttribute("start", "i");
            else // (atom->string() == ATOM_LIST_NUMERIC)
                writer.writeAttribute("start", "1");
        }
        break;

    case Atom::ListItemNumber:
        break;
    case Atom::ListTagLeft:
    {
        writer.writeStartElement("definition");

        writer.writeTextElement("term", plainCode(
                                    marker->markedUpEnumValue(atom->next()->string(), relative)));
    }
        break;

    case Atom::ListTagRight:
        writer.writeEndElement(); // definition
        break;

    case Atom::ListItemLeft:
        writer.writeStartElement("item");
        break;

    case Atom::ListItemRight:
        writer.writeEndElement(); // item
        break;

    case Atom::ListRight:
        writer.writeEndElement(); // list
        break;

    case Atom::NoteLeft:
        writer.writeStartElement("para");
        writer.writeTextElement("bold", "Note:");
        writer.writeCharacters(" ");
        break;

    case Atom::NoteRight:
        writer.writeEndElement(); // para
        break;

    case Atom::Nop:
        break;

    case Atom::ParaLeft:
        writer.writeStartElement("para");
        break;

    case Atom::ParaRight:
        writer.writeEndElement(); // para
        break;

    case Atom::QuotationLeft:
        writer.writeStartElement("quote");
        break;

    case Atom::QuotationRight:
        writer.writeEndElement(); // quote
        break;

    case Atom::RawString:
        writer.writeCharacters(atom->string());
        break;

    case Atom::SectionLeft:
        writer.writeStartElement("section");
        writer.writeAttribute("id", Doc::canonicalTitle(Text::sectionHeading(atom).toString()));
        break;

    case Atom::SectionRight:
        writer.writeEndElement(); // section
        break;

    case Atom::SectionHeadingLeft:
    {
        writer.writeStartElement("heading");
        int unit = atom->string().toInt(); // + hOffset(relative)
        writer.writeAttribute("level", QString::number(unit));
        inSectionHeading = true;
    }
        break;

    case Atom::SectionHeadingRight:
        writer.writeEndElement(); // heading
        inSectionHeading = false;
        break;

    case Atom::SidebarLeft:
    case Atom::SidebarRight:
        break;

    case Atom::SnippetCommand:
        if (quoting_) {
            writer.writeStartElement(atom->string());

        }
        break;

    case Atom::SnippetIdentifier:
        if (quoting_) {
            writer.writeAttribute("identifier", atom->string());
            writer.writeEndElement();
            keepQuoting = true;
        }
        break;

    case Atom::SnippetLocation:
        if (quoting_) {
            const QString location = atom->string();
            writer.writeAttribute("location", location);
            const QString resolved = Doc::resolveFile(Location(), location);
            if (!resolved.isEmpty())
                writer.writeAttribute("path", resolved);
        }
        break;

    case Atom::String:
        writer.writeCharacters(atom->string());
        break;
    case Atom::TableLeft:
        writer.writeStartElement("table");
        if (atom->string().contains("%"))
            writer.writeAttribute("width", atom->string());
        break;

    case Atom::TableRight:
        writer.writeEndElement(); // table
        break;

    case Atom::TableHeaderLeft:
        writer.writeStartElement("header");
        break;

    case Atom::TableHeaderRight:
        writer.writeEndElement(); // header
        break;

    case Atom::TableRowLeft:
        writer.writeStartElement("row");
        break;

    case Atom::TableRowRight:
        writer.writeEndElement(); // row
        break;

    case Atom::TableItemLeft:
    {
        writer.writeStartElement("item");
        QStringList spans = atom->string().split(",");
        if (spans.size() == 2) {
            if (spans.at(0) != "1")
                writer.writeAttribute("colspan", spans.at(0).trimmed());
            if (spans.at(1) != "1")
                writer.writeAttribute("rowspan", spans.at(1).trimmed());
        }
    }
        break;
    case Atom::TableItemRight:
        writer.writeEndElement(); // item
        break;

    case Atom::Target:
        writer.writeStartElement("target");
        writer.writeAttribute("name", Doc::canonicalTitle(atom->string()));
        writer.writeEndElement();
        break;

    case Atom::UnhandledFormat:
    case Atom::UnknownCommand:
        writer.writeCharacters(atom->typeString());
        break;
    default:
        break;
    }

    hasQuotingInformation = keepQuoting;

    if (atom)
        return atom->next();

    return 0;
}

void WebXMLGenerator::startLink(QXmlStreamWriter &writer, const Atom *atom,
                                const Node *node, const QString &link)
{
    QString fullName = node->fullName();
    if (!fullName.isEmpty() && !link.isEmpty()) {
        writer.writeStartElement("link");
        writer.writeAttribute("raw", atom->string());
        writer.writeAttribute("href", link);
        writer.writeAttribute("type", targetType(node));
        switch (node->type()) {
        case Node::Enum:
            writer.writeAttribute("enum", fullName);
            break;
        case Node::Document:
            writer.writeAttribute("page", fullName);
            break;
        case Node::Property:
        {
            const PropertyNode *propertyNode = static_cast<const PropertyNode *>(node);
            if (propertyNode->getters().size() > 0)
                writer.writeAttribute("getter", propertyNode->getters().at(0)->fullName());
        }
        default:
            ;
        }
        inLink = true;
    }
}

QString WebXMLGenerator::targetType(const Node *node)
{
    switch (node->type()) {
    case Node::Namespace:
        return "namespace";
    case Node::Class:
        return "class";
    case Node::Document:
        return "page";
    case Node::Enum:
        return "enum";
    case Node::Typedef:
        return "typedef";
    case Node::Property:
        return "property";
    case Node::Function:
        return "function";
    case Node::Variable:
        return "variable";
    case Node::Module:
        return "module";
    default:
        break;
    }
    return QString();
}

void WebXMLGenerator::generateRelations(QXmlStreamWriter &writer, const Node *node)
{
    if (node && !node->links().empty()) {
        QPair<QString, QString> anchorPair;
        const Node *linkNode;

        for (QMap<Node::LinkType, QPair<QString, QString> >::const_iterator it =
             node->links().cbegin(), end = node->links().cend(); it != end; ++it) {

            linkNode = qdb_->findNodeForTarget(it.value().first, node);

            if (!linkNode)
                linkNode = node;

            if (linkNode == node)
                anchorPair = it.value();
            else
                anchorPair = anchorForNode(linkNode);

            writer.writeStartElement("relation");
            writer.writeAttribute("href", anchorPair.first);
            writer.writeAttribute("type", targetType(linkNode));

            switch (it.key()) {
            case Node::StartLink:
                writer.writeAttribute("meta", "start");
                break;
            case Node::NextLink:
                writer.writeAttribute("meta", "next");
                break;
            case Node::PreviousLink:
                writer.writeAttribute("meta", "previous");
                break;
            case Node::ContentsLink:
                writer.writeAttribute("meta", "contents");
                break;
            case Node::IndexLink:
                writer.writeAttribute("meta", "index");
                break;
            default:
                writer.writeAttribute("meta", "");
            }
            writer.writeAttribute("description", anchorPair.second);
            writer.writeEndElement(); // link
        }
    }
}

void WebXMLGenerator::generateAnnotatedList(QXmlStreamWriter &writer,
    const Node *relative, const NodeMap &nodeMap)
{
    writer.writeStartElement("table");
    writer.writeAttribute("width", "100%");

    for (NodeMap::const_iterator it = nodeMap.cbegin(),
         end = nodeMap.cend(); it != end; ++it) {
        const Node *node = it.value();

        writer.writeStartElement("row");
        writer.writeStartElement("heading");
        generateFullName(writer, node, relative);
        writer.writeEndElement(); // heading

        writer.writeStartElement("item");
        writer.writeCharacters(node->doc().briefText().toString());
        writer.writeEndElement(); // item
        writer.writeEndElement(); // row
    }
    writer.writeEndElement(); // table
}

void WebXMLGenerator::generateFullName(QXmlStreamWriter &writer,
    const Node *node, const Node *relative)
{
    writer.writeStartElement("link");
    writer.writeAttribute("href", fullDocumentLocation(node));
    writer.writeAttribute("type", targetType(node));
    writer.writeCharacters(node->fullName(relative));
    writer.writeEndElement(); // link
}

const QPair<QString,QString> WebXMLGenerator::anchorForNode(const Node *node)
{
    QPair<QString,QString> anchorPair;

    anchorPair.first = fullDocumentLocation(node);
    if (node->isDocumentNode())
        anchorPair.second = node->title();

    return anchorPair;
}

QT_END_NAMESPACE
