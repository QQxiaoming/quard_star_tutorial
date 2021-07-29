/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QUrl>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("QQuickWidget Viewer");
    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", "The URL to open.");
    parser.process(app);

    QUrl url;
    if (parser.positionalArguments().isEmpty())  {
        QFileDialog fileDialog(nullptr, "Select QML File");
        fileDialog.setMimeTypeFilters(QStringList("text/x-qml"));
        if (fileDialog.exec() != QDialog::Accepted)
            return 0;
        url = fileDialog.selectedUrls().constFirst();
    } else {
        url = QUrl::fromUserInput(parser.positionalArguments().constFirst(),
                                  QDir::currentPath(), QUrl::AssumeLocalFile);
        if (!url.isValid()) {
            std::cerr << qPrintable(url.errorString()) << '\n';
            return -1;
        }
    }

    QQuickWidget w(url);
    w.setAttribute(Qt::WA_AcceptTouchEvents);
    if (w.status() == QQuickWidget::Error)
        return -1;
    QObject::connect(w.engine(), &QQmlEngine::quit, &app, &QCoreApplication::quit);
    w.show();

    std::cout << "Qt " << QT_VERSION_STR << ' ' << qPrintable(app.platformName());
    if (QOpenGLContext *openglContext = w.quickWindow()->openglContext()) {
        QOpenGLFunctions *glFunctions = openglContext->functions();
        std::cout << " OpenGL \"" << glFunctions->glGetString(GL_RENDERER)
            << "\" \"" << glFunctions->glGetString(GL_VERSION) << '"';
    }
    const qreal devicePixelRatio = w.devicePixelRatioF();
    if (!qFuzzyCompare(devicePixelRatio, qreal(1)))
        std::cout << ", DPR=" << devicePixelRatio;
    std::cout << '\n';

    return app.exec();

}
