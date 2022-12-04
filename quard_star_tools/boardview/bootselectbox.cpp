#include "bootselectbox.h"
#include "boardwindow.h"
#include "ui_bootselectbox.h"

BootSelectBox::BootSelectBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BootSelectBox)
{
    ui->setupUi(this);

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

BootSelectBox::~BootSelectBox()
{
    delete ui;
}

void BootSelectBox::showEvent(QShowEvent* event) {
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());

    if(bw->getBootCfg() == "pflash") {
        ui->pFlashRadioButton->setChecked(true);
    } else if(bw->getBootCfg() == "spi") {
        ui->norFlashRadioButton->setChecked(true);
    } else if(bw->getBootCfg() == "sd") {
        ui->sdRadioButton->setChecked(true);
    }
    ui->updateCheckBox->setChecked(bw->getUpdateCfg());

    QDialog::showEvent(event);
}

void BootSelectBox::buttonBoxAccepted(void)
{
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());

    if(ui->pFlashRadioButton->isChecked()) {
        bw->getBootCfg() = "pflash";
    } else if(ui->norFlashRadioButton->isChecked()) {
        bw->getBootCfg() = "spi";
    } else if(ui->sdRadioButton->isChecked()) {
        bw->getBootCfg() = "sd";
    } 
    bw->getUpdateCfg() = ui->updateCheckBox->isChecked();

    emit this->accepted();
}

void BootSelectBox::buttonBoxRejected(void)
{
    emit this->rejected();
}
