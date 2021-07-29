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
#include "codemarker.h"
#include "config.h"
#include "node.h"
#include <qdebug.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

QString CodeMarker::defaultLang;
QList<CodeMarker *> CodeMarker::markers;

/*!
  When a code marker constructs itself, it puts itself into
  the static list of code markers. All the code markers in
  the static list get initialized in initialize(), which is
  not called until after the qdoc configuration file has
  been read.
 */
CodeMarker::CodeMarker()
{
    markers.prepend(this);
}

/*!
  When a code marker destroys itself, it removes itself from
  the static list of code markers.
 */
CodeMarker::~CodeMarker()
{
    markers.removeAll(this);
}

/*!
  A code market performs no initialization by default. Marker-specific
  initialization is performed in subclasses.
 */
void CodeMarker::initializeMarker(const Config& ) // config
{
}

/*!
  Terminating a code marker is trivial.
 */
void CodeMarker::terminateMarker()
{
    // nothing.
}

/*!
  All the code markers in the static list are initialized
  here, after the qdoc configuration file has been loaded.
 */
void CodeMarker::initialize(const Config& config)
{
    defaultLang = config.getString(CONFIG_LANGUAGE);
    QList<CodeMarker *>::ConstIterator m = markers.constBegin();
    while (m != markers.constEnd()) {
        (*m)->initializeMarker(config);
        ++m;
    }
}

/*!
  All the code markers in the static list are terminated here.
 */
void CodeMarker::terminate()
{
    QList<CodeMarker *>::ConstIterator m = markers.constBegin();
    while (m != markers.constEnd()) {
        (*m)->terminateMarker();
        ++m;
    }
}

