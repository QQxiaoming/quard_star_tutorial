#ifndef NETSELECTBOX_H
#define NETSELECTBOX_H

#include <QDialog>

namespace Ui {
class NetSelectBox;
}

class NetSelectBox : public QDialog
{
    Q_OBJECT

public:
    explicit NetSelectBox(QWidget *parent = nullptr);
    ~NetSelectBox();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void buttonBox_accepted(void);
    void buttonBox_rejected(void);

private:
    Ui::NetSelectBox *ui;
};

#endif // NETSELECTBOX_H
