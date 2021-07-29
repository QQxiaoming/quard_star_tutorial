/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "directshowcameraimageencodercontrol.h"
#include "dscamerasession.h"
#include <QImageWriter>

QT_BEGIN_NAMESPACE

DirectShowCameraImageEncoderControl::DirectShowCameraImageEncoderControl(DSCameraSession *session)
    : QImageEncoderControl(session)
    , m_session(session)
{
}

QList<QSize> DirectShowCameraImageEncoderControl::supportedResolutions(const QImageEncoderSettings &settings, bool *continuous) const
{
    QList<QSize> res;
    if (!settings.codec().isEmpty() && !supportedImageCodecs().contains(settings.codec(), Qt::CaseInsensitive))
        return res;

    QList<QSize> resolutions = m_session->supportedResolutions(continuous);
    QSize r = settings.resolution();
    if (!r.isValid())
        return resolutions;

    if (resolutions.contains(r))
        res << settings.resolution();

    return res;
}

QStringList DirectShowCameraImageEncoderControl::supportedImageCodecs() const
{
    QStringList supportedCodecs;
    for (const QByteArray &type: QImageWriter::supportedImageFormats()) {
        supportedCodecs << type;
    }

    return supportedCodecs;
}

QString DirectShowCameraImageEncoderControl::imageCodecDescription(const QString &codecName) const
{
    Q_UNUSED(codecName);
    return QString();
}

QImageEncoderSettings DirectShowCameraImageEncoderControl::imageSettings() const
{
    return m_session->imageEncoderSettings();
}

void DirectShowCameraImageEncoderControl::setImageSettings(const QImageEncoderSettings &settings)
{
    m_session->setImageEncoderSettings(settings);
}

QT_END_NAMESPACE
