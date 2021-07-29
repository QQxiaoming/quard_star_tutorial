/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "qquickwebenginetestsupport_p.h"

#include "qquickwebengineloadrequest_p.h"
#include <QQuickWindow>
#include <QtTest/qtest.h>
#include <QtCore/QTimer>

QT_BEGIN_NAMESPACE

namespace QTest {
    int Q_TESTLIB_EXPORT defaultMouseDelay();
}

QQuickWebEngineErrorPage::QQuickWebEngineErrorPage()
{
}

void QQuickWebEngineErrorPage::loadFinished(bool success, const QUrl &url)
{
    Q_UNUSED(success);
    QTimer::singleShot(0, this, [this, url]() {
       QQuickWebEngineLoadRequest loadRequest(url, QQuickWebEngineView::LoadSucceededStatus);
       emit loadingChanged(&loadRequest);
    });
}

void QQuickWebEngineErrorPage::loadStarted(const QUrl &provisionalUrl)
{
    QTimer::singleShot(0, this, [this, provisionalUrl]() {
        QQuickWebEngineLoadRequest loadRequest(provisionalUrl, QQuickWebEngineView::LoadStartedStatus);
        emit loadingChanged(&loadRequest);
    });
}

QQuickWebEngineTestInputContext::QQuickWebEngineTestInputContext()
    : m_visible(false)
{
}

QQuickWebEngineTestInputContext::~QQuickWebEngineTestInputContext()
{
    release();
}

void QQuickWebEngineTestInputContext::create()
{
    QInputMethodPrivate *inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
    inputMethodPrivate->testContext = this;
}

void QQuickWebEngineTestInputContext::release()
{
    QInputMethodPrivate *inputMethodPrivate = QInputMethodPrivate::get(qApp->inputMethod());
    inputMethodPrivate->testContext = 0;
}

void QQuickWebEngineTestInputContext::showInputPanel()
{
    m_visible = true;
}

void QQuickWebEngineTestInputContext::hideInputPanel()
{
    m_visible = false;
}

bool QQuickWebEngineTestInputContext::isInputPanelVisible() const
{
    return m_visible;
}


QQuickWebEngineTestEvent::QQuickWebEngineTestEvent()
{
}

bool QQuickWebEngineTestEvent::mouseMultiClick(QObject *item, qreal x, qreal y, int clickCount)
{
    QTEST_ASSERT(item);

    QWindow *view = eventWindow(item);
    if (!view)
        return false;

    for (int i = 0; i < clickCount; ++i) {
        mouseEvent(QMouseEvent::MouseButtonPress, view, item, QPointF(x, y));
        mouseEvent(QMouseEvent::MouseButtonRelease, view, item, QPointF(x, y));
    }
    QTest::lastMouseTimestamp += QTest::mouseDoubleClickInterval;

    return true;
}

QWindow *QQuickWebEngineTestEvent::eventWindow(QObject *item)
{
    QWindow *window = qobject_cast<QWindow *>(item);
    if (window)
        return window;

    QQuickItem *quickItem = qobject_cast<QQuickItem *>(item);
    if (quickItem)
        return quickItem->window();

    QQuickItem *testParentItem = qobject_cast<QQuickItem *>(parent());
    if (testParentItem)
        return testParentItem->window();

    return nullptr;
}

void QQuickWebEngineTestEvent::mouseEvent(QEvent::Type type, QWindow *window, QObject *item, const QPointF &_pos)
{
    QTest::qWait(QTest::defaultMouseDelay());
    QTest::lastMouseTimestamp += QTest::defaultMouseDelay();

    QPoint pos;
    QQuickItem *sgitem = qobject_cast<QQuickItem *>(item);
    if (sgitem)
        pos = sgitem->mapToScene(_pos).toPoint();

    QMouseEvent me(type, pos, window->mapFromGlobal(pos), Qt::LeftButton, Qt::LeftButton, 0);
    me.setTimestamp(++QTest::lastMouseTimestamp);

    QSpontaneKeyEvent::setSpontaneous(&me);
    if (!qApp->notify(window, &me))
        QTest::qWarn("Mouse click event not accepted by receiving window");
}


QQuickWebEngineTestSupport::QQuickWebEngineTestSupport()
    : m_errorPage(new QQuickWebEngineErrorPage)
    , m_testInputContext(new QQuickWebEngineTestInputContext)
    , m_testEvent(new QQuickWebEngineTestEvent)
{
}

QQuickWebEngineErrorPage *QQuickWebEngineTestSupport::errorPage() const
{
    return m_errorPage.data();
}

QQuickWebEngineTestInputContext *QQuickWebEngineTestSupport::testInputContext() const
{
    return m_testInputContext.data();
}

QQuickWebEngineTestEvent * QQuickWebEngineTestSupport::testEvent() const
{
    return m_testEvent.data();
}

QT_END_NAMESPACE

#include "moc_qquickwebenginetestsupport_p.cpp"
