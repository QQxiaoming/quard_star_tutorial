/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgtexturereader_p.h"
#include <private/qtexturefilereader_p.h>

#if QT_CONFIG(opengl)
#include <private/qsgcompressedtexture_p.h>
#endif

QT_BEGIN_NAMESPACE

QSGTextureReader::QSGTextureReader(QIODevice *device, const QString &fileName)
{
#if QT_CONFIG(opengl)
    m_reader = new QTextureFileReader(device, fileName);
#else
    Q_UNUSED(device);
    Q_UNUSED(fileName);
#endif
}

QSGTextureReader::~QSGTextureReader()
{
    delete m_reader;
}

QQuickTextureFactory *QSGTextureReader::read()
{
#if QT_CONFIG(opengl)
    if (!m_reader)
        return nullptr;

    QTextureFileData texData = m_reader->read();
    if (!texData.isValid())
        return nullptr;

    return new QSGCompressedTextureFactory(texData);
#else
    return nullptr;
#endif
}

bool QSGTextureReader::isTexture()
{
    return m_reader ? m_reader->canRead() : false;
}

QList<QByteArray> QSGTextureReader::supportedFileFormats()
{
    return QTextureFileReader::supportedFileFormats();
}

QT_END_NAMESPACE
