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
#include <QLoggingCategory>
#include <QQmlComponent>

#include <private/qv4mm_p.h>
#include <private/qv4qobjectwrapper_p.h>
#include <private/qjsvalue_p.h>

#include "../../shared/util.h"

#include <memory>

class tst_qv4mm : public QQmlDataTest
{
    Q_OBJECT

private slots:
    void gcStats();
    void multiWrappedQObjects();
    void accessParentOnDestruction();
    void clearICParent();
};

void tst_qv4mm::gcStats()
{
    QLoggingCategory::setFilterRules("qt.qml.gc.*=true");
    QQmlEngine engine;
    engine.collectGarbage();
}

void tst_qv4mm::multiWrappedQObjects()
{
    QV4::ExecutionEngine engine1;
    QV4::ExecutionEngine engine2;
    {
        QObject object;
        for (int i = 0; i < 10; ++i)
            QV4::QObjectWrapper::wrap(i % 2 ? &engine1 : &engine2, &object);

        QCOMPARE(engine1.memoryManager->m_pendingFreedObjectWrapperValue.size(), 0);
        QCOMPARE(engine2.memoryManager->m_pendingFreedObjectWrapperValue.size(), 0);
        {
            QV4::WeakValue value;
            value.set(&engine1, QV4::QObjectWrapper::wrap(&engine1, &object));
        }

        QCOMPARE(engine1.memoryManager->m_pendingFreedObjectWrapperValue.size(), 1);
        QCOMPARE(engine2.memoryManager->m_pendingFreedObjectWrapperValue.size(), 0);

        // Moves the additional WeakValue from m_multiplyWrappedQObjects to
        // m_pendingFreedObjectWrapperValue. It's still alive after all.
        engine1.memoryManager->runGC();
        QCOMPARE(engine1.memoryManager->m_pendingFreedObjectWrapperValue.size(), 2);

        // engine2 doesn't own the object as engine1 was the first to wrap it above.
        // Therefore, no effect here.
        engine2.memoryManager->runGC();
        QCOMPARE(engine2.memoryManager->m_pendingFreedObjectWrapperValue.size(), 0);
    }

    // Clears m_pendingFreedObjectWrapperValue. Now it's really dead.
    engine1.memoryManager->runGC();
    QCOMPARE(engine1.memoryManager->m_pendingFreedObjectWrapperValue.size(), 0);

    engine2.memoryManager->runGC();
    QCOMPARE(engine2.memoryManager->m_pendingFreedObjectWrapperValue.size(), 0);
}

void tst_qv4mm::accessParentOnDestruction()
{
    QLoggingCategory::setFilterRules("qt.qml.gc.*=false");
    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl("createdestroy.qml"));
    std::unique_ptr<QObject> obj(component.create());
    QVERIFY(obj);
    QPointer<QObject> timer = qvariant_cast<QObject *>(obj->property("timer"));
    QVERIFY(timer);
    QTRY_VERIFY(!timer->property("running").toBool());
    QCOMPARE(obj->property("iterations").toInt(), 100);
    QCOMPARE(obj->property("creations").toInt(), 100);
    QCOMPARE(obj->property("destructions").toInt(), 100);
}

void tst_qv4mm::clearICParent()
{
    QV4::ExecutionEngine engine;
    QV4::Scope scope(engine.rootContext());
    QV4::ScopedObject object(scope, engine.newObject());

    // Keep identifiers in a separate array so that we don't have to allocate them in the loop that
    // should test the GC on InternalClass allocations.
    QV4::ScopedArrayObject identifiers(scope, engine.newArrayObject());
    for (uint i = 0; i < 16 * 1024; ++i) {
        QV4::Scope scope(&engine);
        QV4::ScopedString s(scope);
        s = engine.newIdentifier(QString::fromLatin1("key%1").arg(i));
        identifiers->push_back(s);

        QV4::ScopedValue v(scope);
        v->setDouble(i);
        object->insertMember(s, v);
    }

    // When allocating the InternalClass objects required for deleting properties, the GC should
    // eventually run and remove all but the last two.
    // If we ever manage to avoid allocating the InternalClasses in the first place we will need
    // to change this test.
    for (uint i = 0; i < 16 * 1024; ++i) {
        QV4::Scope scope(&engine);
        QV4::ScopedString s(scope, identifiers->getIndexed(i));
        QV4::Scoped<QV4::InternalClass> ic(scope, object->internalClass());
        QVERIFY(ic->d()->parent != nullptr);
        object->deleteProperty(s->toPropertyKey());
        QVERIFY(object->internalClass() != ic->d());
        QCOMPARE(object->internalClass()->parent, ic->d());
        if (ic->d()->parent == nullptr)
            return;
    }
    QFAIL("Garbage collector was not triggered by large amount of InternalClasses");
}

QTEST_MAIN(tst_qv4mm)

#include "tst_qv4mm.moc"
