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

private:
    QTelnet *telnet;
    QTermWidget *termWidget;
    QString severAddr;
    int severPort;
    Ui::TelnetWindow *ui;
};

#endif // TELNETWINDOW_H
