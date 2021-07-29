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
  puredocparser.cpp
*/

#include <qfile.h>
#include <stdio.h>
#include <errno.h>
#include "codechunk.h"
#include "config.h"
#include "tokenizer.h"
#include <qdebug.h>
#include "qdocdatabase.h"
#include "puredocparser.h"

QT_BEGIN_NAMESPACE

PureDocParser *PureDocParser::pureParser_ = 0;

/*!
  Constructs the pure doc parser.
*/
PureDocParser::PureDocParser()
{
    pureParser_ = this;
}

/*!
  Destroys the pure doc parser.
 */
PureDocParser::~PureDocParser()
{
    pureParser_ = 0;
}

/*!
  Returns a list of the kinds of files that the pure doc
  parser is meant to parse. The elements of the list are
  file suffixes.
 */
QStringList PureDocParser::sourceFileNameFilter()
{
    return QStringList() << "*.qdoc" << "*.qtx" << "*.qtt" << "*.js";
}

/*!
  Parses the source file identified by \a filePath and adds its
  parsed contents to the database. The \a location is used for
  reporting errors.
 */
void PureDocParser::parseSourceFile(const Location& location, const QString& filePath)
{
    QFile in(filePath);
    currentFile_ = filePath;
    if (!in.open(QIODevice::ReadOnly)) {
        location.error(tr("Can't open source file '%1' (%2)").arg(filePath).arg(strerror(errno)));
        currentFile_.clear();
        return;
    }

    reset();
    Location fileLocation(filePath);
    Tokenizer fileTokenizer(fileLocation, in);
    tokenizer = &fileTokenizer;
    readToken();

    /*
      The set of open namespaces is cleared before parsing
      each source file. The word "source" here means cpp file.
     */
    qdb_->clearOpenNamespaces();

    processQdocComments();
    in.close();
    currentFile_.clear();
}

/*!
  This is called by parseSourceFile() to do the actual parsing
  and tree building. It only processes qdoc comments. It skips
  everything else.
 */
bool PureDocParser::processQdocComments()
{
    const QSet<QString>& metacommands = topicCommands() + otherMetaCommands();

    while (tok != Tok_Eoi) {
        if (tok == Tok_Doc) {
            QString comment = lexeme(); // returns an entire qdoc comment.
            Location start_loc(location());
            readToken();

            Doc::trimCStyleComment(start_loc,comment);
            Location end_loc(location());

            // Doc constructor parses the comment.
            Doc doc(start_loc, end_loc, comment, metacommands, topicCommands());
            const TopicList& topics = doc.topicsUsed();
            if (topics.isEmpty()) {
                doc.location().warning(tr("This qdoc comment contains no topic command "
                                          "(e.g., '\\%1', '\\%2').")
                                       .arg(COMMAND_MODULE).arg(COMMAND_PAGE));
                continue;
            }
            if (hasTooManyTopics(doc))
                continue;

            DocList docs;
            NodeList nodes;
            QString topic = topics[0].topic;

            processTopicArgs(doc, topic, nodes, docs);
            processOtherMetaCommands(nodes, docs);
        }
        else {
            readToken();
        }
    }
    return true;
}

QT_END_NAMESPACE
