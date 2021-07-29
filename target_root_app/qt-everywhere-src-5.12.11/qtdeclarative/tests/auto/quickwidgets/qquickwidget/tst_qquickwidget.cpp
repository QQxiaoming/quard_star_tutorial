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

#include <qtest.h>
#include <qtesttouch.h>
#include <QtTest/QSignalSpy>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlcontext.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>
#include "../../shared/util.h"
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#include <QtGui/QImage>
#include <QtCore/QDebug>
#include <QtQml/qqmlengine.h>

#include <QtCore/QLoggingCategory>
#include <QtGui/qstylehints.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>

#include <QtQuickWidgets/QQuickWidget>

Q_LOGGING_CATEGORY(lcTests, "qt.quick.tests")

class MouseRecordingQQWidget : public QQuickWidget
{
public:
    explicit MouseRecordingQQWidget(QWidget *parent = nullptr) : QQuickWidget(parent) {
        setAttribute(Qt::WA_AcceptTouchEvents);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        qCDebug(lcTests) << event;
        m_mouseEvents << *event;
        QQuickWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        qCDebug(lcTests) << event;
        m_mouseEvents << *event;
        QQuickWidget::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        qCDebug(lcTests) << event;
        m_mouseEvents << *event;
        QQuickWidget::mouseReleaseEvent(event);
    }

public:
    QList<QMouseEvent> m_mouseEvents;
};

class MouseRecordingItem : public QQuickItem
{
public:
    MouseRecordingItem(bool acceptTouch, QQuickItem *parent = nullptr)
        : QQuickItem(parent)
        , m_acceptTouch(acceptTouch)
    {
        setSize(QSizeF(300, 300));
        setAcceptedMouseButtons(Qt::LeftButton);
    }

protected:
    void touchEvent(QTouchEvent* event) override {
        event->setAccepted(m_acceptTouch);
        m_touchEvents << *event;
        qCDebug(lcTests) << "accepted?" << event->isAccepted() << event;
    }
    void mousePressEvent(QMouseEvent *event) override {
        qCDebug(lcTests) << event;
        m_mouseEvents << *event;
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        qCDebug(lcTests) << event;
        m_mouseEvents << *event;
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        qCDebug(lcTests) << event;
        m_mouseEvents << *event;
    }

public:
    QList<QMouseEvent> m_mouseEvents;
    QList<QTouchEvent> m_touchEvents;

private:
    bool m_acceptTouch;
};

class tst_qquickwidget : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_qquickwidget();

private slots:
    void showHide();
    void reparentAfterShow();
    void changeGeometry();
    void resizemodeitem();
    void layoutSizeChange();
    void errors();
    void engine();
    void readback();
    void renderingSignals();
    void grab();
    void grabBeforeShow();
    void reparentToNewWindow();
    void nullEngine();
    void keyEvents();
    void shortcuts();
    void enterLeave();
    void mouseEventWindowPos();
    void synthMouseFromTouch_data();
    void synthMouseFromTouch();
    void tabKey();

private:
    QTouchDevice *device = QTest::createTouchDevice();
    const QRect m_availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
};

tst_qquickwidget::tst_qquickwidget()
{
}

void tst_qquickwidget::showHide()
{
    QWidget window;

    QQuickWidget *childView = new QQuickWidget(&window);
    childView->setSource(testFileUrl("rectangle.qml"));

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QVERIFY(!childView->quickWindow()->isVisible()); // this window is always not visible see QTBUG-65761
    QVERIFY(childView->quickWindow()->visibility() != QWindow::Hidden);

    window.hide();
    QVERIFY(!childView->quickWindow()->isVisible());
    QCOMPARE(childView->quickWindow()->visibility(), QWindow::Hidden);
}

void tst_qquickwidget::reparentAfterShow()
{
    QWidget window;

    QQuickWidget *childView = new QQuickWidget(&window);
    childView->setSource(testFileUrl("rectangle.qml"));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QScopedPointer<QQuickWidget> toplevelView(new QQuickWidget);
    toplevelView->setParent(&window);
    toplevelView->setSource(testFileUrl("rectangle.qml"));
    toplevelView->show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
}

void tst_qquickwidget::changeGeometry()
{
    QWidget window;

    QQuickWidget *childView = new QQuickWidget(&window);
    childView->setSource(testFileUrl("rectangle.qml"));

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    childView->setGeometry(100,100,100,100);
}

