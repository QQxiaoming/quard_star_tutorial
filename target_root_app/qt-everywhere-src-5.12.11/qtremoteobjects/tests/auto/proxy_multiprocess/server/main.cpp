/****************************************************************************
**
** Copyright (C) 2019 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
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

#include "mytestserver.h"
#include "rep_subclass_source.h"
#include "../shared.h"

#include <QCoreApplication>
#include <QtTest/QtTest>

class tst_Server_Process : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        const auto objectMode = qEnvironmentVariable("ObjectMode");
        bool templated = qEnvironmentVariableIsSet("TEMPLATED_REMOTING");

        qDebug() << "Starting tests:" << objectMode << "templated =" << templated;
        QRemoteObjectRegistryHost srcNode(QUrl(QStringLiteral("local:testRegistry")));

        MyTestServer parent;
        SubClassSimpleSource subclass;
        subclass.setMyPOD(initialValue);
        subclass.setI(initialI);
        QStringListModel model;
        model.setStringList(QStringList() << "Track1" << "Track2" << "Track3");
        if (objectMode == QLatin1Literal("ObjectPointer")) {
            parent.setSubClass(&subclass);
            parent.setTracks(&model);
            parent.setMyEnum(ParentClassSource::bar);
            parent.setDate(Qt::SystemLocaleShortDate);
            parent.setNsEnum(NS::Bravo);
            parent.setNs2Enum(NS2::NamespaceEnum::Bravo);
            parent.setVariant(QVariant::fromValue(42.0f));
        }

        if (templated)
            srcNode.enableRemoting<ParentClassSourceAPI>(&parent);
        else
            srcNode.enableRemoting(&parent);

        qDebug() << "Waiting for incoming connections";

        QSignalSpy waitForStartedSpy(&parent, &MyTestServer::startedChanged);
        QVERIFY(waitForStartedSpy.isValid());
        QVERIFY(waitForStartedSpy.wait());
        QCOMPARE(waitForStartedSpy.value(0).value(0).toBool(), true);

        // wait for delivery of events
        QTest::qWait(200);

        //Change SubClass and make sure change propagates
        SubClassSimpleSource updatedSubclass;
        updatedSubclass.setMyPOD(updatedValue);
        updatedSubclass.setI(updatedI);
        parent.setSubClass(&updatedSubclass);
        if (objectMode == QLatin1Literal("NullPointer"))
            parent.setTracks(&model);
        parent.setMyEnum(ParentClassSource::foobar);
        parent.setDate(Qt::SystemLocaleLongDate);
        parent.setNsEnum(NS::Charlie);
        parent.setNs2Enum(NS2::NamespaceEnum::Charlie);
        parent.setVariant(QVariant::fromValue(podValue));
        emit parent.enum2(ParentClassSource::foo, ParentClassSource::bar);

        emit parent.advance();

        // wait for quit
        bool quit = false;
        connect(&parent, &MyTestServer::quitApp, [&quit]{quit = true;});
        QTRY_VERIFY_WITH_TIMEOUT(quit, 5000);

        // wait for delivery of events
        QTest::qWait(200);

        qDebug() << "Done. Shutting down.";
    }
};

QTEST_MAIN(tst_Server_Process)

#include "main.moc"
