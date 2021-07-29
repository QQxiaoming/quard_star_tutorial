/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ASSETFIXER_H
#define ASSETFIXER_H

#include <QObject>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QQmlParserStatus>
#include <QUrl>

class AssetFixer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(bool shouldWatch READ shouldWatch WRITE setShouldWatch NOTIFY shouldWatchChanged FINAL)
    Q_PROPERTY(bool shouldFix READ shouldFix WRITE setShouldFix NOTIFY shouldFixChanged FINAL)
    Q_PROPERTY(QString assetDirectory READ assetDirectory WRITE setAssetDirectory NOTIFY assetDirectoryChanged FINAL)
    Q_PROPERTY(QUrl assetDirectoryUrl READ assetDirectoryUrl NOTIFY assetDirectoryChanged FINAL)
    Q_PROPERTY(QDateTime assetDirectoryLastModified READ assetDirectoryLastModified WRITE setAssetDirectoryLastModified
        NOTIFY assetDirectoryLastModifiedChanged FINAL)
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit AssetFixer(QObject *parent = nullptr);

    bool shouldWatch() const;
    void setShouldWatch(bool shouldWatch);

    bool shouldFix() const;
    void setShouldFix(bool shouldFix);

    QString assetDirectory() const;
    void setAssetDirectory(const QString &assetDirectory);

    QUrl assetDirectoryUrl() const;

    QDateTime assetDirectoryLastModified() const;
    void setAssetDirectoryLastModified(const QDateTime &assetDirectoryLastModified);

signals:
    void shouldWatchChanged();
    void shouldFixChanged();
    void assetDirectoryChanged();
    void assetDirectoryLastModifiedChanged();

    void fixSuggested();
    void delayedFixSuggested();
    void reloadSuggested();

    void error(const QString &errorMessage);

public slots:
    void clearImageCache();
    void fixAssets();

protected:
    void componentComplete() override;
    void classBegin() override;

private slots:
    void onAssetsChanged();

private:
    void stopWatching();
    void startWatching();

    bool isAssetDirectoryValid(const QString &assetDirectory);

    bool mComponentComplete;
    bool mFirstWatch;
    bool mShouldWatch;
    bool mShouldFix;
    QString mAssetDirectory;
    QFileSystemWatcher mFileSystemWatcher;
    QDateTime mLastModified;
};

#endif // ASSETFIXER_H
