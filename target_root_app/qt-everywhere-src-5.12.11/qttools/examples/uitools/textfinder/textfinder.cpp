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

#include "textfinder.h"
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTextStream>
#include <QUiLoader>
#include <QVBoxLayout>

//! [4]
static QWidget *loadUiFile(QWidget *parent)
{
    QFile file(":/forms/textfinder.ui");
    file.open(QIODevice::ReadOnly);

    QUiLoader loader;
    return loader.load(&file, parent);
}
//! [4]

//! [5]
static QString loadTextFile()
{
    QFile inputFile(":/forms/input.txt");
    inputFile.open(QIODevice::ReadOnly);
    QTextStream in(&inputFile);
    in.setCodec("UTF-8");
    return in.readAll();
}
//! [5]

//! [0]
TextFinder::TextFinder(QWidget *parent)
    : QWidget(parent)
{
    QWidget *formWidget = loadUiFile(this);

//! [1]
    ui_findButton = findChild<QPushButton*>("findButton");
    ui_textEdit = findChild<QTextEdit*>("textEdit");
    ui_lineEdit = findChild<QLineEdit*>("lineEdit");
//! [0] //! [1]

//! [2]
    QMetaObject::connectSlotsByName(this);
//! [2]

//! [3a]
    ui_textEdit->setText(loadTextFile());
//! [3a]

//! [3b]
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(formWidget);
    setLayout(layout);
//! [3b]

//! [3c]
    setWindowTitle(tr("Text Finder"));
}
//! [3c]

//! [6] //! [7]
void TextFinder::on_findButton_clicked()
{
    QString searchString = ui_lineEdit->text();
    QTextDocument *document = ui_textEdit->document();

    bool found = false;

    // undo previous change (if any)
    document->undo();

    if (searchString.isEmpty()) {
        QMessageBox::information(this, tr("Empty Search Field"),
                                 tr("The search field is empty. "
                                    "Please enter a word and click Find."));
    } else {
        QTextCursor highlightCursor(document);
        QTextCursor cursor(document);

        cursor.beginEditBlock();
//! [6]

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setForeground(Qt::red);

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document->find(searchString, highlightCursor,
                                             QTextDocument::FindWholeWords);

            if (!highlightCursor.isNull()) {
                found = true;
                highlightCursor.movePosition(QTextCursor::WordRight,
                                             QTextCursor::KeepAnchor);
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

//! [8]
        cursor.endEditBlock();
//! [7] //! [9]

        if (found == false) {
            QMessageBox::information(this, tr("Word Not Found"),
                                     tr("Sorry, the word cannot be found."));
        }
    }
}
//! [8] //! [9]
