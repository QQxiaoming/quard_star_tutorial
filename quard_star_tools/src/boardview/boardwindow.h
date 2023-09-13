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
#ifndef BOARDWINDOW_H
#define BOARDWINDOW_H

#include <QMainWindow>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QTreeView>
#include <QMessageBox>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QMenu>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include <QLocale>
#include <QGestureEvent>
#if defined(BUILT_IN_QEMU_MODE)
#include <QProcess>
#endif

#include "qfonticon.h"

#include "fsviewmodel.h"
#include "treemodel.h"

#include "vncwindow.h"
#include "telnetwindow.h"
#include "netselectbox.h"
#include "bootselectbox.h"
#include "qfsviewer.h"

extern QString VERSION;
extern QString GIT_TAG;

namespace Ui {
class BoardWindow;
}

class BoardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BoardWindow(const QString &path,const QString &color = "green",
                    const bool &isDarkTheme = false,
                    QLocale::Language force_translator = QLocale::AnyLanguage,
                    QString ipAddr = "",
                    QWidget *parent = nullptr);
    ~BoardWindow();
    bool powerSwitch(bool power);
    QString& getVCanName(void);
    QString& getTapName(void);
    QString& getBootCfg(void);
    bool& getUpdateCfg(void);
    int sendQemuCmd(const QString &cmd);
    static void appAbout(QWidget *parent = nullptr);
    static void appHelp(QWidget *parent = nullptr);
    static void setAppLangeuage(QLocale::Language lang);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);
    bool gestureEvent(QGestureEvent *event);
    bool event(QEvent *event);

private slots:
    void app_quit(void);

private:
    enum DeviceName {
        LOGO = 0,
        VGA,
        UART2,
        UART1,
        UART0,
        POWER,
        SOC,
        DDR,
        ETH,
        USB0,
        USB1,
        NAND,
        NOR,
        SD,
        SWITCH,
        JTAG,
        AUDIO,
        BOOT,

        UNKNOW = -1,
    };
    bool doPowerSwitch(void);
    QString getOpenFileName(const QString &caption = QString(),
                            const QString &fileName = QString(),
                            const QString &filter = QString());
    void addActionGInfo(QMenu *menu,const DeviceName &title);
    void addActionOFileSystem(QMenu *menu,const DeviceName &title);
    void addActionSetting(QMenu *menu,const DeviceName &title);
    void createStdMenuAction(QMenu *menu);
    Ui::BoardWindow *ui;
    bool isMousePressed = false;
    bool powerOn = false;
    QPoint mStartPos;
#if defined(BUILT_IN_QEMU_MODE)
    QProcess *qemuProcess = nullptr;
#endif
    TelnetWindow *uartWindow[3];
    TelnetWindow *jtagWindow;
    VncWindow *lcdWindow;
    NetSelectBox *netSelect;
    BootSelectBox *bootSelect;
    FSViewWindow *fsView;
    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *contextMenu = nullptr;
#if defined(MOBILE_INTERACTION_MODE)
    QTimer* pressTimer;
    QPoint pressPos;
#endif
    QString ipAddr;
    uint16_t portOffset;
    QString envPath;
    QString skinColor;
    bool isDarkTheme;
    QString maskRomImgPath;
    QString pFlashImgPath;
    QString norFlashImgPath;
    QString nandFlashImgPath;
    QString sdImgPath;
    QString usbFlashImgPath;
    QString rootFSImgPath;
    QString tapName;
    QString vCanName;
    QString bootCfg;
    QLocale::Language language;
    bool forceWebSocket;
    bool updateCfg;
    double scaled_value = 1.0;
    struct space
    {
        DeviceName name;
        QString drawName;
        int x1,y1,x2,y2;
        bool draw;
    }spaceList[18] = {
        {LOGO,  "",          419,558,601,588,false},
        {VGA,   tr("vga"),   315,111,649,262,false},
        {UART2, tr("uart2"), 52 ,381,107,457,false},
        {UART1, tr("uart1"), 52 ,290,107,366,false},
        {UART0, tr("uart0"), 52 ,198,107,272,false},
        {POWER, tr("power"), 127,709,222,837,false},
        {SOC,   tr("soc"),   369,377,470,479,false},
        {DDR,   tr("ddr"),   542,345,620,508,false},
        {ETH,   tr("eth"),   689,179,838,340,false},
        {USB0,  tr("usb0"),  685,367,837,504,false},
        {USB1,  tr("usb1"),  685,539,837,674,false},
        {NAND,  tr("nand"),  197,365,296,425,false},
        {NOR,   tr("nor"),   215,493,263,530,false},
        {SD,    tr("sd"),    145,114,296,263,false},
        {SWITCH,tr("switch"),247,728,385,802,false},
        {JTAG,  tr("jtag"),  52 ,499,131,671,false},
        {AUDIO, tr("audio"), 673,686,744,810,false},
        {BOOT,  tr("boot"),  409,703,650,803,false},
    };
};

#endif // BOARDWINDOW_H
