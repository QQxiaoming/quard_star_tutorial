#ifndef ASCIIBOX_H
#define ASCIIBOX_H

#include <QDialog>

namespace Ui {
class ASCIIBox;
}

class ASCIIBox : public QDialog
{
    Q_OBJECT

public:
    explicit ASCIIBox(int type, QWidget *parent = nullptr);
    ~ASCIIBox();

    enum {
        SEND = 0,
        RECV = 1,
    };

signals:
    void sendData(const char *,int);

public slots:
    void recvData(const char *data,int size);

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);
    void on_pushButton_clicked();

private:
    Ui::ASCIIBox *ui;
    int m_type;
};

#endif // ASCIIBOX_H
