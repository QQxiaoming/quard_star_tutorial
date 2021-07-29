/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include <private/qdebug_p.h>
#include <QtGui/qstylehints.h>
#include <QtQuick/private/qquickpointerhandler_p.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

#include "../../../shared/util.h"
#include "../../shared/viewtestutil.h"

Q_LOGGING_CATEGORY(lcPointerTests, "qt.quick.pointer.tests")

class Event
{
    Q_GADGET
public:
    enum Destination {
        FilterDestination,
        MouseDestination,
        TouchDestination,
        HandlerDestination
    };
    Q_ENUM(Destination)

    Event(Destination d, QEvent::Type t, Qt::TouchPointState s, int grabTransition, QPointF item, QPointF scene)
        : destination(d), type(t), state(s), grabTransition(grabTransition), posWrtItem(item), posWrtScene(scene)
    {}

    Destination destination;
    QEvent::Type type;                      // if this represents a QEvent that was received
    Qt::TouchPointState state;              // if this represents an event (pointer, touch or mouse)
    int grabTransition;                     // if this represents an onGrabChanged() notification (QQuickEventPoint::GrabTransition)
    QPointF posWrtItem;
    QPointF posWrtScene;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const class Event &event) {
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "Event(";
    QtDebugUtils::formatQEnum(dbg, event.destination);
    dbg << ' ';
    QtDebugUtils::formatQEnum(dbg, event.type);
    dbg << ' ';
    QtDebugUtils::formatQEnum(dbg, event.state);
    if (event.grabTransition) {
        dbg << ' ';
        QtDebugUtils::formatQEnum(dbg, QQuickEventPoint::GrabTransition(event.grabTransition));
    }
    dbg << " @ ";
    QtDebugUtils::formatQPoint(dbg, event.posWrtItem);
    dbg << " S ";
    QtDebugUtils::formatQPoint(dbg, event.posWrtScene);
    dbg << ')';
    return dbg;
}
#endif

enum {
    NoGrab = 0,
};

class EventItem : public QQuickItem
{
    Q_OBJECT
public:
    EventItem(QQuickItem *parent = nullptr)
        : QQuickItem(parent), acceptPointer(false), grabPointer(false), acceptMouse(false), acceptTouch(false), filterTouch(false)
    {}

    inline int grabTransition(bool accept, Qt::TouchPointState state) {
        return (accept && (state != Qt::TouchPointReleased)) ? (int)QQuickEventPoint::GrabExclusive : (int)NoGrab;
    }

    void touchEvent(QTouchEvent *event)
    {
        qCDebug(lcPointerTests) << event << "will accept?" << acceptTouch;
        for (const QTouchEvent::TouchPoint &tp : event->touchPoints())
            eventList.append(Event(Event::TouchDestination, event->type(), tp.state(), grabTransition(acceptTouch, tp.state()), tp.pos(), tp.scenePos()));
        event->setAccepted(acceptTouch);
    }
    void mousePressEvent(QMouseEvent *event)
    {
        qCDebug(lcPointerTests) << event;
        eventList.append(Event(Event::MouseDestination, event->type(), Qt::TouchPointPressed, grabTransition(acceptMouse, Qt::TouchPointPressed), event->pos(), event->windowPos()));
        event->setAccepted(acceptMouse);
    }
    void mouseMoveEvent(QMouseEvent *event)
    {
        qCDebug(lcPointerTests) << event;
        eventList.append(Event(Event::MouseDestination, event->type(), Qt::TouchPointMoved, grabTransition(acceptMouse, Qt::TouchPointMoved), event->pos(), event->windowPos()));
        event->setAccepted(acceptMouse);
    }
    void mouseReleaseEvent(QMouseEvent *event)
    {
        qCDebug(lcPointerTests) << event;
        eventList.append(Event(Event::MouseDestination, event->type(), Qt::TouchPointReleased, grabTransition(acceptMouse, Qt::TouchPointReleased), event->pos(), event->windowPos()));
        event->setAccepted(acceptMouse);
    }
    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        qCDebug(lcPointerTests) << event;
        eventList.append(Event(Event::MouseDestination, event->type(), Qt::TouchPointPressed, grabTransition(acceptMouse, Qt::TouchPointPressed), event->pos(), event->windowPos()));
        event->setAccepted(acceptMouse);
    }

    void mouseUngrabEvent()
    {
        qCDebug(lcPointerTests);
        eventList.append(Event(Event::MouseDestination, QEvent::UngrabMouse, Qt::TouchPointReleased, QQuickEventPoint::UngrabExclusive, QPoint(0,0), QPoint(0,0)));
    }

    bool event(QEvent *event)
    {
        qCDebug(lcPointerTests) << event;
        return QQuickItem::event(event);
    }

    QList<Event> eventList;
    bool acceptPointer;
    bool grabPointer;
    bool acceptMouse;
    bool acceptTouch;
    bool filterTouch; // when used as event filter

    bool eventFilter(QObject *o, QEvent *event)
    {
        qCDebug(lcPointerTests) << event << o;
        if (event->type() == QEvent::TouchBegin ||
                event->type() == QEvent::TouchUpdate ||
                event->type() == QEvent::TouchCancel ||
                event->type() == QEvent::TouchEnd) {
            QTouchEvent *touch = static_cast<QTouchEvent*>(event);
            for (const QTouchEvent::TouchPoint &tp : touch->touchPoints())
                eventList.append(Event(Event::FilterDestination, event->type(), tp.state(), QQuickEventPoint::GrabExclusive, tp.pos(), tp.scenePos()));
            if (filterTouch)
                event->accept();
            return true;
        }
        return false;
    }
};

