#include <QScrollBar>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#endif
#include "telnetwindow.h"
#include "ui_telnetwindow.h"

TelnetWindow::TelnetWindow(const QString &addr, int port, QWidget *parent) :
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
    //if(this->isActiveWindow()) qDebug() << data;
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
            QRegExp const escapeSequenceExpression2(R"(\x1B\[D)");
            QRegExp const escapeSequenceExpression3(R"(\x1B\[K)");
            datapool = datapool + data;
            if(datapool.length()>6){
                if(escapeSequenceExpression.indexIn(datapool) == 0){
                    data = datapool.mid(escapeSequenceExpression.matchedLength());
                    datapool.clear();
                } else if(escapeSequenceExpression1.indexIn(datapool) == 0){
                    data = datapool.mid(escapeSequenceExpression1.matchedLength());
                    datapool.clear();
                } else if(escapeSequenceExpression2.indexIn(datapool) == 0){
                    data = '\b'+datapool.mid(escapeSequenceExpression2.matchedLength());
                    datapool.clear();
                } else if(escapeSequenceExpression3.indexIn(datapool) == 0){
                    data = datapool.mid(escapeSequenceExpression3.matchedLength());
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

void TelnetWindow::reConnect(void)
{
    if( telnet->isConnected() ){
        telnet->disconnectFromHost();
    }
    telnet->connectToHost(severaddr,severport);
}

void TelnetWindow::refresh_clicked()
{
    reConnect();
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
    case Qt::Key_Minus:
        telnet->sendData(QByteArray(1,'-'));
        break;
    case Qt::Key_QuoteLeft:
        telnet->sendData(QByteArray(1,'`'));
        break;
    case Qt::Key_Equal:
        telnet->sendData(QByteArray(1,'='));
        break;
    case Qt::Key_Comma:
        telnet->sendData(QByteArray(1,','));
        break;
    case Qt::Key_Semicolon:
        telnet->sendData(QByteArray(1,';'));
        break;
    case Qt::Key_Apostrophe:
        telnet->sendData(QByteArray(1,'\''));
        break;
    case Qt::Key_BracketLeft:
        telnet->sendData(QByteArray(1,'['));
        break;
    case Qt::Key_Backslash:
        telnet->sendData(QByteArray(1,'\\'));
        break;
    case Qt::Key_BracketRight:
        telnet->sendData(QByteArray(1,']'));
        break;
    case Qt::Key_Less:
        telnet->sendData(QByteArray(1,'<'));
        break;
    case Qt::Key_Greater:
        telnet->sendData(QByteArray(1,'>'));
        break;
    case Qt::Key_Question:
        telnet->sendData(QByteArray(1,'?'));
        break;
    case Qt::Key_Colon:
        telnet->sendData(QByteArray(1,':'));
        break;
    case Qt::Key_QuoteDbl:
        telnet->sendData(QByteArray(1,'\"'));
        break;
    case Qt::Key_BraceLeft:
        telnet->sendData(QByteArray(1,'{'));
        break;
    case Qt::Key_BraceRight:
        telnet->sendData(QByteArray(1,'}'));
        break;
    case Qt::Key_Bar:
        telnet->sendData(QByteArray(1,'|'));
        break;
    case Qt::Key_AsciiTilde:
        telnet->sendData(QByteArray(1,'~'));
        break;
    case Qt::Key_Exclam:
        telnet->sendData(QByteArray(1,'!'));
        break;
    case Qt::Key_At:
        telnet->sendData(QByteArray(1,'@'));
        break;
    case Qt::Key_NumberSign:
        telnet->sendData(QByteArray(1,'#'));
        break;
    case Qt::Key_Dollar:
        telnet->sendData(QByteArray(1,'$'));
        break;
    case Qt::Key_Percent:
        telnet->sendData(QByteArray(1,'%'));
        break;
    case Qt::Key_AsciiCircum:
        telnet->sendData(QByteArray(1,'^'));
        break;
    case Qt::Key_Ampersand:
        telnet->sendData(QByteArray(1,'&'));
        break;
    case Qt::Key_Asterisk:
        telnet->sendData(QByteArray(1,'*'));
        break;
    case Qt::Key_ParenLeft:
        telnet->sendData(QByteArray(1,'('));
        break;
    case Qt::Key_ParenRight:
        telnet->sendData(QByteArray(1,')'));
        break;
    case Qt::Key_Underscore:
        telnet->sendData(QByteArray(1,'_'));
        break;
    case Qt::Key_Plus:
        telnet->sendData(QByteArray(1,'+'));
        break;
    case Qt::Key_PageDown:
        telnet->sendData(QByteArray("\e[6~",4));
        break;
    case Qt::Key_PageUp:
        telnet->sendData(QByteArray("\e[5~",4));
        break;
    case Qt::Key_End:
        telnet->sendData(QByteArray("\e[4~",4));
        break;
    case Qt::Key_Delete:
        telnet->sendData(QByteArray("\e[3~",4));
        break;
    case Qt::Key_Insert:
        telnet->sendData(QByteArray("\e[2~",4));
        break;
    case Qt::Key_Home:
        telnet->sendData(QByteArray("\e[1~",4));
        break;
    case Qt::Key_F1:
        telnet->sendData(QByteArray("\e[[A",4));
        break;
    case Qt::Key_F2:
        telnet->sendData(QByteArray("\e[[B",4));
        break;    
    case Qt::Key_F3:
        telnet->sendData(QByteArray("\e[[C",4));
        break;
    case Qt::Key_F4:
        telnet->sendData(QByteArray("\e[[D",4));
        break;    
    case Qt::Key_F5:
        telnet->sendData(QByteArray("\e[[E",4));
        break;
    case Qt::Key_F6:  
        telnet->sendData(QByteArray("\e[17~",5)); 
        break;
    case Qt::Key_F7:  
        telnet->sendData(QByteArray("\e[18~",5)); 
        break;
    case Qt::Key_F8:  
        telnet->sendData(QByteArray("\e[19~",5)); 
        break;
    case Qt::Key_F9:  
        telnet->sendData(QByteArray("\e[20~",5)); 
        break;
    case Qt::Key_F10: 
        telnet->sendData(QByteArray("\e[21~",5)); 
        break;
    case Qt::Key_F11: 
        telnet->sendData(QByteArray("\e[23~",5)); 
        break;
    case Qt::Key_F12: 
        telnet->sendData(QByteArray("\e[24~",5));
        break;
    case Qt::Key_Left:
        telnet->sendData(QByteArray("\e[D",3));
        break;
    case Qt::Key_Up :
        telnet->sendData(QByteArray("\e[A",3));
        break;
    case Qt::Key_Right:
        telnet->sendData(QByteArray("\e[C",3));
        break;
    case Qt::Key_Down:
        telnet->sendData(QByteArray("\e[B",3));
        break;
    case Qt::Key_Escape:
        telnet->sendData(QByteArray(1,'\e'));
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
