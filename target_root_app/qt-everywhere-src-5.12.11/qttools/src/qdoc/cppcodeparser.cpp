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
  cppcodeparser.cpp
*/

#include <qfile.h>
#include <stdio.h>
#include <errno.h>
#include "codechunk.h"
#include "config.h"
#include "cppcodeparser.h"
#include "tokenizer.h"
#include "qdocdatabase.h"
#include <qdebug.h>
#include "generator.h"

QT_BEGIN_NAMESPACE

/* qmake ignore Q_OBJECT */

static bool inMacroCommand_ = false;
QStringList CppCodeParser::exampleFiles;
QStringList CppCodeParser::exampleDirs;
QSet<QString> CppCodeParser::excludeDirs;
QSet<QString> CppCodeParser::excludeFiles;

static QSet<QString> topicCommands_;
static QSet<QString> otherMetaCommands_;

/*!
  The constructor initializes some regular expressions
  and calls reset().
 */
CppCodeParser::CppCodeParser()
    : varComment("/\\*\\s*([a-zA-Z_0-9]+)\\s*\\*/"), sep("(?:<[^>]+>)?::")
{
    reset();
    if (topicCommands_.isEmpty()) {
        topicCommands_ << COMMAND_CLASS
                       << COMMAND_DITAMAP
                       << COMMAND_ENUM
                       << COMMAND_EXAMPLE
                       << COMMAND_EXTERNALPAGE
                       << COMMAND_FILE
                       << COMMAND_FN
                       << COMMAND_GROUP
                       << COMMAND_HEADERFILE
                       << COMMAND_MACRO
                       << COMMAND_MODULE
                       << COMMAND_NAMESPACE
                       << COMMAND_PAGE
                       << COMMAND_PROPERTY
                       << COMMAND_TYPEALIAS
                       << COMMAND_TYPEDEF
                       << COMMAND_VARIABLE
                       << COMMAND_QMLTYPE
                       << COMMAND_QMLPROPERTY
                       << COMMAND_QMLPROPERTYGROUP
                       << COMMAND_QMLATTACHEDPROPERTY
                       << COMMAND_QMLSIGNAL
                       << COMMAND_QMLATTACHEDSIGNAL
                       << COMMAND_QMLMETHOD
                       << COMMAND_QMLATTACHEDMETHOD
                       << COMMAND_QMLBASICTYPE
                       << COMMAND_QMLMODULE
                       << COMMAND_JSTYPE
                       << COMMAND_JSPROPERTY
                       << COMMAND_JSPROPERTYGROUP
                       << COMMAND_JSATTACHEDPROPERTY
                       << COMMAND_JSSIGNAL
                       << COMMAND_JSATTACHEDSIGNAL
                       << COMMAND_JSMETHOD
                       << COMMAND_JSATTACHEDMETHOD
                       << COMMAND_JSBASICTYPE
                       << COMMAND_JSMODULE;
    }
    if (otherMetaCommands_.isEmpty()) {
        otherMetaCommands_ = commonMetaCommands();
        otherMetaCommands_ << COMMAND_INHEADERFILE
                           << COMMAND_OVERLOAD
                           << COMMAND_REIMP
                           << COMMAND_RELATES
                           << COMMAND_CONTENTSPAGE
                           << COMMAND_NEXTPAGE
                           << COMMAND_PREVIOUSPAGE
                           << COMMAND_INDEXPAGE
                           << COMMAND_STARTPAGE
                           << COMMAND_QMLINHERITS
                           << COMMAND_QMLINSTANTIATES
                           << COMMAND_QMLDEFAULT
                           << COMMAND_QMLREADONLY
                           << COMMAND_QMLABSTRACT
                           << COMMAND_ABSTRACT;
    }
}

/*!
  The destructor is trivial.
 */
CppCodeParser::~CppCodeParser()
{
    // nothing.
}

/*!
  The constructor initializes a map of special node types
  for identifying important nodes. And it initializes
  some filters for identifying and excluding certain kinds of files.
 */
void CppCodeParser::initializeParser(const Config &config)
{
    CodeParser::initializeParser(config);

    /*
      All these can appear in a C++ namespace. Don't add
      anything that can't be in a C++ namespace.
     */
    nodeTypeMap.insert(COMMAND_NAMESPACE, Node::Namespace);
    nodeTypeMap.insert(COMMAND_CLASS, Node::Class);
    nodeTypeMap.insert(COMMAND_ENUM, Node::Enum);
    nodeTypeMap.insert(COMMAND_TYPEALIAS, Node::Typedef);
    nodeTypeMap.insert(COMMAND_TYPEDEF, Node::Typedef);
    nodeTypeMap.insert(COMMAND_PROPERTY, Node::Property);
    nodeTypeMap.insert(COMMAND_VARIABLE, Node::Variable);

    exampleFiles = config.getCanonicalPathList(CONFIG_EXAMPLES);
    exampleDirs = config.getCanonicalPathList(CONFIG_EXAMPLEDIRS);
    QStringList exampleFilePatterns = config.getStringList(
                CONFIG_EXAMPLES + Config::dot + CONFIG_FILEEXTENSIONS);

    // Used for excluding dirs and files from the list of example files
    excludeDirs = QSet<QString>::fromList(config.getCanonicalPathList(CONFIG_EXCLUDEDIRS));
    excludeFiles = QSet<QString>::fromList(config.getCanonicalPathList(CONFIG_EXCLUDEFILES));

    if (!exampleFilePatterns.isEmpty())
        exampleNameFilter = exampleFilePatterns.join(' ');
    else
        exampleNameFilter = "*.cpp *.h *.js *.xq *.svg *.xml *.dita *.ui";

    QStringList exampleImagePatterns = config.getStringList(
                CONFIG_EXAMPLES + Config::dot + CONFIG_IMAGEEXTENSIONS);

    if (!exampleImagePatterns.isEmpty())
        exampleImageFilter = exampleImagePatterns.join(' ');
    else
        exampleImageFilter = "*.png";
}

/*!
  Clear the map of common node types and call
  the same function in the base class.
 */
void CppCodeParser::terminateParser()
{
    nodeTypeMap.clear();
    excludeDirs.clear();
    excludeFiles.clear();
    CodeParser::terminateParser();
}

/*!
  Returns "Cpp".
 */
QString CppCodeParser::language()
{
    return "Cpp";
}

/*!
  Returns a list of extensions for header files.
 */
QStringList CppCodeParser::headerFileNameFilter()
{
    return QStringList();
}

/*!
  Returns a list of extensions for source files, i.e. not
  header files.
 */
