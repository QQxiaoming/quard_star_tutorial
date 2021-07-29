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
#ifndef MMRENDERERMEDIAPLAYERCONTROL_H
#define MMRENDERERMEDIAPLAYERCONTROL_H

#include "mmrenderermetadata.h"
#include <qmediaplayercontrol.h>
#include <QtCore/qabstractnativeeventfilter.h>
#include <QtCore/qpointer.h>
#include <QtCore/qtimer.h>

typedef struct mmr_connection mmr_connection_t;
typedef struct mmr_context mmr_context_t;
typedef struct mmrenderer_monitor mmrenderer_monitor_t;
typedef struct strm_dict strm_dict_t;

QT_BEGIN_NAMESPACE

class MmRendererAudioRoleControl;
class MmRendererCustomAudioRoleControl;
class MmRendererMetaDataReaderControl;
class MmRendererPlayerVideoRendererControl;
class MmRendererVideoWindowControl;

class MmRendererMediaPlayerControl : public QMediaPlayerControl, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit MmRendererMediaPlayerControl(QObject *parent = 0);

    QMediaPlayer::State state() const override;

    QMediaPlayer::MediaStatus mediaStatus() const override;

    qint64 duration() const override;

    qint64 position() const override;
    void setPosition(qint64 position) override;

    int volume() const override;
    void setVolume(int volume) override;

    bool isMuted() const override;
    void setMuted(bool muted) override;

    int bufferStatus() const override;

    bool isAudioAvailable() const override;
    bool isVideoAvailable() const override;

    bool isSeekable() const override;

    QMediaTimeRange availablePlaybackRanges() const override;

    qreal playbackRate() const override;
    void setPlaybackRate(qreal rate) override;

    QMediaContent media() const override;
    const QIODevice *mediaStream() const override;
    void setMedia(const QMediaContent &media, QIODevice *stream) override;

    void play() override;
    void pause() override;
    void stop() override;

    MmRendererPlayerVideoRendererControl *videoRendererControl() const;
    void setVideoRendererControl(MmRendererPlayerVideoRendererControl *videoControl);

    MmRendererVideoWindowControl *videoWindowControl() const;
    void setVideoWindowControl(MmRendererVideoWindowControl *videoControl);
    void setMetaDataReaderControl(MmRendererMetaDataReaderControl *metaDataReaderControl);
    void setAudioRoleControl(MmRendererAudioRoleControl *audioRoleControl);
    void setCustomAudioRoleControl(MmRendererCustomAudioRoleControl *customAudioRoleControl);

protected:
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
    virtual void resetMonitoring() = 0;

    void openConnection();
    void emitMmError(const QString &msg);
    void emitPError(const QString &msg);
    void setMmPosition(qint64 newPosition);
    void setMmBufferStatus(const QString &bufferStatus);
    void setMmBufferLevel(int level, int capacity);
    void handleMmStopped();
    void handleMmSuspend(const QString &reason);
    void handleMmSuspendRemoval(const QString &bufferStatus);
    void handleMmPause();
    void handleMmPlay();
    void updateMetaData(const strm_dict_t *dict);

    // must be called from subclass dtors (calls virtual function stopMonitoring())
    void destroy();

    mmr_context_t *m_context;
    int m_id;
    QString m_contextName;

private Q_SLOTS:
    void continueLoadMedia();

private:
    QByteArray resourcePathForUrl(const QUrl &url);
    void closeConnection();
    void attach();
    void detach();

    // All these set the specified value to the backend, but neither emit changed signals
    // nor change the member value.
    void setVolumeInternal(int newVolume);
    void setPlaybackRateInternal(qreal rate);
    void setPositionInternal(qint64 position);

    void setMediaStatus(QMediaPlayer::MediaStatus status);
    void setState(QMediaPlayer::State state);

    enum StopCommand { StopMmRenderer, IgnoreMmRenderer };
    void stopInternal(StopCommand stopCommand);

    QMediaContent m_media;
    mmr_connection_t *m_connection;
    int m_audioId;
    QMediaPlayer::State m_state;
    int m_volume;
    bool m_muted;
    qreal m_rate;
    QPointer<MmRendererPlayerVideoRendererControl> m_videoRendererControl;
    QPointer<MmRendererVideoWindowControl> m_videoWindowControl;
    QPointer<MmRendererMetaDataReaderControl> m_metaDataReaderControl;
    QPointer<MmRendererAudioRoleControl> m_audioRoleControl;
    QPointer<MmRendererCustomAudioRoleControl> m_customAudioRoleControl;
    MmRendererMetaData m_metaData;
    qint64 m_position;
    QMediaPlayer::MediaStatus m_mediaStatus;
    bool m_playAfterMediaLoaded;
    bool m_inputAttached;
    int m_bufferLevel;
    QTimer m_loadingTimer;
};

QT_END_NAMESPACE

#endif
