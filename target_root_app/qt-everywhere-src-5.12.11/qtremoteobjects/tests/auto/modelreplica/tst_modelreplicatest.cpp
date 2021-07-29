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
#include "rep_model_merged.h"

class ModelreplicaTest : public QObject
{
    Q_OBJECT

public:
    ModelreplicaTest() = default;

private Q_SLOTS:
    void basicFunctions();
    void basicFunctions_data();
    void nullModel();
};

void ModelreplicaTest::basicFunctions_data()
{
    QTest::addColumn<bool>("templated");
    QTest::newRow("non-templated enableRemoting") << false;
    QTest::newRow("templated enableRemoting") << true;
}

void ModelreplicaTest::basicFunctions()
{
    QFETCH(bool, templated);

    QRemoteObjectRegistryHost host(QUrl("tcp://localhost:5555"));
    auto model = new QStringListModel();
    model->setStringList(QStringList() << "Track1" << "Track2" << "Track3");
    MediaSimpleSource source;
    source.setTracks(model);
    if (templated)
        host.enableRemoting<MediaSourceAPI>(&source);
    else
        host.enableRemoting(&source);

    QRemoteObjectNode client(QUrl("tcp://localhost:5555"));
    const QScopedPointer<MediaReplica> replica(client.acquire<MediaReplica>());
    QSignalSpy tracksSpy(replica->tracks(), &QAbstractItemModelReplica::initialized);
    QVERIFY(replica->waitForSource(100));
    QVERIFY(tracksSpy.wait());
    // Rep file only uses display role
    QCOMPARE(QVector<int>{Qt::DisplayRole}, replica->tracks()->availableRoles());

    QCOMPARE(model->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++)
    {
        // We haven't received any data yet
        QCOMPARE(QVariant(), replica->tracks()->data(replica->tracks()->index(i, 0)));
    }

    // Wait for data to be fetch and confirm
    QTest::qWait(100);
    QCOMPARE(model->rowCount(), replica->tracks()->rowCount());
    for (int i = 0; i < replica->tracks()->rowCount(); i++)
    {
        QCOMPARE(model->data(model->index(i), Qt::DisplayRole), replica->tracks()->data(replica->tracks()->index(i, 0)));
    }
}

void ModelreplicaTest::nullModel()
{
    QRemoteObjectRegistryHost host(QUrl("tcp://localhost:5555"));
    MediaSimpleSource source;
    host.enableRemoting(&source);

    QRemoteObjectNode client(QUrl("tcp://localhost:5555"));
    const QScopedPointer<MediaReplica> replica(client.acquire<MediaReplica>());
    QVERIFY(replica->waitForSource(100));

    auto model = new QStringListModel(this);
    model->setStringList(QStringList() << "Track1" << "Track2" << "Track3");
    source.setTracks(model);

    QTRY_VERIFY(replica->tracks());
    QTRY_COMPARE(replica->tracks()->rowCount(), 3);
    QTRY_COMPARE(replica->tracks()->data(replica->tracks()->index(0, 0)), "Track1");

    model = new QStringListModel(this);
    model->setStringList(QStringList() << "New Track1" << "New Track2" << "New Track3"  << "New Track4");
    source.setTracks(model);
    QTRY_COMPARE(replica->tracks()->rowCount(), 4);
    QTRY_COMPARE(replica->tracks()->data(replica->tracks()->index(3, 0)), "New Track4");
}

QTEST_MAIN(ModelreplicaTest)

#include "tst_modelreplicatest.moc"