QStringList CppCodeParser::sourceFileNameFilter()
{
    return QStringList();
}

/*!
  Returns the set of strings reopresenting the topic commands.
 */
const QSet<QString>& CppCodeParser::topicCommands()
{
    return topicCommands_;
}

/*!
  Process the topic \a command found in the \a doc with argument \a arg.
 */
Node* CppCodeParser::processTopicCommand(const Doc& doc,
                                         const QString& command,
                                         const ArgLocPair& arg)
{
    ExtraFuncData extra;
    if (command == COMMAND_FN) {
        Q_UNREACHABLE();
    }
    else if (nodeTypeMap.contains(command)) {
        /*
          We should only get in here if the command refers to
          something that can appear in a C++ namespace,
          i.e. a class, another namespace, an enum, a typedef,
          a property or a variable. I think these are handled
          this way to allow the writer to refer to the entity
          without including the namespace qualifier.
         */
        Node::NodeType type =  nodeTypeMap[command];
        QStringList words = arg.first.split(QLatin1Char(' '));
        QStringList path = words[0].split("::");
        Node *node = 0;

        node = qdb_->findNodeInOpenNamespace(path, type);
        if (node == 0)
            node = qdb_->findNodeByNameAndType(path, type);
        if (node == 0) {
            if (isWorthWarningAbout(doc)) {
                doc.location().warning(tr("Cannot find '%1' specified with '\\%2' in any header file")
                                       .arg(arg.first).arg(command));
            }
            lastPath_ = path;

        }
        else if (node->isAggregate()) {
            if (type == Node::Namespace) {
                NamespaceNode* ns = static_cast<NamespaceNode*>(node);
                ns->markSeen();
                ns->setWhereDocumented(ns->tree()->camelCaseModuleName());
            }
            /*
              This treats a class as a namespace.
             */
            if ((type == Node::Class) || (type == Node::Namespace)) {
                if (path.size() > 1) {
                    path.pop_back();
                    QString ns = path.join(QLatin1String("::"));
                    qdb_->insertOpenNamespace(ns);
                }
            }
        }
        return node;
    }
    else if (command == COMMAND_EXAMPLE) {
        if (Config::generateExamples) {
            ExampleNode* en = new ExampleNode(qdb_->primaryTreeRoot(), arg.first);
            en->setLocation(doc.startLocation());
            createExampleFileNodes(en);
            return en;
        }
    }
    else if (command == COMMAND_EXTERNALPAGE) {
        DocumentNode* dn = new DocumentNode(qdb_->primaryTreeRoot(),
                                            arg.first,
                                            Node::ExternalPage,
                                            Node::ArticlePage);
        dn->setLocation(doc.startLocation());
        return dn;
    }
    else if (command == COMMAND_FILE) {
        DocumentNode* dn = new DocumentNode(qdb_->primaryTreeRoot(),
                                            arg.first,
                                            Node::File,
                                            Node::NoPageType);
        dn->setLocation(doc.startLocation());
        return dn;
    }
    else if (command == COMMAND_HEADERFILE) {
        DocumentNode* dn = new DocumentNode(qdb_->primaryTreeRoot(),
                                            arg.first,
                                            Node::HeaderFile,
                                            Node::ApiPage);
        dn->setLocation(doc.startLocation());
        return dn;
    }
    else if (command == COMMAND_GROUP) {
        CollectionNode* cn = qdb_->addGroup(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    }
    else if (command == COMMAND_MODULE) {
        CollectionNode* cn = qdb_->addModule(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    }
    else if (command == COMMAND_QMLMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode* cn = qdb_->addQmlModule(blankSplit[0]);
        cn->setLogicalModuleInfo(blankSplit);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    }
    else if (command == COMMAND_JSMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode* cn = qdb_->addJsModule(blankSplit[0]);
        cn->setLogicalModuleInfo(blankSplit);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    }
    else if (command == COMMAND_PAGE) {
        Node::PageType ptype = Node::ArticlePage;
        QStringList args = arg.first.split(QLatin1Char(' '));
        if (args.size() > 1) {
            QString t = args[1].toLower();
            if (t == "howto")
                ptype = Node::HowToPage;
            else if (t == "api")
                ptype = Node::ApiPage;
            else if (t == "example")
                ptype = Node::ExamplePage;
            else if (t == "overview")
                ptype = Node::OverviewPage;
            else if (t == "tutorial")
                ptype = Node::TutorialPage;
            else if (t == "faq")
                ptype = Node::FAQPage;
            else if (t == "ditamap")
                ptype = Node::DitaMapPage;
            else if (t == "attribution")
                ptype = Node::AttributionPage;
        }
        DocumentNode* dn = 0;
        if (ptype == Node::DitaMapPage)
            dn = new DitaMapNode(qdb_->primaryTreeRoot(), args[0]);
        else
            dn = new DocumentNode(qdb_->primaryTreeRoot(), args[0], Node::Page, ptype);
        dn->setLocation(doc.startLocation());
        return dn;
    }
    else if (command == COMMAND_DITAMAP) {
        DocumentNode* dn = new DitaMapNode(qdb_->primaryTreeRoot(), arg.first);
        dn->setLocation(doc.startLocation());
        return dn;
    }
    else if ((command == COMMAND_QMLTYPE) || (command == COMMAND_JSTYPE)) {
        QmlTypeNode* qcn = new QmlTypeNode(qdb_->primaryTreeRoot(), arg.first);
        if (command == COMMAND_JSTYPE)
            qcn->setGenus(Node::JS);
        qcn->setLocation(doc.startLocation());
        return qcn;
    }
    else if ((command == COMMAND_QMLBASICTYPE) || (command == COMMAND_JSBASICTYPE)) {
        QmlBasicTypeNode* n = new QmlBasicTypeNode(qdb_->primaryTreeRoot(), arg.first);
        if (command == COMMAND_JSBASICTYPE)
            n->setGenus(Node::JS);
        n->setLocation(doc.startLocation());
        return n;
    }
    else if ((command == COMMAND_QMLSIGNAL) ||
             (command == COMMAND_QMLMETHOD) ||
             (command == COMMAND_QMLATTACHEDSIGNAL) ||
             (command == COMMAND_QMLATTACHEDMETHOD) ||
             (command == COMMAND_JSSIGNAL) ||
             (command == COMMAND_JSMETHOD) ||
             (command == COMMAND_JSATTACHEDSIGNAL) ||
             (command == COMMAND_JSATTACHEDMETHOD)) {
        Q_UNREACHABLE();
    }
    return 0;
}

/*!
  A QML property group argument has the form...

  <QML-module>::<QML-type>::<name>

  This function splits the argument into those parts.
  A <QML-module> is the QML equivalent of a C++ namespace.
  So this function splits \a arg on "::" and stores the
  parts in \a module, \a qmlTypeName, and \a name, and returns
  true. If any part is not found, a qdoc warning is emitted
  and false is returned.
 */
bool CppCodeParser::splitQmlPropertyGroupArg(const QString& arg,
                                             QString& module,
                                             QString& qmlTypeName,
                                             QString& name,
                                             const Location& location)
{
    QStringList colonSplit = arg.split("::");
    if (colonSplit.size() == 3) {
        module = colonSplit[0];
        qmlTypeName = colonSplit[1];
        name = colonSplit[2];
        return true;
    }
    QString msg = "Unrecognizable QML module/component qualifier for " + arg;
    location.warning(tr(msg.toLatin1().data()));
    return false;
}

/*!
  A QML property argument has the form...

  <type> <QML-type>::<name>
  <type> <QML-module>::<QML-type>::<name>

  This function splits the argument into one of those
  two forms. The three part form is the old form, which
  was used before the creation of Qt Quick 2 and Qt
  Components. A <QML-module> is the QML equivalent of a
  C++ namespace. So this function splits \a arg on "::"
  and stores the parts in \a type, \a module, \a qmlTypeName,
  and \a name, and returns \c true. If any part other than
  \a module is not found, a qdoc warning is emitted and
  false is returned.

  \note The two QML types \e{Component} and \e{QtObject}
  never have a module qualifier.
 */
bool CppCodeParser::splitQmlPropertyArg(const QString& arg,
                                        QString& type,
                                        QString& module,
                                        QString& qmlTypeName,
                                        QString& name,
                                        const Location &location)
{
    QStringList blankSplit = arg.split(QLatin1Char(' '));
    if (blankSplit.size() > 1) {
        type = blankSplit[0];
        QStringList colonSplit(blankSplit[1].split("::"));
        if (colonSplit.size() == 3) {
            module = colonSplit[0];
            qmlTypeName = colonSplit[1];
            name = colonSplit[2];
            return true;
        }
        if (colonSplit.size() == 2) {
            module.clear();
            qmlTypeName = colonSplit[0];
            name = colonSplit[1];
            return true;
        }
        QString msg = "Unrecognizable QML module/component qualifier for " + arg;
        location.warning(tr(msg.toLatin1().data()));
    }
    else {
        QString msg = "Missing property type for " + arg;
        location.warning(tr(msg.toLatin1().data()));
    }
    return false;
}

/*!
  Process the topic \a command group found in the \a doc with arguments \a args.

  Currently, this function is called only for \e{qmlproperty}
  and \e{qmlattachedproperty}.
 */
void CppCodeParser::processQmlProperties(const Doc& doc,
                                         NodeList& nodes,
                                         DocList& docs,
                                         bool jsProps)
{
    QString arg;
    QString type;
    QString topic;
    QString module;
    QString qmlTypeName;
    QString property;
    QmlPropertyNode* qpn = 0;
    QmlTypeNode* qmlType = 0;
    QmlPropertyGroupNode* qpgn = 0;

    Topic qmlPropertyGroupTopic;
    const TopicList& topics = doc.topicsUsed();
    for (int i=0; i<topics.size(); ++i) {
        if ((topics.at(i).topic == COMMAND_QMLPROPERTYGROUP) ||
            (topics.at(i).topic == COMMAND_JSPROPERTYGROUP)) {
            qmlPropertyGroupTopic = topics.at(i);
            break;
        }
    }
    if (qmlPropertyGroupTopic.isEmpty() && topics.size() > 1) {
        qmlPropertyGroupTopic = topics.at(0);
        if (jsProps)
            qmlPropertyGroupTopic.topic = COMMAND_JSPROPERTYGROUP;
        else
            qmlPropertyGroupTopic.topic = COMMAND_QMLPROPERTYGROUP;
        arg = qmlPropertyGroupTopic.args;
        if (splitQmlPropertyArg(arg, type, module, qmlTypeName, property, doc.location())) {
            int i = property.indexOf('.');
            if (i != -1) {
                property = property.left(i);
                qmlPropertyGroupTopic.args = module + "::" + qmlTypeName + "::" + property;
                doc.location().warning(tr("No QML property group command found; using \\%1 %2")
                                       .arg(COMMAND_QMLPROPERTYGROUP).arg(qmlPropertyGroupTopic.args));
            }
            else {
                /*
                  Assumption: No '.' in the property name
                  means there is no property group.
                 */
                qmlPropertyGroupTopic.clear();
            }
        }
    }

    if (!qmlPropertyGroupTopic.isEmpty()) {
        arg = qmlPropertyGroupTopic.args;
        if (splitQmlPropertyGroupArg(arg, module, qmlTypeName, property, doc.location())) {
            qmlType = qdb_->findQmlType(module, qmlTypeName);
            if (qmlType) {
                qpgn = new QmlPropertyGroupNode(qmlType, property);
                qpgn->setLocation(doc.startLocation());
                if (jsProps)
                    qpgn->setGenus(Node::JS);
                nodes.append(qpgn);
                docs.append(doc);
            }
        }
    }
    for (int i=0; i<topics.size(); ++i) {
        if (topics.at(i).topic == COMMAND_QMLPROPERTYGROUP) {
             continue;
        }
        topic = topics.at(i).topic;
        arg = topics.at(i).args;
        if ((topic == COMMAND_QMLPROPERTY) || (topic == COMMAND_QMLATTACHEDPROPERTY) ||
            (topic == COMMAND_JSPROPERTY) || (topic == COMMAND_JSATTACHEDPROPERTY)) {
            bool attached = ((topic == COMMAND_QMLATTACHEDPROPERTY) ||
                             (topic == COMMAND_JSATTACHEDPROPERTY));
            if (splitQmlPropertyArg(arg, type, module, qmlTypeName, property, doc.location())) {
                Aggregate* aggregate = qdb_->findQmlType(module, qmlTypeName);
                if (!aggregate)
                    aggregate = qdb_->findQmlBasicType(module, qmlTypeName);
                if (aggregate) {
                    if (aggregate->hasQmlProperty(property, attached) != 0) {
                        QString msg = tr("QML property documented multiple times: '%1'").arg(arg);
                        doc.startLocation().warning(msg);
                    }
                    else if (qpgn) {
                        qpn = new QmlPropertyNode(qpgn, property, type, attached);
                        qpn->setLocation(doc.startLocation());
                        if (jsProps)
                            qpn->setGenus(Node::JS);
                    }
                    else {
                        qpn = new QmlPropertyNode(aggregate, property, type, attached);
                        qpn->setLocation(doc.startLocation());
                        if (jsProps)
                            qpn->setGenus(Node::JS);
                        nodes.append(qpn);
                        docs.append(doc);
                    }
                }
            }
        } else if (qpgn) {
            doc.startLocation().warning(
                tr("Invalid use of '\\%1'; not allowed in a '\\%2'").arg(
                    topic, qmlPropertyGroupTopic.topic));
        }
    }
}

/*!
  Returns the set of strings representing the common metacommands
  plus some other metacommands.
 */
const QSet<QString>& CppCodeParser::otherMetaCommands()
{
    return otherMetaCommands_;
}

/*!
  Process the metacommand \a command in the context of the
  \a node associated with the topic command and the \a doc.
  \a arg is the argument to the metacommand.
 */
void CppCodeParser::processOtherMetaCommand(const Doc& doc,
                                            const QString& command,
                                            const ArgLocPair& argLocPair,
                                            Node *node)
{
    QString arg = argLocPair.first;
    if (command == COMMAND_INHEADERFILE) {
        if (node != 0 && node->isAggregate()) {
            ((Aggregate *) node)->addInclude(arg);
        }
        else {
            doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_INHEADERFILE));
        }
    }
    else if (command == COMMAND_OVERLOAD) {
        if (node && (node->isFunction() || node->isSharedCommentNode()))
            node->setOverloadFlag(true);
        else
            doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_OVERLOAD));
    }
    else if (command == COMMAND_REIMP) {
        if (node != 0 && node->parent() && !node->parent()->isInternal()) {
            if (node->type() == Node::Function) {
                FunctionNode *func = (FunctionNode *) node;
                if (func->reimplementedFrom().isEmpty() && isWorthWarningAbout(doc)) {
                    doc.location().warning(tr("Cannot find base function for '\\%1' in %2()")
                                           .arg(COMMAND_REIMP).arg(node->name()),
                                           tr("The function either doesn't exist in any "
                                              "base class with the same signature or it "
                                              "exists but isn't virtual."));
                }
                func->setReimplemented(true);
            }
            else {
                doc.location().warning(tr("Ignored '\\%1' in %2").arg(COMMAND_REIMP).arg(node->name()));
            }
        }
    }
    else if (command == COMMAND_RELATES) {
        QStringList path = arg.split("::");
        Node* n = qdb_->findRelatesNode(path);
        if (!n) {
            // Store just a string to write to the index file
            if (Generator::preparing())
                node->setRelates(arg);
            else
                doc.location().warning(tr("Cannot find '%1' in '\\%2'").arg(arg).arg(COMMAND_RELATES));

        }
        else if (node->parent() != n)
            node->setRelates(static_cast<Aggregate*>(n));
        else
            doc.location().warning(tr("Invalid use of '\\%1' (already a member of '%2')")
                                   .arg(COMMAND_RELATES, arg));
    }
    else if (command == COMMAND_CONTENTSPAGE) {
        setLink(node, Node::ContentsLink, arg);
    }
    else if (command == COMMAND_NEXTPAGE) {
        setLink(node, Node::NextLink, arg);
    }
    else if (command == COMMAND_PREVIOUSPAGE) {
        setLink(node, Node::PreviousLink, arg);
    }
    else if (command == COMMAND_INDEXPAGE) {
        setLink(node, Node::IndexLink, arg);
    }
    else if (command == COMMAND_STARTPAGE) {
        setLink(node, Node::StartLink, arg);
    }
    else if (command == COMMAND_QMLINHERITS) {
        if (node->name() == arg)
            doc.location().warning(tr("%1 tries to inherit itself").arg(arg));
        else if (node->isQmlType() || node->isJsType()) {
            QmlTypeNode* qmlType = static_cast<QmlTypeNode*>(node);
            qmlType->setQmlBaseName(arg);
        }
    }
    else if (command == COMMAND_QMLINSTANTIATES) {
        if (node->isQmlType() || node->isJsType()) {
            ClassNode* classNode = qdb_->findClassNode(arg.split("::"));
            if (classNode)
                node->setClassNode(classNode);
            else
                doc.location().warning(tr("C++ class %1 not found: \\instantiates %1").arg(arg));
        }
        else
            doc.location().warning(tr("\\instantiates is only allowed in \\qmltype"));
    }
    else if (command == COMMAND_QMLDEFAULT) {
        if (node->type() == Node::QmlProperty) {
            QmlPropertyNode* qpn = static_cast<QmlPropertyNode*>(node);
            qpn->setDefault();
        }
        else if (node->type() == Node::QmlPropertyGroup) {
            QmlPropertyGroupNode* qpgn = static_cast<QmlPropertyGroupNode*>(node);
            NodeList::ConstIterator p = qpgn->childNodes().constBegin();
            while (p != qpgn->childNodes().constEnd()) {
                if ((*p)->type() == Node::QmlProperty) {
                    QmlPropertyNode* qpn = static_cast<QmlPropertyNode*>(*p);
                    qpn->setDefault();
                }
                ++p;
            }
        }
    }
    else if (command == COMMAND_QMLREADONLY) {
        if (node->type() == Node::QmlProperty) {
            QmlPropertyNode* qpn = static_cast<QmlPropertyNode*>(node);
            qpn->setReadOnly(1);
        }
        else if (node->type() == Node::QmlPropertyGroup) {
            QmlPropertyGroupNode* qpgn = static_cast<QmlPropertyGroupNode*>(node);
            NodeList::ConstIterator p = qpgn->childNodes().constBegin();
            while (p != qpgn->childNodes().constEnd()) {
                if ((*p)->type() == Node::QmlProperty) {
                    QmlPropertyNode* qpn = static_cast<QmlPropertyNode*>(*p);
                    qpn->setReadOnly(1);
                }
                ++p;
            }
        }
    }
    else if ((command == COMMAND_QMLABSTRACT) || (command == COMMAND_ABSTRACT)) {
        if (node->isQmlType() || node->isJsType())
            node->setAbstract(true);
    }
    else {
        processCommonMetaCommand(doc.location(),command,argLocPair,node);
    }
}

