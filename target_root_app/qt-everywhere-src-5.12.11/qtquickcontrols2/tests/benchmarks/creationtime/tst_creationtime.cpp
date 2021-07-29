/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml>
#include <QtTest>

#include "../../auto/shared/visualtestutil.h"

using namespace QQuickVisualTestUtil;

class tst_CreationTime : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void controls();
    void controls_data();

    void fusion();
    void fusion_data();

    void imagine();
    void imagine_data();

    void material();
    void material_data();

    void universal();
    void universal_data();

    void calendar();
    void calendar_data();

private:
    QQmlEngine engine;
};

void tst_CreationTime::init()
{
    engine.clearComponentCache();
}

static void doBenchmark(QQmlEngine *engine, const QUrl &url)
{
    QQmlComponent component(engine);
    component.loadUrl(url);

    QObjectList objects;
    objects.reserve(4096);
    QBENCHMARK {
        QObject *object = component.create();
        QVERIFY2(object, qPrintable(component.errorString()));
        objects += object;
    }
    qDeleteAll(objects);
}

void tst_CreationTime::controls()
{
    QFETCH(QUrl, url);
    doBenchmark(&engine, url);
}

void tst_CreationTime::controls_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(&engine, "controls", "QtQuick/Controls.2", QStringList() << "ApplicationWindow");
}

void tst_CreationTime::fusion()
{
    QFETCH(QUrl, url);
    doBenchmark(&engine, url);
}

void tst_CreationTime::fusion_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(&engine, "controls/fusion", "QtQuick/Controls.2/Fusion", QStringList() << "ApplicationWindow" << "ButtonPanel" << "CheckIndicator" << "RadioIndicator" << "SliderGroove" << "SliderHandle" << "SwitchIndicator");
}

void tst_CreationTime::imagine()
{
    QFETCH(QUrl, url);
    doBenchmark(&engine, url);
}

void tst_CreationTime::imagine_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(&engine, "controls/imagine", "QtQuick/Controls.2/Imagine", QStringList() << "ApplicationWindow");
}

void tst_CreationTime::material()
{
    QFETCH(QUrl, url);
    doBenchmark(&engine, url);
}

void tst_CreationTime::material_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(&engine, "controls/material", "QtQuick/Controls.2/Material", QStringList() << "ApplicationWindow" << "Ripple" << "SliderHandle" << "CheckIndicator" << "RadioIndicator" << "SwitchIndicator" << "BoxShadow" << "ElevationEffect" << "CursorDelegate");
}

void tst_CreationTime::universal()
{
    QFETCH(QUrl, url);
    doBenchmark(&engine, url);
}

void tst_CreationTime::universal_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(&engine, "controls/universal", "QtQuick/Controls.2/Universal", QStringList() << "ApplicationWindow" << "CheckIndicator" << "RadioIndicator" << "SwitchIndicator");
}

void tst_CreationTime::calendar()
{
    QFETCH(QUrl, url);
    doBenchmark(&engine, url);
}

void tst_CreationTime::calendar_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(&engine, "calendar", "Qt/labs/calendar");
}

QTEST_MAIN(tst_CreationTime)

#include "tst_creationtime.moc"
