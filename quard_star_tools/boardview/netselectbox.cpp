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
    ui->tapComboBox->addItem("N/A");
    ui->canComboBox->addItem("N/A");

    foreach (QNetworkInterface var, netList) {
        ui->tapComboBox->addItem(var.humanReadableName());
        ui->canComboBox->addItem(var.humanReadableName());
    }

    ui->tapComboBox->setCurrentText(bw->tap_name.isEmpty()?"N/A":bw->tap_name);
    ui->canComboBox->setCurrentText(bw->vcan_name.isEmpty()?"N/A":bw->vcan_name);

    if(bw->tap_name.isEmpty() || bw->tap_name == "N/A"){
        ui->UserRadioButton->setChecked(true);
        ui->tapComboBox->setEnabled(false);
    } else {
        ui->TapRadioButton->setChecked(true);
        ui->tapComboBox->setEnabled(true);
    }

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBox_accepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBox_rejected()));
    QObject::connect(ui->groupBox, SIGNAL(clicked()), this, SLOT(groupBox_clicked()));
    QObject::connect(ui->UserRadioButton, SIGNAL(clicked()), this, SLOT(UserRadioButton_clicked()));
    QObject::connect(ui->TapRadioButton, SIGNAL(clicked()), this, SLOT(TapRadioButton_clicked()));

    QDialog::showEvent(event);
}

void NetSelectBox::buttonBox_accepted(void)
{
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());
    bw->tap_name = ui->tapComboBox->currentText();
    bw->vcan_name = ui->canComboBox->currentText();
    if(bw->qemu_process->state() == QProcess::Running) {
        if(ui->groupBox->isChecked()){
            bw->telnet[3]->telnet->sendData(QByteArray("set_link net0 on\n",18));
        } else {
            bw->telnet[3]->telnet->sendData(QByteArray("set_link net0 off\n",19));
        }
    }
    emit this->accepted();
}

void NetSelectBox::buttonBox_rejected(void)
{
    emit this->rejected();
}

void NetSelectBox::groupBox_clicked(void)
{
    if(ui->groupBox->isChecked()){
        if(ui->UserRadioButton->isChecked()){
            ui->tapComboBox->setEnabled(false);
        }
        if(ui->TapRadioButton->isChecked()){
            ui->tapComboBox->setEnabled(true);
        }
    } else {
        ui->tapComboBox->setEnabled(false);
    }
}

void NetSelectBox::UserRadioButton_clicked(void)
{
    if(ui->UserRadioButton->isChecked()){
        ui->tapComboBox->setEnabled(false);
    }
}

void NetSelectBox::TapRadioButton_clicked(void)
{
    if(ui->TapRadioButton->isChecked()){
        ui->tapComboBox->setEnabled(true);
    }
}
