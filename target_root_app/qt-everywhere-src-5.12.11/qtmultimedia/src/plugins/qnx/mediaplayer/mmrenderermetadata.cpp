/****************************************************************************
**
** Copyright (C) 2016 Research In Motion
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
#include "mmrenderermetadata.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qstringlist.h>

#include <mm/renderer/events.h>
#include <sys/neutrino.h>
#include <sys/strm.h>

static const char *strm_string_getx(const strm_string_t *sstr, const char *defaultValue)
{
    return sstr ? strm_string_get(sstr) : defaultValue;
}

#if _NTO_VERSION < 700
static strm_dict_t *mmr_metadata_split(strm_dict_t const *, const char *, unsigned)
{
    return nullptr;
}
#endif

QT_BEGIN_NAMESPACE

MmRendererMetaData::MmRendererMetaData()
{
    clear();
}

static const char * titleKey = "md_title_name";
static const char * artistKey = "md_title_artist";
static const char * commentKey = "md_title_comment";
static const char * genreKey = "md_title_genre";
static const char * yearKey = "md_title_year";
static const char * durationKey = "md_title_duration";
static const char * bitRateKey = "md_title_bitrate";
static const char * sampleKey = "md_title_samplerate";
static const char * albumKey = "md_title_album";
static const char * trackKey = "md_title_track";
static const char * widthKey = "md_video_width";
static const char * heightKey = "md_video_height";
static const char * mediaTypeKey = "md_title_mediatype";
static const char * pixelWidthKey = "md_video_pixel_width";
static const char * pixelHeightKey = "md_video_pixel_height";
static const char * seekableKey = "md_title_seekable";
static const char * trackSampleKey = "sample_rate";
static const char * trackBitRateKey = "bitrate";
static const char * trackWidthKey = "width";
static const char * trackHeightKey = "height";
static const char * trackPixelWidthKey = "pixel_width";
static const char * trackPixelHeightKey = "pixel_height";

static const int mediaTypeAudioFlag = 4;
static const int mediaTypeVideoFlag = 2;

bool MmRendererMetaData::update(const strm_dict_t *dict)
{
    if (!dict) {
        clear();
        return true;
    }

    const strm_string_t *value;

    value = strm_dict_find_rstr(dict, durationKey);
    m_duration = QByteArray(strm_string_getx(value, "0")).toLongLong();

    value = strm_dict_find_rstr(dict, mediaTypeKey);
    m_mediaType = QByteArray(strm_string_getx(value, "-1")).toInt();

    value = strm_dict_find_rstr(dict, titleKey);
    m_title = QString::fromLatin1(QByteArray(strm_string_getx(value, nullptr)));

    value = strm_dict_find_rstr(dict, seekableKey);
    m_seekable = (strcmp(strm_string_getx(value, "1"), "0") != 0);

    value = strm_dict_find_rstr(dict, artistKey);
    m_artist = QString::fromLatin1(QByteArray(strm_string_getx(value, nullptr)));

    value = strm_dict_find_rstr(dict, commentKey);
    m_comment = QString::fromLatin1(QByteArray(strm_string_getx(value, nullptr)));

    value = strm_dict_find_rstr(dict, genreKey);
    m_genre = QString::fromLatin1(QByteArray(strm_string_getx(value, nullptr)));

    value = strm_dict_find_rstr(dict, yearKey);
    m_year = QByteArray(strm_string_getx(value, "0")).toInt();

    value = strm_dict_find_rstr(dict, albumKey);
    m_album = QString::fromLatin1(QByteArray(strm_string_getx(value, nullptr)));

    value = strm_dict_find_rstr(dict, trackKey);
    m_track = QByteArray(strm_string_getx(value, "0")).toInt();

    strm_dict_t *at = mmr_metadata_split(dict, "audio", 0);
    if (at) {
        value = strm_dict_find_rstr(at, trackSampleKey);
        m_sampleRate = QByteArray(strm_string_getx(value, "0")).toInt();

        value = strm_dict_find_rstr(at, trackBitRateKey);
        m_audioBitRate = QByteArray(strm_string_getx(value, "0")).toInt();

        strm_dict_destroy(at);
    } else {
        value = strm_dict_find_rstr(dict, sampleKey);
        m_sampleRate = QByteArray(strm_string_getx(value, "0")).toInt();

        value = strm_dict_find_rstr(dict, bitRateKey);
        m_audioBitRate = QByteArray(strm_string_getx(value, "0")).toInt();
    }

    strm_dict_t *vt = mmr_metadata_split(dict, "video", 0);
    if (vt) {
        value = strm_dict_find_rstr(vt, trackWidthKey);
        m_width = QByteArray(strm_string_getx(value, "0")).toInt();

        value = strm_dict_find_rstr(vt, trackHeightKey);
        m_height = QByteArray(strm_string_getx(value, "0")).toInt();

        value = strm_dict_find_rstr(vt, trackPixelWidthKey);
        m_pixelWidth = QByteArray(strm_string_getx(value, "1")).toFloat();

        value = strm_dict_find_rstr(vt, trackPixelHeightKey);
        m_pixelHeight = QByteArray(strm_string_getx(value, "1")).toFloat();

        strm_dict_destroy(vt);
    } else {
        value = strm_dict_find_rstr(dict, widthKey);
        m_width = QByteArray(strm_string_getx(value, "0")).toInt();

        value = strm_dict_find_rstr(dict, heightKey);
        m_height = QByteArray(strm_string_getx(value, "0")).toInt();

        value = strm_dict_find_rstr(dict, pixelWidthKey);
        m_pixelWidth = QByteArray(strm_string_getx(value, "1")).toFloat();

        value = strm_dict_find_rstr(dict, pixelHeightKey);
        m_pixelHeight = QByteArray(strm_string_getx(value, "1")).toFloat();
    }

    return true;
}

void MmRendererMetaData::clear()
{
    strm_dict_t *dict;
    dict = strm_dict_new();
    update(dict);
    strm_dict_destroy(dict);
}

qlonglong MmRendererMetaData::duration() const
{
    return m_duration;
}

// Handling of pixel aspect ratio
//
// If the pixel aspect ratio is different from 1:1, it means the video needs to be stretched in
// order to look natural.
// For example, if the pixel width is 2, and the pixel height is 1, it means a video of 300x200
// pixels needs to be displayed as 600x200 to look correct.
// In order to support this the easiest way, we simply pretend that the actual size of the video
// is 600x200, which will cause the video to be displayed in an aspect ratio of 3:1 instead of 3:2,
// and therefore look correct.

int MmRendererMetaData::height() const
{
    return m_height * m_pixelHeight;
}

int MmRendererMetaData::width() const
{
    return m_width * m_pixelWidth;
}

bool MmRendererMetaData::hasVideo() const
{
    // By default, assume no video if we can't extract the information
    if (m_mediaType == -1)
        return false;

    return (m_mediaType & mediaTypeVideoFlag);
}

bool MmRendererMetaData::hasAudio() const
{
    // By default, assume audio only if we can't extract the information
    if (m_mediaType == -1)
        return true;

    return (m_mediaType & mediaTypeAudioFlag);
}

QString MmRendererMetaData::title() const
{
    return m_title;
}

bool MmRendererMetaData::isSeekable() const
{
    return m_seekable;
}

QString MmRendererMetaData::artist() const
{
    return m_artist;
}

QString MmRendererMetaData::comment() const
{
    return m_comment;
}

QString MmRendererMetaData::genre() const
{
    return m_genre;
}

int MmRendererMetaData::year() const
{
    return m_year;
}

QString MmRendererMetaData::mediaType() const
{
    if (hasVideo())
        return QLatin1String("video");
    else if (hasAudio())
        return QLatin1String("audio");
    else
        return QString();
}

int MmRendererMetaData::audioBitRate() const
{
    return m_audioBitRate;
}

int MmRendererMetaData::sampleRate() const
{
    return m_sampleRate;
}

QString MmRendererMetaData::album() const
{
    return m_album;
}

int MmRendererMetaData::track() const
{
    return m_track;
}

QSize MmRendererMetaData::resolution() const
{
    return QSize(width(), height());
}

QT_END_NAMESPACE