/*!
  The topic command has been processed resulting in the \a doc
  and \a node passed in here. Process the other meta commands,
  which are found in \a doc, in the context of the topic \a node.
 */
void CppCodeParser::processOtherMetaCommands(const Doc& doc, Node *node)
{
    QStringList metaCommands = doc.metaCommandsUsed().toList();
    metaCommands.sort();
    QStringList::ConstIterator cmd = metaCommands.constBegin();
    while (cmd != metaCommands.constEnd()) {
        ArgList args = doc.metaCommandArgs(*cmd);
        ArgList::ConstIterator arg = args.constBegin();
        while (arg != args.constEnd()) {
            processOtherMetaCommand(doc, *cmd, *arg, node);
            ++arg;
        }
        ++cmd;
    }
}

/*!
  Resets the C++ code parser to its default initialized state.
 */
void CppCodeParser::reset()
{
    tokenizer = 0;
    tok = 0;
    access = Node::Public;
    metaness_ = FunctionNode::Plain;
    lastPath_.clear();
    physicalModuleName.clear();
}

/*!
  Get the next token from the file being parsed and store it
  in the token variable.
 */
void CppCodeParser::readToken()
{
    tok = tokenizer->getToken();
}

/*!
  Return the current location in the file being parsed,
  i.e. the file name, line number, and column number.
 */
