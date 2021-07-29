/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

#include "mainwindow.h"
#include "stylesheetdialog.h"
#include "ui_mainwindow.h"

static QMap<QString, QString> defaultStyleSheets = {
    {"Upside down", "body { -webkit-transform: rotate(180deg); }"}
};

MainWindow::MainWindow(const QUrl &url) :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->urlBar, &QLineEdit::returnPressed, this, &MainWindow::urlEntered);
    connect(ui->webEngineView, &QWebEngineView::urlChanged, this, &MainWindow::urlChanged);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::showStyleSheetsDialog);
    connect(ui->reloadButton, &QPushButton::clicked, this, &MainWindow::reloadRequested);

    QSettings settings;
    settings.beginGroup("styleSheets");
    QStringList styleSheets = settings.allKeys();
    if (styleSheets.empty()) {
        // Add back default style sheets if the user cleared them out
        loadDefaultStyleSheets();
    } else {
        for (auto name : qAsConst(styleSheets)) {
            StyleSheet styleSheet = settings.value(name).value<StyleSheet>();
            if (styleSheet.second)
                insertStyleSheet(name, styleSheet.first, false);
        }
    }
    settings.endGroup();

    ui->webEngineView->setUrl(url);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::insertStyleSheet(const QString &name, const QString &source, bool immediately)
{
    QWebEngineScript script;
    QString s = QString::fromLatin1("(function() {"\
                                    "    css = document.createElement('style');"\
                                    "    css.type = 'text/css';"\
                                    "    css.id = '%1';"\
                                    "    document.head.appendChild(css);"\
                                    "    css.innerText = '%2';"\
                                    "})()").arg(name).arg(source.simplified());
    if (immediately)
        ui->webEngineView->page()->runJavaScript(s, QWebEngineScript::ApplicationWorld);

    script.setName(name);
    script.setSourceCode(s);
    script.setInjectionPoint(QWebEngineScript::DocumentReady);
    script.setRunsOnSubFrames(true);
    script.setWorldId(QWebEngineScript::ApplicationWorld);
    ui->webEngineView->page()->scripts().insert(script);
}

void MainWindow::removeStyleSheet(const QString &name, bool immediately)
{
    QString s = QString::fromLatin1("(function() {"\
                                    "    var element = document.getElementById('%1');"\
                                    "    element.outerHTML = '';"\
                                    "    delete element;"\
                                    "})()").arg(name);
    if (immediately)
        ui->webEngineView->page()->runJavaScript(s, QWebEngineScript::ApplicationWorld);

    QWebEngineScript script = ui->webEngineView->page()->scripts().findScript(name);
    ui->webEngineView->page()->scripts().remove(script);
}

bool MainWindow::hasStyleSheet(const QString &name)
{
    QWebEngineScript script = ui->webEngineView->page()->scripts().findScript(name);
    return !script.isNull();
}

void MainWindow::loadDefaultStyleSheets()
{
    QSettings settings;
    settings.beginGroup("styleSheets");

    auto it = defaultStyleSheets.constBegin();
    while (it != defaultStyleSheets.constEnd()) {
        settings.setValue(it.key(), QVariant::fromValue(qMakePair(it.value(), true)));
        insertStyleSheet(it.key(), it.value(), false);
        ++it;
    }

    settings.endGroup();
}

void MainWindow::urlEntered()
{
    ui->webEngineView->setUrl(QUrl::fromUserInput(ui->urlBar->text()));
}

void MainWindow::urlChanged(const QUrl &url)
{
    ui->urlBar->setText(url.toString());
}

void MainWindow::showStyleSheetsDialog()
{
    StylesheetDialog *dialog = new StylesheetDialog(this);
    dialog->show();
}

void MainWindow::reloadRequested()
{
    ui->webEngineView->reload();
}
