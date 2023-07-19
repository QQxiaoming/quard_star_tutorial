#ifndef TELNETWINDOW_H
#define TELNETWINDOW_H

#include <QMainWindow>

#include "QTelnet.h"
#include "qtermwidget.h"

namespace Ui {
class TelnetWindow;
}

class TelnetWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TelnetWindow(const QString &addr, int port, QWidget *parent = nullptr);
    ~TelnetWindow();
    void reConnect(void);
    void sendData(const QByteArray &ba);

private slots:
    void refreshClicked();
    void sendData(const char *data, int len);
    void recvData(const char *buff, int len);

    void on_actionFind_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionReset_triggered();
    void on_actionSave_log_triggered();

    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionReset_Zoom_triggered();

    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QTelnet *telnet;
    QTermWidget *termWidget;
    QString severAddr;
    int severPort;
    QFont orig_font;
    Ui::TelnetWindow *ui;
};

#endif // TELNETWINDOW_H
