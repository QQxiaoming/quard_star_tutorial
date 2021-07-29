/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
#include <QtQml/private/qqmlobjectmodel_p.h>
#include <QtQml/private/qqmlchangeset_p.h>
#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

class tst_QQmlObjectModel : public QObject
{
    Q_OBJECT

private slots:
    void changes();
};

static bool compareItems(QQmlObjectModel *model, const QObjectList &items)
{
    for (int i = 0; i < items.count(); ++i) {
        if (model->get(i) != items.at(i))
            return false;
    }
    return true;
}

static bool verifyChangeSet(const QQmlChangeSet &changeSet, int expectedInserts, int expectedRemoves, bool isMove, int moveId = -1)
{
    int actualRemoves = 0;
    for (const QQmlChangeSet::Change &r : changeSet.removes()) {
        if (r.isMove() != isMove && (!isMove || moveId == r.moveId))
            return false;
        actualRemoves += r.count;
    }

    int actualInserts = 0;
    for (const QQmlChangeSet::Change &i : changeSet.inserts()) {
        if (i.isMove() != isMove && (!isMove || moveId == i.moveId))
            return false;
        actualInserts += i.count;
    }

    return actualRemoves == expectedRemoves && actualInserts == expectedInserts;
}

Q_DECLARE_METATYPE(QQmlChangeSet)

void tst_QQmlObjectModel::changes()
{
    QQmlObjectModel model;

    qRegisterMetaType<QQmlChangeSet>();

    QSignalSpy countSpy(&model, SIGNAL(countChanged()));
    QSignalSpy childrenSpy(&model, SIGNAL(childrenChanged()));
    QSignalSpy modelUpdateSpy(&model, SIGNAL(modelUpdated(QQmlChangeSet,bool)));

    int count = 0;
    int countSignals = 0;
    int childrenSignals = 0;
    int modelUpdateSignals = 0;

    QObjectList items;
    QObject item0, item1, item2, item3;

    // append(item0) -> [item0]
    model.append(&item0); items.append(&item0);
    QCOMPARE(model.count(), ++count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 1, 0, false));

    // insert(0, item1) -> [item1, item0]
    model.insert(0, &item1); items.insert(0, &item1);
    QCOMPARE(model.count(), ++count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 1, 0, false));

    // append(item2) -> [item1, item0, item2]
    model.append(&item2); items.append(&item2);
    QCOMPARE(model.count(), ++count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 1, 0, false));

    // insert(2, item3) -> [item1, item0, item3, item2]
    model.insert(2, &item3); items.insert(2, &item3);
    QCOMPARE(model.count(), ++count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 1, 0, false));

    // move(0, 1) -> [item0, item1, item3, item2]
    model.move(0, 1); items.move(0, 1);
    QCOMPARE(model.count(), count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 1, 1, true, 1));

    // move(3, 2) -> [item0, item1, item2, item3]
    model.move(3, 2); items.move(3, 2);
    QCOMPARE(model.count(), count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 1, 1, true, 2));

    // remove(0) -> [item1, item2, item3]
    model.remove(0); items.removeAt(0);
    QCOMPARE(model.count(), --count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 0, 1, false));

    // remove(2) -> [item1, item2]
    model.remove(2); items.removeAt(2);
    QCOMPARE(model.count(), --count);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 0, 1, false));

    // clear() -> []
    model.clear(); items.clear();
    QCOMPARE(model.count(), 0);
    QVERIFY(compareItems(&model, items));
    QCOMPARE(countSpy.count(), ++countSignals);
    QCOMPARE(childrenSpy.count(), ++childrenSignals);
    QCOMPARE(modelUpdateSpy.count(), ++modelUpdateSignals);
    QVERIFY(verifyChangeSet(modelUpdateSpy.last().first().value<QQmlChangeSet>(), 0, 2, false));
}

QTEST_MAIN(tst_QQmlObjectModel)

#include "tst_qqmlobjectmodel.moc"
