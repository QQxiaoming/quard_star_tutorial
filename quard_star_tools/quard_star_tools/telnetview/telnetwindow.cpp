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
    ui->teOutput->setFocusPolicy(Qt::NoFocus);
    ui->refresh_pushbuttion->setFocusPolicy(Qt::NoFocus);

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

void TelnetWindow::insertPlainText(QByteArray data)
{
    for (int i = 0; i < data.size(); ++i) {
        switch (data.at(i)) {
        case 0x07:
            break;
        case '\b':
            ui->teOutput->textCursor().deletePreviousChar();
            break;
        default:
            ui->teOutput->insertPlainText(QByteArray(1,data.at(i)));
            break;
        }
    }
    ui->teOutput->verticalScrollBar()->setValue(0xFFFFFFF);
}

void TelnetWindow::addText(const char *msg, int count)
{
    QByteArray data = QByteArray(msg, count);

    do {
        if(datapool.isEmpty()){
            int offset = data.indexOf('\x1B', 0);
            if(offset >= 0) {
                insertPlainText(data.left(offset));
                datapool = datapool + data.mid(offset);
                data.clear();
            } else {
                insertPlainText(data);
                break;
            }
        } else {
            QRegExp const escapeSequenceExpression(R"(\x1B\[([\d;]+)m)");
            QRegExp const escapeSequenceExpression1(R"(\x1B\[m)");
            datapool = datapool + data;
            if(datapool.length()>6){
                if(escapeSequenceExpression.indexIn(datapool) == 0){
                    QByteArray cut = datapool.left(escapeSequenceExpression.matchedLength());
                    //qDebug() << cut;
                    data = datapool.mid(escapeSequenceExpression.matchedLength());
                    datapool.clear();
                } else if(escapeSequenceExpression1.indexIn(datapool) == 0){
                    QByteArray cut = datapool.left(escapeSequenceExpression1.matchedLength());
                    //qDebug() << cut;
                    data = datapool.mid(escapeSequenceExpression1.matchedLength());
                    datapool.clear();
                } else {
                    insertPlainText(datapool.left(1));
                    data = datapool.mid(2);
                    datapool.clear();
                }
            } else {
                break;
            }
        }
    }while(1);
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
    case Qt::Key_Tab:
        telnet->sendData(QByteArray(1,'\t'));
        break;
    case Qt::Key_Period:
        telnet->sendData(QByteArray(1,'.'));
        break;
    case Qt::Key_Slash:
        telnet->sendData(QByteArray(1,'/'));
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
