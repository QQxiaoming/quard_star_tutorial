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

#include "qqmllistmodelworkeragent_p.h"
#include "qqmllistmodel_p_p.h"
#include <private/qqmldata_p.h>
#include <private/qqmlengine_p.h>
#include <qqmlinfo.h>

#include <QtCore/qcoreevent.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>


QT_BEGIN_NAMESPACE

QQmlListModelWorkerAgent::Sync::~Sync()
{
}

QQmlListModelWorkerAgent::QQmlListModelWorkerAgent(QQmlListModel *model)
: m_ref(1), m_orig(model), m_copy(new QQmlListModel(model, this))
{
}

QQmlListModelWorkerAgent::~QQmlListModelWorkerAgent()
{
    mutex.lock();
    syncDone.wakeAll();
    mutex.unlock();
}

void QQmlListModelWorkerAgent::setEngine(QV4::ExecutionEngine *eng)
{
    m_copy->m_engine = eng;
}

void QQmlListModelWorkerAgent::addref()
{
    m_ref.ref();
}

void QQmlListModelWorkerAgent::release()
{
    bool del = !m_ref.deref();

    if (del)
        deleteLater();
}

void QQmlListModelWorkerAgent::modelDestroyed()
{
    m_orig = nullptr;
}

int QQmlListModelWorkerAgent::count() const
{
    return m_copy->count();
}

void QQmlListModelWorkerAgent::clear()
{
    m_copy->clear();
}

void QQmlListModelWorkerAgent::remove(QQmlV4Function *args)
{
    m_copy->remove(args);
}

void QQmlListModelWorkerAgent::append(QQmlV4Function *args)
{
    m_copy->append(args);
}

void QQmlListModelWorkerAgent::insert(QQmlV4Function *args)
{
    m_copy->insert(args);
}

QQmlV4Handle QQmlListModelWorkerAgent::get(int index) const
{
    return m_copy->get(index);
}

void QQmlListModelWorkerAgent::set(int index, const QQmlV4Handle &value)
{
    m_copy->set(index, value);
}

void QQmlListModelWorkerAgent::setProperty(int index, const QString& property, const QVariant& value)
{
    m_copy->setProperty(index, property, value);
}

void QQmlListModelWorkerAgent::move(int from, int to, int count)
{
    m_copy->move(from, to, count);
}

void QQmlListModelWorkerAgent::sync()
{
    Sync *s = new Sync(m_copy);

    mutex.lock();
    QCoreApplication::postEvent(this, s);
    syncDone.wait(&mutex);
    mutex.unlock();
}

bool QQmlListModelWorkerAgent::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        bool cc = false;
        QMutexLocker locker(&mutex);
        if (m_orig) {
            Sync *s = static_cast<Sync *>(e);

            cc = (m_orig->count() != s->list->count());

            Q_ASSERT(m_orig->m_dynamicRoles == s->list->m_dynamicRoles);
            if (m_orig->m_dynamicRoles)
                QQmlListModel::sync(s->list, m_orig);
            else
                ListModel::sync(s->list->m_listModel, m_orig->m_listModel);
        }

        syncDone.wakeAll();
        locker.unlock();

        if (cc)
            emit m_orig->countChanged();
        return true;
    }

    return QObject::event(e);
}

QT_END_NAMESPACE

#include "moc_qqmllistmodelworkeragent_p.cpp"
