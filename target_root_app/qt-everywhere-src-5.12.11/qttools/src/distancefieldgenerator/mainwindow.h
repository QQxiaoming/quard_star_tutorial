/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

class QLabel;
class QProgressBar;
class DistanceFieldModel;
class QListWidgetItem;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void open(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFont();
    void startProgressBar(quint16 glyphCount);
    void stopProgressBar();
    void updateProgressBar();
    void selectAll();
    void updateSelection();
    void updateUnicodeRanges();
    void populateUnicodeRanges();
    void save();
    void saveAs();
    void displayError(const QString &errorString);
    void selectString();
    void about();

private:
    void setupConnections();
    void writeFile();
    QByteArray createSfntTable();

    Ui::MainWindow *ui;
    QString m_fontDir;
    QString m_fontFile;
    QSettings m_settings;
    DistanceFieldModel *m_model;
    QLabel *m_statusBarLabel;
    QProgressBar *m_statusBarProgressBar;
    QString m_fileName;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H
