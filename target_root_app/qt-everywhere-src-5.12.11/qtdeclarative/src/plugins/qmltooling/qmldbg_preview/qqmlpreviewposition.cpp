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

#include "qqmlpreviewposition.h"

#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#include <private/qhighdpiscaling_p.h>

QT_BEGIN_NAMESPACE

static QVector<QQmlPreviewPosition::ScreenData> initScreensData()
{
    QVector<QQmlPreviewPosition::ScreenData> screensData;

    for (QScreen *screen : QGuiApplication::screens()) {
        QQmlPreviewPosition::ScreenData sd{screen->name(), screen->size()};
        screensData.append(sd);
    }
    return screensData;
}

static QScreen *findScreen(const QString &nameOfScreen)
{
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen->name() == nameOfScreen)
            return screen;
    }
    return nullptr;
}

static QDataStream &operator<<(QDataStream &out, const QQmlPreviewPosition::ScreenData &screenData)
{
    out << screenData.name;
    out << screenData.size;
    return out;
}

static QDataStream &operator>>(QDataStream &in, QQmlPreviewPosition::ScreenData &screenData)
{
    in >> screenData.name;
    in >> screenData.size;
    return in;
}

bool QQmlPreviewPosition::ScreenData::operator==(const QQmlPreviewPosition::ScreenData &other) const
{
    return other.size == size && other.name == name;
}

QQmlPreviewPosition::QQmlPreviewPosition()
    : m_settings("QtProject", "QtQmlPreview")
{
    m_savePositionTimer.setSingleShot(true);
    m_savePositionTimer.setInterval(500);
    QObject::connect(&m_savePositionTimer, &QTimer::timeout, [this]() {
        saveWindowPosition();
    });
}

QQmlPreviewPosition::~QQmlPreviewPosition()
{
    saveWindowPosition();
}

void QQmlPreviewPosition::takePosition(QWindow *window, InitializeState state)
{
    Q_ASSERT(window);
    // only save the position if we already tried to get the last saved position
    if (m_initializeState == PositionInitialized) {
        m_hasPosition = true;
        auto screen = window->screen();
        auto nativePosition = QHighDpiScaling::mapPositionToNative(window->framePosition(),
                                                                   screen->handle());
        m_lastWindowPosition = {screen->name(), nativePosition};

        m_savePositionTimer.start();
    }
    if (state == InitializePosition)
        m_initializeState = InitializePosition;
}

void QQmlPreviewPosition::saveWindowPosition()
{
    if (m_hasPosition) {
        const QByteArray positionAsByteArray = fromPositionToByteArray(m_lastWindowPosition);
        if (!m_settingsKey.isNull())
            m_settings.setValue(m_settingsKey, positionAsByteArray);

        m_settings.setValue(QLatin1String("global_lastpostion"), positionAsByteArray);
    }
}

void QQmlPreviewPosition::loadWindowPositionSettings(const QUrl &url)
{
    m_settingsKey = url.toString(QUrl::PreferLocalFile) + QLatin1String("_lastpostion");

    if (m_settings.contains(m_settingsKey)) {
        m_hasPosition = true;
        readLastPositionFromByteArray(m_settings.value(m_settingsKey).toByteArray());
    }
}

void QQmlPreviewPosition::initLastSavedWindowPosition(QWindow *window)
{
    Q_ASSERT(window);
    m_initializeState = PositionInitialized;
    if (m_currentInitScreensData.isEmpty())
        m_currentInitScreensData = initScreensData();
    // if it is the first time we just use the fall back from a last shown qml file
    if (!m_hasPosition) {
        if (!m_settings.contains(QLatin1String("global_lastpostion")))
            return;
        readLastPositionFromByteArray(m_settings.value(QLatin1String("global_lastpostion"))
                                              .toByteArray());
    }
    setPosition(m_lastWindowPosition, window);
}

QByteArray QQmlPreviewPosition::fromPositionToByteArray(
        const QQmlPreviewPosition::Position &position)
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);

    const quint16 majorVersion = 1;
    const quint16 minorVersion = 0;

    stream << majorVersion
           << minorVersion
           << m_currentInitScreensData
           << position.screenName
           << position.nativePosition;
    return array;
}

void QQmlPreviewPosition::readLastPositionFromByteArray(const QByteArray &array)
{
    QDataStream stream(array);
    stream.setVersion(QDataStream::Qt_5_12);

    // no version check for 1.0
    //const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;

    QVector<ScreenData> initScreensData;
    stream >> initScreensData;

    if (m_currentInitScreensData != initScreensData)
        return;

    QString nameOfScreen;
    stream >> nameOfScreen;

    QScreen *screen = findScreen(nameOfScreen);
    if (!screen)
        return;

    QPoint nativePosition;
    stream >> nativePosition;
    if (nativePosition.isNull())
        return;
    m_lastWindowPosition = {nameOfScreen, nativePosition};
}

void QQmlPreviewPosition::setPosition(const QQmlPreviewPosition::Position &position,
                                      QWindow *window)
{
    if (position.nativePosition.isNull())
        return;
    if (QScreen *screen = findScreen(position.screenName)) {
        window->setScreen(screen);
        window->setFramePosition(QHighDpiScaling::mapPositionFromNative(position.nativePosition,
                                                                        screen->handle()));
    }
}

QT_END_NAMESPACE
