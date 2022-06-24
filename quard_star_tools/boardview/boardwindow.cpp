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

extern QString VERSION;
extern QString GIT_TAG;

BoardWindow::BoardWindow(QString path, QString color,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BoardWindow),envPath(path),skinColor(color)
{
    ui->setupUi(this);

    maskromImgPath = envPath + "/mask_rom/mask_rom.bin";
    pflashImgPath = envPath + "/fw/pflash.img";
    norflashImgPath = envPath + "/fw/norflash.img";
    nandflashImgPath = envPath + "/fw/nandflash.img";
    sdImgPath = envPath + "/fw/sd.img";
    usbflashImgPath = envPath + "/fw/usb.img";
    rootfsImgPath = envPath + "/rootfs/rootfs.img";
    vcan_name = "";//TODO:add config host vcan
    tap_name = "";//TODO:add config host tap

    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    QRect screen = QGuiApplication::screenAt(this->mapToGlobal({this->width()/2,0}))->geometry();
    QRect size = this->geometry();
    this->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
    
    QPixmap pix;
    QFileInfo skinFile(":/boardview/icons/board_"+skinColor+".png");
    if(!skinFile.isFile())
        skinColor = "green";
    pix.load(":/boardview/icons/board_"+skinColor+".png",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    resize(pix.size());
    setMask(QBitmap(pix.mask()));

    qemu_process = new QProcess(this);
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

void BoardWindow::powerSwitch(bool power)
{
#if defined(Q_OS_WIN)
    QString program = envPath + "/qemu_w64/qemu-system-riscv64.exe";
#elif defined(Q_OS_LINUX)
    QString program = envPath + "/qemu/bin/qemu-system-riscv64";
#else
    QString program = envPath + "qemu-system-riscv64";
#endif
    QStringList arguments = {
        "-M",         
            "quard-star,mask-rom-path="+maskromImgPath+",canbus=canbus0",
        "-m",         
            "1G",
        "-smp",       
            "8",
        "-global",    
            "quard-star-syscon.boot-cfg=sd",
        "-drive",     
            "if=pflash,bus=0,unit=0,format=raw,file="+pflashImgPath+",id=mtd0",
        "-drive",     
            "if=mtd,bus=0,unit=0,format=raw,file="+norflashImgPath+",id=mtd1",
        "-drive",     
            "if=mtd,bus=1,unit=0,format=raw,file="+nandflashImgPath+",id=mtd2",
        "-drive",     
            "if=none,format=raw,file="+usbflashImgPath+",id=usb0",
        "-drive",     
            "if=sd,format=raw,file="+sdImgPath+",id=sd0",
        "-drive",     
            "if=none,format=raw,file="+rootfsImgPath+",id=disk0",
        "-chardev",   
            "socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1",
        "-object",    
            "can-bus,id=canbus0",
        [&]() -> QString { if(vcan_name.isEmpty()) return ""; else return "-object"; }(),
            [&]() -> QString { if(vcan_name.isEmpty()) return ""; else return "can-host-socketcan,id=socketcan0,if="+vcan_name+",canbus=canbus0"; }(),
        "-netdev",
            [&]() -> QString { if(tap_name.isEmpty()) return  
                "user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu_net0,dns=192.168.31.56,tftp="+envPath+",bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0"; else return
                "tap,ifname="+tap_name+",script=no,downscript=no,id=net0"; }(),
        "-netdev",    
            "user,net=192.168.32.0/24,host=192.168.32.2,hostname=qemu_net1,dns=192.168.32.56,dhcpstart=192.168.32.100,id=net1",
#if defined(Q_OS_LINUX)
        "-audiodev",  
            "sdl,id=audio0",
#elif defined(Q_OS_WIN)
        "-audiodev",  
            "dsound,id=audio0",
#endif
        "-net",       
            "nic,netdev=net0",
        "-device",    
            "usb-storage,drive=usb0",
        "-device",    
            "usb-serial,always-plugged=true,chardev=usb1",
        "-device",   
            "wm8750,audiodev=audio0",
        "-fw_cfg",    
            "name=opt/qemu_cmdline,string=qemu_vc=:vn:24x80:",
#if defined(Q_OS_LINUX)
        "-fsdev",     
            "local,security_model=mapped-xattr,path="+envPath+",id=fsdev0",
#endif
        "-global",    
            "virtio-mmio.force-legacy=false",
        "-device",    
            "virtio-blk-device,drive=disk0,id=hd0",
#if defined(Q_OS_LINUX)
        "-device",    
            "virtio-9p-device,fsdev=fsdev0,mount_tag=hostshare,id=fs0",
#endif
        "-device",    
            "virtio-net-device,netdev=net1",
        "-device",    
            "virtio-gpu-device,xres=1280,yres=720,id=video0",
        "-device",    
            "virtio-mouse-device,id=input0",
        "-device",    
            "virtio-keyboard-device,id=input1",
        "-display",   
            "vnc=127.0.0.1:1",                /*vnc base port is 5900, so this 1 is 5901*/
        "--serial",   
            "telnet:127.0.0.1:3441,server,nowait",
        "--serial",   
            "telnet:127.0.0.1:3442,server,nowait",
        "--serial",   
            "telnet:127.0.0.1:3443,server,nowait",
        "--monitor",  
            "telnet:127.0.0.1:3430,server,nowait",
        "--parallel", 
            "none",
    };

    if(power) {
        qemu_process->kill();
        qemu_process->waitForFinished(-1);
        qemu_process->start(program, arguments);
        for (int i=0;i<200;i++) {
            QThread::msleep(10);
            qApp->processEvents();
        }
        telnet[0]->reConnect();
        telnet[1]->reConnect();
        telnet[2]->reConnect();
        telnet[3]->reConnect();
        vnc->reConnect();
    } else {
        qemu_process->kill();
        qemu_process->waitForFinished(-1);
    }
}

void BoardWindow::about()
{
    QMessageBox::about(this, tr("About"), tr("Version")+" \n "+VERSION+"\n"+tr("Commit")+" \n "+GIT_TAG+"\n"+tr("Author")+"\n qiaoqm@aliyun.com \n"+tr("Website")+"\n https://github.com/QQxiaoming/quard_star_tutorial");
}

void BoardWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void BoardWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QAction *pAbout= new QAction(tr("About"), this);
    QIcon icoAbout(":/boardview/icons/about.png");
    pAbout->setIcon(icoAbout);
    menu->addAction(pAbout);
    connect(pAbout,SIGNAL(triggered()),this,SLOT(about()));

    QAction *pAboutQt= new QAction(tr("About")+" Qt", this);
    QIcon icoAboutQt(":/boardview/icons/aboutqt.png");
    pAboutQt->setIcon(icoAboutQt);
    menu->addAction(pAboutQt);
    connect(pAboutQt,SIGNAL(triggered()),this,SLOT(aboutQt()));

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
    QString paths = ":/boardview/icons/board_"+skinColor+".png";
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
                painter.drawText(spaceList[i].x1,spaceList[i].y1-15,spaceList[i].name);
            else if(spaceList[i].dir == 1)
                painter.drawText(spaceList[i].x1,spaceList[i].y2+15+30,spaceList[i].name);
        }
    }
    
    if(powerOn){
        pen.setWidth(10);
        painter.setPen(pen);
        painter.drawLine(263,612,283,612);
    }
    event->accept();
}

