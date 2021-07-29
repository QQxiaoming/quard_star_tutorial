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

#ifndef WEBXMLGENERATOR_H
#define WEBXMLGENERATOR_H

#include <QtCore/qxmlstream.h>

#include "codemarker.h"
#include "config.h"
#include "htmlgenerator.h"

QT_BEGIN_NAMESPACE

class WebXMLGenerator : public HtmlGenerator
{
public:
    explicit WebXMLGenerator() {}
    ~WebXMLGenerator() {}

    void initializeGenerator(const Config &config) override;
    void terminateGenerator() override;
    QString format() override;
    //void generateDocs() override;

protected:
    int generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker) override;
    void generateCppReferencePage(Node *node, CodeMarker *marker) override;
    void generateDocumentNode(DocumentNode *dn, CodeMarker *marker) override;
    void generateDocumentation(Node *node) override;
    QString fileExtension() const override;

    virtual const Atom *addAtomElements(QXmlStreamWriter &writer, const Atom *atom,
                                 const Node *relative, CodeMarker *marker);
    virtual void generateIndexSections(QXmlStreamWriter &writer, Node *node,
                                       CodeMarker *marker);


private:
    const QPair<QString,QString> anchorForNode(const Node *node);
    void generateAnnotatedList(QXmlStreamWriter &writer, const Node *relative, const NodeMap &nodeMap);
    void generateFullName(QXmlStreamWriter &writer, const Node *node,
                          const Node *relative);
    void generateRelations(QXmlStreamWriter &writer, const Node *node);
    void startLink(QXmlStreamWriter &writer, const Atom *atom, const Node *node,
                   const QString &link);
    QString targetType(const Node *node);

    bool inLink;
    bool inContents;
    bool inSectionHeading;
    bool hasQuotingInformation;
    int numTableRows;
    QString quoteCommand;
};

QT_END_NAMESPACE

#endif