const Location& CppCodeParser::location()
{
    return tokenizer->location();
}

/*!
  Return the previous string read from the file being parsed.
 */
QString CppCodeParser::previousLexeme()
{
    return tokenizer->previousLexeme();
}

/*!
  Return the current string string from the file being parsed.
 */
QString CppCodeParser::lexeme()
{
    return tokenizer->lexeme();
}

bool CppCodeParser::match(int target)
{
    if (tok == target) {
        readToken();
        return true;
    }
    return false;
}

/*!
  Skip to \a target. If \a target is found before the end
  of input, return true. Otherwise return false.
 */
bool CppCodeParser::skipTo(int target)
{
    while ((tok != Tok_Eoi) && (tok != target))
        readToken();
    return tok == target;
}

bool CppCodeParser::matchModuleQualifier(QString& name)
{
    bool matches = (lexeme() == QString('.'));
    if (matches) {
        do {
            name += lexeme();
            readToken();
        } while ((tok == Tok_Ident) || (lexeme() == QString('.')));
    }
    return matches;
}

bool CppCodeParser::matchTemplateAngles(CodeChunk *dataType)
{
    bool matches = (tok == Tok_LeftAngle);
    if (matches) {
        int leftAngleDepth = 0;
        int parenAndBraceDepth = 0;
        do {
            if (tok == Tok_LeftAngle) {
                leftAngleDepth++;
            }
            else if (tok == Tok_RightAngle) {
                leftAngleDepth--;
            }
            else if (tok == Tok_LeftParen || tok == Tok_LeftBrace) {
                ++parenAndBraceDepth;
            }
            else if (tok == Tok_RightParen || tok == Tok_RightBrace) {
                if (--parenAndBraceDepth < 0)
                    return false;
            }
            if (dataType != 0)
                dataType->append(lexeme());
            readToken();
        } while (leftAngleDepth > 0 && tok != Tok_Eoi);
    }
    return matches;
}

