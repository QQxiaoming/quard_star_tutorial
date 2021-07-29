/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company
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

#include <QString>
#include <QtTest>
#include "rep_class_merged.h"

class SubClassReplicaTest : public QObject
{
    Q_OBJECT

public:
    SubClassReplicaTest();

private Q_SLOTS:
    void basicFunctions();
    void basicFunctions_data();
};

SubClassReplicaTest::SubClassReplicaTest()
{
}

void SubClassReplicaTest::basicFunctions_data()
{
    QTest::addColumn<bool>("templated");
    QTest::addColumn<bool>("nullobject");
    QTest::newRow("non-templated pointer") << false << false;
    QTest::newRow("templated pointer") << true << false;
    QTest::newRow("non-templated nullptr") << false << true;
    QTest::newRow("templated nullptr") << true << true;
}

void SubClassReplicaTest::basicFunctions()
{
    QFETCH(bool, templated);
    QFETCH(bool, nullobject);

    QRemoteObjectRegistryHost host(QUrl("local:test"));
    SubClassSimpleSource subclass1, subclass2;
    ParentClassSimpleSource parent;
    parent.setSub1(&subclass1);
    if (nullobject)
        parent.setSub2(nullptr);
    else
        parent.setSub2(&subclass2);
    if (templated)
        host.enableRemoting<ParentClassSourceAPI>(&parent);
    else
        host.enableRemoting(&parent);

    QRemoteObjectNode client(QUrl("local:test"));
    const QScopedPointer<ParentClassReplica> replica(client.acquire<ParentClassReplica>());
    QVERIFY(replica->waitForSource(1000));

    auto sub1 = replica->sub1();
    QSignalSpy spy(sub1, &SubClassReplica::valueChanged);
    subclass1.setValue(10);
    QVERIFY(spy.wait());
    QCOMPARE(subclass1.value(), sub1->value());
    if (nullobject) {
        QCOMPARE(replica->sub2(), nullptr);
        QCOMPARE(parent.sub2(), nullptr);
    } else
        QCOMPARE(subclass2.value(), replica->sub2()->value());
}

QTEST_MAIN(SubClassReplicaTest)

#include "tst_subclassreplicatest.moc"
