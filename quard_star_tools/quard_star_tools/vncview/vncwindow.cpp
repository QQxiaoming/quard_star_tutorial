#include <QThread>

#include "vncwindow.h"
#include "ui_vncwindow.h"

VncWindow::VncWindow(QString addr, int port, QWidget *parent)
    : QMainWindow(parent),severaddr(addr),severport(port)
    , ui(new Ui::VncWindow)
{
    int retry = 0;
    ui->setupUi(this);
    while((!ui->vncView->connectToVncServer(severaddr,"",port))&&retry<5) {
        retry++;
        QThread::sleep(1*retry);
    }
    if(retry<5) {
        ui->vncView->startFrameBufferUpdate();
    }

    QObject::connect(ui->refresh_pushbuttion, SIGNAL(clicked()), this, SLOT(refresh_clicked()));
}

VncWindow::~VncWindow()
{
    delete ui;
}

void VncWindow::refresh_clicked()
{
    if(ui->vncView->isConnectedToServer()) {
        ui->vncView->disconnectFromVncServer();
    }
    if(ui->vncView->connectToVncServer("127.0.0.1","",5901)) {
        ui->vncView->startFrameBufferUpdate();
    }
}
