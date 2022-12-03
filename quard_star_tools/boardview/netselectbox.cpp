#include "netselectbox.h"
#include "boardwindow.h"
#include "ui_netselectbox.h"
#include <QNetworkInterface>

NetSelectBox::NetSelectBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetSelectBox)
{
    ui->setupUi(this);

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBox_accepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBox_rejected()));
    QObject::connect(ui->groupBox, SIGNAL(clicked()), this, SLOT(groupBox_clicked()));
    QObject::connect(ui->UserRadioButton, SIGNAL(clicked()), this, SLOT(UserRadioButton_clicked()));
    QObject::connect(ui->TapRadioButton, SIGNAL(clicked()), this, SLOT(TapRadioButton_clicked()));
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

    ui->tapComboBox->setCurrentText(bw->getTapName().isEmpty()?"N/A":bw->getTapName());
    ui->canComboBox->setCurrentText(bw->getVCanName().isEmpty()?"N/A":bw->getVCanName());

    if(bw->getTapName().isEmpty()){
        ui->UserRadioButton->setChecked(true);
        ui->tapComboBox->setEnabled(false);
    } else {
        ui->TapRadioButton->setChecked(true);
        ui->tapComboBox->setEnabled(true);
    }
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    ui->canComboBox->setEnabled(false);
#elif defined(Q_OS_LINUX)
    ui->canComboBox->setEnabled(true);
#endif

    QDialog::showEvent(event);
}

void NetSelectBox::buttonBox_accepted(void)
{
    BoardWindow *bw = static_cast<BoardWindow *>(this->parent());
    bw->getTapName() = ui->tapComboBox->currentText() == "N/A"?"":ui->tapComboBox->currentText();
    bw->getVCanName() = ui->canComboBox->currentText() == "N/A"?"":ui->canComboBox->currentText();
    if(ui->groupBox->isChecked()) {
        bw->sendQemuCmd("set_link net0 on\n");
    } else {
        bw->sendQemuCmd("set_link net0 off\n");
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
        ui->tapComboBox->setCurrentText("N/A");
        ui->tapComboBox->setEnabled(false);
    }
}

void NetSelectBox::TapRadioButton_clicked(void)
{
    if(ui->TapRadioButton->isChecked()){
        ui->tapComboBox->setEnabled(true);
    }
}
