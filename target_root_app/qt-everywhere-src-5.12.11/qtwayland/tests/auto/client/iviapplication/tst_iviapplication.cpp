/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "mockcompositor.h"

#include <QWindow>

#include <QtTest/QtTest>

static const QSize screenSize(1600, 1200);

class TestWindow : public QWindow
{
public:
    TestWindow()
    {
        setSurfaceType(QSurface::RasterSurface);
        setGeometry(0, 0, 32, 32);
        create();
    }
};

class tst_WaylandClientIviApplication : public QObject
{
    Q_OBJECT
public:
    tst_WaylandClientIviApplication(MockCompositor *c)
        : m_compositor(c)
    {
        QSocketNotifier *notifier = new QSocketNotifier(m_compositor->waylandFileDescriptor(), QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &tst_WaylandClientIviApplication::processWaylandEvents);
        // connect to the event dispatcher to make sure to flush out the outgoing message queue
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::awake, this, &tst_WaylandClientIviApplication::processWaylandEvents);
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this, &tst_WaylandClientIviApplication::processWaylandEvents);
    }

public slots:
    void processWaylandEvents()
    {
        m_compositor->processWaylandEvents();
    }

    void cleanup()
    {
        // make sure the surfaces from the last test are properly cleaned up
        // and don't show up as false positives in the next test
        QTRY_VERIFY(!m_compositor->surface());
        QTRY_VERIFY(!m_compositor->iviSurface());
    }

private slots:
    void createDestroyWindow();
    void configure();
    void uniqueIviIds();

private:
    MockCompositor *m_compositor = nullptr;
};

void tst_WaylandClientIviApplication::createDestroyWindow()
{
    TestWindow window;
    window.show();

    QTRY_VERIFY(m_compositor->surface());
    QTRY_VERIFY(m_compositor->iviSurface());

    window.destroy();
    QTRY_VERIFY(!m_compositor->surface());
    QTRY_VERIFY(!m_compositor->iviSurface());
}

void tst_WaylandClientIviApplication::configure()
{
    TestWindow window;
    window.show();

    QSharedPointer<MockIviSurface> iviSurface;
    QTRY_VERIFY(iviSurface = m_compositor->iviSurface());

    // Unconfigured ivi surfaces decide their own size
    QTRY_COMPARE(window.frameGeometry(), QRect(QPoint(), QSize(32, 32)));

    m_compositor->sendIviSurfaceConfigure(iviSurface, {123, 456});
    QTRY_COMPARE(window.frameGeometry(), QRect(QPoint(), QSize(123, 456)));
}

void tst_WaylandClientIviApplication::uniqueIviIds()
{
    TestWindow windowA, windowB;
    windowA.show();
    windowB.show();

    QSharedPointer<MockIviSurface> iviSurface0, iviSurface1;
    QTRY_VERIFY(iviSurface0 = m_compositor->iviSurface(0));
    QTRY_VERIFY(iviSurface1 = m_compositor->iviSurface(1));
    QTRY_VERIFY(iviSurface0->iviId != iviSurface1->iviId);
}

int main(int argc, char **argv)
{
    setenv("XDG_RUNTIME_DIR", ".", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin
    setenv("QT_WAYLAND_SHELL_INTEGRATION", "ivi-shell", 1);
    setenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1", 1); // window decorations don't make much sense on ivi-application

    MockCompositor compositor;
    compositor.setOutputMode(screenSize);

    QGuiApplication app(argc, argv);
    compositor.applicationInitialized();

    tst_WaylandClientIviApplication tc(&compositor);
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_iviapplication.moc>
