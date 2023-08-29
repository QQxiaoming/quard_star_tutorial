/*
 * This file is part of the https://github.com/QQxiaoming/quard_star_tutorial.git
 * project.
 *
 * Copyright (C) 2021 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
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
#include <QShortcut>

#include "qfonticon.h"
#include "qfsviewer.h"
#include "ui_boardwindow.h"
#include "boardwindow.h"

BoardWindow::BoardWindow(const QString &path,const QString &color,
                         const bool &isSysDarkTheme,
                         QLocale::Language lang,
                         QString ip_addr,
                         QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BoardWindow),
    ipAddr(ip_addr),
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
    language(lang),
    updateCfg(false)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("QWidget#boardWindowWidget {background-color: transparent;}");
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
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
#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
    if(ipAddr.isEmpty()) {
        ipAddr = QHostAddress(QHostAddress::LocalHost).toString();
        portOffset = 0;
        qemuProcess = new QProcess(this);
    } else 
#endif
    {
        portOffset = 10000;
        bool ok = false;
        ipAddr = QInputDialog::getText(this, tr("Input IP Address"),
                    tr("IP Address:"), QLineEdit::Normal, ipAddr, &ok);
        if (!ok || ipAddr.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("IP Address is empty!"));
        } else {
            QRegExp re("^((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)$");
            if(!re.exactMatch(ipAddr)) {
                QMessageBox::critical(this, tr("Error"), tr("IP Address is invalid!"));
                ipAddr = QHostAddress(QHostAddress::LocalHost).toString();
            }
        }
        qDebug() << "IP Address:" << ipAddr;
    }

    uartWindow[0] = new TelnetWindow(ipAddr,portOffset+3441,parent);
    uartWindow[0]->setWindowTitle("UART0");
    uartWindow[1] = new TelnetWindow(ipAddr,portOffset+3442,parent);
    uartWindow[1]->setWindowTitle("UART1");
    uartWindow[2] = new TelnetWindow(ipAddr,portOffset+3443,parent);
    uartWindow[2]->setWindowTitle("UART2");
    jtagWindow = new TelnetWindow(ipAddr,portOffset+3430,parent);
    jtagWindow->setWindowTitle("JTAG(Monitor)");
    lcdWindow = new VncWindow(ipAddr,portOffset+5901,parent);
    lcdWindow->setWindowTitle("LCD");
    netSelect = new NetSelectBox(this);
    bootSelect = new BootSelectBox(this);
    fsView = new FSViewWindow(nullptr);

    setFixedSize(this->size());

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon(":/boardview/icons/about.png"));
        trayIcon->setToolTip(QApplication::applicationName());
        QMenu *trayIconMenu = new QMenu(this);
        createStdMenuAction(trayIconMenu);
        connect(trayIconMenu, &QMenu::aboutToShow, this,
            [&,trayIconMenu](void)
            {
                trayIconMenu->clear();
                createStdMenuAction(trayIconMenu);
            }
        );
        trayIcon->setContextMenu(trayIconMenu);
        trayIcon->show();
    } else {
        qDebug() << "Couldn't detect any system tray on this system.";
    }

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    pressTimer = new QTimer(this);
    pressTimer->setInterval(500);
    pressTimer->setSingleShot(true);
#endif

    QShortcut *uartWindowShowShortCut0 = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_1), this);
    connect(uartWindowShowShortCut0, &QShortcut::activated, this, [&](void) { uartWindow[0]->show(); });
    QShortcut *uartWindowShowShortCut1 = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_2), this);
    connect(uartWindowShowShortCut1, &QShortcut::activated, this, [&](void) { uartWindow[1]->show(); });
    QShortcut *uartWindowShowShortCut2 = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_3), this);
    connect(uartWindowShowShortCut2, &QShortcut::activated, this, [&](void) { uartWindow[2]->show(); });
    QShortcut *jtagWindowShowShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_4), this);
    connect(jtagWindowShowShortCut, &QShortcut::activated, this, [&](void) { jtagWindow->show(); });
    QShortcut *lcdWindowShowShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_5), this);
    connect(lcdWindowShowShortCut, &QShortcut::activated, this, [&](void) { lcdWindow->show(); });
    QShortcut *runShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_R), this);
    connect(runShortCut, &QShortcut::activated, this, [&](void) { doPowerSwitch(); });
    QShortcut *helpShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_H), this);
    connect(helpShortCut, &QShortcut::activated, this, [&](void) { BoardWindow::appHelp(this); });
    QShortcut *hideShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_Q), this);
    connect(hideShortCut, &QShortcut::activated, this, [&](void) { this->hide(); });
}

BoardWindow::~BoardWindow()
{
#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
    if(qemuProcess) {
        qemuProcess->kill();
        qemuProcess->waitForFinished(-1);
        delete qemuProcess;
    }
#endif
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    delete pressTimer;
#endif
    delete trayIcon;
    delete fsView;
    delete netSelect;
    delete bootSelect;
    delete lcdWindow;
    delete uartWindow[0];
    delete uartWindow[1];
    delete uartWindow[2];
    delete jtagWindow;
    delete ui;
}

bool BoardWindow::doPowerSwitch(void)
{
    powerOn = !powerOn;
    this->repaint();
    if(!powerSwitch(powerOn)){
        powerOn = !powerOn;
        this->repaint();
        return false;
    }

    return true;
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
#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
    if(power) {
        if(qemuProcess) {
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
                    QMessageBox::critical(this, tr("Error"), tr("power up error!") + "\nexitcode: " +QString::number(exitcode));
                    return false;
                }
            }
        }
        uartWindow[0]->reConnect();
        uartWindow[1]->reConnect();
        uartWindow[2]->reConnect();
        jtagWindow->reConnect();
        lcdWindow->reConnect();
    } else {
        if(qemuProcess) {
            qemuProcess->kill();
            qemuProcess->waitForFinished(-1);
        }
    }
#else
    uartWindow[0]->reConnect();
    uartWindow[1]->reConnect();
    uartWindow[2]->reConnect();
    jtagWindow->reConnect();
    lcdWindow->reConnect();
#endif
    return true;
}

int BoardWindow::sendQemuCmd(const QString &cmd)
{
#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
    if(qemuProcess) {
        if(qemuProcess->state() == QProcess::Running) {
            jtagWindow->sendData(cmd.toUtf8());
            return 0;
        }
        return -1;
    } else {
        jtagWindow->sendData(cmd.toUtf8());
        return 0;
    }
#else
    jtagWindow->sendData(cmd.toUtf8());
    return 0;
#endif
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
    QAction *pGInfo= new QAction(tr("Get Info"), menu);
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
    QAction *pOFileSystem= new QAction(tr("Open FileSystem"), menu);
    QIcon icoOpen(QFontIcon::icon(QChar(0xf07c)));
    pOFileSystem->setIcon(icoOpen);
    pOFileSystem->setToolTip(QString::number(title));
    menu->addAction(pOFileSystem);
    connect(pOFileSystem,&QAction::triggered,this,
        [&](void)
        {
            QAction* pGInfo = qobject_cast<QAction*>(sender());
            DeviceName title = static_cast<DeviceName>(pGInfo->toolTip().toInt());
            uint64_t fatfs_offset = 0;
            uint64_t fatfs_size = 0;
            uint64_t ext4_offset = 0;
            uint64_t ext4_size = 0;
            FSViewWindow::mbr_t mbr = FSViewWindow::get_mbr(rootFSImgPath);
            if(mbr.boot_signature == FSViewWindow::MBR_BOOT_SIGNATURE) {
                for(int i = 0; i < 4; ++i){
                    if(mbr.partition_table[i].type == FSViewWindow::LIBPARTMBR_TYPE_EMPTY){
                        continue;
                    }
                    switch(mbr.partition_table[i].type) {
                        case FSViewWindow::LIBPARTMBR_TYPE_LINUX_NATIVE:
                            ext4_offset = mbr.partition_table[i].starting_lba * 512;
                            ext4_size = mbr.partition_table[i].number_of_sectors * 512;
                            break;
                        case FSViewWindow::LIBPARTMBR_TYPE_FAT32_LBA:
                            fatfs_offset = mbr.partition_table[i].starting_lba * 512;
                            fatfs_size = mbr.partition_table[i].number_of_sectors * 512;
                            break;
                        default:
                            break;
                    }
                }
            }

            switch (title)
            {
                case SOC:
                {
                    // p1: start=2048, size=196608, type=c
                    if(fatfs_size > 0 && fatfs_offset > 0) {
                        fsView->show();
                        int ret = fsView->setFatFSImgView(rootFSImgPath,fatfs_offset,fatfs_size);
                        if(ret != 0) {
                            QMessageBox::warning(this, tr("Error"), tr("Load file system failed!"));
                            fsView->hide();
                        }
                    } else {
                        QMessageBox::about(this, tr("Open FileSystem"), tr("No FileSystem, maybe is a binary image file."));
                    }
                    break;
                }
                case NAND:
                {
                    // p2: start=198656, size=8189952, type=83
                    if(ext4_size > 0 && ext4_offset > 0) {
                        fsView->show();
                        int ret = fsView->setExt4FSImgView(rootFSImgPath,ext4_offset,ext4_size);
                        if(ret != 0) {
                            QMessageBox::warning(this, tr("Error"), tr("Load file system failed!"));
                            fsView->hide();
                        }
                    } else {
                        QMessageBox::about(this, tr("Open FileSystem"), tr("No FileSystem, maybe is a binary image file."));
                    }
                    break;
                }
                case NOR:
                {
                    fsView->show();
                    int ret = fsView->setJffs2FSImgView(norFlashImgPath,0,QFileInfo(norFlashImgPath).size());
                    if(ret != 0) {
                        QMessageBox::warning(this, tr("Error"), tr("Load file system failed!"));
                        fsView->hide();
                    }
                    break;
                }
                case SD:
                case USB0:
                case USB1:
                default:
                    QMessageBox::about(this, tr("Open FileSystem"), tr("No FileSystem, maybe is a binary image file."));
                    break;
            }
        }
    );
}

void BoardWindow::addActionSetting(QMenu *menu,const DeviceName &title)
{
    QAction *pSetting= new QAction(tr("Setting"), menu);
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
                                                            
void BoardWindow::createStdMenuAction(QMenu *menu)
{
    QMenu *pTheme = new QMenu(tr("Theme"), menu); 
    QIcon icoTheme(":/boardview/icons/theme.png");
    pTheme->setIcon(icoTheme);
    menu->addMenu(pTheme);

    QAction *pLight = new QAction(tr("Light"), pTheme);
    QAction *pDark = new QAction(tr("Dark"), pTheme);

    pLight->setCheckable(true);
    pLight->setChecked(!isDarkTheme); 
    pTheme->addAction(pLight);
    connect(pLight,&QAction::triggered,this,
        [&,pLight,pDark](void)
        {
            isDarkTheme = false;
            pLight->setChecked(!isDarkTheme);
            pDark->setChecked(isDarkTheme);
            QFile ftheme(":/qdarkstyle/light/lightstyle.qss");
            if (!ftheme.exists())   {
                qDebug() << "Unable to set stylesheet, file not found!";
            } else {
                ftheme.open(QFile::ReadOnly | QFile::Text);
                QTextStream ts(&ftheme);
                qApp->setStyleSheet(ts.readAll());
            }
            QFontIcon::instance()->setColor(Qt::black);
        }
    );

    pDark->setCheckable(true);
    pDark->setChecked(isDarkTheme); 
    pTheme->addAction(pDark);
    connect(pDark,&QAction::triggered,this,
        [&,pLight,pDark](void)
        {
            isDarkTheme = true;
            pLight->setChecked(!isDarkTheme);
            pDark->setChecked(isDarkTheme);
            QFile ftheme(":/qdarkstyle/dark/darkstyle.qss");
            if (!ftheme.exists())   {
                qDebug() << "Unable to set stylesheet, file not found!";
            } else {
                ftheme.open(QFile::ReadOnly | QFile::Text);
                QTextStream ts(&ftheme);
                qApp->setStyleSheet(ts.readAll());
            }
            QFontIcon::instance()->setColor(Qt::white);
        }
    );

    QMenu *pLanguage = new QMenu(tr("Language"), menu);
    QIcon icoLanguage(":/boardview/icons/lang.png");
    pLanguage->setIcon(icoLanguage);
    menu->addMenu(pLanguage);

    QAction *pChinese = new QAction(tr("Chinese"), pLanguage);
    QAction *pEnglish = new QAction(tr("English"), pLanguage);
    QAction *pJapanese = new QAction(tr("Japanese"), pLanguage);

    pChinese->setCheckable(true);
    pChinese->setChecked(this->language == QLocale::Chinese);
    pLanguage->addAction(pChinese);
    connect(pChinese,&QAction::triggered,this,
        [&,pChinese,pEnglish,pJapanese](void)
        {
            pChinese->setChecked(true);
            pEnglish->setChecked(false);
            pJapanese->setChecked(false);
            this->language = QLocale::Chinese;
            BoardWindow::setAppLangeuage(QLocale::Chinese);
            ui->retranslateUi(this);
            uartWindow[0]->retranslateUi();
            uartWindow[1]->retranslateUi();
            uartWindow[2]->retranslateUi();
        }
    );

    pEnglish->setCheckable(true);
    pEnglish->setChecked(this->language == QLocale::English);
    pLanguage->addAction(pEnglish);
    connect(pEnglish,&QAction::triggered,this,
        [&,pChinese,pEnglish,pJapanese](void)
        {
            pChinese->setChecked(false);
            pEnglish->setChecked(true);
            pJapanese->setChecked(false);
            this->language = QLocale::English;
            BoardWindow::setAppLangeuage(QLocale::English);
            ui->retranslateUi(this);
            uartWindow[0]->retranslateUi();
            uartWindow[1]->retranslateUi();
            uartWindow[2]->retranslateUi();
        }
    );

    pJapanese->setCheckable(true);
    pJapanese->setChecked(this->language == QLocale::Japanese);
    pLanguage->addAction(pJapanese);
    connect(pJapanese,&QAction::triggered,this,
        [&,pChinese,pEnglish,pJapanese](void)
        {
            pChinese->setChecked(false);
            pEnglish->setChecked(false);
            pJapanese->setChecked(true);
            this->language = QLocale::Japanese;
            BoardWindow::setAppLangeuage(QLocale::Japanese);
            ui->retranslateUi(this);
            uartWindow[0]->retranslateUi();
            uartWindow[1]->retranslateUi();
            uartWindow[2]->retranslateUi();
        }
    );
    
    QAction *pMinimize = new QAction(tr("Hide"), menu);
    if(this->isHidden()) {
        pMinimize->setText(tr("Show"));
    } else {
        pMinimize->setText(tr("Hide"));
    }
    QIcon icoMinimize(":/boardview/icons/minimize.png");
    pMinimize->setIcon(icoMinimize);
    menu->addAction(pMinimize);
    connect(pMinimize,&QAction::triggered,this,
        [&,pMinimize](void)
        {
            if(this->isHidden()) {
                pMinimize->setText(tr("Hide"));
                this->show();
                this->activateWindow();
            } else {
                pMinimize->setText(tr("Show"));
                this->hide();
                static bool first = true;
                if(first) {
                    first = false;
                    trayIcon->showMessage(QApplication::applicationName(),tr("Hide to tray!"));
                }
            }
        }
    );

    QAction *pHelp = new QAction(tr("Help"), menu);
    QIcon icoHelp(":/boardview/icons/help.png");
    pHelp->setIcon(icoHelp);
    menu->addAction(pHelp);
    connect(pHelp,&QAction::triggered,this,
        [&](void)
        {
            BoardWindow::appHelp(this);
        }
    );

    QAction *pAbout = new QAction(tr("About"), menu);
    QIcon icoAbout(":/boardview/icons/about.png");
    pAbout->setIcon(icoAbout);
    menu->addAction(pAbout);
    connect(pAbout,&QAction::triggered,this,
        [&](void)
        {
            BoardWindow::appAbout(this);
        }
    );

    QAction *pAboutQt = new QAction(tr("About")+" Qt", menu);
    QIcon icoAboutQt(":/boardview/icons/aboutqt.png");
    pAboutQt->setIcon(icoAboutQt);
    menu->addAction(pAboutQt);
    connect(pAboutQt,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::aboutQt(this);
        }
    );

    QAction *pExit = new QAction(tr("Exit"), menu);
    QIcon icoExit(":/boardview/icons/exit.png");
    pExit->setIcon(icoExit);
    menu->addAction(pExit);
    connect(pExit, SIGNAL(triggered()), this, SLOT(app_quit()));
}

void BoardWindow::contextMenuEvent(QContextMenuEvent *event)
{
    //TODO: why this way crash?
    //QMenu *contextMenu = new QMenu(this); 
    //contextMenu->setAttribute(Qt::WA_DeleteOnClose); 
    // Now we renew menu, because use Qt::WA_DeleteOnClose can't work
    if(contextMenu) delete contextMenu;
    contextMenu = new QMenu(this); 

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
            addActionGInfo(contextMenu,spaceDoMain);
            addActionOFileSystem(contextMenu,spaceDoMain);
            break;
        case DDR:
        case JTAG:
        case POWER:
        case SWITCH:
            addActionGInfo(contextMenu,spaceDoMain);
            break;
        case USB0:
        case USB1:
            addActionGInfo(contextMenu,spaceDoMain);
            addActionOFileSystem(contextMenu,spaceDoMain);
            addActionSetting(contextMenu,spaceDoMain);
            break;
        case VGA:
        case UART0:
        case UART1:
        case UART2:
        case ETH:
        case AUDIO:
        case BOOT:
            addActionGInfo(contextMenu,spaceDoMain);
            addActionSetting(contextMenu,spaceDoMain);
            break;
        case LOGO:
        case UNKNOW:
            createStdMenuAction(contextMenu);
            break;
        default:
            break;
    }

    if(!contextMenu->isEmpty()) {
        contextMenu->move(cursor().pos());
        contextMenu->show();
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
        #if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
            QToolTip::showText(this->pos()+QPoint(spaceList[i].x1,spaceList[i].y2),spaceList[i].drawName);
        #endif
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
    this->raise();
    if( event->button() == Qt::LeftButton) {
        isMousePressed = true;
        mStartPos = event->pos();
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
        pressTimer->start();
        pressPos = QCursor::pos();
#endif
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
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
        if(isMousePressed && pressTimer->remainingTime() <= 0) {
            if(QCursor::pos() == pressPos) {
                QContextMenuEvent e(QContextMenuEvent::Mouse,event->pos());
                contextMenuEvent(&e);
            }
        }
        pressTimer->stop();
#endif
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
                        doPowerSwitch();
                        break;
                    default:
                        break;
                }
            }
        }
    }
    event->accept();
}

void BoardWindow::showEvent(QShowEvent *event)
{
    this->raise();
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
#if !(defined(Q_OS_IOS) || defined(Q_OS_ANDROID))
    if(qemuProcess) {
        if(qemuProcess->state() == QProcess::Running) {
            qemuProcess->kill();
            qemuProcess->waitForFinished(-1);
        }
    }
#endif
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

void BoardWindow::appHelp(QWidget *parent)
{
    QMessageBox::about(parent, tr("Help"), 
                tr("1. Move the mouse over the component to explore.") + "\n" +
                tr("2. Right-click the component to view the settings.") + "\n" +
                tr("3. Double-click the component to enter the interface.")
            ); 
}

void BoardWindow::setAppLangeuage(QLocale::Language lang)
{
    static QTranslator *qtTranslator = nullptr;
    static QTranslator *qtbaseTranslator = nullptr;
    static QTranslator *appTranslator = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString qlibpath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    QString qlibpath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    if(qtTranslator == nullptr) {
        qtTranslator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(qtTranslator);
        delete qtTranslator;
        qtTranslator = new QTranslator(qApp);
    }
    if(qtbaseTranslator == nullptr) {
        qtbaseTranslator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(qtbaseTranslator);
        delete qtbaseTranslator;
        qtbaseTranslator = new QTranslator(qApp);
    }
    if(appTranslator == nullptr) {
        appTranslator = new QTranslator(qApp);
    } else {
        qApp->removeTranslator(appTranslator);
        delete appTranslator;
        appTranslator = new QTranslator(qApp);
    }
    switch(lang) {
    case QLocale::Chinese:
        if(qtTranslator->load("qt_zh_CN.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_zh_CN.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quard_star_tools_zh_CN.qm"))
            qApp->installTranslator(appTranslator);
        break;
    case QLocale::Japanese:
        if(qtTranslator->load("qt_ja.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_ja.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quard_star_tools_ja_JP.qm"))
            qApp->installTranslator(appTranslator);
        break;
    default:
    case QLocale::English:
        if(qtTranslator->load("qt_en.qm",qlibpath))
            qApp->installTranslator(qtTranslator);
        if(qtbaseTranslator->load("qtbase_en.qm",qlibpath))
            qApp->installTranslator(qtbaseTranslator);
        if(appTranslator->load(":/lang/lang/quard_star_tools_en_US.qm"))
            qApp->installTranslator(appTranslator);
        break;
    }
    QTermWidget::setLangeuage(lang);
}