#define QCOMPARE_EVENT(i, d, t, s, g) \
    {\
        const Event &event = eventItem1->eventList.at(i);\
        QCOMPARE(event.destination, d);\
        QCOMPARE(event.type, t);\
        QCOMPARE(event.state, s);\
        QCOMPARE(event.grabTransition, g);\
    }\

class EventHandler : public QQuickPointerHandler
{
public:
    void handlePointerEventImpl(QQuickPointerEvent *event) override
    {
        QQuickPointerHandler::handlePointerEventImpl(event);
        if (!enabled())
            return;
        if (event->isPressEvent())
            ++pressEventCount;
        if (event->isReleaseEvent())
            ++releaseEventCount;
        EventItem *item = qmlobject_cast<EventItem *>(target());
        if (!item) {
            event->point(0)->setGrabberPointerHandler(this);
            return;
        }
        qCDebug(lcPointerTests) << item->objectName() << event;
        int c = event->pointCount();
        for (int i = 0; i < c; ++i) {
            QQuickEventPoint *point = event->point(i);
            if (item->acceptPointer)
                point->setAccepted(item->acceptPointer); // does NOT imply a grab
            if (item->grabPointer)
                setExclusiveGrab(point, true);
            qCDebug(lcPointerTests) << "        " << i << ":" << point << "accepted?" << item->acceptPointer << "grabbed?" << (point->exclusiveGrabber() == this);
            item->eventList.append(Event(Event::HandlerDestination, QEvent::Pointer,
                static_cast<Qt::TouchPointState>(point->state()),
                item->grabPointer ? (int)QQuickEventPoint::GrabExclusive : (int)NoGrab,
                eventPos(point), point->scenePosition()));
        }
    }

    void onGrabChanged(QQuickPointerHandler *, QQuickEventPoint::GrabTransition stateChange, QQuickEventPoint *point) override
    {
        EventItem *item = qmlobject_cast<EventItem *>(target());
        if (item)
            item->eventList.append(Event(Event::HandlerDestination, QEvent::None,
                static_cast<Qt::TouchPointState>(point->state()), stateChange, eventPos(point), point->scenePosition()));
    }

    int pressEventCount = 0;
    int releaseEventCount = 0;
};

class tst_PointerHandlers : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_PointerHandlers()
        :touchDevice(QTest::createTouchDevice())
    {}

private slots:
    void initTestCase();

    void touchEventDelivery_data();
    void touchEventDelivery();
    void mouseEventDelivery();
    void touchReleaseOutside_data();
    void touchReleaseOutside();
    void dynamicCreation();
    void handlerInWindow();
    void dynamicCreationInWindow();

protected:
    bool eventFilter(QObject *, QEvent *event)
    {
        Qt::TouchPointState tpState;
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            tpState = Qt::TouchPointPressed;
            break;
        case QEvent::MouseMove:
            tpState = Qt::TouchPointMoved;
            break;
        case QEvent::MouseButtonRelease:
            tpState = Qt::TouchPointReleased;
            break;
        default:
            // So far we aren't recording filtered touch events here - they would be quite numerous in some cases
            return false;
        }
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        filteredEventList.append(Event(Event::FilterDestination, event->type(), tpState,
                                       0, me->pos(), me->globalPos()));
        return false;
    }

