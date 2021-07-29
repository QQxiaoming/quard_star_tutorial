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

#ifndef COLLECTIONCONFIGREADER_H
#define COLLECTIONCONFIGREADER_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

QT_USE_NAMESPACE

class CollectionConfigReader : public QXmlStreamReader
{
public:
    void readData(const QByteArray &contents);

    QString title() const { return m_title; }
    QString homePage() const { return m_homePage; }
    QString startPage() const { return m_startPage; }
    QString applicationIcon() const { return m_applicationIcon; }
    QString currentFilter() const { return m_currentFilter; }
    bool enableFilterFunctionality() const
        { return m_enableFilterFunctionality; }
    bool hideFilterFunctionality() const
        { return m_hideFilterFunctionality; }
    bool enableAddressBar() const { return m_enableAddressBar; }
    bool hideAddressBar() const { return m_hideAddressBar; }
    bool enableDocumentationManager() const
        { return m_enableDocumentationManager; }

    QMap<QString, QString> aboutMenuTexts() const
        { return m_aboutMenuTexts; }
    QString aboutIcon() const { return m_aboutIcon; }
    QMap<QString, QString> aboutTextFiles() const
        { return m_aboutTextFiles; }

    QMap<QString, QString> filesToGenerate() const
        { return m_filesToGenerate; }

    QStringList filesToRegister() const { return m_filesToRegister; }

    QString cacheDirectory() const { return m_cacheDirectory; }
    bool cacheDirRelativeToCollection() const { return m_cacheDirRelativeToCollection; }

    bool fullTextSearchFallbackEnabled() const {
        return m_enableFullTextSearchFallback;
    }

private:
    void raiseErrorWithLine();
    void readConfig();
    void readAssistantSettings();
    void readMenuTexts();
    void readAboutDialog();
    void readDocFiles();
    void readGenerate();
    void readFiles();
    void readRegister();

    QMap<QString, QString> m_aboutMenuTexts;
    QMap<QString, QString> m_aboutTextFiles;
    QMap<QString, QString> m_filesToGenerate;
    QStringList m_filesToRegister;
    QString m_title;
    QString m_homePage;
    QString m_startPage;
    QString m_applicationIcon;
    QString m_currentFilter;
    QString m_aboutIcon;
    QString m_cacheDirectory;
    bool m_enableFilterFunctionality;
    bool m_hideFilterFunctionality;
    bool m_enableAddressBar;
    bool m_hideAddressBar;
    bool m_enableDocumentationManager;
    bool m_cacheDirRelativeToCollection;
    bool m_enableFullTextSearchFallback;
};

#endif
