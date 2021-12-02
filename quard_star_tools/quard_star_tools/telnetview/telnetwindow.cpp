#include <QScrollBar>

#include "telnetwindow.h"
#include "ui_telnetwindow.h"

TelnetWindow::TelnetWindow(QString addr, int port, QWidget *parent) :
    QMainWindow(parent),severaddr(addr),severport(port),
    ui(new Ui::TelnetWindow)
{
    ui->setupUi(this);

    QPalette p = ui->teOutput->palette();
    p.setColor(QPalette::Active, QPalette::Base, Qt::black);
    p.setColor(QPalette::Inactive, QPalette::Base, Qt::black);
    p.setColor(QPalette::Active, QPalette::Text, Qt::white);
    p.setColor(QPalette::Inactive, QPalette::Text, Qt::white);
    ui->teOutput->setPalette(p);
    ui->teOutput->setTextInteractionFlags(Qt::NoTextInteraction);

    telnet = new QTelnet(this);
   	
    connect(telnet, SIGNAL(newData(const char*,int)), this, SLOT(addText(const char*,int)) );
    connect(ui->refresh_pushbuttion, SIGNAL(clicked()), this, SLOT(refresh_clicked()));

    telnet->connectToHost(severaddr,severport);
}

TelnetWindow::~TelnetWindow()
{
    delete telnet;
    delete ui;
}

void TelnetWindow::addText(const char *msg, int count)
{
	ui->teOutput->insertPlainText( QByteArray(msg, count) );
    ui->teOutput->verticalScrollBar()->setValue(0xFFFFFFF);
}

void TelnetWindow::refresh_clicked()
{
    if( telnet->isConnected() ){
        telnet->disconnectFromHost();
    }
    telnet->connectToHost(severaddr,severport);
}

void TelnetWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()){
    case Qt::Key_Return:
        telnet->sendData(QByteArray(1,'\n'));
        break;
    case Qt::Key_Space:
        telnet->sendData(QByteArray(1,' '));
        break;
    case Qt::Key_Backspace:
        telnet->sendData(QByteArray(1,'\b'));
        break;
    case 0x30 ... 0x39:
        telnet->sendData(QByteArray(1,static_cast<char>(event->key())));
        break;
    case 0x41 ... 0x5a:
        telnet->sendData(QByteArray(1,static_cast<char>(event->key()+0x20)));
        break;
    default:
        qDebug("undefine key 0x%x",event->key());
        break;
    }

    event->accept();
}
