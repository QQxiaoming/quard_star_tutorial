#include <QThread>
#include <QMessageBox>
#include "boardwindow.h"
#include "vncwindow.h"
#include "ui_vncwindow.h"

VncWindow::VncWindow(const QString &addr, int port, QWidget *parent)
    : QMainWindow(parent),severAddr(addr),severPort(port)
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

void VncWindow::on_actionHelp_triggered()
{
    QMessageBox::about(this, tr("Help"), tr("TODO"));
}

void VncWindow::on_actionAbout_triggered()
{
    BoardWindow::appAbout(this);
}

void VncWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

