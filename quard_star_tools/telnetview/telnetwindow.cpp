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
    sendASCIIBox = new ASCIIBox(ASCIIBox::SEND,this);
    recvASCIIBox = new ASCIIBox(ASCIIBox::RECV,this);

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

    ui->actionReceiveASCII->setCheckable(true);
    ui->actionReceiveASCII->setChecked(false);

    connect(telnet,SIGNAL(newData(const char*,int)),this,SLOT(recvData(const char*,int)));
    connect(termWidget, SIGNAL(sendData(const char *,int)),this,SLOT(sendData(const char*,int)));
    connect(termWidget, SIGNAL(dupDisplayOutput(const char*,int)),this,SLOT(dupDisplayOutput(const char*,int)));
    termWidget->startTerminalTeletype();

    connect(sendASCIIBox, SIGNAL(sendData(const char *,int)),this,SLOT(sendData(const char*,int)));
    connect(ui->refreshPushbuttion, SIGNAL(clicked()), this, SLOT(refreshClicked()));

    orig_font = this->termWidget->getTerminalFont();
}

TelnetWindow::~TelnetWindow()
{
    ui->actionSave_log->setCheckable(false);
    ui->actionSave_Rawlog->setCheckable(false);
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

void TelnetWindow::dupDisplayOutput(const char* data,int len)
{
    if(ui->actionSave_log->isChecked()) {
        if(log_file_mutex.tryLock()) {
            if(log_file != nullptr) {
                if(ui->actionadd_time_on_each_line->isChecked()) {
                    QString lineText = QTime::currentTime().toString("hh:mm:ss - ");
                    log_file->write(lineText.toUtf8());
                }
                log_file->write(data, len);
                if(ui->actionFflush_file->isChecked()) {
                    log_file->flush();
                }
            }
            log_file_mutex.unlock();
        }
    }
}

void TelnetWindow::recvData(const char *buff, int len)
{
    if(ui->actionSave_Rawlog->isChecked()) {
        if(raw_log_file_mutex.tryLock()) {
            if(raw_log_file != nullptr) {
                raw_log_file->write(buff, len);
                    if(ui->actionFflush_file->isChecked()) {
                    raw_log_file->flush();
                }
            }
            raw_log_file_mutex.unlock();
        }
    }
    this->termWidget->recvData(buff, len);
}



void TelnetWindow::on_actionSave_screen_triggered()
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

void TelnetWindow::on_actionSave_log_triggered()
{
    log_file_mutex.lock();
    if(!ui->actionSave_log->isChecked()) {
        ui->actionSave_log->setChecked(false);
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
                ui->actionSave_log->setChecked(true);
            }
        } else {
            ui->actionSave_log->setChecked(false);
        }
    }
    log_file_mutex.unlock();
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
        QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save Raw log..."),
            QDir::homePath() + QDate::currentDate().toString("/yyyy-MM-dd-") + QTime::currentTime().toString("hh-mm-ss") + ".bin", tr("binary files (*.bin)"));
        if (!savefile_name.isEmpty()) {
            raw_log_file = new QFile(savefile_name);
            if (!raw_log_file->open(QIODevice::WriteOnly)) {
                QMessageBox::warning(this, tr("Save Raw log"), tr("Cannot write file %1:\n%2.").arg(savefile_name).arg(raw_log_file->errorString()));
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
    sendASCIIBox->show();
}

void TelnetWindow::on_actionReceiveASCII_triggered()
{
    if(ui->actionReceiveASCII->isChecked()) {
        ui->actionReceiveASCII->setChecked(true);
        connect(telnet,SIGNAL(newData(const char*,int)),recvASCIIBox,SLOT(recvData(const char*,int)));
        recvASCIIBox->show();
    } else {
        ui->actionReceiveASCII->setChecked(false);
        disconnect(telnet,SIGNAL(newData(const char*,int)),recvASCIIBox,SLOT(recvData(const char*,int)));
        recvASCIIBox->hide();
    }
}

void TelnetWindow::on_actionSendBinary_triggered()
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


