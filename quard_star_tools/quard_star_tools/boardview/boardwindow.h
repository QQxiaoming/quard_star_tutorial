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
    explicit BoardWindow(QWidget *parent = nullptr);
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
    struct space
    {
        QString name;
        int x1,y1,x2,y2;
        int dir;
        bool draw;
    }spaceList[16] = {
        {"vga",264,72,582,225,1,false},
        {"uart2",18,341,69,408,1,false},
        {"uart1",18,251,69,322,1,false},
        {"uart0",18,162,69,229,1,false},
        {"power",74,649,168,806,0,false},
        {"soc",322,334,420,428,0,false},
        {"ddr",486,303,561,459,0,false},
        {"eth",629,141,767,290,0,false},
        {"usb0",625,323,769,451,0,false},
        {"usb1",625,488,769,616,0,false},
        {"nand",159,317,254,379,0,false},
        {"nor",177,447,224,480,0,false},
        {"sd",107,80,250,220,1,false},
        {"switch",205,668,334,740,0,false},
        {"jtag",16,452,97,622,0,false},
        {"audio",615,630,678,752,0,false},
    };
};












#endif // BOARDWINDOW_H
