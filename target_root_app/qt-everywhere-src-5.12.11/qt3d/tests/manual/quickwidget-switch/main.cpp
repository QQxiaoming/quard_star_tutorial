/****************************************************************************
**
** Copyright (C) 2020 The Qt Company.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include <QDesktopWidget>
#include <QScreen>
#include <QApplication>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QPushButton>

void configureMainWindow(QMainWindow *w, QMdiArea *mdiArea, QPushButton *button)
{
    auto widget = new QWidget;
    auto layout = new QVBoxLayout;
    layout->addWidget(mdiArea);
    layout->addWidget(button);
    widget->setLayout(layout);
    w->setCentralWidget(widget);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow w1;
    auto mdiArea1 = new QMdiArea;
    auto button1 = new QPushButton("Switch to this window");
    configureMainWindow(&w1, mdiArea1, button1);
    w1.setGeometry(QGuiApplication::screens().at(0)->geometry());
    w1.show();

    QMainWindow w2;
    auto mdiArea2 = new QMdiArea;
    auto button2 = new QPushButton("Switch to this window");
    configureMainWindow(&w2, mdiArea2, button2);
    w2.setGeometry(QGuiApplication::screens().at(1)->geometry());
    w2.show();

    QMdiSubWindow* subWindow = new QMdiSubWindow();

    QQuickWidget *quickWidget = new QQuickWidget();
    quickWidget->resize(QSize(400, 400));
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl("qrc:/main.qml"));

    subWindow->setWidget(quickWidget);

    QObject::connect(button1, &QPushButton::clicked,
                     [mdiArea1, mdiArea2, subWindow, button1, button2]() {
        mdiArea2->removeSubWindow(subWindow);
        mdiArea1->addSubWindow(subWindow);
        subWindow->show();
        button1->setEnabled(false);
        button2->setEnabled(true);
    });

    QObject::connect(button2, &QPushButton::clicked,
                     [mdiArea1, mdiArea2, subWindow, button1, button2]() {
        mdiArea1->removeSubWindow(subWindow);
        mdiArea2->addSubWindow(subWindow);
        subWindow->show();
        button1->setEnabled(true);
        button2->setEnabled(false);
    });

    mdiArea2->addSubWindow(subWindow);
    button2->setEnabled(false);
    subWindow->show();

    return app.exec();
}
