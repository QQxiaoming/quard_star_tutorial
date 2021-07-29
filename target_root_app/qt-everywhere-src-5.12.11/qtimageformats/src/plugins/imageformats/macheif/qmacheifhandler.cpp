/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the MacHeif plugin in the Qt ImageFormats module.
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

#include "qmacheifhandler.h"
#include "qiiofhelpers_p.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

QMacHeifHandler::QMacHeifHandler()
    : d(new QIIOFHelper(this))
{
}

QMacHeifHandler::~QMacHeifHandler()
{
}

bool QMacHeifHandler::canRead(QIODevice *iod)
{
    bool bCanRead = false;
    char buf[12];
    if (iod && iod->peek(buf, 12) == 12) {
        bCanRead = (!qstrncmp(buf + 4, "ftyp", 4) &&
                    (!qstrncmp(buf + 8, "heic", 4) ||
                     !qstrncmp(buf + 8, "heix", 4) ||
                     !qstrncmp(buf + 8, "mif1", 4)));
    }
    return bCanRead;
}

bool QMacHeifHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("heic");
        return true;
    }
    return false;
}

bool QMacHeifHandler::read(QImage *image)
{
    return d->readImage(image);
}

bool QMacHeifHandler::write(const QImage &image)
{
    return d->writeImage(image, QStringLiteral("public.heic"));
}

QVariant QMacHeifHandler::option(ImageOption option) const
{
    return d->imageProperty(option);
}

void QMacHeifHandler::setOption(ImageOption option, const QVariant &value)
{
    d->setOption(option, value);
}

bool QMacHeifHandler::supportsOption(ImageOption option) const
{
    return option == Quality
        || option == Size
        || option == ImageTransformation
        || option == TransformedByDefault;
}

QT_END_NAMESPACE
