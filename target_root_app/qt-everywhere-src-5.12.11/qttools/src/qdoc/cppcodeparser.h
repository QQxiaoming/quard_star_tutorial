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

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include <qregexp.h>

#include "codeparser.h"

QT_BEGIN_NAMESPACE

class ClassNode;
class CodeChunk;
class CppCodeParserPrivate;
class FunctionNode;
class Aggregate;
class Tokenizer;

class CppCodeParser : public CodeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::CppCodeParser)

    struct ExtraFuncData {
        Aggregate* root; // Used as the parent.
        Node::NodeType type; // The node type: Function, etc.
        bool isAttached; // If true, the method is attached.
        bool isMacro;    // If true, we are parsing a macro signature.
        ExtraFuncData() : root(0), type(Node::Function), isAttached(false), isMacro(false) { }
        ExtraFuncData(Aggregate* r, Node::NodeType t, bool a)
          : root(r), type(t), isAttached(a), isMacro(false) { }
    };

public:
    CppCodeParser();
    ~CppCodeParser();

    void initializeParser(const Config& config) override;
    void terminateParser() override;
    QString language() override;
    QStringList headerFileNameFilter() override;
    QStringList sourceFileNameFilter() override;
    bool parseParameters(const QString& parameters, QVector<Parameter>& pvect, bool& isQPrivateSignal);
    Node *parseMacroArg(const Location &location, const QString &macroArg) override;
    Node *parseOtherFuncArg(const QString &topic,
                            const Location &location,
                            const QString &funcArg) override;
    const Location& declLoc() const { return declLoc_; }
    void setDeclLoc() { declLoc_ = location(); }
    static bool isJSMethodTopic(const QString &t);
    static bool isQMLMethodTopic(const QString &t);
    static bool isJSPropertyTopic(const QString &t);
    static bool isQMLPropertyTopic(const QString &t);

protected:
    static const QSet<QString>& topicCommands();
    static const QSet<QString>& otherMetaCommands();
    virtual Node* processTopicCommand(const Doc& doc,
                                      const QString& command,
                                      const ArgLocPair& arg);
    void processQmlProperties(const Doc& doc, NodeList& nodes, DocList& docs, bool jsProps);
    bool splitQmlPropertyGroupArg(const QString& arg,
                                  QString& module,
                                  QString& element,
                                  QString& name,
                                  const Location& location);
    bool splitQmlPropertyArg(const QString& arg,
                             QString& type,
                             QString& module,
                             QString& element,
                             QString& name,
                             const Location& location);
    virtual void processOtherMetaCommand(const Doc& doc,
                                         const QString& command,
                                         const ArgLocPair& argLocPair,
                                         Node *node);
    void processOtherMetaCommands(const Doc& doc, Node *node);
    void processOtherMetaCommands(NodeList &nodes, DocList& docs);
    void processTopicArgs(const Doc &doc, const QString &topic, NodeList &nodes, DocList &docs);
    bool hasTooManyTopics(const Doc &doc) const;

 protected:
    void reset();
    void readToken();
    const Location& location();
    QString previousLexeme();
    QString lexeme();

 private:
    bool match(int target);
    bool skipTo(int target);
    bool matchModuleQualifier(QString& name);
    bool matchTemplateAngles(CodeChunk *type = 0);
    bool matchDataType(CodeChunk *type, QString *var = 0, bool qProp = false);
    bool matchParameter(QVector<Parameter>& pvect, bool& isQPrivateSignal);
    bool matchUsingDecl(Aggregate* parent);
    void createExampleFileNodes(DocumentNode *dn);

 protected:
    QMap<QString, Node::NodeType> nodeTypeMap;
    Tokenizer *tokenizer;
    int tok;
    Node::Access access;
    FunctionNode::Metaness metaness_;
    QString physicalModuleName;
    QStringList lastPath_;
    QRegExp varComment;
    QRegExp sep;
    Location declLoc_;

 private:

    static QStringList exampleFiles;
    static QStringList exampleDirs;
    static QSet<QString> excludeDirs;
    static QSet<QString> excludeFiles;
    QString exampleNameFilter;
    QString exampleImageFilter;
};

