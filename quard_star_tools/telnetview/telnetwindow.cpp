#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QDebug>
#include <unistd.h>
#include "boardwindow.h"
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
#if defined(Q_OS_WIN)
    font.setFixedPitch(true);
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

    orig_font = this->termWidget->getTerminalFont();
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
    this->termWidget->recvData(buff, len);
}

void TelnetWindow::on_actionFind_triggered()
{
    this->termWidget->toggleShowSearchBar();
}


void TelnetWindow::on_actionCopy_triggered()
{
    this->termWidget->copyClipboard();
}


void TelnetWindow::on_actionPaste_triggered()
{
    this->termWidget->pasteClipboard();
}


void TelnetWindow::on_actionHelp_triggered()
{
    QMessageBox::about(this, tr("Help"), tr("TODO"));
}


void TelnetWindow::on_actionAbout_triggered()
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


void TelnetWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}


void TelnetWindow::on_actionReset_triggered()
{
    this->termWidget->clear();
}


void TelnetWindow::on_actionSave_log_triggered()
{
    QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save log..."),
        QDate::currentDate().toString("yyyy-MM-dd-") + QTime::currentTime().toString("hh:mm:ss") + ".log", tr("Log files (*.log)"));
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


void TelnetWindow::on_actionZoom_In_triggered()
{
    this->termWidget->zoomIn();
}


void TelnetWindow::on_actionZoom_Out_triggered()
{
    this->termWidget->zoomOut();
}


void TelnetWindow::on_actionReset_Zoom_triggered()
{
    this->termWidget->setTerminalFont(orig_font);
}

void TelnetWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if( event->button() == Qt::MiddleButton) {
        this->termWidget->pasteSelection();
    }
    event->accept();
}
