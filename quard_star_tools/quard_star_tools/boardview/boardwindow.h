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
    }spaceList[11] = {
        {"hdmi",360,440,735,830,0,false},
        {"uart0",204,231,36,98,1,false},
        {"uart1",316,343,36,98,1,false},
        {"uart2",428,455,36,98,1,false},
        {"power",138,246,736,839,0,false},
        {"soc",353,559,252,463,0,false},
        {"ddr",604,748,250,468,0,false},
        {"eth",1011,1317,40,273,1,false},
        {"usb0",1070,1319,353,537,0,false},
        {"usb1",1065,1313,612,798,0,false},
        {"nand",132,280,122,302,0,false},
    };
};












#endif // BOARDWINDOW_H
