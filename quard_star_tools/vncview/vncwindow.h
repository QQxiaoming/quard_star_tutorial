#ifndef VNCWINDOW_H
#define VNCWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class VncWindow; }
QT_END_NAMESPACE

class VncWindow : public QMainWindow
{
    Q_OBJECT

public:
    VncWindow(const QString &addr, int port, QWidget *parent = nullptr);
    ~VncWindow();
    void reConnect(void);
    QString severaddr;
    int severport;

private slots:
    void refresh_clicked();

private:
    Ui::VncWindow *ui;
};
#endif // VNCWINDOW_H
