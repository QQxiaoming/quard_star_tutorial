/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#ifndef QQMLPROFILERSERVICE_P_H
#define QQMLPROFILERSERVICE_P_H

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

#include <private/qqmlconfigurabledebugservice_p.h>
#include <private/qqmldebugserviceinterfaces_p.h>
#include <private/qqmlprofilerdefinitions_p.h>
#include <private/qqmlabstractprofileradapter_p.h>
#include <private/qqmlboundsignal_p.h>
#include <private/qqmldebugconnector_p.h>
#include <private/qversionedpacket_p.h>

#include <QtCore/qelapsedtimer.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qmutex.h>
#include <QtCore/qvector.h>
#include <QtCore/qstringbuilder.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qtimer.h>

#include <limits>

QT_BEGIN_NAMESPACE

class QUrl;
using QQmlDebugPacket = QVersionedPacket<QQmlDebugConnector>;

class QQmlProfilerServiceImpl :
        public QQmlConfigurableDebugService<QQmlProfilerService>,
        public QQmlProfilerDefinitions
{
    Q_OBJECT
public:

    void engineAboutToBeAdded(QJSEngine *engine) override;
    void engineAboutToBeRemoved(QJSEngine *engine) override;
    void engineAdded(QJSEngine *engine) override;
    void engineRemoved(QJSEngine *engine) override;

    void addGlobalProfiler(QQmlAbstractProfilerAdapter *profiler) override;
    void removeGlobalProfiler(QQmlAbstractProfilerAdapter *profiler) override;

    void startProfiling(QJSEngine *engine,
                        quint64 features = std::numeric_limits<quint64>::max()) override;
    void stopProfiling(QJSEngine *engine) override;

    QQmlProfilerServiceImpl(QObject *parent = 0);
    ~QQmlProfilerServiceImpl() override;

    void dataReady(QQmlAbstractProfilerAdapter *profiler) override;

signals:
    void startFlushTimer();
    void stopFlushTimer();

protected:
    void stateAboutToBeChanged(State state) override;
    void messageReceived(const QByteArray &) override;

private:
    friend class QQmlProfilerServiceFactory;

    void sendMessages();
    void addEngineProfiler(QQmlAbstractProfilerAdapter *profiler, QJSEngine *engine);
    void removeProfilerFromStartTimes(const QQmlAbstractProfilerAdapter *profiler);
    void flush();

    QElapsedTimer m_timer;
    QTimer m_flushTimer;
    bool m_waitingForStop;

    bool m_globalEnabled;
    quint64 m_globalFeatures;

    QList<QQmlAbstractProfilerAdapter *> m_globalProfilers;
    QMultiHash<QJSEngine *, QQmlAbstractProfilerAdapter *> m_engineProfilers;
    QList<QJSEngine *> m_stoppingEngines;
    QMultiMap<qint64, QQmlAbstractProfilerAdapter *> m_startTimes;
};

QT_END_NAMESPACE

#endif // QQMLPROFILERSERVICE_P_H

