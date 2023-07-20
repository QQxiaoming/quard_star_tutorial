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
    QStringList availableColorSchemes = termWidget->availableColorSchemes();
    availableColorSchemes.sort();
    foreach(QString colorScheme, availableColorSchemes) {
        QAction *action = ui->menuColors->addAction(colorScheme, this, 
                [=](){
                    termWidget->setColorScheme(colorScheme);
                    foreach(QAction *action, ui->menuColors->actions()) {
                        if(action->text() == colorScheme)
                            action->setChecked(true);
                        else {
                            action->setChecked(false);
                        }
                    }
                });
        action->setCheckable(true);
        if(colorScheme == "WhiteOnBlack") {
            termWidget->setColorScheme("WhiteOnBlack");
            action->setChecked(true);
        }
    }

    QStringList availableKeyBindings = termWidget->availableKeyBindings();
    availableKeyBindings.sort();
    foreach(QString keyBinding, availableKeyBindings) {
        QAction *action = ui->menuKeyBindings->addAction(keyBinding, this, 
                [=](){
                    termWidget->setKeyBindings(keyBinding);
                    foreach(QAction *action, ui->menuKeyBindings->actions()) {
                        if(action->text() == keyBinding)
                            action->setChecked(true);
                        else {
                            action->setChecked(false);
                        }
                    }
                });
        action->setCheckable(true);
        if(keyBinding == "linux") {
            termWidget->setKeyBindings("linux");
            action->setChecked(true);
        }
    }

    ui->actionSave_log->setCheckable(true);
    ui->actionSave_log->setChecked(false);
    ui->actionSave_Rawlog->setCheckable(true);
    ui->actionSave_Rawlog->setChecked(false);

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
    if(ui->actionSave_Rawlog->isChecked()) {
        if(raw_log_file_mutex.tryLock()) {
            if(raw_log_file != nullptr) {
                raw_log_file->write(buff, len);
            }
            raw_log_file_mutex.unlock();
        }
    }
    if(ui->actionSave_log->isChecked()) {
        if(log_file_mutex.tryLock()) {
            log_file_mutex.unlock();
        }
    }
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

void TelnetWindow::on_actionSave_Rawlog_triggered()
{
    raw_log_file_mutex.lock();
    if(!ui->actionSave_Rawlog->isChecked()) {
        ui->actionSave_Rawlog->setChecked(false);
        if(raw_log_file != nullptr) {
            raw_log_file->close();
            delete raw_log_file;
            raw_log_file = nullptr;
        }
    } else {
        QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save log..."),
            QDate::currentDate().toString("yyyy-MM-dd-") + QTime::currentTime().toString("hh:mm:ss") + ".bin", tr("binary files (*.bin)"));
        if (!savefile_name.isEmpty()) {
            raw_log_file = new QFile(savefile_name);
            if (!raw_log_file->open(QIODevice::WriteOnly)) {
                QMessageBox::warning(this, tr("Save log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(raw_log_file->errorString()));
                delete raw_log_file;
                raw_log_file = nullptr;
            } else {
                ui->actionSave_Rawlog->setChecked(true);
            }
        } else {
            ui->actionSave_Rawlog->setChecked(false);
        }
    }
    raw_log_file_mutex.unlock();
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

void TelnetWindow::on_actionSendASCII_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionReceiveASCII_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionSendBinary_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionSendXmodem_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionReceiveXmodem_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionSendYmodem_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionReceiveYmodem_triggered()
{
    //TODO:
    QMessageBox::information(this, tr("Information"), tr("This feature is not ready yet, so stay tuned!"));
}

void TelnetWindow::on_actionHelp_triggered()
{
    QMessageBox::about(this, tr("Help"), 
        tr("1. The central window is the terminal operation window.") + "\n" +
        tr("2. The menu bar provides portable tools and terminal configuration.") + "\n" +
        tr("3. The refresh button at the bottom is used to refresh and reconnect, which is used to connect when the simulation restarts.")
    );
}

void TelnetWindow::on_actionAbout_triggered()
{
    BoardWindow::appAbout(this);
}

void TelnetWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}
