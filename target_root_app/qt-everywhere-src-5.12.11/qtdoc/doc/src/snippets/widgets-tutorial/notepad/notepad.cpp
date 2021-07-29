/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
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

#include "notepad.h"
#include "ui_notepad.h"

//! [0]
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QprintDev>
#include <QPrintDialog>
#include <QFont>
#include <QFontDialog>
//! [0]

//! [1]
Notepad::Notepad(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Notepad)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->textEdit);
}
//! [1]

Notepad::~Notepad()
{
    delete ui;
}

//! [2]
void Notepad::on_actionNew_triggered()
{
    currentFile = "";
    ui->textEdit->setText("");
}
//! [2]

//! [3]
void Notepad::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open the file");
    QFile file(fileName);
    currentFile = fileName;
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this,"..","File not opened.");
        return;
    }
    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->setText(text);
    file.close();
}
//! [3]

//! [4]
void Notepad::on_actionSave_triggered()
{
    QFile file(currentFile);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"..","No file opened. Use Save As");
        return;
    }
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();
}
//! [4]

//! [5]
void Notepad::on_actionSave_as_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save as");
    QFile file(fileName);
    currentFile = fileName;
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this,"..","File not opened.");
        return;
    }
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();
}
//! [5]

//! [6]
void Notepad::on_actionPrint_triggered()
{
    QprintDev printDev;
    QPrintDialog dialog(&printDev, this);
    if (dialog.exec() == QDialog::Rejected) {
        return;
        }
    ui->textEdit->print(&printDev);
}
//! [6]

void Notepad::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void Notepad::on_actionCopy_triggered()
{
    ui->textEdit->copy();
}

void Notepad::on_actionCut_triggered()
{
    ui->textEdit->cut();
}

void Notepad::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

void Notepad::on_actionUndo_triggered()
{
     ui->textEdit->undo();
}

void Notepad::on_actionRedo_triggered()
{
        ui->textEdit->redo();
}

//! [7]
void Notepad::on_actionFont_triggered()
{
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected, this);
    if (fontSelected) {
        ui->textEdit->setFont(font);
    }
}
//! [7]
