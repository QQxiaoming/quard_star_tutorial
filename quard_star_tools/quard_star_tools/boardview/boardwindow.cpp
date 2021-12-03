#include <QPainter>
#include <QMenu>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QThread>
#include <QBitmap>
#include <QFileDialog>

#include "ui_boardwindow.h"
#include "boardwindow.h"

const QString VERSION = APP_VERSION;
const QString GIT_TAG =
#include <git_tag.inc>
;

static QString envPath = "/home/qqm/Downloads/quard_star_tutorial/output";
static QString maskromImgPath = envPath + "/mask_rom/mask_rom.bin";
static QString pflashImgPath = envPath + "/fw/fw.bin";
static QString norflashImgPath = envPath + "/fw/norflash.img";
static QString sdImgPath = envPath + "/fw/sd.img";
static QString usbflashImgPath = envPath + "/fw/usb.img";
static QString rootfsImgPath = envPath + "/rootfs/rootfs.img";
static QString program = envPath + "/qemu/bin/qemu-system-riscv64";

BoardWindow::BoardWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BoardWindow)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    QRect screen = QGuiApplication::screenAt(this->mapToGlobal({this->width()/2,0}))->geometry();
    QRect size = this->geometry();
    this->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
    
    QPixmap pix;
    pix.load(":/boardview/icons/board.png",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    resize(pix.size());
    setMask(QBitmap(pix.mask()));


    QStringList arguments = {
        "-M",         "quard-star,mask-rom-path="+maskromImgPath,
        "-m",         "1G",
        "-smp",       "8",
        "-drive",     "if=pflash,bus=0,unit=0,format=raw,file="+pflashImgPath+",id=mtd0",
        "-drive",     "if=mtd,format=raw,file="+norflashImgPath+",id=mtd1",
        "-drive",     "if=none,format=raw,file="+usbflashImgPath+",id=usb0",
        "-drive",     "if=sd,format=raw,file="+sdImgPath+",id=sd0",
        "-drive",     "if=none,format=raw,file="+rootfsImgPath+",id=disk0",
        "-chardev",   "socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1",
        "-fsdev",     "local,security_model=mapped-xattr,path="+envPath+",id=fsdev0",
        "-netdev",    "user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu,dns=192.168.31.56,tftp="+envPath+",bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0",
        "-global",    "virtio-mmio.force-legacy=false",
        "-device",    "virtio-blk-device,drive=disk0,id=hd0",
        "-device",    "virtio-gpu-device,xres=1280,yres=720,id=video0",
        "-device",    "virtio-mouse-device,id=input0",
        "-device",    "virtio-keyboard-device,id=input1",
        "-device",    "virtio-9p-device,fsdev=fsdev0,mount_tag=hostshare,id=fs0",
        "-device",    "virtio-net-device,netdev=net0",
        "-device",    "usb-storage,drive=usb0",
        "-device",    "usb-serial,always-plugged=true,chardev=usb1",
        "-fw_cfg",    "name=opt/qemu_cmdline,string=qemu_vc=:vn:24x80:",
        "-global",    "quard-star-syscon.boot-cfg=sd",
        "-display",   "vnc=127.0.0.1:1",                /*vnc base port is 5900, so this 1 is 5901*/
        "--serial",   "telnet::3441,server,nowait",
        "--serial",   "telnet::3442,server,nowait",
        "--serial",   "telnet::3443,server,nowait",
        "--monitor",  "telnet::3430,server,nowait",
        "--parallel", "none",
    };

    qemu_process = new QProcess(this);
    qemu_process->start(program, arguments);
    telnet[0] = new TelnetWindow("127.0.0.1",3441,this);
    telnet[1] = new TelnetWindow("127.0.0.1",3442,this);
    telnet[2] = new TelnetWindow("127.0.0.1",3443,this);
    telnet[3] = new TelnetWindow("127.0.0.1",3430,this);
    vnc = new VncWindow("127.0.0.1",5901,this);
}

BoardWindow::~BoardWindow()
{
    qemu_process->kill();
    qemu_process->waitForFinished(-1);
    delete qemu_process;
    delete ui;
    delete vnc;
    delete telnet[0];
    delete telnet[1];
    delete telnet[2];
    delete telnet[3];
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
    QFont font;
    QPen pen;
    pen.setWidth(5);
    pen.setColor(Qt::red);
    pen.setStyle(Qt::DashLine);
    font.setPointSize(30);
    painter.setFont(font);
    painter.setPen(pen);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));

    for(size_t i=0;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
        if(spaceList[i].draw) {
            painter.drawLine(spaceList[i].x1,spaceList[i].y1,spaceList[i].x1,spaceList[i].y2);
            painter.drawLine(spaceList[i].x1,spaceList[i].y1,spaceList[i].x2,spaceList[i].y1);
            painter.drawLine(spaceList[i].x2,spaceList[i].y1,spaceList[i].x2,spaceList[i].y2);
            painter.drawLine(spaceList[i].x1,spaceList[i].y2,spaceList[i].x2,spaceList[i].y2);
            if(spaceList[i].dir == 0)
                painter.drawText(spaceList[i].x1-10,spaceList[i].y1-15,spaceList[i].name);
            else if(spaceList[i].dir == 1)
                painter.drawText(spaceList[i].x1-10,spaceList[i].y2+15+30,spaceList[i].name);
        }
    }
    
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

    bool do_repaint = false;
    for(size_t i=0;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
        if( event->pos().x() >= spaceList[i].x1 && event->pos().x() <= spaceList[i].x2 &&
            event->pos().y() >= spaceList[i].y1 && event->pos().y() <= spaceList[i].y2) {
            if(!spaceList[i].draw) {
                spaceList[i].draw = !spaceList[i].draw;
                do_repaint = true;
            }
        } else {
            if(spaceList[i].draw) {
                spaceList[i].draw = !spaceList[i].draw;
                do_repaint = true;
            }
        }
    }
    if(do_repaint)
        this->repaint();
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
        for(size_t i=0;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
            if( event->pos().x() >= spaceList[i].x1 && event->pos().x() <= spaceList[i].x2 &&
                event->pos().y() >= spaceList[i].y1 && event->pos().y() <= spaceList[i].y2) {
                if(spaceList[i].name == "hdmi") {
                    vnc->show();
                } else if(spaceList[i].name == "uart0") {
                    telnet[0]->show();
                } else if(spaceList[i].name == "uart1") {
                    telnet[1]->show();
                } else if(spaceList[i].name == "uart2") {
                    telnet[2]->show();
                } else if(spaceList[i].name == "power") {
                    telnet[3]->show();
                } else if(spaceList[i].name == "nand") {
                    QFileDialog::getOpenFileName(this, tr("Select IMG"), norflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "soc") {
                    QFileDialog::getOpenFileName(this, tr("Select IMG"), pflashImgPath, "IMG files(*.img *.bin)");
                }

            }
        }
        qDebug() << event->pos().x();
        qDebug() << event->pos().y();
    }
    event->accept();
}
