#ifndef TELNETWINDOW_H
#define TELNETWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QMutex>
#include <QMenu>

#include "QTelnet.h"
#include "qtermwidget.h"
#include "asciibox.h"

namespace Ui {
class TelnetWindow;
}

class TelnetWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TelnetWindow(const QString &addr, int port,
                          QLocale::Language force_translator = QLocale::AnyLanguage,
                          QWidget *parent = nullptr);
    ~TelnetWindow();
    void reConnect(void);
    void sendData(const QByteArray &ba);

private slots:
    void sendData(const char *data, int len);
    void recvData(const char *buff, int len);
    void dupDisplayOutput(const char* data,int len);
    void recvASCIIstop();

private:
    void on_actionSave_screen_triggered();
    void on_actionSave_log_triggered();
    void on_actionSave_Rawlog_triggered();

    void on_actionFind_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionReset_triggered();

    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionReset_Zoom_triggered();

    void on_actionSendASCII_triggered();
    void on_actionReceiveASCII_triggered();
    void on_actionSendBinary_triggered();
    void on_actionSendXmodem_triggered();
    void on_actionReceiveXmodem_triggered();
    void on_actionSendYmodem_triggered();
    void on_actionReceiveYmodem_triggered();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    bool isMousePressed = false;
    QPoint mStartPos;
    double scaled_value = 1.0;
    QTelnet *telnet;
    QTermWidget *termWidget;
    ASCIIBox *sendASCIIBox;
    ASCIIBox *recvASCIIBox;
    QString severAddr;
    int severPort;
    QFont orig_font;
    QFile *raw_log_file = nullptr;
    QMutex raw_log_file_mutex;
    QFile *log_file = nullptr;
    QMutex log_file_mutex;
    QMenu *menu;
    QAction *actionSave_log;
    QAction *actionSave_Rawlog;
    QAction *actionReceiveASCII;
    QAction *actionadd_time_on_each_line;
    QAction *actionFflush_file;
    Ui::TelnetWindow *ui;
};

#endif // TELNETWINDOW_H
