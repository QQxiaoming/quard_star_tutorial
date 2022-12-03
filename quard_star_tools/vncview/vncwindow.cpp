#include <QThread>

#include "vncwindow.h"
#include "ui_vncwindow.h"

VncWindow::VncWindow(const QString &addr, int port, QWidget *parent)
    : QMainWindow(parent),severaddr(addr),severport(port)
    , ui(new Ui::VncWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->refreshPushbuttion, SIGNAL(clicked()), this, SLOT(refreshClicked()));
}

VncWindow::~VncWindow()
{
    delete ui;
}

void VncWindow::reConnect(void)
{
    if(ui->vncView->isConnectedToServer()) {
        ui->vncView->disconnectFromVncServer();
    }
    if(ui->vncView->connectToVncServer("127.0.0.1","",5901)) {
        ui->vncView->startFrameBufferUpdate();
    }
}

void VncWindow::refreshClicked()
{
    reConnect();
}