#define COMMAND_ABSTRACT                Doc::alias("abstract")
#define COMMAND_CLASS                   Doc::alias("class")
#define COMMAND_CONTENTSPAGE            Doc::alias("contentspage")
#define COMMAND_DITAMAP                 Doc::alias("ditamap")
#define COMMAND_ENUM                    Doc::alias("enum")
#define COMMAND_EXAMPLE                 Doc::alias("example")
#define COMMAND_EXTERNALPAGE            Doc::alias("externalpage")
#define COMMAND_FILE                    Doc::alias("file")
#define COMMAND_FN                      Doc::alias("fn")
#define COMMAND_GROUP                   Doc::alias("group")
#define COMMAND_HEADERFILE              Doc::alias("headerfile")
#define COMMAND_INDEXPAGE               Doc::alias("indexpage")
#define COMMAND_INHEADERFILE            Doc::alias("inheaderfile")
#define COMMAND_MACRO                   Doc::alias("macro")
#define COMMAND_MODULE                  Doc::alias("module")
#define COMMAND_NAMESPACE               Doc::alias("namespace")
#define COMMAND_OVERLOAD                Doc::alias("overload")
#define COMMAND_NEXTPAGE                Doc::alias("nextpage")
#define COMMAND_PAGE                    Doc::alias("page")
#define COMMAND_PREVIOUSPAGE            Doc::alias("previouspage")
#define COMMAND_PROPERTY                Doc::alias("property")
#define COMMAND_REIMP                   Doc::alias("reimp")
#define COMMAND_RELATES                 Doc::alias("relates")
#define COMMAND_STARTPAGE               Doc::alias("startpage")
#define COMMAND_TYPEALIAS               Doc::alias("typealias")
#define COMMAND_TYPEDEF                 Doc::alias("typedef")
#define COMMAND_VARIABLE                Doc::alias("variable")
#define COMMAND_QMLABSTRACT             Doc::alias("qmlabstract")
#define COMMAND_QMLTYPE                 Doc::alias("qmltype")
#define COMMAND_QMLPROPERTY             Doc::alias("qmlproperty")
#define COMMAND_QMLPROPERTYGROUP        Doc::alias("qmlpropertygroup")
#define COMMAND_QMLATTACHEDPROPERTY     Doc::alias("qmlattachedproperty")
#define COMMAND_QMLINHERITS             Doc::alias("inherits")
#define COMMAND_QMLINSTANTIATES         Doc::alias("instantiates")
#define COMMAND_QMLSIGNAL               Doc::alias("qmlsignal")
#define COMMAND_QMLATTACHEDSIGNAL       Doc::alias("qmlattachedsignal")
#define COMMAND_QMLMETHOD               Doc::alias("qmlmethod")
#define COMMAND_QMLATTACHEDMETHOD       Doc::alias("qmlattachedmethod")
#define COMMAND_QMLDEFAULT              Doc::alias("default")
#define COMMAND_QMLREADONLY             Doc::alias("readonly")
#define COMMAND_QMLBASICTYPE            Doc::alias("qmlbasictype")
#define COMMAND_QMLMODULE               Doc::alias("qmlmodule")
#define COMMAND_AUDIENCE                Doc::alias("audience")
#define COMMAND_CATEGORY                Doc::alias("category")
#define COMMAND_PRODNAME                Doc::alias("prodname")
#define COMMAND_COMPONENT               Doc::alias("component")
#define COMMAND_AUTHOR                  Doc::alias("author")
#define COMMAND_PUBLISHER               Doc::alias("publisher")
#define COMMAND_COPYRYEAR               Doc::alias("copyryear")
#define COMMAND_COPYRHOLDER             Doc::alias("copyrholder")
#define COMMAND_PERMISSIONS             Doc::alias("permissions")
#define COMMAND_LIFECYCLEVERSION        Doc::alias("lifecycleversion")
#define COMMAND_LIFECYCLEWSTATUS        Doc::alias("lifecyclestatus")
#define COMMAND_LICENSEYEAR             Doc::alias("licenseyear")
#define COMMAND_LICENSENAME             Doc::alias("licensename")
#define COMMAND_LICENSEDESCRIPTION      Doc::alias("licensedescription")
#define COMMAND_RELEASEDATE             Doc::alias("releasedate")
#define COMMAND_QTVARIABLE              Doc::alias("qtvariable")
// Some of these are not used currenmtly, but they are included now for completeness.
#define COMMAND_JSTYPE                 Doc::alias("jstype")
#define COMMAND_JSPROPERTY             Doc::alias("jsproperty")
#define COMMAND_JSPROPERTYGROUP        Doc::alias("jspropertygroup")
#define COMMAND_JSATTACHEDPROPERTY     Doc::alias("jsattachedproperty")
#define COMMAND_JSSIGNAL               Doc::alias("jssignal")
#define COMMAND_JSATTACHEDSIGNAL       Doc::alias("jsattachedsignal")
#define COMMAND_JSMETHOD               Doc::alias("jsmethod")
#define COMMAND_JSATTACHEDMETHOD       Doc::alias("jsattachedmethod")
#define COMMAND_JSBASICTYPE            Doc::alias("jsbasictype")
#define COMMAND_JSMODULE               Doc::alias("jsmodule")

QT_END_NAMESPACE

#endif
