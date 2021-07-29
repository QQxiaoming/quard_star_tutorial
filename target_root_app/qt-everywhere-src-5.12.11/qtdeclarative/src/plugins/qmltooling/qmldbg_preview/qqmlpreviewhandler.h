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

#ifndef QQMLPREVIEWHANDLER_H
#define QQMLPREVIEWHANDLER_H

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

#include "qqmlpreviewposition.h"

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qrect.h>
#include <QtCore/qpointer.h>
#include <QtCore/qelapsedtimer.h>
#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

class QQmlEngine;
class QQuickItem;
class QQmlPreviewUrlInterceptor;
class QQuickWindow;
class QTranslator;

class QQmlPreviewHandler : public QObject
{
    Q_OBJECT
public:
    explicit QQmlPreviewHandler(QObject *parent = nullptr);
    ~QQmlPreviewHandler();

    void addEngine(QQmlEngine *engine);
    void removeEngine(QQmlEngine *engine);

    void loadUrl(const QUrl &url);
    void rerun();
    void zoom(qreal newFactor);
    void language(const QUrl &context, const QLocale &locale);

    void clear();

    struct FpsInfo {
        quint16 numSyncs;
        quint16 minSync;
        quint16 maxSync;
        quint16 totalSync;

        quint16 numRenders;
        quint16 minRender;
        quint16 maxRender;
        quint16 totalRender;
    };

signals:
    void error(const QString &message);
    void fps(const FpsInfo &info);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    void doZoom();
    void tryCreateObject();
    void showObject(QObject *object);
    void setCurrentWindow(QQuickWindow *window);

    void beforeSynchronizing();
    void afterSynchronizing();
    void beforeRendering();
    void frameSwapped();

    void fpsTimerHit();
    void removeTranslators();

    QScopedPointer<QQuickItem> m_dummyItem;
    QList<QQmlEngine *> m_engines;
    QVector<QPointer<QObject>> m_createdObjects;
    QScopedPointer<QQmlComponent> m_component;
    QPointer<QQuickWindow> m_currentWindow;
    qreal m_zoomFactor = 1.0;
    bool m_supportsMultipleWindows;
    QQmlPreviewPosition m_lastPosition;

    QTimer m_fpsTimer;

    struct FrameTime {
        void beginFrame();
        void recordFrame();
        void endFrame();
        void reset();

        QElapsedTimer timer;
        qint64 elapsed = -1;
        quint16 min = std::numeric_limits<quint16>::max();
        quint16 max = 0;
        quint16 total = 0;
        quint16 number = 0;
    };

    FrameTime m_rendering;
    FrameTime m_synchronizing;

    QScopedPointer<QTranslator> m_qtTranslator;
    QScopedPointer<QTranslator> m_qmlTranslator;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QQmlPreviewHandler::FpsInfo)

#endif // QQMLPREVIEWHANDLER_H
