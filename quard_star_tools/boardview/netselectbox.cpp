#include "netselectbox.h"
#include "boardwindow.h"
#include "ui_netselectbox.h"
#include <QNetworkInterface>

NetSelectBox::NetSelectBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetSelectBox)
{
    ui->setupUi(this);
}

NetSelectBox::~NetSelectBox()
{
    delete ui;
}

void NetSelectBox::showEvent(QShowEvent* event) {
    QNetworkInterface interface;
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());
    QList<QNetworkInterface> netList = interface.allInterfaces();

    ui->tapComboBox->clear();
    ui->canComboBox->clear();
    ui->tapComboBox->addItem("");
    ui->canComboBox->addItem("");

    foreach (QNetworkInterface var, netList) {
        ui->tapComboBox->addItem(var.humanReadableName());
        ui->canComboBox->addItem(var.humanReadableName());
    }

    ui->tapComboBox->setCurrentText(bw->tap_name);
    ui->canComboBox->setCurrentText(bw->vcan_name);

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBox_accepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBox_rejected()));

    QDialog::showEvent(event);
}

void NetSelectBox::buttonBox_accepted(void)
{
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());
    bw->tap_name = ui->tapComboBox->currentText();
    bw->vcan_name = ui->canComboBox->currentText();
    emit this->accepted();
}

void NetSelectBox::buttonBox_rejected(void)
{
    emit this->rejected();
}