bool CppCodeParser::matchDataType(CodeChunk *dataType, QString *var, bool qProp)
{
    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
    */
    for (;;) {
        bool virgin = true;

        if (tok != Tok_Ident) {
            /*
              There is special processing for 'Foo::operator int()'
              and such elsewhere. This is the only case where we
              return something with a trailing gulbrandsen ('Foo::').
            */
            if (tok == Tok_operator)
                return true;

            /*
              People may write 'const unsigned short' or
              'short unsigned const' or any other permutation.
            */
            while (match(Tok_const) || match(Tok_volatile))
                dataType->append(previousLexeme());
            QString pending;
            while (tok == Tok_signed || tok == Tok_int || tok == Tok_unsigned ||
                   tok == Tok_short || tok == Tok_long || tok == Tok_int64) {
                if (tok == Tok_signed)
                    pending = lexeme();
                else {
                    if (tok == Tok_unsigned && !pending.isEmpty())
                        dataType->append(pending);
                    pending.clear();
                    dataType->append(lexeme());
                }
                readToken();
                virgin = false;
            }
            if (!pending.isEmpty()) {
                dataType->append(pending);
                pending.clear();
            }
            while (match(Tok_const) || match(Tok_volatile))
                dataType->append(previousLexeme());

            if (match(Tok_Tilde))
                dataType->append(previousLexeme());
        }

        if (virgin) {
            if (match(Tok_Ident)) {
                /*
                  This is a hack until we replace this "parser"
                  with the real one used in Qt Creator.
                 */
                if (!inMacroCommand_ && lexeme() == "(" &&
                    ((previousLexeme() == "QT_PREPEND_NAMESPACE") || (previousLexeme() == "NS"))) {
                    readToken();
                    readToken();
                    dataType->append(previousLexeme());
                    readToken();
                }
                else
                    dataType->append(previousLexeme());
            }
            else if (match(Tok_void) || match(Tok_int) || match(Tok_char) ||
                     match(Tok_double) || match(Tok_Ellipsis)) {
                dataType->append(previousLexeme());
            }
            else {
                return false;
            }
        }
        else if (match(Tok_int) || match(Tok_char) || match(Tok_double)) {
            dataType->append(previousLexeme());
        }

        matchTemplateAngles(dataType);

        while (match(Tok_const) || match(Tok_volatile))
            dataType->append(previousLexeme());

        if (match(Tok_Gulbrandsen))
            dataType->append(previousLexeme());
        else
            break;
    }

    while (match(Tok_Ampersand) || match(Tok_Aster) || match(Tok_const) ||
           match(Tok_Caret) || match(Tok_Ellipsis))
        dataType->append(previousLexeme());

    if (match(Tok_LeftParenAster)) {
        /*
          A function pointer. This would be rather hard to handle without a
          tokenizer hack, because a type can be followed with a left parenthesis
          in some cases (e.g., 'operator int()'). The tokenizer recognizes '(*'
          as a single token.
        */
        dataType->append(" "); // force a space after the type
        dataType->append(previousLexeme());
        dataType->appendHotspot();
        if (var != 0 && match(Tok_Ident))
            *var = previousLexeme();
        if (!match(Tok_RightParen))
            return false;
        dataType->append(previousLexeme());
        if (!match(Tok_LeftParen))
            return false;
        dataType->append(previousLexeme());

        /* parse the parameters. Ignore the parameter name from the type */
        while (tok != Tok_RightParen && tok != Tok_Eoi) {
            QString dummy;
            if (!matchDataType(dataType, &dummy))
                return false;
            if (match(Tok_Comma))
                dataType->append(previousLexeme());
        }
        if (!match(Tok_RightParen))
            return false;
        dataType->append(previousLexeme());
    }
    else {
        /*
          The common case: Look for an optional identifier, then for
          some array brackets.
        */
        dataType->appendHotspot();

        if (var != 0) {
            if (match(Tok_Ident)) {
                *var = previousLexeme();
            }
            else if (match(Tok_Comment)) {
                /*
                  A neat hack: Commented-out parameter names are
                  recognized by qdoc. It's impossible to illustrate
                  here inside a C-style comment, because it requires
                  an asterslash. It's also impossible to illustrate
                  inside a C++-style comment, because the explanation
                  does not fit on one line.
                */
                if (varComment.exactMatch(previousLexeme()))
                    *var = varComment.cap(1);
            }
            else if (match(Tok_LeftParen)) {
                *var = "(";
                while (tok != Tok_RightParen && tok != Tok_Eoi) {
                    (*var).append(lexeme());
                    readToken();
                }
                (*var).append(")");
                readToken();
                if (match(Tok_LeftBracket)) {
                    (*var).append("[");
                    while (tok != Tok_RightBracket && tok != Tok_Eoi) {
                        (*var).append(lexeme());
                        readToken();
                    }
                    (*var).append("]");
                    readToken();
                }
            }
            else if (qProp && (match(Tok_default) || match(Tok_final) || match(Tok_override))) {
                // Hack to make 'default', 'final' and 'override'  work again in Q_PROPERTY
                *var = previousLexeme();
            }
        }

        if (tok == Tok_LeftBracket) {
            int bracketDepth0 = tokenizer->bracketDepth();
            while ((tokenizer->bracketDepth() >= bracketDepth0 &&
                    tok != Tok_Eoi) ||
                   tok == Tok_RightBracket) {
                dataType->append(lexeme());
                readToken();
            }
        }
    }
    return true;
}

