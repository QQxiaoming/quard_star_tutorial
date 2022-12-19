#ifndef BOOTSELECTBOX_H
#define BOOTSELECTBOX_H

#include <QDialog>

namespace Ui {
class BootSelectBox;
}

class BootSelectBox : public QDialog
{
    Q_OBJECT

public:
    explicit BootSelectBox(QWidget *parent = nullptr);
    ~BootSelectBox();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void buttonBoxAccepted(void);
    void buttonBoxRejected(void);

private:
    Ui::BootSelectBox *ui;
};

#endif // BOOTSELECTBOX_H
