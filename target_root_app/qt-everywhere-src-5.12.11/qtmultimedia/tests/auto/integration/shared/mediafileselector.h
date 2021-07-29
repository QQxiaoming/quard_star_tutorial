/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#ifndef MEDIAFILESELECTOR_H
#define MEDIAFILESELECTOR_H

#include <QMediaContent>
#include <QMediaPlayer>

QT_BEGIN_NAMESPACE

namespace MediaFileSelector {

static QMediaContent selectMediaFile(const QStringList& mediaCandidates)
{
    QMediaPlayer player;

    QSignalSpy errorSpy(&player, SIGNAL(error(QMediaPlayer::Error)));

    foreach (QString s, mediaCandidates) {
        QFileInfo mediaFile(s);
        if (!mediaFile.exists())
            continue;
        QMediaContent media = QMediaContent(QUrl::fromLocalFile(mediaFile.absoluteFilePath()));
        player.setMedia(media);
        player.play();

        for (int i = 0; i < 2000 && player.mediaStatus() != QMediaPlayer::BufferedMedia && errorSpy.isEmpty(); i+=50) {
            QTest::qWait(50);
        }

        if (player.mediaStatus() == QMediaPlayer::BufferedMedia && errorSpy.isEmpty()) {
            return media;
        }
        errorSpy.clear();
    }

    return QMediaContent();
}

} // MediaFileSelector namespace

QT_END_NAMESPACE

#endif