private:
    void createView(QScopedPointer<QQuickView> &window, const char *fileName);
    QTouchDevice *touchDevice;
    QList<Event> filteredEventList;
};

void tst_PointerHandlers::createView(QScopedPointer<QQuickView> &window, const char *fileName)
{
    window.reset(new QQuickView);
//    window->setGeometry(0,0,240,320);
    window->setSource(testFileUrl(fileName));
    QTRY_COMPARE(window->status(), QQuickView::Ready);
    QQuickViewTestUtil::centerOnScreen(window.data());
    QQuickViewTestUtil::moveMouseAway(window.data());

    window->show();
    QVERIFY(QTest::qWaitForWindowActive(window.data()));
    QVERIFY(window->rootObject() != nullptr);
}

void tst_PointerHandlers::initTestCase()
{
    QQmlDataTest::initTestCase();
    qmlRegisterType<EventItem>("Qt.test", 1, 0, "EventItem");
    qmlRegisterType<EventHandler>("Qt.test", 1, 0, "EventHandler");
}

void tst_PointerHandlers::touchEventDelivery_data()
{
    QTest::addColumn<bool>("synthMouse"); // AA_SynthesizeMouseForUnhandledTouchEvents
    QTest::newRow("no synth") << false;
    QTest::newRow("synth") << true;
}

