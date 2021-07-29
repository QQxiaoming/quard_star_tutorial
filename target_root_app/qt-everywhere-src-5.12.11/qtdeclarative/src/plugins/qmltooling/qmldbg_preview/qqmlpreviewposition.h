/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QML preview debug service.
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

#ifndef QQMLPREVIEWPOSITION_H
#define QQMLPREVIEWPOSITION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qvector.h>
#include <QtCore/qpoint.h>
#include <QtCore/qurl.h>
#include <QtCore/qtimer.h>
#include <QtCore/qsettings.h>
#include <QtCore/qsize.h>
#include <QtCore/qdatastream.h>

QT_BEGIN_NAMESPACE

class QWindow;

class QQmlPreviewPosition
{
public:
    class ScreenData {
    public:
        bool operator==(const QQmlPreviewPosition::ScreenData &other) const;
        QString name;
        QSize size;
    };
    class Position {
    public:
        QString screenName;
        QPoint nativePosition;
    };
    enum InitializeState {
        InitializePosition,
        PositionInitialized
    };

    QQmlPreviewPosition();
    ~QQmlPreviewPosition();


    void takePosition(QWindow *window, InitializeState state = PositionInitialized);
    void initLastSavedWindowPosition(QWindow *window);
    void loadWindowPositionSettings(const QUrl &url);

private:
    void setPosition(const QQmlPreviewPosition::Position &position, QWindow *window);
    QByteArray fromPositionToByteArray(const Position &position);
    void readLastPositionFromByteArray(const QByteArray &array);
    void saveWindowPosition();

    bool m_hasPosition = false;
    InitializeState m_initializeState = InitializePosition;
    QSettings m_settings;
    QString m_settingsKey;
    QTimer m_savePositionTimer;
    Position m_lastWindowPosition;
    QVector<QWindow *> m_positionedWindows;

    QVector<ScreenData> m_currentInitScreensData;
};

QT_END_NAMESPACE

#endif // QQMLPREVIEWPOSITION_H
