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
    : QMainWindow(nullptr),severAddr(addr),severPort(port)
    , ui(new Ui::VncWindow)
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
    pix.load(":/boardview/icons/ttf.png",0,
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
    
    vncView = new QVNCClientWidget(this);
    ui->verticalLayout->addWidget(vncView);
    ui->verticalLayout->setContentsMargins(20/scaled_value, 80/scaled_value, 20/scaled_value,100/scaled_value);
    
    menu = new QMenu(this);

    QAction *pReFresh = new QAction(tr("Refresh"), this);
    pReFresh->setIcon(QFontIcon::icon(QChar(0xf021)));
    menu->addAction(pReFresh);
    connect(pReFresh,&QAction::triggered,this,
        [&](void)
        {
            reConnect();
        }
    );

    QAction *pHelp = new QAction(tr("Help"), this);
    pHelp->setIcon(QFontIcon::icon(QChar(0xf02d)));
    menu->addAction(pHelp);
    connect(pHelp,&QAction::triggered,this,
        [&](void)
        {
            QMessageBox::about(this, tr("Help"), 
                tr("1. The central window is the LCD emulation output window.") + "\n" +
                tr("2. The refresh button at the bottom is used to refresh and reconnect, which is used to connect when the simulation restarts.")
            );
        }
    );

    QAction *pAbout = new QAction(tr("About"), this);
    pAbout->setIcon(QFontIcon::icon(QChar(0xf05a)));
    menu->addAction(pAbout);
    connect(pAbout,&QAction::triggered,this,
        [&](void)
        {
            BoardWindow::appAbout(this);
        }
    );

    QAction *pClose = new QAction(tr("Close"), this);
    pClose->setIcon(QFontIcon::icon(QChar(0xf08b)));
    menu->addAction(pClose);
    connect(pClose, &QAction::triggered,this,
        [&](void)
        {
            this->hide();
        }
    );

    setFixedSize(this->size());
    Q_UNUSED(parent);
}

VncWindow::~VncWindow()
{
    delete menu;
    delete vncView;
    delete ui;
}

void VncWindow::reConnect(void)
{
    if(vncView->isConnectedToServer()) {
        vncView->disconnectFromVncServer();
    }
    if(vncView->connectToVncServer("127.0.0.1","",5901)) {
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

    if(!menu->isEmpty()) {
        menu->move(cursor().pos());
        menu->show();
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
    if( event->button() == Qt::LeftButton) {
        isMousePressed = true;
        mStartPos = event->pos();
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
        isMousePressed = false;
    }
    event->accept();
}
