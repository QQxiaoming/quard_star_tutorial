#ifndef TELNETWINDOW_H
#define TELNETWINDOW_H

#include <QMainWindow>
#include "QTelnet.h"

namespace Ui {
class TelnetWindow;
}

class TelnetWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TelnetWindow(QString addr, int port, QWidget *parent = nullptr);
    ~TelnetWindow();
    QString severaddr;
    int severport;

public slots:
    void addText(const char *msg, int count);
    void refresh_clicked();

protected:
    void keyPressEvent(QKeyEvent *event);
    
private:
    Ui::TelnetWindow *ui;
    QTelnet *telnet;

};

#endif // TELNETWINDOW_H
