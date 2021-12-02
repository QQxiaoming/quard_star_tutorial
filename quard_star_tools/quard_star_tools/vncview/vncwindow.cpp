#include "vncwindow.h"
#include "ui_vncwindow.h"

VncWindow::VncWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VncWindow)
{
    ui->setupUi(this);
    if(ui->vncView->connectToVncServer("127.0.0.1","",5901)) {
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
