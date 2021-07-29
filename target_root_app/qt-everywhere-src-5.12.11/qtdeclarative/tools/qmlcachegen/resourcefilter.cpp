/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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
#include <QString>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>

int filterResourceFile(const QString &input, const QString &output)
{
    enum State {
        InitialState,
        InRCC,
        InResource,
        InFile
    };
    State state = InitialState;

    QString prefix;
    QString currentFileName;
    QXmlStreamAttributes fileAttributes;

    QFile file(input);
    if (!file.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "Cannot open %s for reading.\n", qPrintable(input));
        return EXIT_FAILURE;
    }

    QDir inputDirectory = QFileInfo(file).absoluteDir();
    QDir outputDirectory = QFileInfo(output).absoluteDir();

    QString outputString;
    QXmlStreamWriter writer(&outputString);
    writer.setAutoFormatting(true);

    QStringList remainingFiles;

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::StartDocument: {
            QStringRef version = reader.documentVersion();
            if (!version.isEmpty())
                writer.writeStartDocument(version.toString());
            else
                writer.writeStartDocument();
            break;
        }
        case QXmlStreamReader::EndDocument:
            writer.writeEndDocument();
            break;
        case QXmlStreamReader::StartElement:
            if (reader.name() == QStringLiteral("RCC")) {
                if (state != InitialState) {
                    fprintf(stderr, "Unexpected RCC tag in line %d\n", int(reader.lineNumber()));
                    return EXIT_FAILURE;
                }
                state = InRCC;
            } else if (reader.name() == QStringLiteral("qresource")) {
                if (state != InRCC) {
                    fprintf(stderr, "Unexpected qresource tag in line %d\n", int(reader.lineNumber()));
                    return EXIT_FAILURE;
                }
                state = InResource;
                QXmlStreamAttributes attributes = reader.attributes();
                if (attributes.hasAttribute(QStringLiteral("prefix")))
                    prefix = attributes.value(QStringLiteral("prefix")).toString();
                if (!prefix.startsWith(QLatin1Char('/')))
                    prefix.prepend(QLatin1Char('/'));
                if (!prefix.endsWith(QLatin1Char('/')))
                    prefix.append(QLatin1Char('/'));
            } else if (reader.name() == QStringLiteral("file")) {
                if (state != InResource) {
                    fprintf(stderr, "Unexpected file tag in line %d\n", int(reader.lineNumber()));
                    return EXIT_FAILURE;
                }
                state = InFile;
                fileAttributes = reader.attributes();
                continue;
            }
            writer.writeStartElement(reader.name().toString());
            writer.writeAttributes(reader.attributes());
            continue;

        case QXmlStreamReader::EndElement:
            if (reader.name() == QStringLiteral("file")) {
                if (state != InFile) {
                    fprintf(stderr, "Unexpected end of file tag in line %d\n", int(reader.lineNumber()));
                    return EXIT_FAILURE;
                }
                state = InResource;
                continue;
            } else if (reader.name() == QStringLiteral("qresource")) {
                if (state != InResource) {
                    fprintf(stderr, "Unexpected end of qresource tag in line %d\n", int(reader.lineNumber()));
                    return EXIT_FAILURE;
                }
                state = InRCC;
            } else if (reader.name() == QStringLiteral("RCC")) {
                if (state != InRCC) {
                    fprintf(stderr, "Unexpected end of RCC tag in line %d\n", int(reader.lineNumber()));
                    return EXIT_FAILURE;
                }
                state = InitialState;
            }
            writer.writeEndElement();
            continue;

        case QXmlStreamReader::Characters:
            if (reader.isWhitespace())
                break;
            if (state != InFile)
                return EXIT_FAILURE;
            currentFileName = reader.text().toString();
            if (currentFileName.isEmpty())
                continue;

            if (!currentFileName.endsWith(QStringLiteral(".qml"))
                && !currentFileName.endsWith(QStringLiteral(".js"))
                && !currentFileName.endsWith(QStringLiteral(".mjs"))) {
                writer.writeStartElement(QStringLiteral("file"));

                if (!fileAttributes.hasAttribute(QStringLiteral("alias")))
                    fileAttributes.append(QStringLiteral("alias"), currentFileName);

                currentFileName = inputDirectory.absoluteFilePath(currentFileName);
                currentFileName = outputDirectory.relativeFilePath(currentFileName);

                remainingFiles << currentFileName;

                writer.writeAttributes(fileAttributes);
                writer.writeCharacters(currentFileName);
                writer.writeEndElement();
            }
            continue;

        default: break;
    }
    }

    if (!remainingFiles.isEmpty()) {
        QFile outputFile(output);
        if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            fprintf(stderr, "Cannot open %s for writing.\n", qPrintable(output));
            return EXIT_FAILURE;
        }
        const QByteArray outputStringUtf8 = outputString.toUtf8();
        if (outputFile.write(outputStringUtf8) != outputStringUtf8.size())
            return EXIT_FAILURE;

        outputFile.close();
        if (outputFile.error() != QFileDevice::NoError)
            return EXIT_FAILURE;

        // The build system expects this output if we wrote a qrc file and no output
        // if no files remain.
        fprintf(stdout, "New resource file written with %d files.\n", remainingFiles.count());
    }

    return EXIT_SUCCESS;
}
