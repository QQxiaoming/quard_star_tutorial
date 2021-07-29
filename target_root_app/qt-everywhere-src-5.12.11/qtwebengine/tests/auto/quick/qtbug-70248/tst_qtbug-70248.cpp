/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "qtwebengineglobal.h"
#include <QQuickWebEngineProfile>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTest>
#include <QSignalSpy>

class tst_qtbug_70248: public QObject {
    Q_OBJECT
public:
    tst_qtbug_70248(){}
private slots:
    void test();
};

void tst_qtbug_70248::test()
{
    QtWebEngine::initialize();
    QScopedPointer<QQmlApplicationEngine> engine;
    QQuickWebEngineProfile::defaultProfile()->setOffTheRecord(true);
    engine.reset(new QQmlApplicationEngine());
    engine->load(QUrl(QStringLiteral("qrc:/test.qml")));
    QQuickWindow *widnow = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
    QVERIFY(widnow);
}

#include "tst_qtbug-70248.moc"
QTEST_MAIN(tst_qtbug_70248)

