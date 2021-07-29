/*
 * Qt implementation of OpenWnn library
 * This file is part of the Qt Virtual Keyboard module.
 * Contact: http://www.qt.io/licensing/
 *
 * Copyright (C) 2017  The Qt Company
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wnnlookuptable.h"
#include <qalgorithms.h>
#if WNN_LOOKUP_TABLE_CREATE
#include <QFile>
#endif

WnnLookupTable::WnnLookupTable(const char **keys, const char **values, const int length) :
    keys(keys),
    values(values),
    length(length)
{
}

const QString WnnLookupTable::value(const QString &what) const
{
    const char **key = qBinaryFind(keys, keys + length, what.toUtf8().constData(),
                [] (const char *lhs, const char *rhs) { return strcmp(lhs, rhs) < 0; });
    int index = key - keys;
    if (index == length)
        return QString();
    return QString::fromUtf8(values[index]);
}

#if WNN_LOOKUP_TABLE_CREATE
void WnnLookupTable::create(const QMap<QString, QString> &map, const QString &tablePrefix, const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QList<QString> keys = map.keys();
        std::sort(keys.begin(), keys.end(), [] (const QString &lhs, const QString &rhs) {
            return strcmp(lhs.toUtf8().constData(), rhs.toUtf8().constData()) < 0;
        });
        file.write(QString("static const int %1Length = %2;\n").arg(tablePrefix).arg(keys.count()).toUtf8().constData());
        file.write(QString("static const char *%1Key[%1Length];\n").arg(tablePrefix).toUtf8().constData());
        file.write(QString("static const char *%1Value[%1Length];\n").arg(tablePrefix).toUtf8().constData());
        file.write(QString("const char *%1Key[] = {\n").arg(tablePrefix).toUtf8().constData());
        int c = 0;
        for (const QString &key : keys) {
            if (c > 0 && c % 16 != 0)
                file.write(", ");
            file.write("\"");
            QString escaped = QString(key).replace("\\", "\\\\").replace("'", "\\'").replace("\"", "\\\"");
            file.write(escaped.toUtf8().constData());
            file.write("\"");
            c++;
            if (c % 16 == 0)
                file.write(",\n");
        }
        file.write("\n};\n");
        file.write(QString("const char *%1Value[] = {\n").arg(tablePrefix).toUtf8().constData());
        c = 0;
        for (const QString &key : keys) {
            if (c > 0 && c % 16 != 0)
                file.write(", ");
            file.write("\"");
            QString escaped = QString(map.value(key)).replace("\\", "\\\\").replace("'", "\\'").replace("\"", "\\\"");
            file.write(escaped.toUtf8().constData());
            file.write("\"");
            c++;
            if (c % 16 == 0)
                file.write(",\n");
        }
        file.write("\n};\n");
    }
}
#endif
