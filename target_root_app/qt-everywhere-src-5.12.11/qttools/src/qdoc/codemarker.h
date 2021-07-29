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

#ifndef CODEMARKER_H
#define CODEMARKER_H

#include "atom.h"
#include "sections.h"

QT_BEGIN_NAMESPACE

class Config;

class CodeMarker
{
public:
    CodeMarker();
    virtual ~CodeMarker();

    virtual void initializeMarker(const Config& config);
    virtual void terminateMarker();
    virtual bool recognizeCode(const QString& /*code*/) { return true; }
    virtual bool recognizeExtension(const QString& /*extension*/) { return true; }
    virtual bool recognizeLanguage(const QString& /*language*/) { return false; }
    virtual Atom::AtomType atomType() const { return Atom::Code; }
    virtual QString markedUpCode(const QString &code,
                                 const Node* /*relative*/,
                                 const Location& /*location*/) { return protect(code); }
    virtual QString markedUpSynopsis(const Node* /*node*/,
                                     const Node* /*relative*/,
                                     Section::Style /*style*/) { return QString(); }
    virtual QString markedUpQmlItem(const Node* , bool) { return QString(); }
    virtual QString markedUpName(const Node* /*node*/) { return QString(); }
    virtual QString markedUpFullName(const Node* /*node*/,
                                     const Node* /*relative*/) { return QString(); }
    virtual QString markedUpEnumValue(const QString& /*enumValue*/,
                                      const Node* /*relative*/) { return QString(); }
    virtual QString markedUpIncludes(const QStringList& /*includes*/) { return QString(); }
    virtual QString functionBeginRegExp(const QString& /*funcName*/) { return QString(); }
    virtual QString functionEndRegExp(const QString& /*funcName*/) { return QString(); }
    virtual QStringList macRefsForNode(Node* node);

    static void initialize(const Config& config);
    static void terminate();
    static CodeMarker *markerForCode(const QString& code);
    static CodeMarker *markerForFileName(const QString& fileName);
    static CodeMarker *markerForLanguage(const QString& lang);
    static const Node *nodeForString(const QString& string);
    static QString stringForNode(const Node *node);

    QString typified(const QString &string, bool trailingSpace = false);

protected:
    static QString protect(const QString &string);
    static void appendProtectedString(QString *output, const QStringRef &str);
    QString taggedNode(const Node* node);
    QString taggedQmlNode(const Node* node);
    QString linkTag(const Node *node, const QString& body);

private:
    QString macName(const Node *parent, const QString &name = QString());

    static QString defaultLang;
    static QList<CodeMarker *> markers;
};

class PlainCodeMarker : public CodeMarker
{
 public:
    PlainCodeMarker() { }
    ~PlainCodeMarker() { }
};

QT_END_NAMESPACE

#endif
