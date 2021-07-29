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

#include <QtTest/QtTest>
#include <QModelIndex>

// Helper class which can be used by tests for starting a task and
// waiting for its completion. It takes care of running an event
// loop while waiting, until finished() method is called (or the
// timeout is reached).
class WaitHelper : public QObject
{
    Q_OBJECT

public:
    WaitHelper() { m_promise.reportStarted(); }

    ~WaitHelper()
    {
        if (m_promise.future().isRunning())
            m_promise.reportFinished();
    }

    /*
        Starts an event loop and waits until finish() method is called
        or the timeout is reached.
    */
    bool wait(int timeout = 30000)
    {
        if (m_promise.future().isFinished())
            return true;

        QFutureWatcher<void> watcher;
        QSignalSpy watcherSpy(&watcher, &QFutureWatcher<void>::finished);
        watcher.setFuture(m_promise.future());
        return watcherSpy.wait(timeout);
    }

protected:
    /*
        The derived classes need to call this method to stop waiting.
    */
    void finish() { m_promise.reportFinished(); }

private:
    QFutureInterface<void> m_promise;
};

namespace {

inline bool compareIndices(const QModelIndex &lhs, const QModelIndex &rhs)
{
    QModelIndex left = lhs;
    QModelIndex right = rhs;
    while (left.row() == right.row() && left.column() == right.column() && left.isValid() && right.isValid()) {
        left = left.parent();
        right = right.parent();
    }
    if (left.isValid() || right.isValid())
        return false;
    return true;
}

struct WaitForDataChanged : public WaitHelper
{
    WaitForDataChanged(const QAbstractItemModel *model, const QVector<QModelIndex> &pending)
        : WaitHelper(), m_model(model), m_pending(pending)
    {
        connect(m_model, &QAbstractItemModel::dataChanged, this,
                [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                       const QVector<int> &roles) {
                    Q_UNUSED(roles)

                    checkAndRemoveRange(topLeft, bottomRight);
                    if (m_pending.isEmpty())
                        finish();
                });
    }

    void checkAndRemoveRange(const QModelIndex &topLeft, const QModelIndex &bottomRight)
    {
        QVERIFY(topLeft.parent() == bottomRight.parent());
        QVector<QModelIndex>  toRemove;
        for (int i = 0; i < m_pending.size(); ++i) {
            const QModelIndex &pending = m_pending.at(i);
            if (pending.isValid()  && compareIndices(pending.parent(), topLeft.parent())) {
                const bool fitLeft = topLeft.column() <= pending.column();
                const bool fitRight = bottomRight.column() >= pending.column();
                const bool fitTop = topLeft.row() <= pending.row();
                const bool fitBottom = bottomRight.row() >= pending.row();
                if (fitLeft && fitRight && fitTop && fitBottom)
                    toRemove.append(pending);
            }
        }
        foreach (const QModelIndex &index, toRemove) {
            const int ind = m_pending.indexOf(index);
            m_pending.remove(ind);
        }
    }

private:
    const QAbstractItemModel *m_model = nullptr;
    QVector<QModelIndex> m_pending;
};

} // namespace