void tst_qquickwidget::resizemodeitem()
{
    QWidget window;
    window.setGeometry(m_availableGeometry.left(), m_availableGeometry.top(), 400, 400);

    QScopedPointer<QQuickWidget> view(new QQuickWidget);
    view->setParent(&window);
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    QCOMPARE(QSize(0,0), view->initialSize());
    view->setSource(testFileUrl("resizemodeitem.qml"));
    QQuickItem* item = qobject_cast<QQuickItem*>(view->rootObject());
    QVERIFY(item);
    window.show();

    view->showNormal();
    // initial size from root object
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 200.0);
    QCOMPARE(view->size(), QSize(200, 200));
    QCOMPARE(view->size(), view->sizeHint());
    QCOMPARE(view->size(), view->initialSize());

    // size update from view
    view->resize(QSize(80,100));

    QTRY_COMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QCOMPARE(view->size(), QSize(80, 100));
    QCOMPARE(view->size(), view->sizeHint());

    view->setResizeMode(QQuickWidget::SizeViewToRootObject);

    // size update from view disabled
    view->resize(QSize(60,80));
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QTRY_COMPARE(view->size(), QSize(60, 80));

    // size update from root object
    item->setWidth(250);
    item->setHeight(350);
    QCOMPARE(item->width(), 250.0);
    QCOMPARE(item->height(), 350.0);
    QTRY_COMPARE(view->size(), QSize(250, 350));
    QCOMPARE(view->size(), QSize(250, 350));
    QCOMPARE(view->size(), view->sizeHint());

    // reset window
    window.hide();
    view.reset(new QQuickWidget(&window));
    view->setResizeMode(QQuickWidget::SizeViewToRootObject);
    view->setSource(testFileUrl("resizemodeitem.qml"));
    item = qobject_cast<QQuickItem*>(view->rootObject());
    QVERIFY(item);
    window.show();

    view->showNormal();

    // initial size for root object
    QCOMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 200.0);
    QCOMPARE(view->size(), view->sizeHint());
    QCOMPARE(view->size(), view->initialSize());

    // size update from root object
    item->setWidth(80);
    item->setHeight(100);
    QCOMPARE(item->width(), 80.0);
    QCOMPARE(item->height(), 100.0);
    QTRY_COMPARE(view->size(), QSize(80, 100));
    QCOMPARE(view->size(), view->sizeHint());

    // size update from root object disabled
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    item->setWidth(60);
    item->setHeight(80);
    QCOMPARE(view->width(), 80);
    QCOMPARE(view->height(), 100);
    QCOMPARE(QSize(item->width(), item->height()), view->sizeHint());

    // size update from view
    view->resize(QSize(200,300));
    QTRY_COMPARE(item->width(), 200.0);
    QCOMPARE(item->height(), 300.0);
    QCOMPARE(view->size(), QSize(200, 300));
    QCOMPARE(view->size(), view->sizeHint());

    window.hide();

    // if we set a specific size for the view then it should keep that size
    // for SizeRootObjectToView mode.
    view.reset(new QQuickWidget(&window));
    view->resize(300, 300);
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    QCOMPARE(QSize(0,0), view->initialSize());
    view->setSource(testFileUrl("resizemodeitem.qml"));
    view->resize(300, 300);
    item = qobject_cast<QQuickItem*>(view->rootObject());
    QVERIFY(item);
    window.show();

    view->showNormal();

    // initial size from root object
    QCOMPARE(item->width(), 300.0);
    QCOMPARE(item->height(), 300.0);
    QTRY_COMPARE(view->size(), QSize(300, 300));
    QCOMPARE(view->size(), view->sizeHint());
    QCOMPARE(view->initialSize(), QSize(200, 200)); // initial object size
}

void tst_qquickwidget::layoutSizeChange()
{
    QWidget window;
    window.resize(400, 400);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    QScopedPointer<QQuickWidget> view(new QQuickWidget);
    layout->addWidget(view.data());
    QLabel *label = new QLabel("Label");
    layout->addWidget(label);
    layout->addStretch(1);


    view->resize(300,300);
    view->setResizeMode(QQuickWidget::SizeViewToRootObject);
    QCOMPARE(QSize(0,0), view->initialSize());
    view->setSource(testFileUrl("rectangle.qml"));
    QQuickItem* item = qobject_cast<QQuickItem*>(view->rootObject());
    QVERIFY(item);
    QCOMPARE(item->height(), 200.0);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window, 5000));
    QTRY_COMPARE(view->height(), 200);
    QTRY_COMPARE(label->y(), 200);

    item->setSize(QSizeF(100,100));
    QCOMPARE(item->height(), 100.0);
    QTRY_COMPARE(view->height(), 100);
    QTRY_COMPARE(label->y(), 100);
}

