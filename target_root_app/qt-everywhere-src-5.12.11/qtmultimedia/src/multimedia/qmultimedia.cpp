/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
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

#include "qmultimedia.h"

QT_BEGIN_NAMESPACE

/*!
    \namespace QMultimedia
    \ingroup multimedia-namespaces
    \ingroup multimedia
    \inmodule QtMultimedia

    \ingroup multimedia
    \ingroup multimedia_core

    \brief The QMultimedia namespace contains miscellaneous identifiers used throughout the Qt Multimedia library.

*/

static void qRegisterMultimediaMetaTypes()
{
    qRegisterMetaType<QMultimedia::AvailabilityStatus>();
    qRegisterMetaType<QMultimedia::SupportEstimate>();
    qRegisterMetaType<QMultimedia::EncodingMode>();
    qRegisterMetaType<QMultimedia::EncodingQuality>();
}

Q_CONSTRUCTOR_FUNCTION(qRegisterMultimediaMetaTypes)


/*!
    \enum QMultimedia::SupportEstimate

    Enumerates the levels of support a media service provider may have for a feature.

    \value NotSupported The feature is not supported.
    \value MaybeSupported The feature may be supported.
    \value ProbablySupported The feature is probably supported.
    \value PreferredService The service is the preferred provider of a service.
*/

/*!
    \enum QMultimedia::EncodingQuality

    Enumerates quality encoding levels.

    \value VeryLowQuality
    \value LowQuality
    \value NormalQuality
    \value HighQuality
    \value VeryHighQuality
*/

/*!
    \enum QMultimedia::EncodingMode

    Enumerates encoding modes.

    \value ConstantQualityEncoding Encoding will aim to have a constant quality, adjusting bitrate to fit.
    \value ConstantBitRateEncoding Encoding will use a constant bit rate, adjust quality to fit.
    \value AverageBitRateEncoding Encoding will try to keep an average bitrate setting, but will use
            more or less as needed.
    \value TwoPassEncoding The media will first be processed to determine the characteristics,
            and then processed a second time allocating more bits to the areas
            that need it.
*/

/*!
    \enum QMultimedia::AvailabilityStatus

    Enumerates Service status errors.

    \value Available The service is operating correctly.
    \value ServiceMissing There is no service available to provide the requested functionality.
    \value ResourceError The service could not allocate resources required to function correctly.
    \value Busy The service must wait for access to necessary resources.
*/

QT_END_NAMESPACE
