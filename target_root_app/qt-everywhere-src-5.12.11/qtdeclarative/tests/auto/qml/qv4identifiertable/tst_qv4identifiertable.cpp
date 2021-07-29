/****************************************************************************
**
** Copyright (C) 2016 basysKom GmbH.
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

#include <qtest.h>
#include <QQmlEngine>
#include <private/qv4identifiertable_p.h>

class tst_qv4identifiertable : public QObject
{
    Q_OBJECT

private slots:
    void sweepFirstEntryInBucket();
    void sweepCenterEntryInBucket();
    void sweepLastEntryInBucket();
    void sweepFirstEntryInSameBucketWithDifferingHash();
    void dontSweepAcrossBucketBoundaries();
    void sweepAcrossBucketBoundariesIfFirstBucketFull();
    void sweepBucketGap();
};

void tst_qv4identifiertable::sweepFirstEntryInBucket()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/1);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));
    auto entry3 = engine.newString(QStringLiteral("three"));

    entry1->createHashValue();
    entry2->createHashValue();
    entry3->createHashValue();

    // All strings go into the same bucket
    entry1->stringHash = 0;
    entry2->stringHash = 0;
    entry3->stringHash = 0;

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);
    table.asPropertyKey(entry3);

    QCOMPARE(table.size, 3);
    QCOMPARE(table.alloc, 5);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], entry3);
    QCOMPARE(table.entriesByHash[3], nullptr);

    // first entry not marked
    entry2->setMarkBit();
    entry3->setMarkBit();

    table.sweep();

    QCOMPARE(table.entriesByHash[0], entry2);
    QCOMPARE(table.entriesByHash[1], entry3);
    QCOMPARE(table.entriesByHash[2], nullptr);
    QCOMPARE(table.entriesByHash[3], nullptr);
}

void tst_qv4identifiertable::sweepCenterEntryInBucket()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/1);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));
    auto entry3 = engine.newString(QStringLiteral("three"));

    entry1->createHashValue();
    entry2->createHashValue();
    entry3->createHashValue();

    // All strings go into the same bucket
    entry1->stringHash = 0;
    entry2->stringHash = 0;
    entry3->stringHash = 0;

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);
    table.asPropertyKey(entry3);

    QCOMPARE(table.size, 3);
    QCOMPARE(table.alloc, 5);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], entry3);
    QCOMPARE(table.entriesByHash[3], nullptr);

    entry1->setMarkBit();
    // second entry not marked
    entry3->setMarkBit();

    table.sweep();

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry3);
    QCOMPARE(table.entriesByHash[2], nullptr);
    QCOMPARE(table.entriesByHash[3], nullptr);
}

void tst_qv4identifiertable::sweepLastEntryInBucket()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/1);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));
    auto entry3 = engine.newString(QStringLiteral("three"));

    entry1->createHashValue();
    entry2->createHashValue();
    entry3->createHashValue();

    // All strings go into the same bucket
    entry1->stringHash = 0;
    entry2->stringHash = 0;
    entry3->stringHash = 0;

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);
    table.asPropertyKey(entry3);

    QCOMPARE(table.size, 3);
    QCOMPARE(table.alloc, 5);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], entry3);
    QCOMPARE(table.entriesByHash[3], nullptr);

    entry1->setMarkBit();
    entry2->setMarkBit();
    // third entry not marked

    table.sweep();

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], nullptr);
    QCOMPARE(table.entriesByHash[3], nullptr);
}

void tst_qv4identifiertable::sweepFirstEntryInSameBucketWithDifferingHash()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/1);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));

    entry1->createHashValue();
    entry2->createHashValue();

    // First and second entry have differing hash but end up in the
    // same bucket after modulo alloc.
    entry1->stringHash = 0;
    entry2->stringHash = 5;

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);

    QCOMPARE(table.size, 2);
    QCOMPARE(table.alloc, 5);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], nullptr);

    // first entry not marked
    entry2->setMarkBit();

    table.sweep();

    QCOMPARE(table.entriesByHash[0], entry2);
    QCOMPARE(table.entriesByHash[1], nullptr);
    QCOMPARE(table.entriesByHash[2], nullptr);
}

void tst_qv4identifiertable::dontSweepAcrossBucketBoundaries()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/1);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));
    auto entry3 = engine.newString(QStringLiteral("three"));

    entry1->createHashValue();
    entry2->createHashValue();
    entry3->createHashValue();

    // Different buckets for both entries.
    entry1->stringHash = 0;
    entry2->stringHash = 1;

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);

    QCOMPARE(table.size, 2);
    QCOMPARE(table.alloc, 5);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], nullptr);

    // first entry not marked
    entry2->setMarkBit();

    table.sweep();

    QCOMPARE(table.entriesByHash[0], nullptr);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], nullptr);
}

void tst_qv4identifiertable::sweepAcrossBucketBoundariesIfFirstBucketFull()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/3);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));
    auto entry3 = engine.newString(QStringLiteral("three"));
    auto entry4 = engine.newString(QStringLiteral("four"));

    entry1->createHashValue();
    entry2->createHashValue();
    entry3->createHashValue();
    entry4->createHashValue();

    // First, third and fourth entry have the same bucket (after modulo) and
    // would typically end up in order [entry1, entry3, entry4, entry2]. However
    // since null termination isn't guaranteed, an insertion order of
    // entry1, entry2, entry3 and entry4 results in a
    // table [entry1, entry2, entry3, entry4].
    entry1->stringHash = 0;
    entry2->stringHash = 1;
    entry3->stringHash = 11;
    entry4->stringHash = 11;

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);
    table.asPropertyKey(entry3);
    table.asPropertyKey(entry4);

    QCOMPARE(table.size, 4);
    QCOMPARE(table.alloc, 11);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], entry3);
    QCOMPARE(table.entriesByHash[3], entry4);
    QCOMPARE(table.entriesByHash[4], nullptr);

    // first entry not marked
    entry2->setMarkBit();
    entry3->setMarkBit();
    entry4->setMarkBit();

    table.sweep();

    QCOMPARE(table.entriesByHash[0], entry3);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], entry4);
    QCOMPARE(table.entriesByHash[3], nullptr);
}

void tst_qv4identifiertable::sweepBucketGap()
{
    QV4::ExecutionEngine engine;
    QV4::IdentifierTable table(&engine, /*numBits*/3);

    auto entry1 = engine.newString(QStringLiteral("one"));
    auto entry2 = engine.newString(QStringLiteral("two"));
    auto entry3 = engine.newString(QStringLiteral("three"));
    auto entry4 = engine.newString(QStringLiteral("four"));

    entry1->createHashValue();
    entry2->createHashValue();
    entry3->createHashValue();
    entry4->createHashValue();

    // We have two buckets where the second entry in the first bucket
    // flows into the second bucket. So insertion into the second bucket
    // will shift by one and create
    // [entry1][entry2 (would map to first but overflows into second), entry3, entry4]
    // Garbage collecting the first entry should not only move the second entry
    // into its own first bucket (where there is space now), it is also critical to
    // not leave a gap but move the third and fourth entries to the beginning of
    // their bucket:
    // [entry2][entry3, entry4]
    entry1->stringHash = 0;  // % 11 -> 0
    entry2->stringHash = 11; // % 11 -> 0, but ends up at idx 1 because 0 taken
    entry3->stringHash = 12; // % 11 -> 1, but ends up at idx 2 because 1 taken
    entry4->stringHash = 12; // % 11 -> 1, but ends up at idx 3 because 1+2 taken

    // trigger insertion
    table.asPropertyKey(entry1);
    table.asPropertyKey(entry2);
    table.asPropertyKey(entry3);
    table.asPropertyKey(entry4);

    QCOMPARE(table.size, 4);
    QCOMPARE(table.alloc, 11);

    QCOMPARE(table.entriesByHash[0], entry1);
    QCOMPARE(table.entriesByHash[1], entry2);
    QCOMPARE(table.entriesByHash[2], entry3);
    QCOMPARE(table.entriesByHash[3], entry4);
    QCOMPARE(table.entriesByHash[4], nullptr);

    // first entry not marked
    entry2->setMarkBit();
    entry3->setMarkBit();
    entry4->setMarkBit();

    table.sweep();

    QCOMPARE(table.entriesByHash[0], entry2);
    QCOMPARE(table.entriesByHash[1], entry3);
    QCOMPARE(table.entriesByHash[2], entry4);
    QCOMPARE(table.entriesByHash[3], nullptr);
}

QTEST_MAIN(tst_qv4identifiertable)

#include "tst_qv4identifiertable.moc"
