#include <QPainter>
#include <QMenu>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>

#include "ui_boardwindow.h"
#include "boardwindow.h"

const QString VERSION = APP_VERSION;
const QString GIT_TAG =
#include <git_tag.inc>
;

BoardWindow::BoardWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BoardWindow)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    QRect screen = QGuiApplication::screenAt(this->mapToGlobal({this->width()/2,0}))->geometry();
    QRect size = this->geometry();
    this->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);

    QString program = "/home/qqm/Downloads/quard_star_tutorial/run.sh";
    QStringList arguments = {"customize5"};
    qemu_process = new QProcess(this);
    qemu_process->start(program, arguments);
    vnc = new VncWindow(this);
    telnet[0] = new TelnetWindow("127.0.0.1",3441,this);
    telnet[1] = new TelnetWindow("127.0.0.1",3442,this);
    telnet[2] = new TelnetWindow("127.0.0.1",3443,this);
}

BoardWindow::~BoardWindow()
{
    qemu_process->kill();
    delete qemu_process;
    delete ui;
    delete vnc;
    delete telnet[0];
    delete telnet[1];
    delete telnet[2];
}

void BoardWindow::about()
{
    QMessageBox::about(this, "About", "Version \n " + VERSION + "\nCommit \n " + GIT_TAG + "\nAuthor\n qiaoqm@aliyun.com \nWebsite\n https://github.com/QQxiaoming/quard_star_tutorial");
}

void BoardWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QAction *pAbout= new QAction(tr("About"), this);
    QIcon icoNew(":/boardview/icons/about.png");
    pAbout->setIcon(icoNew);
    menu->addAction(pAbout);
    connect(pAbout,SIGNAL(triggered()),this,SLOT(about()));

    QAction *pExit = new QAction(tr("Exit"), this);
    QIcon icoExit(":/boardview/icons/exit.png");
    pExit->setIcon(icoExit);
    menu->addAction(pExit);
    connect(pExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    menu->move(cursor().pos());
    menu->show();

    event->accept();
}

void BoardWindow::paintEvent(QPaintEvent *event)
{
    QString paths = ":/boardview/icons/board.png";
    QPainter painter(this);

    painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));

    event->accept();
}

void BoardWindow::mousePressEvent(QMouseEvent *event)
{
    isMousePressed = true;
    mStartPos = event->pos();
    event->accept();
}

void BoardWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isMousePressed)
    {
        QPoint deltaPos = event->pos() - mStartPos;
        this->move(this->pos()+deltaPos);
    }
    event->accept();
}

void BoardWindow::mouseReleaseEvent(QMouseEvent *event)
{
    isMousePressed = false;
    event->accept();
}

void BoardWindow::mouseDoubleClickEvent(QMouseEvent *event)
{

    if( event->button() == Qt::LeftButton) {
        vnc->show();
        telnet[0]->show();
        telnet[2]->show();
    }
    event->accept();
}
