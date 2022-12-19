#ifndef TELNETWINDOW_H
#define TELNETWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>

#include "QTelnet.h"

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
    void addText(const char *msg, int count);
    void refreshClicked();

protected:
    void keyPressEvent(QKeyEvent *event);
    
private:
    void insertPlainText(QByteArray data);
    QTelnet *telnet;
    QByteArray dataPool;
    QString severAddr;
    int severPort;
    Ui::TelnetWindow *ui;
};

#endif // TELNETWINDOW_H
