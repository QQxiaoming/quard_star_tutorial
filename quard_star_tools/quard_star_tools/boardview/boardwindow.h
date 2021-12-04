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
        int x1,x2,y1,y2;
        int dir;
        bool draw;
    }spaceList[16] = {
        {"vga",552,816,12,148,1,false},
        {"uart0",45,134,68,158,1,false},
        {"uart1",45,134,189,277,1,false},
        {"uart2",45,134,310,400,1,false},
        {"power",128,231,559,742,0,false},
        {"soc",310,506,225,410,0,false},
        {"ddr",535,648,225,410,0,false},
        {"eth",854,1068,69,250,1,false},
        {"usb0",854,1068,291,438,0,false},
        {"usb1",854,1068,478,626,0,false},
        {"nand",220,373,69,159,1,false},
        {"nor",433,514,78,141,1,false},
        {"sd",675,795,548,697,0,false},
        {"switch",260,438,559,636,0,false},
        {"jtag",45,134,429,539,0,false},
        {"audio",522,592,548,708,0,false},
    };
};












#endif // BOARDWINDOW_H
