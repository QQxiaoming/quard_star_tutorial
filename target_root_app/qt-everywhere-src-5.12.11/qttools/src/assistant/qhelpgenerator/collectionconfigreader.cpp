/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "collectionconfigreader.h"

#include <QtGui/QGuiApplication>

class QCG {
    Q_DECLARE_TR_FUNCTIONS(QCollectionGenerator)
};

void CollectionConfigReader::raiseErrorWithLine()
{
    raiseError(QCG::tr("Unknown token at line %1.").arg(lineNumber()));
}

void CollectionConfigReader::readData(const QByteArray &contents)
{
    m_enableFilterFunctionality = true;
    m_hideFilterFunctionality = true;
    m_enableAddressBar = true;
    m_hideAddressBar = true;
    m_enableDocumentationManager = true;
    m_enableFullTextSearchFallback = false;

    addData(contents);
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("QHelpCollectionProject")
                && attributes().value(QLatin1String("version")) == QLatin1String("1.0"))
                readConfig();
            else
                raiseError(QCG::tr("Unknown token at line %1. "
                                   "Expected \"QtHelpCollectionProject\".")
                           .arg(lineNumber()));
        }
    }
}

void CollectionConfigReader::readConfig()
{
    bool ok = false;
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("assistant"))
                readAssistantSettings();
            else if (name() == QLatin1String("docFiles"))
                readDocFiles();
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("QHelpCollectionProject")) {
            ok = true;
        }
    }
    if (!ok && !hasError())
        raiseError(QCG::tr("Missing end tags."));
}

void CollectionConfigReader::readAssistantSettings()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("title")) {
                m_title = readElementText();
            } else if (name() == QLatin1String("homePage")) {
                m_homePage = readElementText();
            } else if (name() == QLatin1String("startPage")) {
                m_startPage = readElementText();
            } else if (name() == QLatin1String("currentFilter")) {
                m_currentFilter = readElementText();
            } else if (name() == QLatin1String("applicationIcon")) {
                m_applicationIcon = readElementText();
            } else if (name() == QLatin1String("enableFilterFunctionality")) {
                if (attributes().value(QLatin1String("visible")) == QLatin1String("true"))
                    m_hideFilterFunctionality = false;
                if (readElementText() == QLatin1String("false"))
                    m_enableFilterFunctionality = false;
            } else if (name() == QLatin1String("enableDocumentationManager")) {
                if (readElementText() == QLatin1String("false"))
                    m_enableDocumentationManager = false;
            } else if (name() == QLatin1String("enableAddressBar")) {
                if (attributes().value(QLatin1String("visible")) == QLatin1String("true"))
                    m_hideAddressBar = false;
                if (readElementText() == QLatin1String("false"))
                    m_enableAddressBar = false;
            } else if (name() == QLatin1String("aboutMenuText")) {
                readMenuTexts();
            } else if (name() == QLatin1String("aboutDialog")) {
                readAboutDialog();
            } else if (name() == "cacheDirectory") {
                m_cacheDirRelativeToCollection =
                    attributes().value(QLatin1String("base"))
                    == QLatin1String("collection");
                m_cacheDirectory = readElementText();
            } else if (name() == QLatin1String("enableFullTextSearchFallback")) {
                if (readElementText() == QLatin1String("true"))
                    m_enableFullTextSearchFallback = true;
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("assistant")) {
            break;
        }
    }
}

void CollectionConfigReader::readMenuTexts()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("text")) {
                QString lang = attributes().value(QLatin1String("language")).toString();
                if (lang.isEmpty())
                    lang = QLatin1String("default");
                m_aboutMenuTexts.insert(lang, readElementText());
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("aboutMenuText")) {
            break;
        }
    }
}

void CollectionConfigReader::readAboutDialog()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file")) {
                QString lang = attributes().value(QLatin1String("language")).toString();
                if (lang.isEmpty())
                    lang = QLatin1String("default");
                m_aboutTextFiles.insert(lang, readElementText());
            } else if (name() == QLatin1String("icon")) {
                m_aboutIcon = readElementText();
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("aboutDialog")) {
            break;
        }
    }
}

void CollectionConfigReader::readDocFiles()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("generate")) {
                readGenerate();
            } else if (name() == QLatin1String("register")) {
                readRegister();
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("docFiles")) {
            break;
        }
    }
}

void CollectionConfigReader::readGenerate()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file"))
                readFiles();
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("generate")) {
            break;
        }
    }
}

void CollectionConfigReader::readFiles()
{
    QString input;
    QString output;
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("input"))
                input = readElementText();
            else if (name() == QLatin1String("output"))
                output = readElementText();
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("file")) {
            break;
        }
    }
    if (input.isEmpty() || output.isEmpty()) {
        raiseError(QCG::tr("Missing input or output file for help file generation."));
        return;
    }
    m_filesToGenerate.insert(input, output);
}

void CollectionConfigReader::readRegister()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file"))
                m_filesToRegister.append(readElementText());
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("register")) {
            break;
        }
    }
}


