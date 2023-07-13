#include <QScrollBar>
#include <QDebug>
#include <unistd.h>
#include "telnetwindow.h"
#include "ui_telnetwindow.h"

TelnetWindow::TelnetWindow(const QString &addr, int port, QWidget *parent) :
    QMainWindow(parent),severAddr(addr),severPort(port),
    ui(new Ui::TelnetWindow)
{
    ui->setupUi(this);
    telnet = new QTelnet(this);
    termWidget = new QTermWidget(0,nullptr);

    ui->teOutput->addWidget(termWidget);

    QFont font = QApplication::font();
#ifdef Q_OS_MACOS
    font.setFamily(QStringLiteral("Monaco"));
#elif defined(Q_WS_QWS)
    font.setFamily(QStringLiteral("fixed"));
#else
    font.setFamily(QStringLiteral("Monospace"));
#endif
    font.setPointSize(16);
    termWidget->setTerminalFont(font);
    termWidget->setScrollBarPosition(QTermWidget::ScrollBarRight);
    termWidget->setColorScheme("WhiteOnBlack");
//#if defined(Q_OS_WIN)
//    termWidget->setKeyBindings("default");
//#elif defined(Q_OS_LINUX)
//    termWidget->setKeyBindings("linux");
//#elif defined(Q_OS_MACOS)
//    termWidget->setKeyBindings("macbook");
//#else
//    termWidget->setKeyBindings("default");
//#endif
    termWidget->setKeyBindings("linux");

    // Write what we input to remote terminal via socket
    connect(termWidget, SIGNAL(sendData(const char *,int)),this,SLOT(sendData(const char*,int)));
    // Read anything from remote terminal via socket and show it on widget.
    connect(telnet,SIGNAL(newData(const char*,int)),this,SLOT(recvData(const char*,int)));
    // Here we start an empty pty.
    termWidget->startTerminalTeletype();

    connect(ui->refreshPushbuttion, SIGNAL(clicked()), this, SLOT(refreshClicked()));
}

TelnetWindow::~TelnetWindow()
{
    delete telnet;
    delete termWidget;
    delete ui;
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

void TelnetWindow::refreshClicked()
{
    reConnect();
}

void TelnetWindow::sendData(const char *data, int len)
{
    this->telnet->sendData(data, len);
}

void TelnetWindow::recvData(const char *buff, int len)
{
    size_t w = write(this->termWidget->getPtySlaveFd(), buff, static_cast<size_t>(len));
    Q_UNUSED(w);
}
