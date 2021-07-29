/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CLANGCODEPARSER_H
#define CLANGCODEPARSER_H

#include <QTemporaryDir>

#include "cppcodeparser.h"

QT_BEGIN_NAMESPACE

class ClangCodeParser : public CppCodeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::ClangCodeParser)

public:
    ~ClangCodeParser();

    void initializeParser(const Config& config) override;
    void terminateParser() override;
    QString language() override;
    QStringList headerFileNameFilter() override;
    QStringList sourceFileNameFilter() override;
    void parseHeaderFile(const Location& location, const QString& filePath) override;
    void parseSourceFile(const Location& location, const QString& filePath) override;
    void precompileHeaders() override;
    Node *parseFnArg(const Location &location, const QString &fnArg) override;

 private:
    void getDefaultArgs();
    void getMoreArgs();
    void buildPCH();

private:
    QHash<QString, QString> allHeaders_; // file name->path
    QVector<QByteArray> includePaths_;
    QScopedPointer<QTemporaryDir> pchFileDir_;
    QByteArray pchName_;
    QVector<QByteArray> defines_;
    std::vector<const char *> args_;
    QVector<QByteArray> moreArgs_;
};

QT_END_NAMESPACE

#endif
