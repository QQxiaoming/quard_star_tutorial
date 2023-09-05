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
#include <QThread>
#include <QMessageBox>
#include "qfonticon.h"
#include "boardwindow.h"
#include "vncwindow.h"
#include "ui_vncwindow.h"

VncWindow::VncWindow(const QString &addr, int port, QWidget *parent)
    : QMainWindow(parent),severAddr(addr),severPort(port)
    , ui(new Ui::VncWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("QWidget#vncWindowWidget {background-color: transparent;}");

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    this->setWindowFlags(Qt::CustomizeWindowHint | 
                            Qt::WindowTitleHint | Qt::FramelessWindowHint);
#else
    this->setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
#endif

    QPixmap pix;
    pix.load(":/boardview/icons/ttf.png",0,
                Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
#if !defined(MOBILE_MODE)
    QRect screen = QGuiApplication::screenAt(
                       this->mapToGlobal(QPoint(this->width()/2,0)))->geometry();
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
#else
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    if(pix.size().width() > screen.width() || pix.size().height() > screen.height() ) {
        int target_size = qMin(screen.width(),screen.height());
        scaled_value = ((double)pix.size().width())/((double)target_size);
        pix = pix.scaled(QSize(target_size,target_size*pix.size().height()/pix.size().width()));
    }
    resize(pix.size());
    setMask(QBitmap(pix.mask()));
#endif
    vncView = new QVNCClientWidget(this);
    ui->verticalLayout->addWidget(vncView);
    ui->verticalLayout->setContentsMargins(20/scaled_value, 80/scaled_value, 20/scaled_value,100/scaled_value);
    
    setFixedSize(this->size());

#if defined(MOBILE_MODE)
    pressTimer = new QTimer(this);
    pressTimer->setInterval(500);
    pressTimer->setSingleShot(true);
#endif

    QShortcut *reConnectShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_F5), this);
    connect(reConnectShortCut, &QShortcut::activated, this, [&](void) { this->reConnect(); });
    QShortcut *hideShortCut = new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_Q), this);
    connect(hideShortCut, &QShortcut::activated, this, [&](void) { this->hide(); });

    Q_UNUSED(parent);
}

VncWindow::~VncWindow()
{
#if defined(MOBILE_MODE)
    delete pressTimer;
#endif
    delete vncView;
    delete ui;
}

void VncWindow::reConnect(void)
{
    if(vncView->isConnectedToServer()) {
        vncView->disconnectFromVncServer();
    }
    if(vncView->connectToVncServer(severAddr,"",severPort)) {
        vncView->startFrameBufferUpdate();
    }
}

void VncWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if((event->pos().x() >= vncView->pos().x()) &&
        (event->pos().x() <= vncView->pos().x()+vncView->width()) &&
        (event->pos().y() >= vncView->pos().y()) &&
        (event->pos().y() <= vncView->pos().y()+vncView->height())
        ) {
        return;
    }

    if(contextMenu) delete contextMenu;
    contextMenu = new QMenu(this); 

    QAction *pReFresh = new QAction(tr("Refresh"), contextMenu);
    pReFresh->setIcon(QFontIcon::icon(QChar(0xf021)));
    contextMenu->addAction(pReFresh);
    connect(pReFresh,&QAction::triggered,this,
        [&](void)
        {
            reConnect();
        }
    );

    QAction *pHelp = new QAction(tr("Help"), contextMenu);
    pHelp->setIcon(QFontIcon::icon(QChar(0xf02d)));
    contextMenu->addAction(pHelp);
    connect(pHelp,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::about(this, tr("Help"), 
                tr("1. The central window is the LCD emulation output window.") + "\n" +
                tr("2. The refresh button at the bottom is used to refresh and reconnect, which is used to connect when the simulation restarts.")
            );
        }
    );

    QAction *pAbout = new QAction(tr("About"), contextMenu);
    pAbout->setIcon(QFontIcon::icon(QChar(0xf05a)));
    contextMenu->addAction(pAbout);
    connect(pAbout,&QAction::triggered,this,
        [&](void)
        {
            BoardWindow::appAbout(this);
        }
    );

    QAction *pClose = new QAction(tr("Close"), contextMenu);
    pClose->setIcon(QFontIcon::icon(QChar(0xf08b)));
    contextMenu->addAction(pClose);
    connect(pClose, &QAction::triggered,this,
        [&](void)
        {
            this->hide();
        }
    );

    if(!contextMenu->isEmpty()) {
        contextMenu->move(cursor().pos());
        contextMenu->show();
    }

    event->accept();
}

void VncWindow::paintEvent(QPaintEvent *event)
{
    QString paths = ":/boardview/icons/ttf.png";
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(paths));
    event->accept();
}

void VncWindow::mousePressEvent(QMouseEvent *event)
{
    this->raise();
    if( event->button() == Qt::LeftButton) {
        isMousePressed = true;
        mStartPos = event->pos();
#if defined(MOBILE_MODE)
        pressTimer->start();
        pressPos = QCursor::pos();
#endif
    }
    event->accept();
}

void VncWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isMousePressed)
    {
        QPoint deltaPos = event->pos() - mStartPos;
        this->move(this->pos()+deltaPos);
    }
    event->accept();
}

void VncWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton) {
#if defined(MOBILE_MODE)
        if(isMousePressed && pressTimer->remainingTime() <= 0) {
            if(QCursor::pos() == pressPos) {
                QContextMenuEvent e(QContextMenuEvent::Mouse, event->pos(), QCursor::pos());
                contextMenuEvent(&e);
            }
        }
        pressTimer->stop();
#endif
        isMousePressed = false;
    }
    event->accept();
}

void VncWindow::showEvent(QShowEvent *event)
{
    this->raise();
    event->accept();
}

