#include <QMessageBox>
#include "asciibox.h"
#include "ui_asciibox.h"

ASCIIBox::ASCIIBox(int type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ASCIIBox),
    m_type(type)
{
    ui->setupUi(this);

    QFont font = QApplication::font();
    #ifdef Q_OS_MACOS
        font.setFamily(QStringLiteral("Monaco"));
    #elif defined(Q_WS_QWS)
        font.setFamily(QStringLiteral("fixed"));
    #else
        font.setFamily(QStringLiteral("Monospace"));
    #endif
    #if defined(Q_OS_WIN)
        font.setFixedPitch(true);
    #endif
        font.setPointSize(12);
        ui->textEdit->setFont(font);
        QPalette p = ui->textEdit->palette();
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Base, Qt::black);
        ui->textEdit->setPalette(p);

    if(type == SEND) {
        setWindowTitle(tr("Send ASCII Text..."));
        ui->textEdit->setReadOnly(false);
    } else if(type == RECV){
        setWindowTitle(tr("Recv ASCII Text..."));
        ui->textEdit->setReadOnly(true);
        ui->buttonBox->setEnabled(false);
        ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    }

    QObject::connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBoxAccepted()));
    QObject::connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBoxRejected()));
}

ASCIIBox::~ASCIIBox()
{
    delete ui;
}

void ASCIIBox::buttonBoxAccepted(void)
{
    if(m_type == SEND) {
        QString input = ui->textEdit->toPlainText();
        QByteArray array = QByteArray::fromHex(input.toLatin1());
        if(!array.isEmpty()) {
            QMessageBox::information(this, tr("Information"), tr("Will send Hex:\n")+"0x"+array.toHex(' ').replace(" "," 0x"));
            emit sendData(array.constData(),array.size());
            ui->textEdit->setPlainText(array.toHex(' '));
        }
    }
    emit this->accepted();
}

void ASCIIBox::buttonBoxRejected(void)
{
    emit this->rejected();
}

void ASCIIBox::recvData(const char *data,int size)
{
    if(size > 0) {
        QByteArray ba(data,size);
        ui->textEdit->insertPlainText("0x" + ba.toHex(' ').replace(" "," 0x") + " ");
        ui->textEdit->ensureCursorVisible();
    }
}

void ASCIIBox::hideEvent(QHideEvent *event)
{
    emit hideOrClose();
    Q_UNUSED(event);
}

void ASCIIBox::on_pushButton_clicked()
{
    ui->textEdit->clear();
}

