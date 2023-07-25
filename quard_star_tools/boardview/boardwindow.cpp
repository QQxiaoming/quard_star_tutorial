#include <QPainter>
#include <QMenu>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QThread>
#include <QBitmap>
#include <QFileDialog>
#include <QPoint>
#include <QToolTip>
#include "qfonticon.h"

#include "ui_boardwindow.h"
#include "boardwindow.h"

BoardWindow::BoardWindow(const QString &path,const QString &color,
                         const bool &isSysDarkTheme,
                         QLocale::Language force_translator,
                         QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BoardWindow),
    envPath(path),
    skinColor(color),
    isDarkTheme(isSysDarkTheme),
    maskRomImgPath(envPath + "/mask_rom/mask_rom.bin"),
    pFlashImgPath(envPath + "/fw/pflash.img"),
    norFlashImgPath(envPath + "/fw/norflash.img"),
    nandFlashImgPath(envPath + "/fw/nandflash.img"),
    sdImgPath(envPath + "/fw/sd.img"),
    usbFlashImgPath(envPath + "/fw/usb.img"),
    rootFSImgPath(envPath + "/rootfs/rootfs.img"),
    tapName(""),
    vCanName(""),
    bootCfg("sd"),
    updateCfg(false)
{
    ui->setupUi(this);

#if defined(Q_OS_MACOS)
    this->setWindowFlags(Qt::CustomizeWindowHint | 
                            Qt::WindowTitleHint | Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
#endif
    QRect screen = QGuiApplication::screenAt(
                       this->mapToGlobal(QPoint(this->width()/2,0)))->geometry();

    QPixmap pix;
    QFileInfo skinFile(":/boardview/icons/board_"+skinColor+".png");
    if(!skinFile.isFile())
        skinColor = "green";
    pix.load(":/boardview/icons/board_"+skinColor+".png",0,
                Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    if(pix.size().width() > screen.width() || pix.size().height() > screen.height() ) {
        int target_size = qMin(screen.width(),screen.height());
        scaled_value = ((double)pix.size().width())/((double)target_size);
        pix = pix.scaled(QSize(target_size,target_size));
        for(size_t i=0;i<sizeof(spaceList)/sizeof(struct space);i++) {
            spaceList[i].x1 = spaceList[i].x1/scaled_value;
            spaceList[i].x2 = spaceList[i].x2/scaled_value;
            spaceList[i].y1 = spaceList[i].y1/scaled_value;
            spaceList[i].y2 = spaceList[i].y2/scaled_value;
        }
    }
    resize(pix.size());
    setMask(QBitmap(pix.mask()));
    QRect size = this->geometry();
    this->move(qMax(0,(screen.width() - size.width())) / 2,
               qMax(0,(screen.height() - size.height())) / 2);
    qemuProcess = new QProcess(this);
    uartWindow[0] = new TelnetWindow("127.0.0.1",3441,force_translator,this);
    uartWindow[0]->setWindowTitle("UART0");
    uartWindow[1] = new TelnetWindow("127.0.0.1",3442,force_translator,this);
    uartWindow[1]->setWindowTitle("UART1");
    uartWindow[2] = new TelnetWindow("127.0.0.1",3443,force_translator,this);
    uartWindow[2]->setWindowTitle("UART2");
    jtagWindow = new TelnetWindow("127.0.0.1",3430,force_translator,this);
    jtagWindow->setWindowTitle("JTAG(Monitor)");
    lcdWindow = new VncWindow("127.0.0.1",5901,this);
    lcdWindow->setWindowTitle("LCD");
    netSelect = new NetSelectBox(this);
    bootSelect = new BootSelectBox(this);

    setFixedSize(this->size());
}

BoardWindow::~BoardWindow()
{
    qemuProcess->kill();
    qemuProcess->waitForFinished(-1);
    delete qemuProcess;
    delete netSelect;
    delete bootSelect;
    delete lcdWindow;
    delete uartWindow[0];
    delete uartWindow[1];
    delete uartWindow[2];
    delete jtagWindow;
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
            "quard-star,mask-rom-path="+maskRomImgPath+",canbus=canbus0",
        "-m",         
            "1G",
        "-smp",       
            "8",
        "-global",    
            "quard-star-syscon.boot-cfg="+bootCfg,
        "-global",    
            [&]() -> QString {if(updateCfg) return 
            "quard-star-syscon.update-cfg=true"; else return 
            "quard-star-syscon.update-cfg=false";}(),
        "-drive",     
            "if=pflash,bus=0,unit=0,format=raw,file="+pFlashImgPath+",id=mtd0",
        "-drive",     
            "if=mtd,bus=0,unit=0,format=raw,file="+norFlashImgPath+",id=mtd1",
        "-drive",     
            "if=mtd,bus=1,unit=0,format=raw,file="+nandFlashImgPath+",id=mtd2",
        "-drive",     
            "if=none,format=raw,file="+usbFlashImgPath+",id=usb0",
        "-drive",     
            "if=sd,format=raw,file="+sdImgPath+",id=sd0",
        "-drive",     
            "if=none,format=raw,file="+rootFSImgPath+",id=disk0",
        "-chardev",   
            "socket,telnet=on,host=127.0.0.1,port=3450,server=on,wait=off,id=usb1",
        "-object",    
            "can-bus,id=canbus0",
        [&]() -> QString { if(vCanName.isEmpty()) return ""; else return "-object"; }(),
            [&]() -> QString { if(vCanName.isEmpty()) return ""; else return "can-host-socketcan,id=socketcan0,if="+vCanName+",canbus=canbus0"; }(),
        "-netdev",
            [&]() -> QString { if(tapName.isEmpty()) return  
                "user,net=192.168.31.0/24,host=192.168.31.2,hostname=qemu_net0,dns=192.168.31.56,tftp="+envPath+",bootfile=/linux_kernel/Image,dhcpstart=192.168.31.100,hostfwd=tcp::3522-:22,hostfwd=tcp::3580-:80,id=net0"; else return
                "tap,ifname="+tapName+",script=no,downscript=no,id=net0"; }(),
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
        //"-device",    
        //    "virtio-gpu-device,xres=1280,yres=720,id=video0",
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
        qemuProcess->kill();
        qemuProcess->waitForFinished(-1);
        qemuProcess->start(program, arguments);
        for (int i=0;i<200;i++) {
            QThread::msleep(10);
            qApp->processEvents();
        }

        if(qemuProcess->state() == QProcess::NotRunning) {
            int exitcode = qemuProcess->exitCode();
            if(exitcode != 0) {
                QMessageBox::critical(this, tr("Error"), tr("power up error!"));
                return false;
            }
        }
        uartWindow[0]->reConnect();
        uartWindow[1]->reConnect();
        uartWindow[2]->reConnect();
        jtagWindow->reConnect();
        lcdWindow->reConnect();
    } else {
        qemuProcess->kill();
        qemuProcess->waitForFinished(-1);
    }

    return true;
}

int BoardWindow::sendQemuCmd(const QString &cmd)
{
    if(qemuProcess->state() == QProcess::Running) {
        jtagWindow->sendData(cmd.toUtf8());
        return 0;
    }
    return -1;
}

QString& BoardWindow::getVCanName(void)
{
    return vCanName;
}

QString& BoardWindow::getTapName(void)
{
    return tapName;
}

QString& BoardWindow::getBootCfg(void)
{
    return bootCfg;
}

bool& BoardWindow::getUpdateCfg(void)
{
    return updateCfg;
}

void BoardWindow::addActionGInfo(QMenu *menu,const DeviceName &title)
{
    QAction *pGInfo= new QAction(tr("Get Info"), this);
    QIcon icoInfo(QFontIcon::icon(QChar(0xf05a)));
    pGInfo->setIcon(icoInfo);
    pGInfo->setToolTip(QString::number(title));
    menu->addAction(pGInfo);
    connect(pGInfo,&QAction::triggered,this,
            [&](void)
            {
                QAction* pGInfo = qobject_cast<QAction*>(sender());
                DeviceName title = static_cast<DeviceName>(pGInfo->toolTip().toInt());
                QString info;
                switch (title)
                {
                    case SOC:
                        info = tr("Quard Star SOC:\n 8 core riscv64 architecture. \nInternal packaging:\n 128K maskrom, 896k sram, 2M pflash. \nSupport:\n UARTx3, I2Cx3, SPIx2, DDR controller, NAND Flash controller, SDMMC, USB3.0, ETH, LCDC, CAN, DMA, GPIO, PWM, TIMER, ADC, WATCHDOG, RTC, SYSCON.");
                        break;
                    case DDR:
                        info = tr("DDR:\n 1G.");
                        break;
                    case NOR:
                        info = tr("Nor Flash:\n is25wp256 32M.");
                        break;
                    case NAND:
                        info = tr("NAND Flash:\n onenand 256M.");
                        break;
                    case SD:
                        info = tr("SD card:\n SDSC 32M.");
                        break;
                    case USB0:
                        info = tr("USB Flash:\n 32M, Speed 5000 Mb/s.");
                        break;
                    case USB1:
                        info = tr("USB Serial:\n FT232RL, Speed 12 Mb/s.");
                        break;
                    case VGA:
                        info = tr("LCDC:\n VGA port 640x480.");
                        break;
                    case UART0:
                    case UART1:
                    case UART2:
                        info = tr("UART:\n 115200-8-n-1.");
                        break;
                    case JTAG:
                        info = tr("QEMU monitor:\n monitor terminal.");
                        break;
                    case ETH:
                        info = tr("ETH:\n IEEE 802.3 100M full duplex.");
                        break;
                    case AUDIO:
                        info = tr("IIS:\n wm8750.");
                        break;
                    case BOOT:
                        info = tr("boot:\n 000-pflash.\n 001-spi_nor_flash.\n 010-sd.\n 100-uart0.");
                        break;
                    case POWER:
                        info = tr("power:\n DC 12V.");
                        break;
                    case SWITCH:
                        info = tr("switch:\n power switch.");
                        break;
                    default:
                        break;
                }

                QMessageBox::about(this, tr("Get Info"), info);
            }
        );
}

void BoardWindow::addActionOFileSystem(QMenu *menu,const DeviceName &title)
{
    QAction *pOFileSystem= new QAction(tr("Open FileSystem"), this);
    QIcon icoOpen(QFontIcon::icon(QChar(0xf07c)));
    pOFileSystem->setIcon(icoOpen);
    pOFileSystem->setToolTip(QString::number(title));
    menu->addAction(pOFileSystem);
}

void BoardWindow::addActionSetting(QMenu *menu,const DeviceName &title)
{
    QAction *pSetting= new QAction(tr("Setting"), this);
    QIcon icoSetting(QFontIcon::icon(QChar(0xf013)));
    pSetting->setIcon(icoSetting);
    pSetting->setToolTip(QString::number(title));
    menu->addAction(pSetting);
    connect(pSetting,&QAction::triggered,this,
            [&](void)
            {
                QAction* pSetting = qobject_cast<QAction*>(sender());
                DeviceName title = static_cast<DeviceName>(pSetting->toolTip().toInt());
                switch (title)
                {
                    case ETH:
                        netSelect->show();
                        break;
                    case BOOT:
                        bootSelect->show();
                        break;
                    default:
                        QMessageBox::about(this, "TODO", "TODO");
                        break;
                }
            }
        );

}

void BoardWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    DeviceName spaceDoMain = UNKNOW;

    for(size_t i=0;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
        if( event->pos().x() >= spaceList[i].x1 && event->pos().x() <= spaceList[i].x2 &&
            event->pos().y() >= spaceList[i].y1 && event->pos().y() <= spaceList[i].y2) {
            spaceDoMain = spaceList[i].name;
        }
    }

    switch (spaceDoMain)
    {
        case SOC:
        case NOR:
        case NAND:
        case SD:
            addActionGInfo(menu,spaceDoMain);
            addActionOFileSystem(menu,spaceDoMain);
            break;
        case DDR:
        case JTAG:
        case POWER:
        case SWITCH:
            addActionGInfo(menu,spaceDoMain);
            break;
        case USB0:
        case USB1:
            addActionGInfo(menu,spaceDoMain);
            addActionOFileSystem(menu,spaceDoMain);
            addActionSetting(menu,spaceDoMain);
            break;
        case VGA:
        case UART0:
        case UART1:
        case UART2:
        case ETH:
        case AUDIO:
        case BOOT:
            addActionGInfo(menu,spaceDoMain);
            addActionSetting(menu,spaceDoMain);
            break;
        case UNKNOW: 
        {
            QAction *pHelp= new QAction(tr("Help"), this);
            QIcon icoHelp(":/boardview/icons/help.png");
            pHelp->setIcon(icoHelp);
            menu->addAction(pHelp);
            connect(pHelp,&QAction::triggered,this,
                [&](void)
                {
                    QMessageBox::about(this, tr("Help"), 
                        tr("1. Move the mouse over the component to explore.") + "\n" +
                        tr("2. Right-click the component to view the settings.") + "\n" +
                        tr("3. Double-click the component to enter the interface.")
                    );
                }
            );
            QAction *pAbout= new QAction(tr("About"), this);
            QIcon icoAbout(":/boardview/icons/about.png");
            pAbout->setIcon(icoAbout);
            menu->addAction(pAbout);
            connect(pAbout,&QAction::triggered,this,
                [&](void)
                {
                    BoardWindow::appAbout(this);
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
            connect(pExit, SIGNAL(triggered()), this, SLOT(app_quit()));
            break;
        }
        default:
            break;
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
    painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));

    for(size_t i=1;i < (sizeof(spaceList)/sizeof(spaceList[1]));i++) {
        if(spaceList[i].draw) {
            painter.fillRect(spaceList[i].x1,spaceList[i].y1,
                spaceList[i].x2-spaceList[i].x1,spaceList[i].y2-spaceList[i].y1,QBrush(QColor(0,0,255,60)));
            QToolTip::showText(this->pos()+QPoint(spaceList[i].x1,spaceList[i].y2),spaceList[i].drawName);
        }
    }

    if(powerOn){
        QPen pen;
        pen.setWidth(10/scaled_value);
        pen.setColor(Qt::red);
        painter.setPen(pen);
        painter.drawLine(263/scaled_value,612/scaled_value,283/scaled_value,612/scaled_value);
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
                switch (spaceList[i].name)
                {
                    case LOGO:
                        static const QString colorList[5] = {"black","blue","green","red","white"};
                        static int indexColor = 0;
                        skinColor = colorList[indexColor++];
                        if(indexColor >= 5) indexColor = 0;
                        this->repaint();
                        break;
                    case VGA:
                        lcdWindow->show();
                        break;
                    case UART0:
                        uartWindow[0]->show();
                        break;
                    case UART1:
                        uartWindow[1]->show();
                        break;
                    case UART2:
                        uartWindow[2]->show();
                        break;
                    case JTAG:
                        jtagWindow->show();
                        break;
                    case SD:
                        sdImgPath = getOpenFileName(tr("Select SD IMG"), sdImgPath, "IMG files(*.img *.bin)");
                        break;
                    case NOR:
                        norFlashImgPath = getOpenFileName(tr("Select NorFlash IMG"), norFlashImgPath, "IMG files(*.img *.bin)");
                        break;
                    case NAND:
                        nandFlashImgPath = getOpenFileName(tr("Select NandFlash IMG"), nandFlashImgPath, "IMG files(*.img *.bin)");
                        break;
                    case SOC:
                        pFlashImgPath = getOpenFileName(tr("Select PFlash IMG"), pFlashImgPath, "IMG files(*.img *.bin)");
                        break;
                    case USB0:
                        usbFlashImgPath = getOpenFileName(tr("Select USBFlash IMG"), usbFlashImgPath, "IMG files(*.img *.bin)");
                        break;
                    case SWITCH:
                        powerOn = !powerOn;
                        this->repaint();
                        if(!powerSwitch(powerOn)){
                            powerOn = !powerOn;
                            this->repaint();
                        }
                        break;
                    default:
                        break;
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

void BoardWindow::app_quit(void)
{
    if(qemuProcess->state() == QProcess::Running) {
        qemuProcess->kill();
        qemuProcess->waitForFinished(-1);
    }
    qApp->quit();
}

void BoardWindow::appAbout(QWidget *parent)
{
    QMessageBox::about(parent, tr("About"),
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
