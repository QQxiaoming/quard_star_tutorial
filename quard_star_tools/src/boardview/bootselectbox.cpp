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
#include "bootselectbox.h"
#include "boardwindow.h"
#include "ui_bootselectbox.h"

BootSelectBox::BootSelectBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BootSelectBox)
{
    ui->setupUi(this);

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

BootSelectBox::~BootSelectBox()
{
    delete ui;
}

void BootSelectBox::showEvent(QShowEvent* event) {
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());

    if(bw->getBootCfg() == "pflash") {
        ui->pFlashRadioButton->setChecked(true);
    } else if(bw->getBootCfg() == "spi") {
        ui->norFlashRadioButton->setChecked(true);
    } else if(bw->getBootCfg() == "sd") {
        ui->sdRadioButton->setChecked(true);
    }
    ui->updateCheckBox->setChecked(bw->getUpdateCfg());

    QDialog::showEvent(event);
}

void BootSelectBox::buttonBoxAccepted(void)
{
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());

    if(ui->pFlashRadioButton->isChecked()) {
        bw->getBootCfg() = "pflash";
    } else if(ui->norFlashRadioButton->isChecked()) {
        bw->getBootCfg() = "spi";
    } else if(ui->sdRadioButton->isChecked()) {
        bw->getBootCfg() = "sd";
    } 
    bw->getUpdateCfg() = ui->updateCheckBox->isChecked();

    emit this->accepted();
}

void BootSelectBox::buttonBoxRejected(void)
{
    emit this->rejected();
}
