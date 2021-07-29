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
#include "ui_stylesheetdialog.h"

StylesheetDialog::StylesheetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StylesheetDialog)
{
    ui->setupUi(this);

    connect(ui->styleSheetList, &QListWidget::currentItemChanged, this, &StylesheetDialog::currentStyleSheetChanged);
    connect(ui->styleSheetList, &QListWidget::itemClicked, this, &StylesheetDialog::listItemClicked);
    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &StylesheetDialog::fileNameChanged);

    connect(ui->addButton, &QPushButton::clicked, this, &StylesheetDialog::addButtonClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &StylesheetDialog::removeButtonClicked);

    QSettings settings;
    settings.beginGroup("styleSheets");
    for (auto name : settings.allKeys()) {
        QListWidgetItem *listItem = new QListWidgetItem(name,  ui->styleSheetList);
        listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
        bool checked = settings.value(name).value<StyleSheet>().second;
        listItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
    settings.endGroup();
}

StylesheetDialog::~StylesheetDialog()
{
    delete ui;
}

void StylesheetDialog::currentStyleSheetChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!previous) {
        // Select the first item on startup
        ui->styleSheetList->setCurrentItem(current);
    }

    if (!current) {
        ui->fileNameEdit->setText(QString());
        ui->sourceCodeEdit->setPlainText(QString());
        return;
    }

    QSettings settings;
    settings.beginGroup("styleSheets");
    ui->fileNameEdit->setText(current->text());
    const QString source = settings.value(current->text()).value<StyleSheet>().first;
    ui->sourceCodeEdit->setPlainText(source);
    settings.endGroup();
}

void StylesheetDialog::listItemClicked(QListWidgetItem *item)
{
    MainWindow *window = static_cast<MainWindow *>(parent());
    const QString name = item->text();
    bool checkedStateChanged =
            (item->checkState() == Qt::Checked && !window->hasStyleSheet(name)) ||
            (item->checkState() == Qt::Unchecked && window->hasStyleSheet(name));
    if (!checkedStateChanged)
        return;

    QSettings settings;
    settings.beginGroup("styleSheets");
    const QString source = settings.value(name).value<StyleSheet>().first;

    if (item->checkState() == Qt::Checked) {
        settings.setValue(name, QVariant::fromValue(qMakePair(source, true)));
        window->insertStyleSheet(name, source, true);
    } else {
        settings.setValue(name, QVariant::fromValue(qMakePair(source, false)));
        window->removeStyleSheet(name, true);
    }

    settings.endGroup();
}

void StylesheetDialog::fileNameChanged(const QString &text)
{
    QList<QListWidgetItem *> items = ui->styleSheetList->findItems(text, Qt::MatchFixedString);
    if (items.size())
        ui->addButton->setEnabled(false);
    else
        ui->addButton->setEnabled(true);
}

void StylesheetDialog::addButtonClicked()
{
    const QString name = ui->fileNameEdit->text();
    const QString source = ui->sourceCodeEdit->toPlainText();
    if (name.isEmpty() || source.isEmpty())
        return;

    QListWidgetItem *listItem = new QListWidgetItem(ui->fileNameEdit->text(),  ui->styleSheetList);
    listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
    listItem->setCheckState(Qt::Checked);

    MainWindow *window = static_cast<MainWindow *>(parent());
    window->insertStyleSheet(name, source, true);

    QSettings settings;
    settings.beginGroup("styleSheets");
    settings.setValue(name, QVariant::fromValue(qMakePair(source, true)));
    settings.endGroup();

    ui->addButton->setEnabled(false);
}

void StylesheetDialog::removeButtonClicked()
{
    if (ui->styleSheetList->selectedItems().isEmpty())
        return;

    MainWindow *window = static_cast<MainWindow *>(parent());
    QSettings settings;
    settings.beginGroup("styleSheets");

    QListWidgetItem *item = ui->styleSheetList->selectedItems().first();
    const QString name = item->text();
    window->removeStyleSheet(name, true);
    settings.remove(name);
    delete item;

    settings.endGroup();
}
