/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt WebGL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qdatastream.h>
#include <QtCore/qvariant.h>

#ifndef PARAMETERS_H
#define PARAMETERS_H

QT_BEGIN_NAMESPACE

namespace Parameters
{

QVariantList read(const QByteArray &data, QDataStream &stream, quint32 &offset, int count);

template<typename T>
T readNext(QDataStream &stream, quint32 &offset)
{
    T value;
    stream >> value;
    offset += sizeof(T);
    return value;
}

template<>
QString readNext(QDataStream &stream, quint32 &offset)
{
    std::vector<char> data(readNext<quint32>(stream, offset));
    stream.readRawData(&data[0], data.size());
    offset += data.size();
    return QString::fromUtf8(data.data(), data.size());
}

template<>
QByteArray readNext(QDataStream &stream, quint32 &offset)
{
    QByteArray data;
    stream >> data;
    offset += quint32(int(sizeof(qint32)) + data.size());
    return data;
}

QVariantList readNextArray(const QByteArray &data, QDataStream &stream, quint32 &offset)
{
    quint8 count;
    stream >> count;
    offset += sizeof(count);
    return read(data, stream, offset, count);
}

QVariant readNext(const QByteArray &data, QDataStream &stream, quint32 &offset)
{
    char type;
    offset += quint32(stream.readRawData(&type, 1));
    switch (type) {
    case 'i': return readNext<qint32>(stream, offset);
    case 'u': return readNext<quint32>(stream, offset);
    case 'd': return readNext<double>(stream, offset);
    case 'b': return readNext<quint8>(stream, offset);
    case 's': return readNext<QString>(stream, offset);
    case 'x': return readNext<QByteArray>(stream, offset);
    case 'a': return readNextArray(data, stream, offset);
    }
    return QVariant();
}

QVariantList read(const QByteArray &data, QDataStream &stream, quint32 &offset)
{
    QVariantList parameters;
    for (const auto size = data.size(); int(offset + 4) < size;)
        parameters.append(readNext(data, stream, offset));
    return parameters;
}

QVariantList read(const QByteArray &data, QDataStream &stream, quint32 &offset, int count)
{
    QVariantList parameters;
    for (int i = 0; i < count; ++i)
        parameters.append(readNext(data, stream, offset));
    return parameters;
}

}

QT_END_NAMESPACE

#endif // PARAMETERS_H