void tst_qquickwidget::errors()
{
    QQuickWidget *view = new QQuickWidget;
    QScopedPointer<QQuickWidget> cleanupView(view);
    QVERIFY(view->errors().isEmpty()); // don't crash

    QQmlTestMessageHandler messageHandler;
    view->setSource(testFileUrl("error1.qml"));
    QCOMPARE(view->status(), QQuickWidget::Error);
    QCOMPARE(view->errors().count(), 1);
}

void tst_qquickwidget::engine()
{
    QScopedPointer<QQmlEngine> engine(new QQmlEngine);
    QScopedPointer<QQuickWidget> view(new QQuickWidget(engine.data(), nullptr));
    QScopedPointer<QQuickWidget> view2(new QQuickWidget(view->engine(), nullptr));

    QVERIFY(view->engine());
    QVERIFY(view2->engine());
    QCOMPARE(view->engine(), view2->engine());
}

void tst_qquickwidget::readback()
{
    QWidget window;

    QScopedPointer<QQuickWidget> view(new QQuickWidget);
    view->setSource(testFileUrl("rectangle.qml"));

    view->show();
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QImage img = view->grabFramebuffer();
    QVERIFY(!img.isNull());
    QCOMPARE(img.width(), view->width());
    QCOMPARE(img.height(), view->height());

    QRgb pix = img.pixel(5, 5);
    QCOMPARE(pix, qRgb(255, 0, 0));
}

void tst_qquickwidget::renderingSignals()
{
    QQuickWidget widget;
    QQuickWindow *window = widget.quickWindow();
    QVERIFY(window);

    QSignalSpy beforeRenderingSpy(window, &QQuickWindow::beforeRendering);
    QSignalSpy beforeSyncSpy(window, &QQuickWindow::beforeSynchronizing);
    QSignalSpy afterRenderingSpy(window, &QQuickWindow::afterRendering);

    QVERIFY(beforeRenderingSpy.isValid());
    QVERIFY(beforeSyncSpy.isValid());
    QVERIFY(afterRenderingSpy.isValid());

    QCOMPARE(beforeRenderingSpy.size(), 0);
    QCOMPARE(beforeSyncSpy.size(), 0);
    QCOMPARE(afterRenderingSpy.size(), 0);

    widget.setSource(testFileUrl("rectangle.qml"));

    QCOMPARE(beforeRenderingSpy.size(), 0);
    QCOMPARE(beforeSyncSpy.size(), 0);
    QCOMPARE(afterRenderingSpy.size(), 0);

    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    QTRY_VERIFY(beforeRenderingSpy.size() > 0);
    QTRY_VERIFY(beforeSyncSpy.size() > 0);
    QTRY_VERIFY(afterRenderingSpy.size() > 0);
}

void tst_qquickwidget::grab()
{
    QQuickWidget view;
    view.setSource(testFileUrl("rectangle.qml"));
    QPixmap pixmap = view.grab();
    QRgb pixel = pixmap.toImage().pixel(5, 5);
    QCOMPARE(pixel, qRgb(255, 0, 0));
}

// QTBUG-49929, verify that Qt Designer grabbing the contents before drag
// does not crash due to missing GL contexts or similar.
void tst_qquickwidget::grabBeforeShow()
{
    QQuickWidget widget;
    QVERIFY(!widget.grab().isNull());
}

void tst_qquickwidget::reparentToNewWindow()
{
    QWidget window1;
    QWidget window2;

    QQuickWidget *qqw = new QQuickWidget(&window1);
    qqw->setSource(testFileUrl("rectangle.qml"));
    window1.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window1));
    window2.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window2));

    QSignalSpy afterRenderingSpy(qqw->quickWindow(), &QQuickWindow::afterRendering);
    qqw->setParent(&window2);
    qqw->show();

    QTRY_VERIFY(afterRenderingSpy.size() > 0);

    QImage img = qqw->grabFramebuffer();
    QCOMPARE(img.pixel(5, 5), qRgb(255, 0, 0));
}