void tst_PointerHandlers::touchEventDelivery()
{
    QFETCH(bool, synthMouse);
    qApp->setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, synthMouse);

    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "singleitem.qml");
    QQuickView * window = windowPtr.data();

    EventItem *eventItem1 = window->rootObject()->findChild<EventItem*>("eventItem1");
    QVERIFY(eventItem1);

    // Do not accept anything
    QPoint p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    QTRY_COMPARE(eventItem1->eventList.size(), synthMouse ? 3 : 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::TouchDestination, QEvent::TouchBegin, Qt::TouchPointPressed, NoGrab);
    if (synthMouse)
        QCOMPARE_EVENT(2, Event::MouseDestination, QEvent::MouseButtonPress, Qt::TouchPointPressed, NoGrab);
    p1 += QPoint(10, 0);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 4 : 3);
    QCOMPARE_EVENT(eventItem1->eventList.size() - 1, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointMoved, NoGrab);
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 5 : 4);
    QCOMPARE_EVENT(eventItem1->eventList.size() - 1, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, NoGrab);
    eventItem1->eventList.clear();

    // Accept touch
    eventItem1->acceptTouch = true;
    p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::TouchDestination, QEvent::TouchBegin, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    auto pointerEvent = QQuickWindowPrivate::get(window)->pointerEventInstance(QQuickPointerDevice::touchDevices().at(0));
    QCOMPARE(pointerEvent->point(0)->exclusiveGrabber(), eventItem1);
    p1 += QPoint(10, 0);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 4);
    QCOMPARE_EVENT(2, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointMoved, NoGrab);
    QCOMPARE_EVENT(3, Event::TouchDestination, QEvent::TouchUpdate, Qt::TouchPointMoved, QQuickEventPoint::GrabExclusive);
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 6);
    QCOMPARE_EVENT(4, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, NoGrab);
    QCOMPARE_EVENT(5, Event::TouchDestination, QEvent::TouchEnd, Qt::TouchPointReleased, NoGrab);
    eventItem1->eventList.clear();

    // wait to avoid getting a double click event
    QTest::qWait(qApp->styleHints()->mouseDoubleClickInterval() + 10);

    // Accept mouse
    eventItem1->acceptTouch = false;
    eventItem1->acceptMouse = true;
    eventItem1->setAcceptedMouseButtons(Qt::LeftButton);
    p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 3 : 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::TouchDestination, QEvent::TouchBegin, Qt::TouchPointPressed, NoGrab);
    if (synthMouse)
        QCOMPARE_EVENT(2, Event::MouseDestination, QEvent::MouseButtonPress, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    QCOMPARE(window->mouseGrabberItem(), synthMouse ? eventItem1 : nullptr);

    QPointF localPos = eventItem1->mapFromScene(p1);
    QPointF scenePos = p1; // item is at 0,0
    QCOMPARE(eventItem1->eventList.at(1).posWrtItem, localPos);
    QCOMPARE(eventItem1->eventList.at(1).posWrtScene, scenePos);
    if (synthMouse) {
        QCOMPARE(eventItem1->eventList.at(2).posWrtItem, localPos);
        QCOMPARE(eventItem1->eventList.at(2).posWrtScene, scenePos);
    }

    p1 += QPoint(10, 0);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 6 : 3);
    if (synthMouse) {
        QCOMPARE_EVENT(3, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointMoved, NoGrab);
        QCOMPARE_EVENT(4, Event::TouchDestination, QEvent::TouchUpdate, Qt::TouchPointMoved, NoGrab);
        QCOMPARE_EVENT(5, Event::MouseDestination, QEvent::MouseMove, Qt::TouchPointMoved, QQuickEventPoint::GrabExclusive);
    }
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 10 : 4);
    if (synthMouse) {
        QCOMPARE_EVENT(6, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, NoGrab);
        QCOMPARE_EVENT(7, Event::TouchDestination, QEvent::TouchEnd, Qt::TouchPointReleased, NoGrab);
        QCOMPARE_EVENT(8, Event::MouseDestination, QEvent::MouseButtonRelease, Qt::TouchPointReleased, NoGrab);
        QCOMPARE_EVENT(9, Event::MouseDestination, QEvent::UngrabMouse, Qt::TouchPointReleased, QQuickEventPoint::UngrabExclusive);
    } else {
        QCOMPARE_EVENT(3, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, NoGrab);
    }
    eventItem1->eventList.clear();

    // wait to avoid getting a double click event
    QTest::qWait(qApp->styleHints()->mouseDoubleClickInterval() + 10);

    // Accept mouse buttons but not the touch event
    eventItem1->acceptTouch = false;
    eventItem1->acceptMouse = false;
    eventItem1->setAcceptedMouseButtons(Qt::LeftButton);
    p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 3 : 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::TouchDestination, QEvent::TouchBegin, Qt::TouchPointPressed, NoGrab);
    if (synthMouse)
        QCOMPARE_EVENT(2, Event::MouseDestination, QEvent::MouseButtonPress, Qt::TouchPointPressed, NoGrab);
    QCOMPARE(pointerEvent->point(0)->exclusiveGrabber(), nullptr);
    p1 += QPoint(10, 0);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 4 : 3);
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), synthMouse ? 5 : 4);
    eventItem1->eventList.clear();

    // wait to avoid getting a double click event
    QTest::qWait(qApp->styleHints()->mouseDoubleClickInterval() + 10);

    // Accept touch
    eventItem1->acceptTouch = true;
    eventItem1->setAcceptedMouseButtons(Qt::LeftButton);
    p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::TouchDestination, QEvent::TouchBegin, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    p1 += QPoint(10, 0);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 4);
    QCOMPARE_EVENT(2, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointMoved, NoGrab);
    QCOMPARE_EVENT(3, Event::TouchDestination, QEvent::TouchUpdate, Qt::TouchPointMoved, QQuickEventPoint::GrabExclusive);
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 6);
    QCOMPARE_EVENT(4, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, NoGrab);
    QCOMPARE_EVENT(5, Event::TouchDestination, QEvent::TouchEnd, Qt::TouchPointReleased, NoGrab);
    eventItem1->eventList.clear();

    // Accept pointer events
    eventItem1->acceptPointer = true;
    eventItem1->grabPointer = true;
    p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::None, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    QCOMPARE_EVENT(1, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    p1 += QPoint(10, 0);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 3);
    QCOMPARE_EVENT(2, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointMoved, QQuickEventPoint::GrabExclusive);
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    QCOMPARE(eventItem1->eventList.size(), 5);
    qCDebug(lcPointerTests) << eventItem1->eventList;
    QCOMPARE_EVENT(3, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, QQuickEventPoint::GrabExclusive);
    QCOMPARE_EVENT(4, Event::HandlerDestination, QEvent::None, Qt::TouchPointReleased, QQuickEventPoint::UngrabExclusive);
    eventItem1->eventList.clear();
}

