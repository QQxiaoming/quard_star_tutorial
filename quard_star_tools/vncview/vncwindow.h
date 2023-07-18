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

private slots:
    void refreshClicked();

    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

private:
    QString severAddr;
    int severPort;
    Ui::VncWindow *ui;
};
#endif // VNCWINDOW_H
