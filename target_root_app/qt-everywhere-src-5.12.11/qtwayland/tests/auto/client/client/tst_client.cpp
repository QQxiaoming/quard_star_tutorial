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

#include "mockcompositor.h"

#include <QBackingStore>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include <QMimeData>
#include <QPixmap>
#include <QDrag>
#include <QWindow>
#include <QOpenGLWindow>

#include <QtTest/QtTest>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtGui/private/qguiapplication_p.h>

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

    void focusInEvent(QFocusEvent *) override
    {
        ++focusInEventCount;
    }

    void focusOutEvent(QFocusEvent *) override
    {
        ++focusOutEventCount;
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        ++keyPressEventCount;
        keyCode = event->nativeScanCode();
    }

    void keyReleaseEvent(QKeyEvent *event) override
    {
        ++keyReleaseEventCount;
        keyCode = event->nativeScanCode();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        ++mousePressEventCount;
        mousePressPos = event->pos();
    }

    void mouseReleaseEvent(QMouseEvent *) override
    {
        ++mouseReleaseEventCount;
    }

    void touchEvent(QTouchEvent *event) override
    {
        Q_UNUSED(event);
        ++touchEventCount;
    }

    QPoint frameOffset() const { return QPoint(frameMargins().left(), frameMargins().top()); }

    int focusInEventCount = 0;
    int focusOutEventCount = 0;
    int keyPressEventCount = 0;
    int keyReleaseEventCount = 0;
    int mousePressEventCount = 0;
    int mouseReleaseEventCount = 0;
    int touchEventCount = 0;

    uint keyCode = 0;
    QPoint mousePressPos;
};

class TestGlWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    TestGlWindow();
    int paintGLCalled = 0;

public slots:
    void hideShow();

protected:
    void paintGL() override;
};

TestGlWindow::TestGlWindow()
{}

void TestGlWindow::hideShow()
{
    setVisible(false);
    setVisible(true);
}

void TestGlWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    ++paintGLCalled;
}

class tst_WaylandClient : public QObject
{
    Q_OBJECT
public:
    tst_WaylandClient(MockCompositor *c)
        : compositor(c)
    {
        QSocketNotifier *notifier = new QSocketNotifier(compositor->waylandFileDescriptor(), QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(processWaylandEvents()));
        // connect to the event dispatcher to make sure to flush out the outgoing message queue
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::awake, this, &tst_WaylandClient::processWaylandEvents);
        connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this, &tst_WaylandClient::processWaylandEvents);
    }

public slots:
    void processWaylandEvents()
    {
        compositor->processWaylandEvents();
    }

    void cleanup()
    {
        // make sure the surfaces from the last test are properly cleaned up
        // and don't show up as false positives in the next test
        QTRY_VERIFY(!compositor->surface());
        QTRY_VERIFY(!compositor->iviSurface());
        QTRY_VERIFY(!compositor->xdgToplevelV6());
    }

private slots:
    void primaryScreen();
    void screens();
    void addScreenWithGeometryChange();
    void windowScreens();
    void removePrimaryScreen();
    void createDestroyWindow();
    void activeWindowFollowsKeyboardFocus();
    void events();
    void backingStore();
    void touchDrag();
    void mouseDrag();
    void dontCrashOnMultipleCommits();
    void hiddenTransientParent();
    void hiddenPopupParent();
    void glWindow();
    void longWindowTitle();
    void longWindowTitleWithUtf16Characters();

private:
    MockCompositor *compositor = nullptr;
};

void tst_WaylandClient::primaryScreen()
{
    compositor->setOutputMode(screenSize);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->size(), screenSize);
}

void tst_WaylandClient::screens()
{
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    compositor->sendAddOutput();
    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QSharedPointer<MockOutput> secondOutput;
    QTRY_VERIFY(secondOutput = compositor->output(1));
    compositor->sendRemoveOutput(secondOutput);
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
}

//QTBUG-62044
void tst_WaylandClient::addScreenWithGeometryChange()
{
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    const QRect oldGeometry = QGuiApplication::primaryScreen()->geometry();
    compositor->sendAddOutput();

    // Move the primary screen to the right
    const QRect newGeometry(QPoint(screenSize.width(), 0), screenSize);
    Q_ASSERT(oldGeometry != newGeometry);
    compositor->sendOutputGeometry(compositor->output(0), newGeometry);

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->geometry(), newGeometry);

    compositor->sendRemoveOutput(compositor->output(1));
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);

    // Move the screen back
    compositor->sendOutputGeometry(compositor->output(0), oldGeometry);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->geometry(), oldGeometry);
}

