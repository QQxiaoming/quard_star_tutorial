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

    //QPixmap pix;
    //pix.load(":/boardview/icons/ttf.png",0,
    //            Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    //pix = pix.scaled(QSize(640,480));
    //resize(pix.size());
    //setMask(QBitmap(pix.mask()));
    
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
    QMessageBox::about(this, tr("Help"), 
        tr("1. The central window is the LCD emulation output window.") + "\n" +
        tr("2. The refresh button at the bottom is used to refresh and reconnect, which is used to connect when the simulation restarts.")
    );
}

void VncWindow::on_actionAbout_triggered()
{
    BoardWindow::appAbout(this);
}

void VncWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

