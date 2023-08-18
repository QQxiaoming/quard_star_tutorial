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
#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QString>
#include <QMutex>
#include <QDebug>
#include <unistd.h>
#include "boardwindow.h"
#include "telnetwindow.h"
#include "qfonticon.h"
#include "ui_telnetwindow.h"

TelnetWindow::TelnetWindow(const QString &addr, int port, QLocale::Language force_translator, QWidget *parent) :
    QMainWindow(nullptr),severAddr(addr),severPort(port),
    ui(new Ui::TelnetWindow)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("QWidget#telnetWindowWidget {background-color: transparent;}");

    telnet = new QTelnet(this);
    termWidget = new QTermWidget(0,force_translator,this);
    sendASCIIBox = new ASCIIBox(ASCIIBox::SEND,this);
    recvASCIIBox = new ASCIIBox(ASCIIBox::RECV,this);

#if defined(Q_OS_MACOS)
    this->setWindowFlags(Qt::CustomizeWindowHint | 
                            Qt::WindowTitleHint | Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
#endif
    QRect screen = QGuiApplication::screenAt(
                       this->mapToGlobal(QPoint(this->width()/2,0)))->geometry();

    QPixmap pix;
    pix.load(":/boardview/icons/terminal.png",0,
                Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    if(pix.size().width() > screen.width() || pix.size().height() > screen.height() ) {
        int target_size = qMin(screen.width(),screen.height());
        scaled_value = ((double)pix.size().width())/((double)target_size);
        pix = pix.scaled(QSize(target_size,target_size*pix.size().height()/pix.size().width()));
    }
    resize(pix.size());
    setMask(QBitmap(pix.mask()));
    QRect size = this->geometry();
    this->move(qMax(0,(screen.width() - size.width())) / 2,
               qMax(0,(screen.height() - size.height())) / 2);
    
    ui->verticalLayout->addWidget(termWidget);
    ui->verticalLayout->setContentsMargins(62/scaled_value, 60/scaled_value, 170/scaled_value,60/scaled_value);

    QFont font = QApplication::font();
#ifdef Q_OS_MACOS
    font.setFamily(QStringLiteral("Monaco"));
#elif defined(Q_WS_QWS)
    font.setFamily(QStringLiteral("fixed"));
#else
    font.setFamily(QStringLiteral("Monospace"));
#endif
#if defined(Q_OS_WIN)
    font.setFixedPitch(true);
#endif
    font.setPointSize(12);
    termWidget->setTerminalFont(font);
    termWidget->setScrollBarPosition(QTermWidget::NoScrollBar);
    
    QStringList availableColorSchemes = termWidget->availableColorSchemes();
    availableColorSchemes.sort();
    currentColorScheme = availableColorSchemes.first();
    foreach(QString colorScheme, availableColorSchemes) {
        if(colorScheme == "WhiteOnBlack") {
            termWidget->setColorScheme("WhiteOnBlack");
            currentColorScheme = "WhiteOnBlack";
        }
    }

    QStringList availableKeyBindings = termWidget->availableKeyBindings();
    availableKeyBindings.sort();
    currentAvailableKeyBindings = availableKeyBindings.first();
    foreach(QString keyBinding, availableKeyBindings) {
        if(keyBinding == "linux") {
            termWidget->setKeyBindings("linux");
            currentAvailableKeyBindings = "linux";
        }
    }

    connect(telnet,SIGNAL(newData(const char*,int)),this,SLOT(recvData(const char*,int)));
    connect(termWidget, SIGNAL(sendData(const char *,int)),this,SLOT(sendData(const char*,int)));
    connect(termWidget, SIGNAL(dupDisplayOutput(const char*,int)),this,SLOT(dupDisplayOutput(const char*,int)));
    termWidget->startTerminalTeletype();

    connect(sendASCIIBox, SIGNAL(sendData(const char *,int)),this,SLOT(sendData(const char*,int)));
    connect(recvASCIIBox, SIGNAL(hideOrClose()),this,SLOT(recvASCIIstop()));

    orig_font = this->termWidget->getTerminalFont();

    setFixedSize(this->size());
    Q_UNUSED(parent);
}

TelnetWindow::~TelnetWindow()
{
    m_save_log = false;
    m_save_Rawlog = false;
    log_file_mutex.lock();
    if(log_file != nullptr) {
        log_file->close();
        delete log_file;
        log_file = nullptr;
    }
    log_file_mutex.unlock();
    raw_log_file_mutex.lock();
    if(raw_log_file != nullptr) {
        raw_log_file->close();
        delete raw_log_file;
        raw_log_file = nullptr;
    }
    raw_log_file_mutex.unlock();
    delete telnet;
    delete termWidget;
    delete sendASCIIBox;
    delete contextMenu;
    delete ui;
}

void TelnetWindow::createContextMenu(void) 
{
    if(contextMenu) delete contextMenu;
    contextMenu = new QMenu(this);

    QMenu *menuFile = new QMenu(tr("File"),contextMenu);
    contextMenu->addMenu(menuFile);
    QMenu *menuEdit = new QMenu(tr("Edit"),contextMenu);
    contextMenu->addMenu(menuEdit);
    QMenu *menuView = new QMenu(tr("View"),contextMenu);
    contextMenu->addMenu(menuView);
    QMenu *menuTransfer = new QMenu(tr("Transfer"),contextMenu);
    contextMenu->addMenu(menuTransfer);
    QMenu *menuOption = new QMenu(tr("Options"),contextMenu);
    contextMenu->addMenu(menuOption);
    QMenu *menuHelp = new QMenu(tr("Help"),contextMenu);
    contextMenu->addMenu(menuHelp);

    QStringList availableColorSchemes = termWidget->availableColorSchemes();
    availableColorSchemes.sort();
    QMenu *menuColors = new QMenu(tr("Colors"),menuOption);
    menuOption->addMenu(menuColors);
    foreach(QString colorScheme, availableColorSchemes) {
        QAction *action = menuColors->addAction(colorScheme, menuColors,
                [=](){
                    termWidget->setColorScheme(colorScheme);
                    currentColorScheme = colorScheme;
                    foreach(QAction *action, menuColors->actions()) {
                        if(action->text() == colorScheme)
                            action->setChecked(true);
                        else {
                            action->setChecked(false);
                        }
                    }
                });
        action->setCheckable(true);
        if(colorScheme == currentColorScheme) {
            action->setChecked(true);
        }
    }

    QStringList availableKeyBindings = termWidget->availableKeyBindings();
    availableKeyBindings.sort();
    QMenu *menuKeyBindings = new QMenu(tr("KeyBindings"),menuOption);
    menuOption->addMenu(menuKeyBindings);
    foreach(QString keyBinding, availableKeyBindings) {
        QAction *action = menuKeyBindings->addAction(keyBinding, menuOption,
                [=](){
                    termWidget->setKeyBindings(keyBinding);
                    currentAvailableKeyBindings = keyBinding;
                    foreach(QAction *action, menuKeyBindings->actions()) {
                        if(action->text() == keyBinding)
                            action->setChecked(true);
                        else {
                            action->setChecked(false);
                        }
                    }
                });
        action->setCheckable(true);
        if(keyBinding == currentAvailableKeyBindings) {
            action->setChecked(true);
        }
    }
    
    QAction *pReFresh = new QAction(tr("Refresh"), menuFile);
    pReFresh->setIcon(QFontIcon::icon(QChar(0xf021)));
    menuFile->addAction(pReFresh);
    connect(pReFresh,&QAction::triggered,this,
        [&](void)
        {
            reConnect();
        }
    );

    QAction *actionSave_screen = new QAction(tr("Save screen"), menuFile);
    menuFile->addAction(actionSave_screen);
    actionSave_screen->setIcon(QFontIcon::icon(QChar(0xf0c7)));
    connect(actionSave_screen,&QAction::triggered,this,
        [&](void)
        {
            QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save log..."),
                QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".log", tr("log files (*.log)"));
            if (!savefile_name.isEmpty()) {
                QFile file(savefile_name);
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QMessageBox::warning(this, tr("Save log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(file.errorString()));
                    return;
                }
                this->termWidget->saveHistory(&file);
                file.close();
            }
        }
    );

    actionSave_log = new QAction(tr("Save log"), menuFile);
    menuFile->addAction(actionSave_log);
    actionSave_log->setCheckable(true);
    actionSave_log->setChecked(m_save_log);
    connect(actionSave_log,&QAction::triggered,this,
        [&](void)
        {
            log_file_mutex.lock();
            if(!actionSave_log->isChecked()) {
                actionSave_log->setChecked(false);
                m_save_log = false;
                if(log_file != nullptr) {
                    log_file->close();
                    delete log_file;
                    log_file = nullptr;
                }
            } else {
                QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save log..."),
                    QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".log", tr("log files (*.log)"));
                if (!savefile_name.isEmpty()) {
                    log_file = new QFile(savefile_name);
                    if (!log_file->open(QIODevice::WriteOnly|QIODevice::Text)) {
                        QMessageBox::warning(this, tr("Save log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(log_file->errorString()));
                        delete log_file;
                        log_file = nullptr;
                    } else {
                        actionSave_log->setChecked(true);
                        m_save_log = true;
                    }
                } else {
                    actionSave_log->setChecked(false);
                    m_save_log = false;
                }
            }
            log_file_mutex.unlock();
        }
    );

    actionSave_Rawlog = new QAction(tr("Save Rawlog"), menuFile);
    menuFile->addAction(actionSave_Rawlog);
    actionSave_Rawlog->setCheckable(true);
    actionSave_Rawlog->setChecked(m_save_Rawlog);
    connect(actionSave_Rawlog,&QAction::triggered,this,
        [&](void)
        {
            raw_log_file_mutex.lock();
            if(!actionSave_Rawlog->isChecked()) {
                actionSave_Rawlog->setChecked(false);
                m_save_Rawlog = false;
                if(raw_log_file != nullptr) {
                    raw_log_file->close();
                    delete raw_log_file;
                    raw_log_file = nullptr;
                }
            } else {
                QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save Raw log..."),
                    QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".bin", tr("binary files (*.bin)"));
                if (!savefile_name.isEmpty()) {
                    raw_log_file = new QFile(savefile_name);
                    if (!raw_log_file->open(QIODevice::WriteOnly)) {
                        QMessageBox::warning(this, tr("Save Raw log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(raw_log_file->errorString()));
                        delete raw_log_file;
                        raw_log_file = nullptr;
                    } else {
                        actionSave_Rawlog->setChecked(true);
                        m_save_Rawlog = true;
                    }
                } else {
                    actionSave_Rawlog->setChecked(false);
                    m_save_Rawlog = false;
                }
            }
            raw_log_file_mutex.unlock();
        }
    );

    QAction *pClose = new QAction(tr("Close"), menuFile);
    pClose->setIcon(QFontIcon::icon(QChar(0xf08b)));
    menuFile->addAction(pClose);
    connect(pClose, &QAction::triggered,this,
        [&](void)
        {
            this->hide();
        }
    );

    QAction *actionFind = new QAction(tr("Find"), menuEdit);
    menuEdit->addAction(actionFind);
    actionFind->setIcon(QFontIcon::icon(QChar(0xf002)));
    connect(actionFind,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->toggleShowSearchBar();
        }
    );

    QAction *actionCopy = new QAction(tr("Copy"), menuEdit);
    menuEdit->addAction(actionCopy);
    actionCopy->setIcon(QFontIcon::icon(QChar(0xf0c5)));
    connect(actionCopy,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->copyClipboard();
        }
    );

    QAction *actionPaste = new QAction(tr("Paste"), menuEdit);
    menuEdit->addAction(actionPaste);
    actionPaste->setIcon(QFontIcon::icon(QChar(0xf0ea)));
    connect(actionPaste,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->pasteClipboard();
        }
    );

    QAction *actionReset = new QAction(tr("Reset"), menuEdit);
    menuEdit->addAction(actionReset);
    actionReset->setIcon(QFontIcon::icon(QChar(0xf01e)));
    connect(actionReset,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->clear();
        }
    );

    QAction *actionZoom_In = new QAction(tr("Zoom In"), menuView);
    menuView->addAction(actionZoom_In);
    actionZoom_In->setIcon(QFontIcon::icon(QChar(0xf065)));
    connect(actionZoom_In,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->zoomIn();
        }
    );

    QAction *actionZoom_Out = new QAction(tr("Zoom Out"), menuView);
    menuView->addAction(actionZoom_Out);
    actionZoom_Out->setIcon(QFontIcon::icon(QChar(0xf066)));
    connect(actionZoom_Out,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->zoomOut();
        }
    );

    QAction *actionReset_Zoom = new QAction(tr("Reset Zoom"), menuView);
    menuView->addAction(actionReset_Zoom);
    actionReset_Zoom->setIcon(QFontIcon::icon(QChar(0xf057)));
    connect(actionReset_Zoom,&QAction::triggered,this,
        [&](void)
        {
            this->termWidget->setTerminalFont(orig_font);
        }
    );

    QAction *actionSendASCII = new QAction(tr("Send ASCII..."), menuTransfer);
    menuTransfer->addAction(actionSendASCII);
    connect(actionSendASCII,&QAction::triggered,this,
        [&](void)
        {
            sendASCIIBox->show();
        }
    );

    actionReceiveASCII = new QAction(tr("Receive ASCII..."), menuTransfer);
    menuTransfer->addAction(actionReceiveASCII);
    actionReceiveASCII->setCheckable(true);
    actionReceiveASCII->setChecked(m_receiveASCII);
    connect(actionReceiveASCII,&QAction::triggered,this,
        [&](void)
        {
            if(actionReceiveASCII->isChecked()) {
                m_receiveASCII = true;
                connect(telnet,SIGNAL(newData(const char*,int)),recvASCIIBox,SLOT(recvData(const char*,int)));
                recvASCIIBox->show();
            } else {
                m_receiveASCII = false;
                disconnect(telnet,SIGNAL(newData(const char*,int)),recvASCIIBox,SLOT(recvData(const char*,int)));
                recvASCIIBox->hide();
            }
        }
    );

    QAction *actionSendBinary = new QAction(tr("Send Binary..."), menuTransfer);
    menuTransfer->addAction(actionSendBinary);
    connect(actionSendBinary,&QAction::triggered,this,
        [&](void)
        {
            QString name = QFileDialog::getOpenFileName(this, tr("Open Binary..."),
                    QDir::homePath(), tr("binary files (*.bin)"));
            if (!name.isEmpty()) {
                QFileInfo fileinfo(name);
                if(fileinfo.exists()) {
                    QFile file(name);
                    file.open(QIODevice::ReadOnly);
                    do {
                        QByteArray readba = file.read(4096);
                        if(readba.isEmpty()) {
                            break;
                        }
                        sendData(readba);
                    } while(true);
                    file.close();
                } else {
                    QMessageBox::warning(this, tr("Open binary file"), tr("Cannot open file %1.").arg(name));
                }
            }
        }
    );

    QAction *actionSendXmodem = new QAction(tr("Send Xmodem..."), menuTransfer);
    menuTransfer->addAction(actionSendXmodem);
    connect(actionSendXmodem,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
        }
    );

    QAction *actionReceiveXmodem = new QAction(tr("Receive Xmodem..."), menuTransfer);
    menuTransfer->addAction(actionReceiveXmodem);
    connect(actionReceiveXmodem,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
        }
    );

    QAction *actionSendYmodem = new QAction(tr("Send Ymodem..."), menuTransfer);
    menuTransfer->addAction(actionSendYmodem);
    connect(actionSendYmodem,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
        }
    );

    QAction *actionReceiveYmodem = new QAction(tr("Receive Ymodem..."), menuTransfer);
    menuTransfer->addAction(actionReceiveYmodem);
    connect(actionReceiveYmodem,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
        }
    );

    QMenu *menuMisc = new QMenu(tr("Misc"),menuOption);
    menuOption->addMenu(menuMisc);
    QMenu *menuLog = new QMenu(tr("Log"),menuMisc);
    menuMisc->addMenu(menuLog);
    actionadd_time_on_each_line = new QAction(tr("Add timestamp on each line..."), menuLog);
    menuLog->addAction(actionadd_time_on_each_line);
    actionadd_time_on_each_line->setCheckable(true);
    actionadd_time_on_each_line->setChecked(m_add_time_on_each_line);
    connect(actionadd_time_on_each_line,&QAction::triggered,this,
        [&](void)
        {
            m_add_time_on_each_line = actionadd_time_on_each_line->isChecked();
        }
    );
    actionFflush_file = new QAction(tr("Fflush file..."), menuLog);
    menuLog->addAction(actionFflush_file);
    actionFflush_file->setCheckable(true);
    actionFflush_file->setChecked(m_fflush_file);
    connect(actionFflush_file,&QAction::triggered,this,
        [&](void)
        {
            m_fflush_file = actionFflush_file->isChecked();
        }
    );

    QAction *pHelp = new QAction(tr("Help"), menuHelp);
    pHelp->setIcon(QFontIcon::icon(QChar(0xf02d)));
    menuHelp->addAction(pHelp);
    connect(pHelp,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::about(this, tr("Help"), 
                tr("1. The central window is the terminal operation window.") + "\n" +
                tr("2. The menu bar provides portable tools and terminal configuration.") + "\n" +
                tr("3. The refresh button at the bottom is used to refresh and reconnect, which is used to connect when the simulation restarts.")
            );
        }
    );

    QAction *pAbout = new QAction(tr("About"), menuHelp);
    pAbout->setIcon(QFontIcon::icon(QChar(0xf05a)));
    menuHelp->addAction(pAbout);
    connect(pAbout,&QAction::triggered,this,
        [&](void)
        {
            BoardWindow::appAbout(this);
        }
    );
}

