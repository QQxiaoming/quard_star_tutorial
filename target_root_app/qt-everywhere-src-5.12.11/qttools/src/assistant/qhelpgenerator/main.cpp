/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "../shared/collectionconfiguration.h"
#include "helpgenerator.h"
#include "collectionconfigreader.h"
#include "qhelpprojectdata_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QTranslator>

#include <QtGui/QGuiApplication>

#include <QtHelp/QHelpEngineCore>


QT_USE_NAMESPACE

class QHG {
    Q_DECLARE_TR_FUNCTIONS(QHelpGenerator)
};

static const char QHP[]  = "qhp";
static const char QCH[]  = "qch";

static const char QHCP[] = "qhcp";
static const char QHC[]  = "qhc";

namespace {
    QString absoluteFilePath(const QString &basePath, const QString &fileName)
    {
        return QDir(basePath).absoluteFilePath(fileName);
    }
}

int generateCollectionFile(const QByteArray &data, const QString &basePath, const QString outputFile)
{
    fputs(qPrintable(QHG::tr("Reading collection config file...\n")), stdout);
    CollectionConfigReader config;
    config.readData(data);
    if (config.hasError()) {
        fputs(qPrintable(QHG::tr("Collection config file error: %1\n")
                         .arg(config.errorString())), stderr);
        return 1;
    }

    const QMap<QString, QString> &filesToGenerate = config.filesToGenerate();
    for (auto it = filesToGenerate.cbegin(), end = filesToGenerate.cend(); it != end; ++it) {
        fputs(qPrintable(QHG::tr("Generating help for %1...\n").arg(it.key())), stdout);
        QHelpProjectData helpData;
        if (!helpData.readData(absoluteFilePath(basePath, it.key()))) {
            fprintf(stderr, "%s\n", qPrintable(helpData.errorMessage()));
            return 1;
        }

        HelpGenerator helpGenerator;
        if (!helpGenerator.generate(&helpData, absoluteFilePath(basePath, it.value()))) {
            fprintf(stderr, "%s\n", qPrintable(helpGenerator.error()));
            return 1;
        }
    }

    fputs(qPrintable(QHG::tr("Creating collection file...\n")), stdout);

    QFileInfo colFi(outputFile);
    if (colFi.exists()) {
        if (!colFi.dir().remove(colFi.fileName())) {
            fputs(qPrintable(QHG::tr("The file %1 cannot be overwritten.\n")
                             .arg(outputFile)), stderr);
            return 1;
        }
    }

    QHelpEngineCore helpEngine(outputFile);
    if (!helpEngine.setupData()) {
        fprintf(stderr, "%s\n", qPrintable(helpEngine.error()));
        return 1;
    }

    for (const QString &file : config.filesToRegister()) {
        if (!helpEngine.registerDocumentation(absoluteFilePath(basePath, file))) {
            fprintf(stderr, "%s\n", qPrintable(helpEngine.error()));
            return 1;
        }
    }
    if (!config.filesToRegister().isEmpty()) {
        if (Q_UNLIKELY(qEnvironmentVariableIsSet("SOURCE_DATE_EPOCH"))) {
            QDateTime dt;
            dt.setSecsSinceEpoch(qEnvironmentVariableIntValue("SOURCE_DATE_EPOCH"));
            CollectionConfiguration::updateLastRegisterTime(helpEngine, dt);
        } else {
            CollectionConfiguration::updateLastRegisterTime(helpEngine);
        }
    }

    if (!config.title().isEmpty())
        CollectionConfiguration::setWindowTitle(helpEngine, config.title());

    if (!config.homePage().isEmpty()) {
        CollectionConfiguration::setDefaultHomePage(helpEngine,
            config.homePage());
    }

    if (!config.startPage().isEmpty()) {
        CollectionConfiguration::setLastShownPages(helpEngine,
            QStringList(config.startPage()));
    }

    if (!config.currentFilter().isEmpty()) {
        helpEngine.setCurrentFilter(config.currentFilter());
    }

    if (!config.cacheDirectory().isEmpty()) {
        CollectionConfiguration::setCacheDir(helpEngine, config.cacheDirectory(),
            config.cacheDirRelativeToCollection());
    }

    CollectionConfiguration::setFilterFunctionalityEnabled(helpEngine,
        config.enableFilterFunctionality());
    CollectionConfiguration::setFilterToolbarVisible(helpEngine,
        !config.hideFilterFunctionality());
    CollectionConfiguration::setDocumentationManagerEnabled(helpEngine,
        config.enableDocumentationManager());
    CollectionConfiguration::setAddressBarEnabled(helpEngine,
        config.enableAddressBar());
    CollectionConfiguration::setAddressBarVisible(helpEngine,
         !config.hideAddressBar());
    uint time = QDateTime::currentMSecsSinceEpoch() / 1000;
    if (Q_UNLIKELY(qEnvironmentVariableIsSet("SOURCE_DATE_EPOCH")))
        time = qEnvironmentVariableIntValue("SOURCE_DATE_EPOCH");
    CollectionConfiguration::setCreationTime(helpEngine, time);
    CollectionConfiguration::setFullTextSearchFallbackEnabled(helpEngine,
        config.fullTextSearchFallbackEnabled());

    if (!config.applicationIcon().isEmpty()) {
        QFile icon(absoluteFilePath(basePath, config.applicationIcon()));
        if (!icon.open(QIODevice::ReadOnly)) {
            fputs(qPrintable(QHG::tr("Cannot open %1.\n").arg(icon.fileName())), stderr);
            return 1;
        }
        CollectionConfiguration::setApplicationIcon(helpEngine, icon.readAll());
    }

    if (config.aboutMenuTexts().count()) {
        QByteArray ba;
        QDataStream s(&ba, QIODevice::WriteOnly);
        const QMap<QString, QString> &aboutMenuTexts = config.aboutMenuTexts();
        for (auto it = aboutMenuTexts.cbegin(), end = aboutMenuTexts.cend(); it != end; ++it)
            s << it.key() << it.value();
        CollectionConfiguration::setAboutMenuTexts(helpEngine, ba);
    }

    if (!config.aboutIcon().isEmpty()) {
        QFile icon(absoluteFilePath(basePath, config.aboutIcon()));
        if (!icon.open(QIODevice::ReadOnly)) {
            fputs(qPrintable(QHG::tr("Cannot open %1.\n").arg(icon.fileName())), stderr);
            return 1;
        }
        CollectionConfiguration::setAboutIcon(helpEngine, icon.readAll());
    }

    if (config.aboutTextFiles().count()) {
        QByteArray ba;
        QDataStream s(&ba, QIODevice::WriteOnly);
        QMap<QString, QByteArray> imgData;

        QRegExp srcRegExp(QLatin1String("src=(\"(.+)\"|([^\"\\s]+)).*>"));
        srcRegExp.setMinimal(true);
        QRegExp imgRegExp(QLatin1String("(<img[^>]+>)"));
        imgRegExp.setMinimal(true);

        const QMap<QString, QString> &aboutMenuTexts = config.aboutTextFiles();
        for (auto it = aboutMenuTexts.cbegin(), end = aboutMenuTexts.cend(); it != end; ++it) {
            s << it.key();
            QFileInfo fi(absoluteFilePath(basePath, it.value()));
            QFile f(fi.absoluteFilePath());
            if (!f.open(QIODevice::ReadOnly)) {
                fputs(qPrintable(QHG::tr("Cannot open %1.\n").arg(f.fileName())), stderr);
                return 1;
            }
            QByteArray data = f.readAll();
            s << data;

            QString contents = QString::fromUtf8(data);
            int pos = 0;
            while ((pos = imgRegExp.indexIn(contents, pos)) != -1) {
                QString imgTag = imgRegExp.cap(1);
                pos += imgRegExp.matchedLength();

                if (srcRegExp.indexIn(imgTag, 0) != -1) {
                    QString src = srcRegExp.cap(2);
                    if (src.isEmpty())
                        src = srcRegExp.cap(3);

                    QFile img(fi.absolutePath() + QDir::separator() + src);
                    if (img.open(QIODevice::ReadOnly)) {
                        if (!imgData.contains(src))
                            imgData.insert(src, img.readAll());
                    } else {
                        fputs(qPrintable(QHG::tr("Cannot open referenced image file %1.\n")
                                         .arg(img.fileName())), stderr);
                    }
                }
            }
        }
        CollectionConfiguration::setAboutTexts(helpEngine, ba);
        if (imgData.count()) {
            QByteArray imageData;
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);
            QDataStream out(&buffer);
            out << imgData;
            CollectionConfiguration::setAboutImages(helpEngine, imageData);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QString error;
    QString outputFile;
    QString inputFile;
    QString basePath;
    bool showHelp = false;
    bool showVersion = false;
    bool checkLinks = false;
    bool silent = false;

    // don't require a window manager even though we're a QGuiApplication
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("minimal"));

    QGuiApplication app(argc, argv);
#ifndef Q_OS_WIN32
    QTranslator translator;
    QTranslator qtTranslator;
    QTranslator qt_helpTranslator;
    QString sysLocale = QLocale::system().name();
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (translator.load(QLatin1String("assistant_") + sysLocale, resourceDir)
        && qtTranslator.load(QLatin1String("qt_") + sysLocale, resourceDir)
        && qt_helpTranslator.load(QLatin1String("qt_help_") + sysLocale, resourceDir)) {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
        app.installTranslator(&qt_helpTranslator);
    }
#endif // Q_OS_WIN32

    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QLatin1String("-o")) {
            if (++i < argc) {
                QFileInfo fi(QString::fromLocal8Bit(argv[i]));
                outputFile = fi.absoluteFilePath();
            } else {
                error = QHG::tr("Missing output file name.");
            }
        } else if (arg == QLatin1String("-v")) {
            showVersion = true;
        } else if (arg == QLatin1String("-h")) {
            showHelp = true;
        } else if (arg == QLatin1String("-c")) {
            checkLinks = true;
        } else if (arg == QLatin1String("-s")) {
            silent = true;
        } else {
            const QFileInfo fi(arg);
            inputFile = fi.absoluteFilePath();
            basePath = fi.absolutePath();
        }
    }

    if (showVersion) {
        fputs(qPrintable(QHG::tr("Qt Help Generator version 1.0 (Qt %1)\n")
                         .arg(QT_VERSION_STR)), stdout);
        return 0;
    }

    enum InputType {
        InputQhp,
        InputQhcp,
        InputUnknown
    };

    InputType inputType = InputUnknown;

    if (!showHelp) {
        if (inputFile.isEmpty()) {
            error = QHG::tr("Missing input file name.");
        } else {
            const QFileInfo fi(inputFile);
            if (fi.suffix() == QHP)
                inputType = InputQhp;
            else if (fi.suffix() == QHCP)
                inputType = InputQhcp;

            if (inputType == InputUnknown)
                error = QHG::tr("Unknown input file type.");
        }
    }

    const QString help = QHG::tr("\nUsage:\n\n"
        "qhelpgenerator <file> [options]\n\n"
        "  -o <output-file>       Generates a Qt compressed help\n"
        "                         called <output-file> (*.qch) for the\n"
        "                         Qt help project <file> (*.qhp).\n"
        "                         Generates a Qt help collection\n"
        "                         called <output-file> (*.qhc) for the\n"
        "                         Qt help collection project <file> (*.qhcp).\n"
        "                         If this option is not specified\n"
        "                         a default name will be used\n"
        "                         (*.qch for *.qhp and *.qhc for *.qhcp).\n"
        "  -c                     Checks whether all links in HTML files\n"
        "                         point to files in this help project.\n"
        "  -s                     Suppresses status messages.\n"
        "  -v                     Displays the version of \n"
        "                         qhelpgenerator.\n\n");

    if (showHelp) {
        fputs(qPrintable(help), stdout);
        return 0;
    } else if (!error.isEmpty()) {
        fprintf(stderr, "%s\n\n%s", qPrintable(error), qPrintable(help));
        return 1;
    }

    // detect input file type (qhp or qhcp)

    QFile file(inputFile);
    if (!file.open(QIODevice::ReadOnly)) {
        fputs(qPrintable(QHG::tr("Could not open %1.\n").arg(inputFile)), stderr);
        return 1;
    }

    const QString outputExtension = inputType == InputQhp ? QCH : QHC;

    if (outputFile.isEmpty()) {
        if (inputType == InputQhcp || !checkLinks) {
            QFileInfo fi(inputFile);
            outputFile = basePath + QDir::separator()
                             + fi.baseName() + QLatin1Char('.') + outputExtension;
        }
    } else {
        // check if the output dir exists -- create if it doesn't
        QFileInfo fi(outputFile);
        QDir parentDir = fi.dir();
        if (!parentDir.exists()) {
            if (!parentDir.mkpath(QLatin1String("."))) {
                fputs(qPrintable(QHG::tr("Could not create output directory: %1\n")
                                 .arg(parentDir.path())), stderr);
            }
        }
    }

    if (inputType == InputQhp) {
        QHelpProjectData *helpData = new QHelpProjectData();
        if (!helpData->readData(inputFile)) {
            fprintf(stderr, "%s\n", qPrintable(helpData->errorMessage()));
            return 1;
        }

        HelpGenerator generator(silent);
        bool success = true;
        if (checkLinks)
            success = generator.checkLinks(*helpData);
        if (success && !outputFile.isEmpty())
            success = generator.generate(helpData, outputFile);
        delete helpData;
        if (!success) {
            fprintf(stderr, "%s\n", qPrintable(generator.error()));
            return 1;
        }
    } else {
        const QByteArray data = file.readAll();
        return generateCollectionFile(data, basePath, outputFile);

    }

    return 0;
}
