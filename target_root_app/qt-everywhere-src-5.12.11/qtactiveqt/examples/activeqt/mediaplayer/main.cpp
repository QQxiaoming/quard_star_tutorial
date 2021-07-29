/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QVariant>
#include <QSettings>
#include <QFileDialog>
#include <QCommandLineParser>

#include "ui_mainwindow.h"

static const char geometryKey[] = "Geometry";

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
    void openMedia(const QString &mediaUrl);

public slots:
    void on_mediaPlayer_PlayStateChange(int newState);
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

private:
    void updateWindowTitle(const QString &state);
    Ui::MainWindow m_ui;
};

MainWindow::MainWindow()
{
    m_ui.setupUi(this);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QByteArray restoredGeometry = settings.value(QLatin1String(geometryKey)).toByteArray();
    if (restoredGeometry.isEmpty() || !restoreGeometry(restoredGeometry)) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        const QSize size = (availableGeometry.size() * 4) / 5;
        resize(size);
        move(availableGeometry.center() - QPoint(size.width(), size.height()) / 2);
    }

    m_ui.mediaPlayer->dynamicCall("enableContextMenu", false);
    m_ui.mediaPlayer->dynamicCall("stretchToFit", true);
    updateWindowTitle("");
}

MainWindow::~MainWindow()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue(QLatin1String(geometryKey), saveGeometry());
}

void MainWindow::on_mediaPlayer_PlayStateChange(int newState)
{
    static const QHash<int, const char *> stateMapping {
        {1,  "Stopped"},
        {2,  "Paused"},
        {3,  "Playing"},
        {4,  "Scanning Forwards"},
        {5,  "Scanning Backwards"},
        {6,  "Buffering"},
        {7,  "Waiting"},
        {8,  "Media Ended"},
        {9,  "Transitioning"},
        {10, "Ready"},
        {11, "Reconnecting"},
    };
    const char *stateStr = stateMapping.value(newState, "");
    updateWindowTitle(tr(stateStr));
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this, tr("Open File"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters({ "application/octet-stream", "video/x-msvideo", "video/mp4", "audio/mpeg", "audio/mp4" });
    if (fileDialog.exec() == QDialog::Accepted)
        openMedia(fileDialog.selectedFiles().first());
}

void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Media Player"),
                tr("This Example has been created using the ActiveQt integration into Qt Designer.\n"
                   "It demonstrates the use of QAxWidget to embed the Windows Media Player ActiveX\n"
                   "control into a Qt application."));
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::openMedia(const QString &mediaUrl)
{
    if (!mediaUrl.isEmpty())
        m_ui.mediaPlayer->dynamicCall("URL", mediaUrl);
}

void MainWindow::updateWindowTitle(const QString &state)
{
    QString appName = QCoreApplication::applicationName();
    QString title = state.isEmpty() ? appName :
                    QString("%1 (%2)").arg(appName, state);
    setWindowTitle(title);
}

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCoreApplication::setApplicationName(QLatin1String("Active Qt Media Player"));
    QCoreApplication::setOrganizationName(QLatin1String("QtProject"));

    MainWindow w;
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The media file to open.");
    parser.process(app);
    if (!parser.positionalArguments().isEmpty())
        w.openMedia(parser.positionalArguments().constFirst());
    w.show();
    return app.exec();
}