void tst_WaylandClient::windowScreens()
{
    QSharedPointer<MockOutput> firstOutput;
    QTRY_VERIFY(firstOutput = compositor->output());

    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QScreen *primaryScreen = QGuiApplication::screens().first();
    QCOMPARE(window.screen(), primaryScreen);

    compositor->sendAddOutput();

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QScreen *secondaryScreen = QGuiApplication::screens().at(1);
    QVERIFY(secondaryScreen);

    window.setScreen(secondaryScreen);
    QCOMPARE(window.screen(), secondaryScreen);

    QSharedPointer<MockOutput> secondOutput;
    QTRY_VERIFY(secondOutput = compositor->output(1));
    compositor->sendSurfaceEnter(surface, firstOutput);

    compositor->sendSurfaceEnter(surface, secondOutput);
    QTRY_COMPARE(window.screen(), primaryScreen);

    compositor->sendSurfaceLeave(surface, firstOutput);
    QTRY_COMPARE(window.screen(), secondaryScreen);

    compositor->sendRemoveOutput(secondOutput);
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QCOMPARE(window.screen(), primaryScreen);
}

void tst_WaylandClient::removePrimaryScreen()
{
    QSharedPointer<MockOutput> firstOutput;
    QTRY_VERIFY(firstOutput = compositor->output());

    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);
    QScreen *primaryScreen = QGuiApplication::screens().first();
    QCOMPARE(window.screen(), primaryScreen);

    compositor->sendAddOutput();

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    QTRY_COMPARE(QGuiApplication::primaryScreen()->virtualSiblings().size(), 2);
    QScreen *secondaryScreen = QGuiApplication::screens().at(1);
    QVERIFY(secondaryScreen);

    compositor->sendRemoveOutput(firstOutput);
    QTRY_COMPARE(QGuiApplication::screens().size(), 1);

    compositor->sendMousePress(surface, window.frameOffset() + QPoint(10, 10));
    QTRY_COMPARE(window.mousePressEventCount, 1);
    compositor->sendMouseRelease(surface);
    QTRY_COMPARE(window.mouseReleaseEventCount, 1);
}

void tst_WaylandClient::createDestroyWindow()
{
    TestWindow window;
    window.show();

    QTRY_VERIFY(compositor->surface());

    window.destroy();
    QTRY_VERIFY(!compositor->surface());
}

void tst_WaylandClient::activeWindowFollowsKeyboardFocus()
{
    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    QTRY_VERIFY(window.isExposed());

    if (compositor->xdgToplevelV6())
        QSKIP("On xdg-shell v6 focus is handled by configure events");

    QCOMPARE(window.focusInEventCount, 0);
    compositor->setKeyboardFocus(surface);
    QTRY_COMPARE(window.focusInEventCount, 1);
    QTRY_COMPARE(QGuiApplication::focusWindow(), &window);

    QCOMPARE(window.focusOutEventCount, 0);
    compositor->setKeyboardFocus(QSharedPointer<MockSurface>(nullptr));
    QTRY_COMPARE(window.focusOutEventCount, 1);
    QTRY_COMPARE(QGuiApplication::focusWindow(), static_cast<QWindow *>(nullptr));
}

