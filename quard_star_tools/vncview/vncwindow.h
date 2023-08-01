#ifndef VNCWINDOW_H
#define VNCWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QVBoxLayout>

#include <qvncclientwidget.h>

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

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    
private:
    bool isMousePressed = false;
    QString severAddr;
    QPoint mStartPos;
    int severPort;
    QVNCClientWidget *vncView;
    QMenu *menu;
    double scaled_value = 1.0;
    Ui::VncWindow *ui;
};
#endif // VNCWINDOW_H
