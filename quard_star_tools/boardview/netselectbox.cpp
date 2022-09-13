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
    BoardWindow *bw = (BoardWindow *)(this->parent());
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
    QDialog::showEvent(event);
}

void NetSelectBox::on_buttonBox_accepted()
{
    BoardWindow *bw = (BoardWindow *)(this->parent());
    bw->tap_name = ui->tapComboBox->currentText();
    bw->vcan_name = ui->canComboBox->currentText();
    this->accepted();
}

void NetSelectBox::on_buttonBox_rejected()
{
    this->rejected();
}
