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

#ifndef WNNLOOKUPTABLE_H
#define WNNLOOKUPTABLE_H

#include <QString>
#if WNN_LOOKUP_TABLE_CREATE
#include <QMap>
#endif

class WnnLookupTable
{
public:
    explicit WnnLookupTable(const char **keys, const char **values, const int length);

    const QString value(const QString &what) const;
    inline const QString operator[](const QString &what) const { return value(what); }

#if WNN_LOOKUP_TABLE_CREATE
    static void create(const QMap<QString, QString> &map, const QString &privatePrefix, const QString &fileName);
#endif

private:
    const char **keys;
    const char **values;
    const int length;
};

#endif // WNNLOOKUPTABLE_H