/*!
  Parse the next function parameter, if there is one, and
  append it to parameter vector \a pvect. Return true if
  a parameter is parsed and appended to \a pvect.
  Otherwise return false.
 */
bool CppCodeParser::matchParameter(QVector<Parameter>& pvect, bool& isQPrivateSignal)
{
    if (match(Tok_QPrivateSignal)) {
        isQPrivateSignal = true;
        return true;
    }

    Parameter p;
    CodeChunk chunk;
    if (!matchDataType(&chunk, &p.name_)) {
        return false;
    }
    p.dataType_ = chunk.toString();
    chunk.clear();
    match(Tok_Comment);
    if (match(Tok_Equal)) {
        int pdepth = tokenizer->parenDepth();
        while (tokenizer->parenDepth() >= pdepth &&
               (tok != Tok_Comma || (tokenizer->parenDepth() > pdepth)) &&
               tok != Tok_Eoi) {
            chunk.append(lexeme());
            readToken();
        }
    }
    p.defaultValue_ = chunk.toString();
    pvect.append(p);
    return true;
}

/*!
  Match a C++ \c using clause. Return \c true if the match
  is successful. Otherwise false.

  If the \c using clause is for a namespace, an open namespace
  <is inserted for qdoc to look in to find things.

  If the \c using clause is a base class member function, the
  member function is added to \a parent as an unresolved
  \c using clause.
 */
bool CppCodeParser::matchUsingDecl(Aggregate* parent)
{
    bool usingNamespace = false;
    readToken(); // skip 'using'

    if (tok == Tok_namespace) {
        usingNamespace = true;
        readToken();
    }

    int openLeftAngles = 0;
    int openLeftParens = 0;
    bool usingOperator = false;
    QString name;
    while (tok != Tok_Semicolon) {
        if ((tok != Tok_Ident) && (tok != Tok_Gulbrandsen)) {
            if (tok == Tok_LeftAngle) {
                ++openLeftAngles;
            }
            else if (tok == Tok_RightAngle) {
                if (openLeftAngles <= 0)
                    return false;
                --openLeftAngles;
            }
            else if (tok == Tok_Comma) {
                if (openLeftAngles <= 0)
                    return false;
            }
            else if (tok == Tok_operator) {
                usingOperator = true;
            }
            else if (tok == Tok_SomeOperator) {
                if (!usingOperator)
                    return false;
            }
            else if (tok == Tok_LeftParen) {
                ++openLeftParens;
            }
            else if (tok == Tok_RightParen) {
                if (openLeftParens <= 0)
                    return false;
                --openLeftParens;
            }
            else {
                return false;
            }
        }
        name += lexeme();
        readToken();
    }

    if (usingNamespace) {
        // 'using namespace Foo;'.
        qdb_->insertOpenNamespace(name);
    }
    else if (parent && parent->isClass()) {
        ClassNode* cn = static_cast<ClassNode*>(parent);
        cn->addUnresolvedUsingClause(name);
    }
    return true;
}

/*!
  This function uses a Tokenizer to parse the \a parameters of a
  function into the parameter vector \a {pvect}.
 */
bool CppCodeParser::parseParameters(const QString& parameters,
                                    QVector<Parameter>& pvect,
                                    bool& isQPrivateSignal)
{
    Tokenizer* outerTokenizer = tokenizer;
    int outerTok = tok;

    QByteArray latin1 = parameters.toLatin1();
    Tokenizer stringTokenizer(Location(), latin1);
    stringTokenizer.setParsingFnOrMacro(true);
    tokenizer = &stringTokenizer;
    readToken();

    inMacroCommand_ = false;
    do {
        if (!matchParameter(pvect, isQPrivateSignal))
            return false;
    } while (match(Tok_Comma));

    tokenizer = outerTokenizer;
    tok = outerTok;
    return true;
}

/*!
 Parse QML/JS signal/method topic commands.
 */