void tst_PointerHandlers::mouseEventDelivery()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "singleitem.qml");
    QQuickView * window = windowPtr.data();

    EventItem *eventItem1 = window->rootObject()->findChild<EventItem*>("eventItem1");
    QVERIFY(eventItem1);

    // Do not accept anything
    QPoint p1 = QPoint(20, 20);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QCOMPARE(eventItem1->eventList.size(), 2);
    p1 += QPoint(10, 0);
    QTest::mouseMove(window, p1);
    QCOMPARE(eventItem1->eventList.size(), 3);
    QTest::mouseRelease(window, Qt::LeftButton);
    QCOMPARE(eventItem1->eventList.size(), 3);
    eventItem1->eventList.clear();

    // wait to avoid getting a double click event
    QTest::qWait(qApp->styleHints()->mouseDoubleClickInterval() + 10);

    // Accept mouse
    eventItem1->acceptTouch = false;
    eventItem1->acceptMouse = true;
    eventItem1->setAcceptedMouseButtons(Qt::LeftButton);
    p1 = QPoint(20, 20);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QCOMPARE(eventItem1->eventList.size(), 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::MouseDestination, QEvent::MouseButtonPress, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    QCOMPARE(window->mouseGrabberItem(), eventItem1);

    QPointF localPos = eventItem1->mapFromScene(p1);
    QPointF scenePos = p1; // item is at 0,0
    QCOMPARE(eventItem1->eventList.at(0).posWrtItem, localPos);
    QCOMPARE(eventItem1->eventList.at(0).posWrtScene, scenePos);
    QCOMPARE(eventItem1->eventList.at(1).posWrtItem, localPos);
    QCOMPARE(eventItem1->eventList.at(1).posWrtScene, scenePos);

    p1 += QPoint(10, 0);
    QTest::mouseMove(window, p1);
    QCOMPARE(eventItem1->eventList.size(), 3);
    QCOMPARE_EVENT(2, Event::MouseDestination, QEvent::MouseMove, Qt::TouchPointMoved, QQuickEventPoint::GrabExclusive);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QCOMPARE(eventItem1->eventList.size(), 5);
    QCOMPARE_EVENT(3, Event::MouseDestination, QEvent::MouseButtonRelease, Qt::TouchPointReleased, NoGrab);
    QCOMPARE_EVENT(4, Event::MouseDestination, QEvent::UngrabMouse, Qt::TouchPointReleased, QQuickEventPoint::UngrabExclusive);
    eventItem1->eventList.clear();

    // wait to avoid getting a double click event
    QTest::qWait(qApp->styleHints()->mouseDoubleClickInterval() + 10);

    // Grab pointer events
    eventItem1->acceptMouse = false;
    eventItem1->acceptPointer = true;
    eventItem1->grabPointer = true;
    p1 = QPoint(20, 20);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(eventItem1->eventList.size(), 3);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::None, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    QCOMPARE_EVENT(1, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, QQuickEventPoint::GrabExclusive);
    QCOMPARE_EVENT(2, Event::MouseDestination, QEvent::MouseButtonPress, Qt::TouchPointPressed, 0);
    p1 += QPoint(10, 0);
    QTest::mouseMove(window, p1);
    QCOMPARE(eventItem1->eventList.size(), 4);
    QCOMPARE_EVENT(3, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointMoved, QQuickEventPoint::GrabExclusive);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QCOMPARE(eventItem1->eventList.size(), 6);
    QCOMPARE_EVENT(4, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointReleased, QQuickEventPoint::GrabExclusive);
    QCOMPARE_EVENT(5, Event::HandlerDestination, QEvent::None, Qt::TouchPointReleased, QQuickEventPoint::UngrabExclusive);
    eventItem1->eventList.clear();
}

void tst_PointerHandlers::touchReleaseOutside_data()
{
    QTest::addColumn<bool>("acceptPointer");
    QTest::addColumn<bool>("grabPointer");
    QTest::addColumn<int>("eventCount");
    QTest::addColumn<int>("endIndexToTest");
    QTest::addColumn<int>("endDestination");    // Event::Destination
    QTest::addColumn<int>("endType");           // QEvent::Type
    QTest::addColumn<int>("endState");          // Qt::TouchPointState
    QTest::addColumn<int>("endGrabState");      // Qt::TouchPointState

    QTest::newRow("reject and ignore") << false << false << 6 << 5 << (int)Event::TouchDestination
        << (int)QEvent::TouchEnd << (int)Qt::TouchPointReleased << (int)NoGrab;
    QTest::newRow("reject and grab") << false << true << 5 << 4 << (int)Event::HandlerDestination
        << (int)QEvent::None << (int)Qt::TouchPointReleased << (int)QQuickEventPoint::UngrabExclusive;
    QTest::newRow("accept and ignore") << true << false << 1 << 0 << (int)Event::HandlerDestination
        << (int)QEvent::Pointer << (int)Qt::TouchPointPressed << (int)NoGrab;
    QTest::newRow("accept and grab") << true << true << 5 << 4 << (int)Event::HandlerDestination
        << (int)QEvent::None << (int)Qt::TouchPointReleased << (int)QQuickEventPoint::UngrabExclusive;
}