void tst_qquickwidget::nullEngine()
{
    QQuickWidget widget;
    // Default should have no errors, even with a null qml engine
    QVERIFY(widget.errors().isEmpty());
    QCOMPARE(widget.status(), QQuickWidget::Null);

    // A QML engine should be created lazily.
    QVERIFY(widget.rootContext());
    QVERIFY(widget.engine());
}

class KeyHandlingWidget : public QQuickWidget
{
public:
    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_A)
            ok = true;
    }

    bool ok = false;
};

void tst_qquickwidget::keyEvents()
{
    // A QQuickWidget should behave like a normal widget when it comes to event handling.
    // Verify that key events actually reach the widget. (QTBUG-45757)
    KeyHandlingWidget widget;
    widget.setSource(testFileUrl("rectangle.qml"));
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(widget.window()));

    // Note: send the event to the QWindow, not the QWidget, in order
    // to simulate the full event processing chain.
    QTest::keyClick(widget.window()->windowHandle(), Qt::Key_A);

    QTRY_VERIFY(widget.ok);
}

class ShortcutEventFilter : public QObject
{
public:
    bool eventFilter(QObject *obj, QEvent *e) override {
        if (e->type() == QEvent::ShortcutOverride)
            shortcutOk = true;

        return QObject::eventFilter(obj, e);
    }

    bool shortcutOk = false;
};

void tst_qquickwidget::shortcuts()
{
    // Verify that ShortcutOverride events do not get lost. (QTBUG-60988)
    KeyHandlingWidget widget;
    widget.setSource(testFileUrl("rectangle.qml"));
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(widget.window()));

    // Send to the widget, verify that the QQuickWindow sees it.

    ShortcutEventFilter filter;
    widget.quickWindow()->installEventFilter(&filter);

    QKeyEvent e(QEvent::ShortcutOverride, Qt::Key_A, Qt::ControlModifier);
    QCoreApplication::sendEvent(&widget, &e);

    QTRY_VERIFY(filter.shortcutOk);
}

void tst_qquickwidget::enterLeave()
{
    QQuickWidget view;
    view.setSource(testFileUrl("enterleave.qml"));

    // Ensure the cursor is away from the window first
    const auto outside = m_availableGeometry.topLeft() + QPoint(50, 50);
    QCursor::setPos(outside);
    QTRY_VERIFY(QCursor::pos() == outside);

    view.move(m_availableGeometry.topLeft() + QPoint(100, 100));
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    QQuickItem *rootItem = view.rootObject();
    QVERIFY(rootItem);
    const QPoint frameOffset = view.geometry().topLeft() - view.frameGeometry().topLeft();

    QTRY_VERIFY(!rootItem->property("hasMouse").toBool());
    // Check the enter
    QCursor::setPos(view.pos() + QPoint(50, 50) + frameOffset);
    QTRY_VERIFY(rootItem->property("hasMouse").toBool());
    // Now check the leave
    QCursor::setPos(outside);
    QTRY_VERIFY(!rootItem->property("hasMouse").toBool());
}

void tst_qquickwidget::mouseEventWindowPos()
{
    QWidget widget;
    widget.resize(100, 100);
    QQuickWidget *quick = new QQuickWidget(&widget);
    quick->setSource(testFileUrl("mouse.qml"));
    quick->move(50, 50);
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));
    QQuickItem *rootItem = quick->rootObject();
    QVERIFY(rootItem);

    QVERIFY(!rootItem->property("wasClicked").toBool());
    QVERIFY(!rootItem->property("wasDoubleClicked").toBool());
    // Moving an item under the mouse cursor will trigger a mouse move event.
    // The above quick->move() will trigger a mouse move event on macOS.
    // Discard that in order to get a clean slate for the actual tests.
    rootItem->setProperty("wasMoved", QVariant(false));

    QWindow *window = widget.windowHandle();
    QVERIFY(window);

    QTest::mouseMove(window, QPoint(60, 60));
    QTest::mouseClick(window, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(60, 60));
    QTRY_VERIFY(rootItem->property("wasClicked").toBool());
    QTest::mouseDClick(window, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(60, 60));
    QTRY_VERIFY(rootItem->property("wasDoubleClicked").toBool());
    QTest::mouseMove(window, QPoint(70, 70));
    QTRY_VERIFY(rootItem->property("wasMoved").toBool());
}