void TelnetWindow::reConnect(void)
{
    if( telnet->isConnected() ){
        telnet->disconnectFromHost();
    }
    telnet->connectToHost(severAddr,severPort);
}

void TelnetWindow::sendData(const QByteArray &ba)
{
    telnet->sendData(ba);
}

void TelnetWindow::sendData(const char *data, int len)
{
    this->telnet->sendData(data, len);
}

void TelnetWindow::dupDisplayOutput(const char* data,int len)
{
    if(m_save_log) {
        if(log_file_mutex.tryLock()) {
            if(log_file != nullptr) {
                if(m_add_time_on_each_line) {
                    QString lineText = QTime::currentTime().toString("hh:mm:ss - ");
                    log_file->write(lineText.toUtf8());
                }
                log_file->write(data, len);
                if(m_fflush_file) {
                    log_file->flush();
                }
            }
            log_file_mutex.unlock();
        }
    }
}

void TelnetWindow::recvData(const char *buff, int len)
{
    if(m_save_Rawlog) {
        if(raw_log_file_mutex.tryLock()) {
            if(raw_log_file != nullptr) {
                raw_log_file->write(buff, len);
                    if(m_fflush_file) {
                    raw_log_file->flush();
                }
            }
            raw_log_file_mutex.unlock();
        }
    }
    this->termWidget->recvData(buff, len);
}

void TelnetWindow::recvASCIIstop()
{
    if(m_receiveASCII) {
        m_receiveASCII = false;
        disconnect(telnet,SIGNAL(newData(const char*,int)),recvASCIIBox,SLOT(recvData(const char*,int)));
    }
}

void TelnetWindow::contextMenuEvent(QContextMenuEvent *event)
{
    createContextMenu(); 
    
    if(!contextMenu->isEmpty()) {
        contextMenu->move(cursor().pos());
        contextMenu->show();
    }

    event->accept();
}

void TelnetWindow::paintEvent(QPaintEvent *event)
{
    QString paths = ":/boardview/icons/terminal.png";
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));
    event->accept();
}

void TelnetWindow::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton) {
        isMousePressed = true;
        mStartPos = event->pos();
    }
    event->accept();
}

void TelnetWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isMousePressed)
    {
        QPoint deltaPos = event->pos() - mStartPos;
        this->move(this->pos()+deltaPos);
    }
    event->accept();
}

void TelnetWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if( event->button() == Qt::MiddleButton) {
        this->termWidget->pasteSelection();
    }
    if( event->button() == Qt::LeftButton) {
        isMousePressed = false;
    }
    event->accept();
}