void tst_WaylandClient::events()
{
    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    QTRY_VERIFY(window.isExposed());

    compositor->setKeyboardFocus(surface);
    QTRY_COMPARE(window.focusInEventCount, 1);
    QTRY_COMPARE(QGuiApplication::focusWindow(), &window);

    uint keyCode = 80; // arbitrarily chosen
    QCOMPARE(window.keyPressEventCount, 0);
    compositor->sendKeyPress(surface, keyCode);
    QTRY_COMPARE(window.keyPressEventCount, 1);
    QTRY_COMPARE(window.keyCode, keyCode);

    QCOMPARE(window.keyReleaseEventCount, 0);
    compositor->sendKeyRelease(surface, keyCode);
    QTRY_COMPARE(window.keyReleaseEventCount, 1);
    QCOMPARE(window.keyCode, keyCode);

    const int touchId = 0;
    compositor->sendTouchDown(surface, window.frameOffset() + QPoint(10, 10), touchId);
    // Note: wl_touch.frame should not be the last event in a test until QTBUG-66563 is fixed.
    // See also: QTBUG-66537
    compositor->sendTouchFrame(surface);
    QTRY_COMPARE(window.touchEventCount, 1);

    compositor->sendTouchUp(surface, touchId);
    compositor->sendTouchFrame(surface);
    QTRY_COMPARE(window.touchEventCount, 2);

    QPoint mousePressPos(16, 16);
    QCOMPARE(window.mousePressEventCount, 0);
    compositor->sendMousePress(surface, window.frameOffset() + mousePressPos);
    QTRY_COMPARE(window.mousePressEventCount, 1);
    QTRY_COMPARE(window.mousePressPos, mousePressPos);

    QCOMPARE(window.mouseReleaseEventCount, 0);
    compositor->sendMouseRelease(surface);
    QTRY_COMPARE(window.mouseReleaseEventCount, 1);
}

void tst_WaylandClient::backingStore()
{
    TestWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    QRect rect(QPoint(), window.size());

    QBackingStore backingStore(&window);
    backingStore.resize(rect.size());

    backingStore.beginPaint(rect);

    QColor color = Qt::magenta;

    QPainter p(backingStore.paintDevice());
    p.fillRect(rect, color);
    p.end();

    backingStore.endPaint();

    QVERIFY(surface->image.isNull());

    backingStore.flush(rect);

    QTRY_COMPARE(surface->image.size(), window.frameGeometry().size());
    QTRY_COMPARE(surface->image.pixel(window.frameMargins().left(), window.frameMargins().top()), color.rgba());

    window.hide();

    // hiding the window should destroy the surface
    QTRY_VERIFY(!compositor->surface());
}

class DndWindow : public QWindow
{
    Q_OBJECT

public:
    DndWindow(QWindow *parent = nullptr)
        : QWindow(parent)
    {
        QImage cursorImage(64,64,QImage::Format_ARGB32);
        cursorImage.fill(Qt::blue);
        m_dragIcon = QPixmap::fromImage(cursorImage);
    }
    ~DndWindow() override{}
    QPoint frameOffset() const { return QPoint(frameMargins().left(), frameMargins().top()); }
    bool dragStarted = false;

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        Q_UNUSED(event);
        if (dragStarted)
            return;
        dragStarted = true;

        QByteArray dataBytes;
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-dnditemdata", dataBytes);
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(m_dragIcon);
        drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
    }
private:
    QPixmap m_dragIcon;
};

void tst_WaylandClient::touchDrag()
{
    DndWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    compositor->setKeyboardFocus(surface);
    QTRY_COMPARE(QGuiApplication::focusWindow(), &window);

    const int id = 0;
    compositor->sendTouchDown(surface, window.frameOffset() + QPoint(10, 10), id);
    compositor->sendTouchFrame(surface);
    compositor->sendTouchMotion(surface, window.frameOffset() + QPoint(20, 20), id);
    compositor->sendTouchFrame(surface);
    compositor->waitForStartDrag();
    compositor->sendDataDeviceDataOffer(surface);
    compositor->sendDataDeviceEnter(surface, window.frameOffset() + QPoint(20, 20));
    compositor->sendDataDeviceMotion(window.frameOffset() + QPoint(21, 21));
    compositor->sendDataDeviceDrop(surface);
    compositor->sendDataDeviceLeave(surface);
    QTRY_VERIFY(window.dragStarted);
}

void tst_WaylandClient::mouseDrag()
{
    DndWindow window;
    window.show();

    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    compositor->setKeyboardFocus(surface);
    QTRY_COMPARE(QGuiApplication::focusWindow(), &window);

    compositor->sendMousePress(surface, window.frameOffset() + QPoint(10, 10));
    compositor->sendDataDeviceDataOffer(surface);
    compositor->sendDataDeviceEnter(surface, window.frameOffset() + QPoint(20, 20));
    compositor->sendDataDeviceMotion(window.frameOffset() + QPoint(21, 21));
    compositor->waitForStartDrag();
    compositor->sendDataDeviceDrop(surface);
    compositor->sendDataDeviceLeave(surface);
    QTRY_VERIFY(window.dragStarted);
}