CodeMarker *CodeMarker::markerForCode(const QString& code)
{
    CodeMarker *defaultMarker = markerForLanguage(defaultLang);
    if (defaultMarker != 0 && defaultMarker->recognizeCode(code))
        return defaultMarker;

    QList<CodeMarker *>::ConstIterator m = markers.constBegin();
    while (m != markers.constEnd()) {
        if ((*m)->recognizeCode(code))
            return *m;
        ++m;
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForFileName(const QString& fileName)
{
    CodeMarker *defaultMarker = markerForLanguage(defaultLang);
    int dot = -1;
    while ((dot = fileName.lastIndexOf(QLatin1Char('.'), dot)) != -1) {
        QString ext = fileName.mid(dot + 1);
        if (defaultMarker != 0 && defaultMarker->recognizeExtension(ext))
            return defaultMarker;
        QList<CodeMarker *>::ConstIterator m = markers.constBegin();
        while (m != markers.constEnd()) {
            if ((*m)->recognizeExtension(ext))
                return *m;
            ++m;
        }
        --dot;
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForLanguage(const QString& lang)
{
    QList<CodeMarker *>::ConstIterator m = markers.constBegin();
    while (m != markers.constEnd()) {
        if ((*m)->recognizeLanguage(lang))
            return *m;
        ++m;
    }
    return 0;
}

const Node *CodeMarker::nodeForString(const QString& string)
{
#if QT_POINTER_SIZE == 4
    const quintptr n = string.toUInt();
#else
    const quintptr n = string.toULongLong();
#endif
    return reinterpret_cast<const Node *>(n);
}

QString CodeMarker::stringForNode(const Node *node)
{
    if (sizeof(const Node *) == sizeof(ulong)) {
        return QString::number(reinterpret_cast<quintptr>(node));
    }
    else {
        return QString::number(reinterpret_cast<qulonglong>(node));
    }
}

static const QString samp  = QLatin1String("&amp;");
static const QString slt   = QLatin1String("&lt;");
static const QString sgt   = QLatin1String("&gt;");
static const QString squot = QLatin1String("&quot;");

QString CodeMarker::protect(const QString& str)
{
    int n = str.length();
    QString marked;
    marked.reserve(n * 2 + 30);
    const QChar *data = str.constData();
    for (int i = 0; i != n; ++i) {
        switch (data[i].unicode()) {
        case '&': marked += samp;  break;
        case '<': marked += slt;   break;
        case '>': marked += sgt;   break;
        case '"': marked += squot; break;
        default : marked += data[i];
        }
    }
    return marked;
}

void CodeMarker::appendProtectedString(QString *output, const QStringRef &str)
{
    int n = str.length();
    output->reserve(output->size() + n * 2 + 30);
    const QChar *data = str.constData();
    for (int i = 0; i != n; ++i) {
        switch (data[i].unicode()) {
        case '&': *output += samp;  break;
        case '<': *output += slt;   break;
        case '>': *output += sgt;   break;
        case '"': *output += squot; break;
        default : *output += data[i];
        }
    }
}

QString CodeMarker::typified(const QString &string, bool trailingSpace)
{
    QString result;
    QString pendingWord;

    for (int i = 0; i <= string.size(); ++i) {
        QChar ch;
        if (i != string.size())
            ch = string.at(i);

        QChar lower = ch.toLower();
        if ((lower >= QLatin1Char('a') && lower <= QLatin1Char('z'))
                || ch.digitValue() >= 0 || ch == QLatin1Char('_')
                || ch == QLatin1Char(':')) {
            pendingWord += ch;
        }
        else {
            if (!pendingWord.isEmpty()) {
                bool isProbablyType = (pendingWord != QLatin1String("const"));
                if (isProbablyType)
                    result += QLatin1String("<@type>");
                result += pendingWord;
                if (isProbablyType)
                    result += QLatin1String("</@type>");
            }
            pendingWord.clear();

            switch (ch.unicode()) {
            case '\0':
                break;
            case '&':
                result += QLatin1String("&amp;");
                break;
            case '<':
                result += QLatin1String("&lt;");
                break;
            case '>':
                result += QLatin1String("&gt;");
                break;
            default:
                result += ch;
            }
        }
    }
    if (trailingSpace && string.size()) {
        if (!string.endsWith(QLatin1Char('*'))
                && !string.endsWith(QLatin1Char('&')))
                result += QLatin1Char(' ');
    }
    return result;
}

QString CodeMarker::taggedNode(const Node* node)
{
    QString tag;
    QString name = node->name();

    switch (node->type()) {
    case Node::Namespace:
        tag = QLatin1String("@namespace");
        break;
    case Node::Class:
        tag = QLatin1String("@class");
        break;
    case Node::Enum:
        tag = QLatin1String("@enum");
        break;
    case Node::Typedef:
        tag = QLatin1String("@typedef");
        break;
    case Node::Function:
        tag = QLatin1String("@function");
        break;
    case Node::Property:
        tag = QLatin1String("@property");
        break;
    case Node::QmlType:
        /*
          Remove the "QML:" prefix, if present.
          There shouldn't be any of these "QML:"
          prefixes in the documentation sources
          after the switch to using QML module
          qualifiers, but this code is kept to
          be backward compatible.
        */
        if (node->name().startsWith(QLatin1String("QML:")))
            name = name.mid(4);
        tag = QLatin1String("@property");
        break;
    case Node::Document:
        tag = QLatin1String("@property");
        break;
    case Node::QmlMethod:
    case Node::QmlSignal:
    case Node::QmlSignalHandler:
        tag = QLatin1String("@function");
        break;
    default:
        tag = QLatin1String("@unknown");
        break;
    }
    return (QLatin1Char('<') + tag + QLatin1Char('>') + protect(name)
            + QLatin1String("</") + tag + QLatin1Char('>'));
}

QString CodeMarker::taggedQmlNode(const Node* node)
{
    QString tag;
    switch (node->type()) {
    case Node::QmlProperty:
        tag = QLatin1String("@property");
        break;
    case Node::QmlSignal:
        tag = QLatin1String("@signal");
        break;
    case Node::QmlSignalHandler:
        tag = QLatin1String("@signalhandler");
        break;
    case Node::QmlMethod:
        tag = QLatin1String("@method");
        break;
    default:
        tag = QLatin1String("@unknown");
        break;
    }
    return QLatin1Char('<') + tag + QLatin1Char('>') + protect(node->name())
            + QLatin1String("</") + tag + QLatin1Char('>');
}

QString CodeMarker::linkTag(const Node *node, const QString& body)
{
    return QLatin1String("<@link node=\"") + stringForNode(node)
            + QLatin1String("\">") + body + QLatin1String("</@link>");
}

static QString encode(const QString &string)
{
    return string;
}

QStringList CodeMarker::macRefsForNode(Node *node)
{
    QString result = QLatin1String("cpp/");
    switch (node->type()) {
    case Node::Class:
    {
        const ClassNode *classe = static_cast<const ClassNode *>(node);
        {
            result += QLatin1String("cl/");
        }
        result += macName(classe); // ### Maybe plainName?
    }
        break;
    case Node::Enum:
    {
        QStringList stringList;
        stringList << encode(result + QLatin1String("tag/") +
                             macName(node));
        foreach (const QString &enumName, node->doc().enumItemNames()) {
            // ### Write a plainEnumValue() and use it here
            stringList << encode(result + QLatin1String("econst/") +
                                 macName(node->parent(), enumName));
        }
        return stringList;
    }
    case Node::Typedef:
        result += QLatin1String("tdef/") + macName(node);
        break;
    case Node::Function:
    {
        bool isMacro = false;
        Q_UNUSED(isMacro)
        const FunctionNode *func = static_cast<const FunctionNode *>(node);

        // overloads are too clever for the Xcode documentation browser
        if (func->isOverload())
            return QStringList();

        if (func->isMacro()) {
            result += QLatin1String("macro/");
        }
        else if (func->isStatic()) {
            result += QLatin1String("clm/");
        }
        else if (!func->parent()->name().isEmpty()) {
            result += QLatin1String("instm/");
        }
        else {
            result += QLatin1String("func/");
        }

        result += macName(func);
        if (result.endsWith(QLatin1String("()")))
            result.chop(2);
    }
        break;
    case Node::Variable:
        result += QLatin1String("data/") + macName(node);
        break;
    case Node::Property:
    {
        NodeList list = static_cast<const PropertyNode*>(node)->functions();
        QStringList stringList;
        foreach (Node* node, list) {
            stringList += macRefsForNode(node);
        }
        return stringList;
    }
    case Node::Namespace:
    case Node::Document:
    case Node::QmlType:
    default:
        return QStringList();
    }

    return QStringList(encode(result));
}

QString CodeMarker::macName(const Node *node, const QString &name)
{
    QString myName = name;
    if (myName.isEmpty()) {
        myName = node->name();
        node = node->parent();
    }

    if (node->name().isEmpty()) {
        return QLatin1Char('/') + protect(myName);
    }
    else {
        return node->plainFullName() + QLatin1Char('/') + protect(myName);
    }
}

QT_END_NAMESPACE
