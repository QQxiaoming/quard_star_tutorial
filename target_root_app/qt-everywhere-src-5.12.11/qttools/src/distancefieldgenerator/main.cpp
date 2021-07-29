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

#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("QtProject"));
    app.setApplicationName(QStringLiteral("Qt Distance Field Generator"));
    app.setApplicationVersion(QStringLiteral(QT_VERSION_STR));

    QCommandLineParser parser;
    parser.setApplicationDescription(
                QCoreApplication::translate("main",
                                            "Allows to prepare a font cache for Qt applications."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QLatin1String("file"),
                                 QCoreApplication::translate("main",
                                                             "Font file (*.ttf, *.otf)"));
    parser.process(app);

    MainWindow mainWindow;
    if (!parser.positionalArguments().isEmpty())
        mainWindow.open(parser.positionalArguments().constFirst());
    mainWindow.show();

    return app.exec();
}