Node* CppCodeParser::parseOtherFuncArg(const QString& topic,
                                         const Location& location,
                                         const QString& funcArg)
{
    QString funcName;
    QString returnType;

    int leftParen = funcArg.indexOf(QChar('('));
    if (leftParen > 0)
        funcName = funcArg.left(leftParen);
    else
        funcName = funcArg;
    int firstBlank = funcName.indexOf(QChar(' '));
    if (firstBlank > 0) {
        returnType = funcName.left(firstBlank);
        funcName = funcName.right(funcName.length() - firstBlank - 1);
    }

    QStringList colonSplit(funcName.split("::"));
    if (colonSplit.size() < 2) {
        QString msg = "Unrecognizable QML module/component qualifier for " + funcArg;
        location.warning(tr(msg.toLatin1().data()));
        return 0;
    }
    QString moduleName;
    QString elementName;
    if (colonSplit.size() > 2) {
        moduleName = colonSplit[0];
        elementName = colonSplit[1];
    } else {
        elementName = colonSplit[0];
    }
    funcName = colonSplit.last();

    Aggregate *aggregate = qdb_->findQmlType(moduleName, elementName);
    bool attached = false;
    if (!aggregate)
        aggregate = qdb_->findQmlBasicType(moduleName, elementName);
    if (!aggregate)
        return 0;

    Node::NodeType nodeType = Node::QmlMethod;
    if (topic == COMMAND_QMLSIGNAL || topic == COMMAND_JSSIGNAL) {
        nodeType = Node::QmlSignal;
    } else if (topic == COMMAND_QMLATTACHEDSIGNAL || topic == COMMAND_JSATTACHEDSIGNAL) {
        nodeType = Node::QmlSignal;
        attached = true;
    } else if (topic == COMMAND_QMLATTACHEDMETHOD || topic == COMMAND_JSATTACHEDMETHOD) {
        attached = true;
    } else {
        Q_ASSERT(topic == COMMAND_QMLMETHOD || topic == COMMAND_JSMETHOD);
    }

    QString params;
    QStringList leftParenSplit = funcArg.split('(');
    if (leftParenSplit.size() > 1) {
        QStringList rightParenSplit = leftParenSplit[1].split(')');
        if (rightParenSplit.size() > 0)
            params = rightParenSplit[0];
    }
    FunctionNode *funcNode = static_cast<FunctionNode*>(new FunctionNode(nodeType, aggregate, funcName, attached));
    funcNode->setAccess(Node::Public);
    funcNode->setLocation(location);
    funcNode->setReturnType(returnType);
    funcNode->setParameters(params);
    return funcNode;
}

/*!
  Parse the \macro arguments ad hoc, without using any actual parser.
 */
Node* CppCodeParser::parseMacroArg(const Location& location, const QString& macroArg)
{
    FunctionNode* newMacroNode = 0;
    QStringList leftParenSplit = macroArg.split('(');
    if (leftParenSplit.size() > 0) {
        QString macroName;
        FunctionNode* oldMacroNode = 0;
        QStringList blankSplit = leftParenSplit[0].split(' ');
        if (blankSplit.size() > 0) {
            macroName = blankSplit.last();
            oldMacroNode = static_cast<FunctionNode*>(qdb_->findMacroNode(macroName));
        }
        QString returnType;
        if (blankSplit.size() > 1) {
            blankSplit.removeLast();
            returnType = blankSplit.join(' ');
        }
        QString params;
        if (leftParenSplit.size() > 1) {
            const QString &afterParen = leftParenSplit.at(1);
            int rightParen = afterParen.indexOf(')');
            if (rightParen >= 0)
                params = afterParen.left(rightParen);
        }
        int i = 0;
        while (i < macroName.length() && !macroName.at(i).isLetter())
            i++;
        if (i > 0) {
            returnType += QChar(' ') + macroName.left(i);
            macroName = macroName.mid(i);
        }
        newMacroNode = static_cast<FunctionNode*>(new FunctionNode(qdb_->primaryTreeRoot(), macroName));
        newMacroNode->setAccess(Node::Public);
        newMacroNode->setLocation(location);
        if (params.isEmpty())
            newMacroNode->setMetaness(FunctionNode::MacroWithoutParams);
        else
            newMacroNode->setMetaness(FunctionNode::MacroWithParams);
        newMacroNode->setReturnType(returnType);
        newMacroNode->setParameters(params);
        if (oldMacroNode && newMacroNode->compare(oldMacroNode)) {
            location.warning(tr("\\macro %1 documented more than once").arg(macroArg));
            oldMacroNode->doc().location().warning(tr("(The previous doc is here)"));
        }
    }
    return newMacroNode;
 }

void CppCodeParser::createExampleFileNodes(DocumentNode *dn)
{
    QString examplePath = dn->name();
    QString proFileName = examplePath + QLatin1Char('/') + examplePath.split(QLatin1Char('/')).last() + ".pro";
    QString fullPath = Config::findFile(dn->doc().location(),
                                        exampleFiles,
                                        exampleDirs,
                                        proFileName);

    if (fullPath.isEmpty()) {
        QString tmp = proFileName;
        proFileName = examplePath + QLatin1Char('/') + "qbuild.pro";
        fullPath = Config::findFile(dn->doc().location(),
                                    exampleFiles,
                                    exampleDirs,
                                    proFileName);
        if (fullPath.isEmpty()) {
            proFileName = examplePath + QLatin1Char('/') + examplePath.split(QLatin1Char('/')).last() + ".qmlproject";
            fullPath = Config::findFile(dn->doc().location(),
                                        exampleFiles,
                                        exampleDirs,
                                        proFileName);
            if (fullPath.isEmpty()) {
                QString details = QLatin1String("Example directories: ") + exampleDirs.join(QLatin1Char(' '));
                if (!exampleFiles.isEmpty())
                    details += QLatin1String(", example files: ") + exampleFiles.join(QLatin1Char(' '));
                dn->location().warning(tr("Cannot find file '%1' or '%2'").arg(tmp).arg(proFileName), details);
                dn->location().warning(tr("  EXAMPLE PATH DOES NOT EXIST: %1").arg(examplePath), details);
                return;
            }
        }
    }

    int sizeOfBoringPartOfName = fullPath.size() - proFileName.size();
    if (fullPath.startsWith("./"))
        sizeOfBoringPartOfName = sizeOfBoringPartOfName - 2;
    fullPath.truncate(fullPath.lastIndexOf('/'));

    QStringList exampleFiles = Config::getFilesHere(fullPath, exampleNameFilter, Location(), excludeDirs, excludeFiles);
    // Search for all image files under the example project, excluding doc/images directory.
    QSet<QString> excludeDocDirs(excludeDirs);
    excludeDocDirs.insert(QDir(fullPath).canonicalPath() + "/doc/images");
    QStringList imageFiles = Config::getFilesHere(fullPath, exampleImageFilter, Location(), excludeDocDirs, excludeFiles);
    if (!exampleFiles.isEmpty()) {
        // move main.cpp and to the end, if it exists
        QString mainCpp;
        QMutableStringListIterator i(exampleFiles);
        i.toBack();
        while (i.hasPrevious()) {
            QString fileName = i.previous();
            if (fileName.endsWith("/main.cpp")) {
                mainCpp = fileName;
                i.remove();
            }
            else if (fileName.contains("/qrc_") || fileName.contains("/moc_")
                     || fileName.contains("/ui_"))
                i.remove();
        }
        if (!mainCpp.isEmpty())
            exampleFiles.append(mainCpp);

        // add any qmake Qt resource files and qmake project files
        exampleFiles += Config::getFilesHere(fullPath, "*.qrc *.pro *.qmlproject qmldir");
    }

    foreach (const QString &exampleFile, exampleFiles) {
        DocumentNode *fileNode = new DocumentNode(dn,
                    exampleFile.mid(sizeOfBoringPartOfName),
                    Node::File,
                    Node::NoPageType);
        if (fileNode->name().endsWith(".qml"))
            fileNode->setGenus(Node::QML);
    }
    foreach (const QString &imageFile, imageFiles) {
        new DocumentNode(dn,
                    imageFile.mid(sizeOfBoringPartOfName),
                    Node::Image,
                    Node::NoPageType);
    }
}

