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
#include "bootselectbox.h"

namespace Ui {
class BoardWindow;
}

class BoardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BoardWindow(const QString &path,const QString &color = "green",
                    const bool &isDarkTheme = false,QWidget *parent = nullptr);
    ~BoardWindow();
    bool powerSwitch(bool power);
    QString& getVCanName(void);
    QString& getTapName(void);
    QString& getBootCfg(void);
    bool& getUpdateCfg(void);
    int sendQemuCmd(const QString &cmd);

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
    QProcess *qemuProcess;
    TelnetWindow *uartWindow[3];
    TelnetWindow *jtagWindow;
    VncWindow *lcdWindow;
    NetSelectBox *netSelect;
    BootSelectBox *bootSelect;
    void showVnc(void);
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
    bool updateCfg;
    enum drawDir {
        UP = 0,
        DOWN = 1,
    };
    struct space
    {
        DeviceName name;
        QString drawName;
        int x1,y1,x2,y2;
        drawDir dir;
        bool draw;
    }spaceList[17] = {
        {VGA,   tr("vga"),   315,111,649,262,DOWN,false},
        {UART2, tr("uart2"), 52 ,381,107,457,DOWN,false},
        {UART1, tr("uart1"), 52 ,290,107,366,DOWN,false},
        {UART0, tr("uart0"), 52 ,198,107,272,DOWN,false},
        {POWER, tr("power"), 127,709,222,837,UP,  false},
        {SOC,   tr("soc"),   369,377,470,479,UP,  false},
        {DDR,   tr("ddr"),   542,345,620,508,UP,  false},
        {ETH,   tr("eth"),   689,179,838,340,UP,  false},
        {USB0,  tr("usb0"),  685,367,837,504,UP,  false},
        {USB1,  tr("usb1"),  685,539,837,674,UP,  false},
        {NAND,  tr("nand"),  197,365,296,425,UP,  false},
        {NOR,   tr("nor"),   215,493,263,530,UP,  false},
        {SD,    tr("sd"),    145,114,296,263,DOWN,false},
        {SWITCH,tr("switch"),247,728,385,802,UP,  false},
        {JTAG,  tr("jtag"),  52 ,499,131,671,UP,  false},
        {AUDIO, tr("audio"), 673,686,744,810,UP,  false},
        {BOOT,  tr("boot"),  409,703,650,803,UP,  false},
    };
};

#endif // BOARDWINDOW_H
