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
    QMessageBox::about(this, tr("About"),
                       tr(
                           "<p>Version</p>"
                           "<p>&nbsp;%1</p>"
                           "<p>Commit</p>"
                           "<p>&nbsp;%2</p>"
                           "<p>Author</p>"
                           "<p>&nbsp;qiaoqm@aliyun.com</p>"
                           "<p>Website</p>"
                           "<p>&nbsp;<a href='https://github.com/QQxiaoming/quard_star_tutorial'>https://github.com/QQxiaoming</p>"
                           "<p>&nbsp;<a href='https://gitee.com/QQxiaoming/quard_star_tutorial'>https://gitee.com/QQxiaoming</a></p>"
                           ).arg(VERSION,GIT_TAG)
                       );
}

void VncWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