/*!
  returns true if \a t is \e {jssignal}, \e {jsmethod},
  \e {jsattachedsignal}, or \e {jsattachedmethod}.
 */
bool CppCodeParser::isJSMethodTopic(const QString &t)
{
    return (t == COMMAND_JSSIGNAL ||
            t == COMMAND_JSMETHOD ||
            t == COMMAND_JSATTACHEDSIGNAL ||
            t == COMMAND_JSATTACHEDMETHOD);
}

/*!
  returns true if \a t is \e {qmlsignal}, \e {qmlmethod},
  \e {qmlattachedsignal}, or \e {qmlattachedmethod}.
 */
bool CppCodeParser::isQMLMethodTopic(const QString &t)
{
    return (t == COMMAND_QMLSIGNAL ||
            t == COMMAND_QMLMETHOD ||
            t == COMMAND_QMLATTACHEDSIGNAL ||
            t == COMMAND_QMLATTACHEDMETHOD);
}

/*!
  Returns true if \a t is \e {jsproperty}, \e {jspropertygroup},
  or \e {jsattachedproperty}.
 */
bool CppCodeParser::isJSPropertyTopic(const QString &t)
{
    return (t == COMMAND_JSPROPERTY ||
            t == COMMAND_JSPROPERTYGROUP ||
            t == COMMAND_JSATTACHEDPROPERTY);
}

/*!
  Returns true if \a t is \e {qmlproperty}, \e {qmlpropertygroup},
  or \e {qmlattachedproperty}.
 */
bool CppCodeParser::isQMLPropertyTopic(const QString &t)
{
    return (t == COMMAND_QMLPROPERTY ||
            t == COMMAND_QMLPROPERTYGROUP ||
            t == COMMAND_QMLATTACHEDPROPERTY);
}

void CppCodeParser::processTopicArgs(const Doc &doc, const QString &topic, NodeList &nodes, DocList &docs)
{
    if (isQMLPropertyTopic(topic)) {
        processQmlProperties(doc, nodes, docs, false);
    } else if (isJSPropertyTopic(topic)) {
        processQmlProperties(doc, nodes, docs, true);
    } else {
        ArgList args = doc.metaCommandArgs(topic);
        Node *node = 0;
        if (args.size() == 1) {
            if (topic == COMMAND_FN)
                node = parserForLanguage("Clang")->parseFnArg(doc.location(), args[0].first);
            else if (topic == COMMAND_MACRO)
                node = parseMacroArg(doc.location(), args[0].first);
            else if (isQMLMethodTopic(topic) || isJSMethodTopic(topic))
                node = parseOtherFuncArg(topic, doc.location(), args[0].first);
            else
                node = processTopicCommand(doc, topic, args[0]);
            if (node != 0) {
                nodes.append(node);
                docs.append(doc);
            }
        } else if (args.size() > 1) {
            QVector<SharedCommentNode*> sharedCommentNodes;
            ArgList::ConstIterator arg = args.constBegin();
            while (arg != args.constEnd()) {
                if (topic == COMMAND_FN)
                    node = parserForLanguage("Clang")->parseFnArg(doc.location(), arg->first);
                else if (topic == COMMAND_MACRO)
                    node = parseMacroArg(doc.location(), arg->first);
                else if (isQMLMethodTopic(topic) || isJSMethodTopic(topic))
                    node = parseOtherFuncArg(topic, doc.location(), arg->first);
                else
                    node = processTopicCommand(doc, topic, *arg);
                if (node != 0) {
                    bool found = false;
                    for (SharedCommentNode *scn : sharedCommentNodes) {
                        if (scn->parent() == node->parent()) {
                            node->setSharedCommentNode(scn);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        SharedCommentNode *scn = new SharedCommentNode(node);
                        sharedCommentNodes.append(scn);
                        nodes.append(scn);
                        docs.append(doc);
                    }
                }
                ++arg;
            }
        }
    }
}

void CppCodeParser::processOtherMetaCommands(NodeList &nodes, DocList& docs)
{
    NodeList::Iterator n = nodes.begin();
    QList<Doc>::Iterator d = docs.begin();
    while (n != nodes.end()) {
        processOtherMetaCommands(*d, *n);
        (*n)->setDoc(*d);
        checkModuleInclusion(*n);
        if ((*n)->isAggregate() && ((Aggregate *)*n)->includes().isEmpty()) {
            Aggregate *m = static_cast<Aggregate *>(*n);
            while (m->parent() && m->physicalModuleName().isEmpty())
                m = m->parent();
            if (m == *n)
                ((Aggregate *)*n)->addInclude((*n)->name());
            else
                ((Aggregate *)*n)->setIncludes(m->includes());
        }
        ++d;
        ++n;
    }
}

bool CppCodeParser::hasTooManyTopics(const Doc &doc) const
{
    QSet<QString> topicCommandsUsed = topicCommands() & doc.metaCommandsUsed();
    if (topicCommandsUsed.count() > 1) {
        bool ok = true;
        for (const auto &t : topicCommandsUsed) {
            if (!t.startsWith(QLatin1String("qml")) && !t.startsWith(QLatin1String("js")))
                ok = false;
        }
        if (ok)
            return false;
        QString topicList;
        for (const auto &t : topicCommandsUsed)
            topicList += QLatin1String(" \\") + t + QLatin1Char(',');
        topicList[topicList.lastIndexOf(',')] = '.';
        int i = topicList.lastIndexOf(',');
        Q_ASSERT(i >= 0); // we had at least two commas
        topicList[i] = ' ';
        topicList.insert(i + 1, "and");
        doc.location().warning(tr("Multiple topic commands found in comment:%1").arg(topicList));
        return true;
    }
    return false;
}

QT_END_NAMESPACE
