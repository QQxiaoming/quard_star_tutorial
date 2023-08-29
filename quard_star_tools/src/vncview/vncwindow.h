/*
 * This file is part of the https://github.com/QQxiaoming/quard_star_tutorial.git
 * project.
 *
 * Copyright (C) 2021 Quard <2014500726@smail.xtu.edu.cn>
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
#ifndef VNCWINDOW_H
#define VNCWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QVBoxLayout>

#include <qvncclientwidget.h>

QT_BEGIN_NAMESPACE
namespace Ui { class VncWindow; }
QT_END_NAMESPACE

class VncWindow : public QMainWindow
{
    Q_OBJECT

public:
    VncWindow(const QString &addr, int port, QWidget *parent = nullptr);
    ~VncWindow();
    void reConnect(void);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);

private:
    bool isMousePressed = false;
    QString severAddr;
    QPoint mStartPos;
    int severPort;
    QVNCClientWidget *vncView;
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    QTimer* pressTimer;
    QPoint pressPos;
#endif
    QMenu *contextMenu = nullptr;
    double scaled_value = 1.0;
    Ui::VncWindow *ui;
};
#endif // VNCWINDOW_H
