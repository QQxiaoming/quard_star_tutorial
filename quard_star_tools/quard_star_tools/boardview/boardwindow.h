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

namespace Ui {
class BoardWindow;
}

class BoardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BoardWindow(QString path,QString color = "green",QWidget *parent = nullptr);
    ~BoardWindow();
    void powerSwitch(bool power);

private slots:
    void about();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    Ui::BoardWindow *ui;
    bool isMousePressed = false;
    bool powerOn = false;
    QPoint mStartPos;
    QProcess *qemu_process;
    VncWindow *vnc;
    TelnetWindow *telnet[4];
    void show_vnc(void);
    QString envPath;
    QString skinColor;
    QString maskromImgPath;
    QString pflashImgPath;
    QString norflashImgPath;
    QString sdImgPath;
    QString usbflashImgPath;
    QString rootfsImgPath;
    struct space
    {
        QString name;
        int x1,y1,x2,y2;
        int dir;
        bool draw;
    }spaceList[16] = {
        {"vga",315,111,649,262,1,false},
        {"uart2",52,381,107,457,1,false},
        {"uart1",52,290,107,366,1,false},
        {"uart0",52,198,107,272,1,false},
        {"power",127,709,222,837,0,false},
        {"soc",369,377,470,479,0,false},
        {"ddr",542,345,620,508,0,false},
        {"eth",689,179,838,340,0,false},
        {"usb0",685,367,837,504,0,false},
        {"usb1",685,539,837,674,0,false},
        {"nand",197,365,296,425,0,false},
        {"nor",215,493,263,530,0,false},
        {"sd",145,114,296,263,1,false},
        {"switch",247,728,385,802,0,false},
        {"jtag",52,499,131,671,0,false},
        {"audio",673,686,744,810,0,false},
    };
};












#endif // BOARDWINDOW_H
