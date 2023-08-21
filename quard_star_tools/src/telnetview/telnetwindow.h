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
#ifndef TELNETWINDOW_H
#define TELNETWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QMutex>
#include <QMenu>

#include "QTelnet.h"
#include "qtermwidget.h"
#include "asciibox.h"

namespace Ui {
class TelnetWindow;
}

class TelnetWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TelnetWindow(const QString &addr, int port,
                          QWidget *parent = nullptr);
    ~TelnetWindow();
    void reConnect(void);
    void sendData(const QByteArray &ba);

private slots:
    void sendData(const char *data, int len);
    void recvData(const char *buff, int len);
    void dupDisplayOutput(const char* data,int len);
    void recvASCIIstop();

private:
    void createContextMenu(void);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    bool isMousePressed = false;
    QPoint mStartPos;
    double scaled_value = 1.0;
    QTelnet *telnet;
    QTermWidget *termWidget;
    ASCIIBox *sendASCIIBox;
    ASCIIBox *recvASCIIBox;
    QString severAddr;
    int severPort;
    QFont orig_font;
    QFile *raw_log_file = nullptr;
    QMutex raw_log_file_mutex;
    QFile *log_file = nullptr;
    QMutex log_file_mutex;
    QMenu *contextMenu = nullptr;
    QAction *actionSave_log;
    QAction *actionSave_Rawlog;
    QAction *actionReceiveASCII;
    QAction *actionadd_time_on_each_line;
    QAction *actionFflush_file;
    Ui::TelnetWindow *ui;
    QString currentColorScheme;
    QString currentAvailableKeyBindings;
    bool m_save_log = false;
    bool m_save_Rawlog = false;
    bool m_receiveASCII = false;
    bool m_add_time_on_each_line = false;
    bool m_fflush_file = false;
};

#endif // TELNETWINDOW_H
