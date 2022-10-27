#ifndef BOARDWINDOW_H
#define BOARDWINDOW_H

#include <QMainWindow>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QProcess>

#include "vncwindow.h"
#include "telnetwindow.h"
#include "netselectbox.h"

namespace Ui {
class BoardWindow;
}

class BoardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BoardWindow(const QString &path,const QString &color = "green",const bool &isDarkTheme = false,QWidget *parent = nullptr);
    ~BoardWindow();
    bool powerSwitch(bool power);
    QString vcan_name;
    QString tap_name;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    enum DeviceName {
        VGA = 0,
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
    QString getOpenFileName(const QString &caption = QString(),
                            const QString &fileName = QString(),
                            const QString &filter = QString());
    void addActionGInfo(QMenu *menu,const DeviceName &title);
    void addActionOFileSystem(QMenu *menu,const DeviceName &title);
    void addActionSetting(QMenu *menu,const DeviceName &title);
    Ui::BoardWindow *ui;
    bool isMousePressed = false;
    bool powerOn = false;
    QPoint mStartPos;
    QProcess *qemu_process;
    VncWindow *vnc;
    TelnetWindow *telnet[4];
    NetSelectBox *netSelect;
    void show_vnc(void);
    QString envPath;
    QString skinColor;
    bool isDarkTheme;
    QString maskromImgPath;
    QString pflashImgPath;
    QString norflashImgPath;
    QString nandflashImgPath;
    QString sdImgPath;
    QString usbflashImgPath;
    QString rootfsImgPath;
    struct space
    {
        DeviceName name;
        QString drawName;
        int x1,y1,x2,y2;
        int dir;
        bool draw;
    }spaceList[17] = {
        {VGA,   tr("vga"),   315,111,649,262,1,false},
        {UART2, tr("uart2"), 52 ,381,107,457,1,false},
        {UART1, tr("uart1"), 52 ,290,107,366,1,false},
        {UART0, tr("uart0"), 52 ,198,107,272,1,false},
        {POWER, tr("power"), 127,709,222,837,0,false},
        {SOC,   tr("soc"),   369,377,470,479,0,false},
        {DDR,   tr("ddr"),   542,345,620,508,0,false},
        {ETH,   tr("eth"),   689,179,838,340,0,false},
        {USB0,  tr("usb0"),  685,367,837,504,0,false},
        {USB1,  tr("usb1"),  685,539,837,674,0,false},
        {NAND,  tr("nand"),  197,365,296,425,0,false},
        {NOR,   tr("nor"),   215,493,263,530,0,false},
        {SD,    tr("sd"),    145,114,296,263,1,false},
        {SWITCH,tr("switch"),247,728,385,802,0,false},
        {JTAG,  tr("jtag"),  52 ,499,131,671,0,false},
        {AUDIO, tr("audio"), 673,686,744,810,0,false},
        {BOOT,  tr("boot"),  409,703,650,803,0,false},
    };
};

#endif // BOARDWINDOW_H