void tst_PointerHandlers::touchReleaseOutside()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "singleitem.qml");
    QQuickView * window = windowPtr.data();

    QFETCH(bool, acceptPointer);
    QFETCH(bool, grabPointer);
    QFETCH(int, eventCount);
    QFETCH(int, endIndexToTest);
    QFETCH(int, endDestination);
    QFETCH(int, endType);
    QFETCH(int, endState);
    QFETCH(int, endGrabState);

    EventItem *eventItem1 = window->rootObject()->findChild<EventItem*>("eventItem1");
    QVERIFY(eventItem1);

    eventItem1->acceptTouch = true;
    eventItem1->acceptPointer = acceptPointer;
    eventItem1->grabPointer = grabPointer;

    QPoint p1 = QPoint(20, 20);
    QTest::touchEvent(window, touchDevice).press(0, p1, window);
    QQuickTouchUtils::flush(window);
    p1.setX(eventItem1->mapToScene(eventItem1->clipRect().bottomRight()).x() + 10);
    QTest::touchEvent(window, touchDevice).move(0, p1, window);
    QTest::touchEvent(window, touchDevice).release(0, p1, window);
    QQuickTouchUtils::flush(window);
    qCDebug(lcPointerTests) << eventItem1->eventList;
    QCOMPARE(eventItem1->eventList.size(), eventCount);
    QCOMPARE_EVENT(endIndexToTest, endDestination, endType, endState, endGrabState);
}

void tst_PointerHandlers::dynamicCreation()
{
    QScopedPointer<QQuickView> windowPtr;
    createView(windowPtr, "dynamicallyCreated.qml");
    QQuickView * window = windowPtr.data();

    EventItem *eventItem1 = window->rootObject()->findChild<EventItem*>("eventItem1");
    QVERIFY(eventItem1);
    EventHandler *handler = window->rootObject()->findChild<EventHandler*>("eventHandler");
    QVERIFY(handler);

    QCOMPARE(handler->parentItem(), eventItem1);
    QCOMPARE(handler->target(), eventItem1);

    QPoint p1(20, 20);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(eventItem1->eventList.size(), 2);
    QCOMPARE_EVENT(0, Event::HandlerDestination, QEvent::Pointer, Qt::TouchPointPressed, NoGrab);
    QCOMPARE_EVENT(1, Event::MouseDestination, QEvent::MouseButtonPress, Qt::TouchPointPressed, NoGrab);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
}

void tst_PointerHandlers::handlerInWindow()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("handlerInWindow.qml"));
    QQuickWindow *window = qobject_cast<QQuickWindow*>(component.create());
    QScopedPointer<QQuickWindow> cleanup(window);
    QVERIFY(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    EventHandler *handler = window->contentItem()->findChild<EventHandler*>("eventHandler");
    QVERIFY(handler);

    QCOMPARE(handler->parentItem(), window->contentItem());
    QCOMPARE(handler->target(), window->contentItem());

    QPoint p1(20, 20);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(handler->pressEventCount, 1);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(handler->releaseEventCount, 1);
}

void tst_PointerHandlers::dynamicCreationInWindow()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadUrl(testFileUrl("dynamicallyCreatedInWindow.qml"));
    QQuickWindow *window = qobject_cast<QQuickWindow*>(component.create());
    QScopedPointer<QQuickWindow> cleanup(window);
    QVERIFY(window);
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    EventHandler *handler = window->contentItem()->findChild<EventHandler*>("eventHandler");
    QVERIFY(handler);

    QCOMPARE(handler->parentItem(), window->contentItem());
    QCOMPARE(handler->target(), window->contentItem());

    QPoint p1(20, 20);
    QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(handler->pressEventCount, 1);
    QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, p1);
    QTRY_COMPARE(handler->releaseEventCount, 1);
}

QTEST_MAIN(tst_PointerHandlers)

#include "tst_qquickpointerhandler.moc"