void tst_WaylandClient::dontCrashOnMultipleCommits()
{
    QSKIP("This test is flaky. See QTBUG-68756.");
    auto window = new TestWindow();
    window->show();

    QRect rect(QPoint(), window->size());

    {
        QBackingStore backingStore(window);
        backingStore.resize(rect.size());
        backingStore.beginPaint(rect);
        QPainter p(backingStore.paintDevice());
        p.fillRect(rect, Qt::magenta);
        p.end();
        backingStore.endPaint();

        backingStore.flush(rect);
        backingStore.flush(rect);
        backingStore.flush(rect);

        compositor->processWaylandEvents();
    }

    delete window;
}

void tst_WaylandClient::hiddenTransientParent()
{
    QWindow parent;
    QWindow transient;

    transient.setTransientParent(&parent);

    parent.show();
    QTRY_VERIFY(compositor->surface());

    parent.hide();
    QTRY_VERIFY(!compositor->surface());

    transient.show();
    QTRY_VERIFY(compositor->surface());
}

void tst_WaylandClient::hiddenPopupParent()
{
    TestWindow toplevel;
    toplevel.show();

    // wl_shell relies on a mouse event in order to send a serial and seat
    // with the set_popup request.
    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);
    QPoint mousePressPos(16, 16);
    QCOMPARE(toplevel.mousePressEventCount, 0);
    compositor->sendMousePress(surface, toplevel.frameOffset() + mousePressPos);
    QTRY_COMPARE(toplevel.mousePressEventCount, 1);

    QWindow popup;
    popup.setTransientParent(&toplevel);
    popup.setFlag(Qt::Popup, true);

    toplevel.hide();
    QTRY_VERIFY(!compositor->surface());

    popup.show();
    QTRY_VERIFY(compositor->surface());
}

void tst_WaylandClient::glWindow()
{
    QSKIP("Skipping GL tests, as not supported by all CI systems: See https://bugreports.qt.io/browse/QTBUG-65802");

    QScopedPointer<TestGlWindow> testWindow(new TestGlWindow);
    testWindow->show();
    QSharedPointer<MockSurface> surface;
    QTRY_VERIFY(surface = compositor->surface());
    compositor->sendShellSurfaceConfigure(surface);

    QTRY_COMPARE(testWindow->paintGLCalled, 1);

    //QTBUG-63411
    QMetaObject::invokeMethod(testWindow.data(), "hideShow", Qt::QueuedConnection);
    testWindow->requestUpdate();
    QTRY_COMPARE(testWindow->paintGLCalled, 2);

    testWindow->requestUpdate();
    QTRY_COMPARE(testWindow->paintGLCalled, 3);

    //confirm we don't crash when we delete an already hidden GL window
    //QTBUG-65553
    testWindow->setVisible(false);
    QTRY_VERIFY(!compositor->surface());
}

void tst_WaylandClient::longWindowTitle()
{
    // See QTBUG-68715
    QWindow window;
    QString absurdlyLongTitle(10000, QLatin1Char('z'));
    window.setTitle(absurdlyLongTitle);
    window.show();
    QTRY_VERIFY(compositor->surface());
}

void tst_WaylandClient::longWindowTitleWithUtf16Characters()
{
    QWindow window;
    QString absurdlyLongTitle = QString("三").repeated(10000);
    Q_ASSERT(absurdlyLongTitle.length() == 10000); // just making sure the test isn't broken
    window.setTitle(absurdlyLongTitle);
    window.show();
    QTRY_VERIFY(compositor->surface());
}

int main(int argc, char **argv)
{
    setenv("XDG_RUNTIME_DIR", ".", 1);
    setenv("QT_QPA_PLATFORM", "wayland", 1); // force QGuiApplication to use wayland plugin

    MockCompositor compositor;
    compositor.setOutputMode(screenSize);

    QGuiApplication app(argc, argv);

    // Initializing some client buffer integrations (i.e. eglInitialize) may block while waiting
    // for a wayland sync. So we call clientBufferIntegration prior to applicationInitialized
    // (while the compositor processes events without waiting) in order to avoid hanging later.
    auto *waylandIntegration = static_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
    waylandIntegration->clientBufferIntegration();

    compositor.applicationInitialized();

    tst_WaylandClient tc(&compositor);
    return QTest::qExec(&tc, argc, argv);
}

#include <tst_client.moc>
