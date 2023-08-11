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
#ifndef NETSELECTBOX_H
#define NETSELECTBOX_H

#include <QDialog>

namespace Ui {
class NetSelectBox;
}

class NetSelectBox : public QDialog
{
    Q_OBJECT

public:
    explicit NetSelectBox(QWidget *parent = nullptr);
    ~NetSelectBox();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);
    void netStateGroupBoxClicked(void);
    void userRadioButtonClicked(void);
    void tapRadioButtonClicked(void);

private:
    Ui::NetSelectBox *ui;
};

#endif // NETSELECTBOX_H
