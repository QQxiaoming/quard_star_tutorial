/*
 * This file is part of the https://github.com/QQxiaoming/quard_star_tutorial.git
 * project.
 *
 * Copyright (C) 2022 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "netselectbox.h"
#include "boardwindow.h"
#include "ui_netselectbox.h"
#if !defined(Q_OS_WASM)
#include <QNetworkInterface>
#endif

NetSelectBox::NetSelectBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetSelectBox)
{
    ui->setupUi(this);

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
    QObject::connect(ui->netStateGroupBox, SIGNAL(clicked()), this, SLOT(netStateGroupBoxClicked()));
    QObject::connect(ui->userRadioButton, SIGNAL(clicked()), this, SLOT(userRadioButtonClicked()));
    QObject::connect(ui->tapRadioButton, SIGNAL(clicked()), this, SLOT(tapRadioButtonClicked()));
}

NetSelectBox::~NetSelectBox()
{
    delete ui;
}

void NetSelectBox::showEvent(QShowEvent* event)
{
#if defined(Q_OS_WASM)
    ui->canComboBox->setEnabled(false);
    ui->tapComboBox->setEnabled(false);
#else
    QNetworkInterface net;
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());
    QList<QNetworkInterface> netList = net.allInterfaces();

    ui->tapComboBox->clear();
    ui->canComboBox->clear();
    ui->tapComboBox->addItem("N/A");
    ui->canComboBox->addItem("N/A");

    foreach (QNetworkInterface var, netList) {
        ui->tapComboBox->addItem(var.humanReadableName());
        ui->canComboBox->addItem(var.humanReadableName());
    }

    ui->tapComboBox->setCurrentText(bw->getTapName().isEmpty()?"N/A":bw->getTapName());
    ui->canComboBox->setCurrentText(bw->getVCanName().isEmpty()?"N/A":bw->getVCanName());

    if(bw->getTapName().isEmpty()){
        ui->userRadioButton->setChecked(true);
        ui->tapComboBox->setEnabled(false);
    } else {
        ui->tapRadioButton->setChecked(true);
        ui->tapComboBox->setEnabled(true);
    }
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS) || defined(MOBILE_MODE)
    ui->canComboBox->setEnabled(false);
#elif defined(Q_OS_LINUX)
    ui->canComboBox->setEnabled(true);
#endif
#endif
    QDialog::showEvent(event);
}

void NetSelectBox::buttonBoxAccepted(void)
{
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());
    bw->getTapName() = ui->tapComboBox->currentText() == "N/A"?"":ui->tapComboBox->currentText();
    bw->getVCanName() = ui->canComboBox->currentText() == "N/A"?"":ui->canComboBox->currentText();
    if(ui->netStateGroupBox->isChecked()) {
        bw->sendQemuCmd("set_link net0 on\n");
    } else {
        bw->sendQemuCmd("set_link net0 off\n");
    }
    emit this->accepted();
}

void NetSelectBox::buttonBoxRejected(void)
{
    emit this->rejected();
}

void NetSelectBox::netStateGroupBoxClicked(void)
{
    if(ui->netStateGroupBox->isChecked()){
        if(ui->userRadioButton->isChecked()){
            ui->tapComboBox->setEnabled(false);
        }
        if(ui->tapRadioButton->isChecked()){
            ui->tapComboBox->setEnabled(true);
        }
    } else {
        ui->tapComboBox->setEnabled(false);
    }
}

void NetSelectBox::userRadioButtonClicked(void)
{
    if(ui->userRadioButton->isChecked()){
        ui->tapComboBox->setCurrentText("N/A");
        ui->tapComboBox->setEnabled(false);
    }
}

void NetSelectBox::tapRadioButtonClicked(void)
{
    if(ui->tapRadioButton->isChecked()){
        ui->tapComboBox->setEnabled(true);
    }
}