void tst_qquickwidget::synthMouseFromTouch_data()
{
    QTest::addColumn<bool>("synthMouse"); // AA_SynthesizeMouseForUnhandledTouchEvents
    QTest::addColumn<bool>("acceptTouch"); // QQuickItem::touchEvent: setAccepted()

    QTest::newRow("no synth, accept") << false << true; // suitable for touch-capable UIs
    QTest::newRow("no synth, don't accept") << false << false;
    QTest::newRow("synth and accept") << true << true;
    QTest::newRow("synth, don't accept") << true << false; // the default
}

void tst_qquickwidget::synthMouseFromTouch()
{
    QFETCH(bool, synthMouse);
    QFETCH(bool, acceptTouch);

    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, synthMouse);
    QWidget window;
    window.setAttribute(Qt::WA_AcceptTouchEvents);
    QScopedPointer<MouseRecordingQQWidget> childView(new MouseRecordingQQWidget(&window));
    MouseRecordingItem *item = new MouseRecordingItem(acceptTouch, nullptr);
    childView->setContent(QUrl(), nullptr, item);
    window.resize(300, 300);
    childView->resize(300, 300);
    window.show();
    QVERIFY(QTest::qWaitForWindowActive(&window));
    QVERIFY(!childView->quickWindow()->isVisible()); // this window is always not visible see QTBUG-65761
    QVERIFY(item->isVisible());

    QPoint p1 = QPoint(20, 20);
    QPoint p2 = QPoint(30, 30);
    QTest::touchEvent(&window, device).press(0, p1, &window);
    QTest::touchEvent(&window, device).move(0, p2, &window);
    QTest::touchEvent(&window, device).release(0, p2, &window);

    QCOMPARE(item->m_touchEvents.count(), !synthMouse && !acceptTouch ? 1 : 3);
    QCOMPARE(item->m_mouseEvents.count(), (acceptTouch || !synthMouse) ? 0 : 3);
    QCOMPARE(childView->m_mouseEvents.count(), 0);
    for (const QMouseEvent &ev : item->m_mouseEvents)
        QCOMPARE(ev.source(), Qt::MouseEventSynthesizedByQt);
}

void tst_qquickwidget::tabKey()
{
    if (QGuiApplication::styleHints()->tabFocusBehavior() != Qt::TabFocusAllControls)
        QSKIP("This function doesn't support NOT iterating all.");
    QWidget window1;
    QQuickWidget *qqw = new QQuickWidget(&window1);
    qqw->setSource(testFileUrl("activeFocusOnTab.qml"));
    QQuickWidget *qqw2 = new QQuickWidget(&window1);
    qqw2->setSource(testFileUrl("noActiveFocusOnTab.qml"));
    qqw2->move(100, 0);
    window1.show();
    qqw->setFocus();
    QVERIFY(QTest::qWaitForWindowExposed(&window1, 5000));
    QVERIFY(qqw->hasFocus());
    QQuickItem *item = qobject_cast<QQuickItem *>(qqw->rootObject());
    QQuickItem *topItem = item->findChild<QQuickItem *>("topRect");
    QQuickItem *middleItem = item->findChild<QQuickItem *>("middleRect");
    QQuickItem *bottomItem = item->findChild<QQuickItem *>("bottomRect");
    topItem->forceActiveFocus();
    QVERIFY(topItem->property("activeFocus").toBool());
    QTest::keyClick(qqw, Qt::Key_Tab);
    QTRY_VERIFY(middleItem->property("activeFocus").toBool());
    QTest::keyClick(qqw, Qt::Key_Tab);
    QTRY_VERIFY(bottomItem->property("activeFocus").toBool());
    QTest::keyClick(qqw, Qt::Key_Backtab);
    QTRY_VERIFY(middleItem->property("activeFocus").toBool());

    qqw2->setFocus();
    QQuickItem *item2 = qobject_cast<QQuickItem *>(qqw2->rootObject());
    QQuickItem *topItem2 = item2->findChild<QQuickItem *>("topRect2");
    QTRY_VERIFY(qqw2->hasFocus());
    QVERIFY(topItem2->property("activeFocus").toBool());
    QTest::keyClick(qqw2, Qt::Key_Tab);
    QTRY_VERIFY(qqw->hasFocus());
    QVERIFY(middleItem->property("activeFocus").toBool());
}

QTEST_MAIN(tst_qquickwidget)

#include "tst_qquickwidget.moc"