void BoardWindow::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton) {
        isMousePressed = true;
        mStartPos = event->pos();
    }
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
    if( event->button() == Qt::LeftButton) {
        isMousePressed = false;
    }
    event->accept();
}

void BoardWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton) {
        for(size_t i=0;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
            if( event->pos().x() >= spaceList[i].x1 && event->pos().x() <= spaceList[i].x2 &&
                event->pos().y() >= spaceList[i].y1 && event->pos().y() <= spaceList[i].y2) {
                if(spaceList[i].name == "vga") {
                    vnc->show();
                } else if(spaceList[i].name == "uart0") {
                    telnet[0]->show();
                } else if(spaceList[i].name == "uart1") {
                    telnet[1]->show();
                } else if(spaceList[i].name == "uart2") {
                    telnet[2]->show();
                } else if(spaceList[i].name == "jtag") {
                    telnet[3]->show();
                } else if(spaceList[i].name == "sd") {
                    sdImgPath = QFileDialog::getOpenFileName(this, tr("Select SD IMG"), sdImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "nor") {
                    norflashImgPath = QFileDialog::getOpenFileName(this, tr("Select NorFlash IMG"), norflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "nand") {
                    nandflashImgPath = QFileDialog::getOpenFileName(this, tr("Select NandFlash IMG"), nandflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "soc") {
                    pflashImgPath = QFileDialog::getOpenFileName(this, tr("Select PFlash IMG"), pflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "usb0") {
                    usbflashImgPath = QFileDialog::getOpenFileName(this, tr("Select USBFlash IMG"), usbflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "switch") {
                    powerOn = !powerOn;
                    this->repaint();
                    powerSwitch(powerOn);
                }
            }
        }
    }
    event->accept();
}
