#include <QPainter>
#include <QMenu>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QThread>
#include <QBitmap>
#include <QFileDialog>
#include <QPoint>

#include "ui_boardwindow.h"
#include "boardwindow.h"

extern QString VERSION;
extern QString GIT_TAG;

BoardWindow::BoardWindow(const QString &path,const QString &color,QWidget *parent) :
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
    vcan_name = "";
    tap_name = "";

#if defined(Q_OS_MACOS)
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
#endif
    QRect screen = QGuiApplication::screenAt(this->mapToGlobal(QPoint(this->width()/2,0)))->geometry();
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
    netSelect = new NetSelectBox(this);
}

BoardWindow::~BoardWindow()
{
    qemu_process->kill();
    qemu_process->waitForFinished(-1);
    delete qemu_process;
    delete netSelect;
    delete vnc;
    delete telnet[0];
    delete telnet[1];
    delete telnet[2];
    delete telnet[3];
    delete ui;
}

bool BoardWindow::powerSwitch(bool power)
{
#if defined(Q_OS_WIN)
    QString program = envPath + "/qemu_w64/qemu-system-riscv64.exe";
#elif defined(Q_OS_LINUX)
    QString program = envPath + "/qemu/bin/qemu-system-riscv64";
#elif defined(Q_OS_MACOS)
    QString program = envPath + "/qemu_macos/bin/qemu-system-riscv64";
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
#elif defined(Q_OS_MACOS)
        "-audiodev",  
            "coreaudio,id=audio0",
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
#if (defined(Q_OS_LINUX) || defined(Q_OS_MACOS))
        "-fsdev",     
            "local,security_model=mapped-xattr,path="+envPath+",id=fsdev0",
#endif
        "-global",    
            "virtio-mmio.force-legacy=false",
        "-device",    
            "virtio-blk-device,drive=disk0,id=hd0",
#if (defined(Q_OS_LINUX) || defined(Q_OS_MACOS))
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

    arguments.removeAll(QString(""));
    if(power) {
        qemu_process->kill();
        qemu_process->waitForFinished(-1);
        qemu_process->start(program, arguments);
        for (int i=0;i<200;i++) {
            QThread::msleep(10);
            qApp->processEvents();
        }

        if(qemu_process->state() == QProcess::NotRunning) {
            int exitcode = qemu_process->exitCode();
            if(exitcode != 0) {
                QMessageBox::critical(this, tr("Error"), tr("power up error!"));
                return false;
            }
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

    return true;
}

void BoardWindow::addActionGInfo(QMenu *menu,const QString &title)
{
    QAction *pGInfo= new QAction(tr("Get Info"), this);
    QIcon icoInfo(":/boardview/icons/info.svg");
    pGInfo->setIcon(icoInfo);
    pGInfo->setToolTip(title);
    menu->addAction(pGInfo);
    connect(pGInfo,&QAction::triggered,this,
            [&](void)
            {
                QAction* pGInfo = qobject_cast<QAction*>(sender());
                QString title = pGInfo->toolTip();
                QString info;
                if(title == "soc") {
                    info = tr("Quard Star SOC:\n 8 core riscv64 architecture. \nInternal packaging:\n 128K maskrom, 896k sram, 2M pflash. \nSupport:\n UARTx3, I2Cx3, SPIx2, DDR controller, NAND Flash controller, SDMMC, USB3.0, ETH, LCDC, CAN, DMA, GPIO, PWM, TIMER, ADC, WATCHDOG, RTC, SYSCON.");
                } else if(title == "ddr") {
                    info = tr("DDR:\n 1G.");
                } else if(title == "nor") {
                    info = tr("Nor Flash:\n is25wp256 32M.");
                } else if(title == "nand") {
                    info = tr("NAND Flash:\n onenand 256M.");
                } else if(title == "sd") {
                    info = tr("SD card:\n SDSC 32M.");
                } else if(title == "usb0") {
                    info = tr("USB Flash:\n 32M, Speed 5000 Mb/s.");
                } else if(title == "usb1") {
                    info = tr("USB Serial:\n FT232RL, Speed 12 Mb/s.");
                } else if(title == "vga") {
                    info = tr("LCDC:\n VGA port.");
                } else if(title == "uart0") {
                    info = tr("UART:\n 115200-8-n-1.");
                } else if(title == "uart1") {
                    info = tr("UART:\n 115200-8-n-1.");
                } else if(title == "uart2") {
                    info = tr("UART:\n 115200-8-n-1.");
                } else if(title == "jtag") {
                    info = tr("QEMU monitor:\n monitor terminal.");
                } else if(title == "eth") {
                    info = tr("ETH:\n IEEE 802.3 100M full duplex.");
                } else if(title == "audio") {
                    info = tr("IIS:\n wm8750.");
                } else if(title == "boot") {
                    info = tr("boot:\n 000-pflash 001-spi_nor_flash 010-sd 100-uart0.");
                } else if(title == "power") {
                    info = tr("power:\n DC 12V.");
                } else if(title == "switch") {
                    info = tr("switch:\n power switch.");
                }
                QMessageBox::about(this, tr("Get Info"), info);
            }
        );
}

void BoardWindow::addActionOFileSystem(QMenu *menu,const QString &title)
{
    QAction *pOFileSystem= new QAction(tr("Open FileSystem"), this);
    QIcon icoOpen(":/boardview/icons/open.svg");
    pOFileSystem->setIcon(icoOpen);
    pOFileSystem->setToolTip(title);
    menu->addAction(pOFileSystem);
}

void BoardWindow::addActionSetting(QMenu *menu,const QString &title)
{
    QAction *pSetting= new QAction(tr("Setting"), this);
    QIcon icoSetting(":/boardview/icons/setting.svg");
    pSetting->setIcon(icoSetting);
    pSetting->setToolTip(title);
    menu->addAction(pSetting);
}

void BoardWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    QString spaceDoMain;

    for(size_t i=0;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
        if( event->pos().x() >= spaceList[i].x1 && event->pos().x() <= spaceList[i].x2 &&
            event->pos().y() >= spaceList[i].y1 && event->pos().y() <= spaceList[i].y2) {
            spaceDoMain = spaceList[i].name;
        }
    }

    if(spaceDoMain == "soc") {
        addActionGInfo(menu,spaceDoMain);
        addActionOFileSystem(menu,spaceDoMain);
    } else if(spaceDoMain == "ddr") {
        addActionGInfo(menu,spaceDoMain);
    } else if(spaceDoMain == "nor") {
        addActionGInfo(menu,spaceDoMain);
        addActionOFileSystem(menu,spaceDoMain);
    } else if(spaceDoMain == "nand") {
        addActionGInfo(menu,spaceDoMain);
        addActionOFileSystem(menu,spaceDoMain);
    } else if(spaceDoMain == "sd") {
        addActionGInfo(menu,spaceDoMain);
        addActionOFileSystem(menu,spaceDoMain);
    } else if(spaceDoMain == "usb0") {
        addActionGInfo(menu,spaceDoMain);
        addActionOFileSystem(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "usb1") {
        addActionGInfo(menu,spaceDoMain);
        addActionOFileSystem(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "vga") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "uart0") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "uart1") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "uart2") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "jtag") {
        addActionGInfo(menu,spaceDoMain);
    } else if(spaceDoMain == "eth") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "audio") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "boot") {
        addActionGInfo(menu,spaceDoMain);
        addActionSetting(menu,spaceDoMain);
    } else if(spaceDoMain == "power") {
        addActionGInfo(menu,spaceDoMain);
    } else if(spaceDoMain == "switch") {
        addActionGInfo(menu,spaceDoMain);
    } else if(spaceDoMain.isEmpty()) {
        QAction *pHelp= new QAction(tr("Help"), this);
        QIcon icoHelp(":/boardview/icons/help.png");
        pHelp->setIcon(icoHelp);
        menu->addAction(pHelp);
        connect(pHelp,&QAction::triggered,this,
            [&](void)
            {
                QMessageBox::about(this, tr("Help"), "TODO");
            }
        );

        QAction *pAbout= new QAction(tr("About"), this);
        QIcon icoAbout(":/boardview/icons/about.png");
        pAbout->setIcon(icoAbout);
        menu->addAction(pAbout);
        connect(pAbout,&QAction::triggered,this,
            [&](void)
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
        );

        QAction *pAboutQt= new QAction(tr("About")+" Qt", this);
        QIcon icoAboutQt(":/boardview/icons/aboutqt.png");
        pAboutQt->setIcon(icoAboutQt);
        menu->addAction(pAboutQt);
        connect(pAboutQt,&QAction::triggered,this,
            [&](void)
            {
                QMessageBox::aboutQt(this);
            }
        );

        QAction *pExit = new QAction(tr("Exit"), this);
        QIcon icoExit(":/boardview/icons/exit.png");
        pExit->setIcon(icoExit);
        menu->addAction(pExit);
        connect(pExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    }

    if(!menu->isEmpty()) {
        menu->move(cursor().pos());
        menu->show();
    }

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
    font.setPointSize(20);
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
                painter.drawText(spaceList[i].x1,spaceList[i].y1-15,spaceList[i].drawName);
            else if(spaceList[i].dir == 1)
                painter.drawText(spaceList[i].x1,spaceList[i].y2+15+30,spaceList[i].drawName);
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
                } else if(spaceList[i].name == "eth") {
                    netSelect->show();
                } else if(spaceList[i].name == "sd") {
                    sdImgPath = getOpenFileName(tr("Select SD IMG"), sdImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "nor") {
                    norflashImgPath = getOpenFileName(tr("Select NorFlash IMG"), norflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "nand") {
                    nandflashImgPath = getOpenFileName(tr("Select NandFlash IMG"), nandflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "soc") {
                    pflashImgPath = getOpenFileName(tr("Select PFlash IMG"), pflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "usb0") {
                    usbflashImgPath = getOpenFileName(tr("Select USBFlash IMG"), usbflashImgPath, "IMG files(*.img *.bin)");
                } else if(spaceList[i].name == "switch") {
                    powerOn = !powerOn;
                    this->repaint();
                    if(!powerSwitch(powerOn)){
                        powerOn = !powerOn;
                        this->repaint();
                    }
                }
            }
        }
    }
    event->accept();
}

QString BoardWindow::getOpenFileName(const QString &caption, const QString &fileName, const QString &filter)
{
    QString path = QFileDialog::getOpenFileName(this, caption, fileName, filter);
    if(path.isEmpty()) {
        return fileName;
    } else {
        return path;
    }
}
