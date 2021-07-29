/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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

#include "menus.h"
#include <QAction>
#include <QAxFactory>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextEdit>
#include <QPixmap>

#include "fileopen.xpm"
#include "filesave.xpm"

QMenus::QMenus(QWidget *parent)
    : QMainWindow(parent, 0) // QMainWindow's default flag is WType_TopLevel
{
    QAction *action;

    QMenu *file = new QMenu(this);

    action = new QAction(QPixmap((const char**)fileopen), tr("&Open"), this);
    action->setShortcut(tr("CTRL+O"));
    connect(action, &QAction::triggered, this, &QMenus::fileOpen);
    file->addAction(action);

    action = new QAction(QPixmap((const char**)filesave), tr("&Save"), this);
    action->setShortcut(tr("CTRL+S"));
    connect(action, &QAction::triggered, this, &QMenus::fileSave);
    file->addAction(action);

    QMenu *edit = new QMenu(this);

    action = new QAction(tr("&Normal"), this);
    action->setShortcut(tr("CTRL+N"));
    action->setToolTip(tr("Normal"));
    action->setStatusTip(tr("Toggles Normal"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &QMenus::editNormal);
    edit->addAction(action);

    action = new QAction(tr("&Bold"), this);
    action->setShortcut(tr("CTRL+B"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &QMenus::editBold);
    edit->addAction(action);

    action = new QAction(tr("&Underline"), this);
    action->setShortcut(tr("CTRL+U"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &QMenus::editUnderline);
    edit->addAction(action);

    QMenu *advanced = new QMenu(this);
    action = new QAction(tr("&Font..."), this);
    connect(action, &QAction::triggered, this, &QMenus::editAdvancedFont);
    advanced->addAction(action);

    action = new QAction(tr("&Style..."), this);
    connect(action, &QAction::triggered, this, &QMenus::editAdvancedStyle);
    advanced->addAction(action);

    edit->addMenu(advanced)->setText(tr("&Advanced"));

    edit->addSeparator();

    action = new QAction(tr("Una&vailable"), this);
    action->setShortcut(tr("CTRL+V"));
    action->setCheckable(true);
    action->setEnabled(false);
    connect(action, &QAction::triggered, this, &QMenus::editUnderline);
    edit->addAction(action);

    QMenu *help = new QMenu(this);

    action = new QAction(tr("&About..."), this);
    action->setShortcut(tr("F1"));
    connect(action, &QAction::triggered, this, &QMenus::helpAbout);
    help->addAction(action);

    action = new QAction(tr("&About Qt..."), this);
    connect(action, &QAction::triggered, this, &QMenus::helpAboutQt);
    help->addAction(action);

    if (!QAxFactory::isServer())
        menuBar()->addMenu(file)->setText(tr("&File"));
    menuBar()->addMenu(edit)->setText(tr("&Edit"));
    menuBar()->addMenu(help)->setText(tr("&Help"));

    m_editor = new QTextEdit(this);
    setCentralWidget(m_editor);

    statusBar();
}

void QMenus::fileOpen()
{
    m_editor->append(tr("File Open selected."));
}

void QMenus::fileSave()
{
    m_editor->append(tr("File Save selected."));
}

void QMenus::editNormal()
{
    m_editor->append(tr("Edit Normal selected."));
}

void QMenus::editBold()
{
    m_editor->append(tr("Edit Bold selected."));
}

void QMenus::editUnderline()
{
    m_editor->append(tr("Edit Underline selected."));
}

void QMenus::editAdvancedFont()
{
    m_editor->append(tr("Edit Advanced Font selected."));
}

void QMenus::editAdvancedStyle()
{
    m_editor->append(tr("Edit Advanced Style selected."));
}

void QMenus::helpAbout()
{
    QMessageBox::about(this, tr("About QMenus"),
                       tr("This example implements an in-place ActiveX control with menus and status messages."));
}

void QMenus::helpAboutQt()
{
    QMessageBox::aboutQt(this);
}
