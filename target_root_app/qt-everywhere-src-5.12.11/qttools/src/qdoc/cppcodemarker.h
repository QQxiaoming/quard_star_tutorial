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

#ifndef CPPCODEMARKER_H
#define CPPCODEMARKER_H

#include "codemarker.h"

QT_BEGIN_NAMESPACE

class CppCodeMarker : public CodeMarker
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::CppCodeMarker)

public:
    CppCodeMarker();
    ~CppCodeMarker();

    bool recognizeCode(const QString& code) override;
    bool recognizeExtension(const QString& ext) override;
    bool recognizeLanguage(const QString& lang) override;
    Atom::AtomType atomType() const override;
    QString markedUpCode(const QString& code, const Node *relative, const Location &location) override;
    QString markedUpSynopsis(const Node *node, const Node *relative, Section::Style style) override;
    QString markedUpQmlItem(const Node *node, bool summary) override;
    QString markedUpName(const Node *node) override;
    QString markedUpFullName(const Node *node, const Node *relative) override;
    QString markedUpEnumValue(const QString &enumValue, const Node *relative) override;
    QString markedUpIncludes(const QStringList& includes) override;
    QString functionBeginRegExp(const QString& funcName) override;
    QString functionEndRegExp(const QString& funcName) override;

private:
    QString addMarkUp(const QString& protectedCode, const Node *relative, const Location &location);
};

QT_END_NAMESPACE

#endif
