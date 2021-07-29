/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickpopup_p.h"
#include "qquickpopup_p_p.h"
#include "qquickpopupanchors_p.h"
#include "qquickpopupitem_p_p.h"
#include "qquickpopuppositioner_p_p.h"
#include "qquickapplicationwindow_p.h"
#include "qquickoverlay_p_p.h"
#include "qquickcontrol_p_p.h"
#include "qquickdialog_p.h"

#include <QtQml/qqmlinfo.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/private/qquicktransition_p.h>
#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Popup
    \inherits QtObject
    \instantiates QQuickPopup
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-popups
    \ingroup qtquickcontrols2-focusscopes
    \brief Base type of popup-like user interface controls.

    Popup is the base type of popup-like user interface controls. It can be
    used with \l Window or \l ApplicationWindow.

    \qml
    import QtQuick.Window 2.2
    import QtQuick.Controls 2.12

    ApplicationWindow {
        id: window
        width: 400
        height: 400
        visible: true

        Button {
            text: "Open"
            onClicked: popup.open()
        }

        Popup {
            id: popup
            x: 100
            y: 100
            width: 200
            height: 300
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        }
    }
    \endqml

    In order to ensure that a popup is displayed above other items in the
    scene, it is recommended to use ApplicationWindow. ApplicationWindow also
    provides background dimming effects.

    Popup does not provide a layout of its own, but requires you to position
    its contents, for instance by creating a \l RowLayout or a \l ColumnLayout.

    Items declared as children of a Popup are automatically parented to the
    Popups's \l contentItem. Items created dynamically need to be explicitly
    parented to the contentItem.

    \section1 Popup Layout

    The following diagram illustrates the layout of a popup within a window:

    \image qtquickcontrols2-popup.png

    The \l implicitWidth and \l implicitHeight of a popup are typically based
    on the implicit sizes of the background and the content item plus any insets
    and paddings. These properties determine how large the popup will be when no
    explicit \l width or \l height is specified.

    The geometry of the \l contentItem is determined by the padding. The following
    example reserves 10px padding between the boundaries of the popup and its content:

    \code
    Popup {
        padding: 10

        contentItem: Text {
            text: "Content"
        }
    }
    \endcode

    The \l background item fills the entire width and height of the popup,
    unless insets or an explicit size have been given for it.

    Negative insets can be used to make the background larger than the popup.
    The following example uses negative insets to place a shadow outside the
    popup's boundaries:

    \code
    Popup {
        topInset: -2
        leftInset: -2
        rightInset: -6
        bottomInset: -6

        background: BorderImage {
            source: ":/images/shadowed-background.png"
        }
    }
    \endcode

    \section1 Popup Sizing

    If only a single item is used within a Popup, it will resize to fit the
    implicit size of its contained item. This makes it particularly suitable
    for use together with layouts.

    \code
    Popup {
        ColumnLayout {
            anchors.fill: parent
            CheckBox { text: qsTr("E-mail") }
            CheckBox { text: qsTr("Calendar") }
            CheckBox { text: qsTr("Contacts") }
        }
    }
    \endcode

    Sometimes there might be two items within the popup:

    \code
    Popup {
        SwipeView {
            // ...
        }
        PageIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }
    }
    \endcode

    In this case, Popup cannot calculate a sensible implicit size. Since we're
    anchoring the \l PageIndicator over the \l SwipeView, we can simply set the
    content size to the view's implicit size:

    \code
    Popup {
        contentWidth: view.implicitWidth
        contentHeight: view.implicitHeight

        SwipeView {
            id: view
            // ...
        }
        PageIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }
     }
    \endcode

    \section1 Popup Positioning

    Similar to items in Qt Quick, Popup's \l x and \l y coordinates are
    relative to its parent. This means that opening a popup that is a
    child of a \l Button, for example, will cause the popup to be positioned
    relative to the button.

    \include qquickoverlay-popup-parent.qdocinc

    Another way to center a popup in the window regardless of its parent item
    is to use \l {anchors.centerIn}:

    \snippet qtquickcontrols2-popup.qml centerIn

    To ensure that the popup is positioned within the bounds of the enclosing
    window, the \l margins property can be set to a non-negative value.

    \sa {Popup Controls}, {Customizing Popup}, ApplicationWindow
*/

/*!
    \qmlsignal void QtQuick.Controls::Popup::opened()

    This signal is emitted when the popup is opened.

    \sa aboutToShow()
*/

/*!
    \qmlsignal void QtQuick.Controls::Popup::closed()

    This signal is emitted when the popup is closed.

    \sa aboutToHide()
*/

/*!
    \qmlsignal void QtQuick.Controls::Popup::aboutToShow()

    This signal is emitted when the popup is about to show.

    \sa opened()
*/

/*!
    \qmlsignal void QtQuick.Controls::Popup::aboutToHide()

    This signal is emitted when the popup is about to hide.

    \sa closed()
*/

const QQuickPopup::ClosePolicy QQuickPopupPrivate::DefaultClosePolicy = QQuickPopup::CloseOnEscape | QQuickPopup::CloseOnPressOutside;

QQuickPopupPrivate::QQuickPopupPrivate()
    : transitionManager(this)
{
}

void QQuickPopupPrivate::init()
{
    Q_Q(QQuickPopup);
    popupItem = new QQuickPopupItem(q);
    popupItem->setVisible(false);
    q->setParentItem(qobject_cast<QQuickItem *>(parent));
    QObject::connect(popupItem, &QQuickControl::paddingChanged, q, &QQuickPopup::paddingChanged);
    QObject::connect(popupItem, &QQuickControl::backgroundChanged, q, &QQuickPopup::backgroundChanged);
    QObject::connect(popupItem, &QQuickControl::contentItemChanged, q, &QQuickPopup::contentItemChanged);
    QObject::connect(popupItem, &QQuickControl::implicitContentWidthChanged, q, &QQuickPopup::implicitContentWidthChanged);
    QObject::connect(popupItem, &QQuickControl::implicitContentHeightChanged, q, &QQuickPopup::implicitContentHeightChanged);
    QObject::connect(popupItem, &QQuickControl::implicitBackgroundWidthChanged, q, &QQuickPopup::implicitBackgroundWidthChanged);
    QObject::connect(popupItem, &QQuickControl::implicitBackgroundHeightChanged, q, &QQuickPopup::implicitBackgroundHeightChanged);
}

void QQuickPopupPrivate::closeOrReject()
{
    Q_Q(QQuickPopup);
    if (QQuickDialog *dialog = qobject_cast<QQuickDialog*>(q))
        dialog->reject();
    else
        q->close();
}

bool QQuickPopupPrivate::tryClose(const QPointF &pos, QQuickPopup::ClosePolicy flags)
{
    if (!interactive)
        return false;

    static const QQuickPopup::ClosePolicy outsideFlags = QQuickPopup::CloseOnPressOutside | QQuickPopup::CloseOnReleaseOutside;
    static const QQuickPopup::ClosePolicy outsideParentFlags = QQuickPopup::CloseOnPressOutsideParent | QQuickPopup::CloseOnReleaseOutsideParent;

    const bool onOutside = closePolicy & (flags & outsideFlags);
    const bool onOutsideParent = closePolicy & (flags & outsideParentFlags);
    if (onOutside || onOutsideParent) {
        if (!contains(pos)) {
            if (!onOutsideParent || !parentItem || !parentItem->contains(parentItem->mapFromScene(pos))) {
                closeOrReject();
                return true;
            }
        }
    }
    return false;
}

bool QQuickPopupPrivate::contains(const QPointF &scenePos) const
{
    return popupItem->contains(popupItem->mapFromScene(scenePos));
}

#if QT_CONFIG(quicktemplates2_multitouch)
bool QQuickPopupPrivate::acceptTouch(const QTouchEvent::TouchPoint &point)
{
    if (point.id() == touchId)
        return true;

    if (touchId == -1 && point.state() != Qt::TouchPointReleased) {
        touchId = point.id();
        return true;
    }

    return false;
}
#endif

bool QQuickPopupPrivate::blockInput(QQuickItem *item, const QPointF &point) const
{
    // don't block presses and releases
    // a) outside a non-modal popup,
    // b) to popup children/content, or
    // b) outside a modal popups's background dimming
    return modal && !popupItem->isAncestorOf(item) && (!dimmer || dimmer->contains(dimmer->mapFromScene(point)));
}

bool QQuickPopupPrivate::handlePress(QQuickItem *item, const QPointF &point, ulong timestamp)
{
    Q_UNUSED(timestamp);
    pressPoint = point;
    tryClose(point, QQuickPopup::CloseOnPressOutside | QQuickPopup::CloseOnPressOutsideParent);
    return blockInput(item, point);
}

bool QQuickPopupPrivate::handleMove(QQuickItem *item, const QPointF &point, ulong timestamp)
{
    Q_UNUSED(timestamp);
    return blockInput(item, point);
}

bool QQuickPopupPrivate::handleRelease(QQuickItem *item, const QPointF &point, ulong timestamp)
{
    Q_UNUSED(timestamp);
    if (item != popupItem && !contains(pressPoint))
        tryClose(point, QQuickPopup::CloseOnReleaseOutside | QQuickPopup::CloseOnReleaseOutsideParent);
    pressPoint = QPointF();
    touchId = -1;
    return blockInput(item, point);
}

void QQuickPopupPrivate::handleUngrab()
{
    Q_Q(QQuickPopup);
    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    if (overlay) {
        QQuickOverlayPrivate *p = QQuickOverlayPrivate::get(overlay);
        if (p->mouseGrabberPopup == q)
            p->mouseGrabberPopup = nullptr;
    }
    pressPoint = QPointF();
    touchId = -1;
}

bool QQuickPopupPrivate::handleMouseEvent(QQuickItem *item, QMouseEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return handlePress(item, event->windowPos(), event->timestamp());
    case QEvent::MouseMove:
        return handleMove(item, event->windowPos(), event->timestamp());
    case QEvent::MouseButtonRelease:
        return handleRelease(item, event->windowPos(), event->timestamp());
    default:
        Q_UNREACHABLE();
        return false;
    }
}

#if QT_CONFIG(quicktemplates2_multitouch)
bool QQuickPopupPrivate::handleTouchEvent(QQuickItem *item, QTouchEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        for (const QTouchEvent::TouchPoint &point : event->touchPoints()) {
            if (!acceptTouch(point))
                return blockInput(item, point.pos());

            switch (point.state()) {
            case Qt::TouchPointPressed:
                return handlePress(item, item->mapToScene(point.pos()), event->timestamp());
            case Qt::TouchPointMoved:
                return handleMove(item, item->mapToScene(point.pos()), event->timestamp());
            case Qt::TouchPointReleased:
                return handleRelease(item, item->mapToScene(point.pos()), event->timestamp());
            default:
                break;
            }
        }
        break;

    case QEvent::TouchCancel:
        handleUngrab();
        break;

    default:
        break;
    }

    return false;
}
#endif

bool QQuickPopupPrivate::prepareEnterTransition()
{
    Q_Q(QQuickPopup);
    if (!window) {
        qmlWarning(q) << "cannot find any window to open popup in.";
        return false;
    }

    if (transitionState == EnterTransition && transitionManager.isRunning())
        return false;

    if (transitionState != EnterTransition) {
        popupItem->setParentItem(QQuickOverlay::overlay(window));
        if (dim)
            createOverlay();
        showOverlay();
        emit q->aboutToShow();
        visible = true;
        transitionState = EnterTransition;
        popupItem->setVisible(true);
        getPositioner()->setParentItem(parentItem);
        emit q->visibleChanged();
    }
    return true;
}

bool QQuickPopupPrivate::prepareExitTransition()
{
    Q_Q(QQuickPopup);
    if (transitionState == ExitTransition && transitionManager.isRunning())
        return false;

    if (transitionState != ExitTransition) {
        // The setFocus(false) call below removes any active focus before we're
        // able to check it in finalizeExitTransition.
        hadActiveFocusBeforeExitTransition = popupItem->hasActiveFocus();
        if (focus)
            popupItem->setFocus(false);
        transitionState = ExitTransition;
        hideOverlay();
        emit q->aboutToHide();
        emit q->openedChanged();
    }
    return true;
}

void QQuickPopupPrivate::finalizeEnterTransition()
{
    Q_Q(QQuickPopup);
    if (focus)
        popupItem->setFocus(true);
    transitionState = NoTransition;
    emit q->openedChanged();
    emit q->opened();
}

void QQuickPopupPrivate::finalizeExitTransition()
{
    Q_Q(QQuickPopup);
    getPositioner()->setParentItem(nullptr);
    popupItem->setParentItem(nullptr);
    popupItem->setVisible(false);
    destroyOverlay();

    if (hadActiveFocusBeforeExitTransition && window) {
        // restore focus to the next popup in chain, or to the window content if there are no other popups open
        QQuickPopup *nextFocusPopup = nullptr;
        if (QQuickOverlay *overlay = QQuickOverlay::overlay(window)) {
            const auto stackingOrderPopups = QQuickOverlayPrivate::get(overlay)->stackingOrderPopups();
            for (auto popup : stackingOrderPopups) {
                if (QQuickPopupPrivate::get(popup)->transitionState != ExitTransition) {
                    nextFocusPopup = popup;
                    break;
                }
            }
        }
        if (nextFocusPopup && nextFocusPopup->hasFocus()) {
            nextFocusPopup->forceActiveFocus();
        } else {
            QQuickApplicationWindow *applicationWindow = qobject_cast<QQuickApplicationWindow*>(window);
            if (applicationWindow)
                applicationWindow->contentItem()->setFocus(true);
            else
                window->contentItem()->setFocus(true);
        }
    }

    visible = false;
    transitionState = NoTransition;
    hadActiveFocusBeforeExitTransition = false;
    emit q->visibleChanged();
    emit q->closed();
}

QMarginsF QQuickPopupPrivate::getMargins() const
{
    Q_Q(const QQuickPopup);
    return QMarginsF(q->leftMargin(), q->topMargin(), q->rightMargin(), q->bottomMargin());
}

void QQuickPopupPrivate::setTopMargin(qreal value, bool reset)
{
    Q_Q(QQuickPopup);
    qreal oldMargin = q->topMargin();
    topMargin = value;
    hasTopMargin = !reset;
    if ((!reset && !qFuzzyCompare(oldMargin, value)) || (reset && !qFuzzyCompare(oldMargin, margins))) {
        emit q->topMarginChanged();
        q->marginsChange(QMarginsF(leftMargin, topMargin, rightMargin, bottomMargin),
                         QMarginsF(leftMargin, oldMargin, rightMargin, bottomMargin));
    }
}

void QQuickPopupPrivate::setLeftMargin(qreal value, bool reset)
{
    Q_Q(QQuickPopup);
    qreal oldMargin = q->leftMargin();
    leftMargin = value;
    hasLeftMargin = !reset;
    if ((!reset && !qFuzzyCompare(oldMargin, value)) || (reset && !qFuzzyCompare(oldMargin, margins))) {
        emit q->leftMarginChanged();
        q->marginsChange(QMarginsF(leftMargin, topMargin, rightMargin, bottomMargin),
                         QMarginsF(oldMargin, topMargin, rightMargin, bottomMargin));
    }
}

void QQuickPopupPrivate::setRightMargin(qreal value, bool reset)
{
    Q_Q(QQuickPopup);
    qreal oldMargin = q->rightMargin();
    rightMargin = value;
    hasRightMargin = !reset;
    if ((!reset && !qFuzzyCompare(oldMargin, value)) || (reset && !qFuzzyCompare(oldMargin, margins))) {
        emit q->rightMarginChanged();
        q->marginsChange(QMarginsF(leftMargin, topMargin, rightMargin, bottomMargin),
                         QMarginsF(leftMargin, topMargin, oldMargin, bottomMargin));
    }
}

void QQuickPopupPrivate::setBottomMargin(qreal value, bool reset)
{
    Q_Q(QQuickPopup);
    qreal oldMargin = q->bottomMargin();
    bottomMargin = value;
    hasBottomMargin = !reset;
    if ((!reset && !qFuzzyCompare(oldMargin, value)) || (reset && !qFuzzyCompare(oldMargin, margins))) {
        emit q->bottomMarginChanged();
        q->marginsChange(QMarginsF(leftMargin, topMargin, rightMargin, bottomMargin),
                         QMarginsF(leftMargin, topMargin, rightMargin, oldMargin));
    }
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlpropertygroup QtQuick.Controls::Popup::anchors
    \qmlproperty Object QtQuick.Controls::Popup::anchors.centerIn

    Anchors provide a way to position an item by specifying its
    relationship with other items.

    A common use case is to center a popup within its parent. One way to do
    this is with the \l {Item::}{x} and \l {Item::}{y} properties. Anchors offer
    a more convenient approach:

    \qml
    Pane {
        // ...

        Popup {
            anchors.centerIn: parent
        }
    }
    \endqml

    It is also possible to center the popup in the window by using \l Overlay:

    \snippet qtquickcontrols2-popup.qml centerIn

    This makes it easy to center a popup in the window from any component.

    \note Popups can only be centered within their immediate parent or
    the window overlay; trying to center in other items will produce a warning.

    \sa {Popup Positioning}, {Item::anchors}
*/
QQuickPopupAnchors *QQuickPopupPrivate::getAnchors()
{
    Q_Q(QQuickPopup);
    if (!anchors)
        anchors = new QQuickPopupAnchors(q);
    return anchors;
}

QQuickPopupPositioner *QQuickPopupPrivate::getPositioner()
{
    Q_Q(QQuickPopup);
    if (!positioner)
        positioner = new QQuickPopupPositioner(q);
    return positioner;
}

void QQuickPopupPrivate::setWindow(QQuickWindow *newWindow)
{
    Q_Q(QQuickPopup);
    if (window == newWindow)
        return;

    if (window) {
        QQuickOverlay *overlay = QQuickOverlay::overlay(window);
        if (overlay)
            QQuickOverlayPrivate::get(overlay)->removePopup(q);
    }

    window = newWindow;

    if (newWindow) {
        QQuickOverlay *overlay = QQuickOverlay::overlay(newWindow);
        if (overlay)
            QQuickOverlayPrivate::get(overlay)->addPopup(q);

        QQuickControlPrivate *p = QQuickControlPrivate::get(popupItem);
        p->resolveFont();
        p->resolvePalette();
        if (QQuickApplicationWindow *appWindow = qobject_cast<QQuickApplicationWindow *>(newWindow))
            p->updateLocale(appWindow->locale(), false); // explicit=false
    }

    emit q->windowChanged(newWindow);

    if (complete && visible && window)
        transitionManager.transitionEnter();
}

void QQuickPopupPrivate::itemDestroyed(QQuickItem *item)
{
    Q_Q(QQuickPopup);
    if (item == parentItem)
        q->setParentItem(nullptr);
}

void QQuickPopupPrivate::reposition()
{
    getPositioner()->reposition();
}

static QQuickItem *createDimmer(QQmlComponent *component, QQuickPopup *popup, QQuickItem *parent)
{
    QQuickItem *item = nullptr;
    if (component) {
        QQmlContext *creationContext = component->creationContext();
        if (!creationContext)
            creationContext = qmlContext(popup);
        QQmlContext *context = new QQmlContext(creationContext, popup);
        context->setContextObject(popup);
        item = qobject_cast<QQuickItem*>(component->beginCreate(context));
    }

    // when there is no overlay component available (with plain QQuickWindow),
    // use a plain QQuickItem as a fallback to block hover events
    if (!item && popup->isModal())
        item = new QQuickItem;

    if (item) {
        item->setOpacity(popup->isVisible() ? 1.0 : 0.0);
        item->setParentItem(parent);
        item->stackBefore(popup->popupItem());
        item->setZ(popup->z());
        if (popup->isModal()) {
            item->setAcceptedMouseButtons(Qt::AllButtons);
#if QT_CONFIG(cursor)
            item->setCursor(Qt::ArrowCursor);
#endif
#if QT_CONFIG(quicktemplates2_hover)
            // TODO: switch to QStyleHints::useHoverEffects in Qt 5.8
            item->setAcceptHoverEvents(true);
            // item->setAcceptHoverEvents(QGuiApplication::styleHints()->useHoverEffects());
            // connect(QGuiApplication::styleHints(), &QStyleHints::useHoverEffectsChanged, item, &QQuickItem::setAcceptHoverEvents);
#endif
        }
        if (component)
            component->completeCreate();
    }
    return item;
}

void QQuickPopupPrivate::createOverlay()
{
    Q_Q(QQuickPopup);
    QQuickOverlay *overlay = QQuickOverlay::overlay(window);
    if (!overlay)
        return;

    QQmlComponent *component = nullptr;
    QQuickOverlayAttached *overlayAttached = qobject_cast<QQuickOverlayAttached *>(qmlAttachedPropertiesObject<QQuickOverlay>(q, false));
    if (overlayAttached)
        component = modal ? overlayAttached->modal() : overlayAttached->modeless();

    if (!component)
        component = modal ? overlay->modal() : overlay->modeless();

    if (!dimmer)
        dimmer = createDimmer(component, q, overlay);
    resizeOverlay();
}

void QQuickPopupPrivate::destroyOverlay()
{
    if (dimmer) {
        dimmer->setParentItem(nullptr);
        dimmer->deleteLater();
        dimmer = nullptr;
    }
}

void QQuickPopupPrivate::toggleOverlay()
{
    destroyOverlay();
    if (dim)
        createOverlay();
}

void QQuickPopupPrivate::showOverlay()
{
    // use QQmlProperty instead of QQuickItem::setOpacity() to trigger QML Behaviors
    if (dim && dimmer)
        QQmlProperty::write(dimmer, QStringLiteral("opacity"), 1.0);
}

void QQuickPopupPrivate::hideOverlay()
{
    // use QQmlProperty instead of QQuickItem::setOpacity() to trigger QML Behaviors
    if (dim && dimmer)
        QQmlProperty::write(dimmer, QStringLiteral("opacity"), 0.0);
}

void QQuickPopupPrivate::resizeOverlay()
{
    if (!dimmer)
        return;

    qreal w = window ? window->width() : 0;
    qreal h = window ? window->height() : 0;
    dimmer->setSize(QSizeF(w, h));
}

QQuickPopupTransitionManager::QQuickPopupTransitionManager(QQuickPopupPrivate *popup)
    : popup(popup)
{
}

void QQuickPopupTransitionManager::transitionEnter()
{
    if (popup->transitionState == QQuickPopupPrivate::ExitTransition)
        cancel();

    if (!popup->prepareEnterTransition())
        return;

    if (popup->window)
        transition(popup->enterActions, popup->enter, popup->q_func());
    else
        finished();
}

void QQuickPopupTransitionManager::transitionExit()
{
    if (!popup->prepareExitTransition())
        return;

    if (popup->window)
        transition(popup->exitActions, popup->exit, popup->q_func());
    else
        finished();
}

void QQuickPopupTransitionManager::finished()
{
    if (popup->transitionState == QQuickPopupPrivate::EnterTransition)
        popup->finalizeEnterTransition();
    else if (popup->transitionState == QQuickPopupPrivate::ExitTransition)
        popup->finalizeExitTransition();
}

QQuickPopup::QQuickPopup(QObject *parent)
    : QObject(*(new QQuickPopupPrivate), parent)
{
    Q_D(QQuickPopup);
    d->init();
}

QQuickPopup::QQuickPopup(QQuickPopupPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    Q_D(QQuickPopup);
    d->init();
}

QQuickPopup::~QQuickPopup()
{
    Q_D(QQuickPopup);
    setParentItem(nullptr);
    d->popupItem->ungrabShortcut();
    delete d->popupItem;
    d->popupItem = nullptr;
    delete d->positioner;
    d->positioner = nullptr;
}

/*!
    \qmlmethod void QtQuick.Controls::Popup::open()

    Opens the popup.

    \sa visible
*/
void QQuickPopup::open()
{
    setVisible(true);
}

/*!
    \qmlmethod void QtQuick.Controls::Popup::close()

    Closes the popup.

    \sa visible
*/
void QQuickPopup::close()
{
    setVisible(false);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::x

    This property holds the x-coordinate of the popup.

    \sa y, z
*/
qreal QQuickPopup::x() const
{
    Q_D(const QQuickPopup);
    return d->effectiveX;
}

void QQuickPopup::setX(qreal x)
{
    Q_D(QQuickPopup);
    setPosition(QPointF(x, d->y));
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::y

    This property holds the y-coordinate of the popup.

    \sa x, z
*/
qreal QQuickPopup::y() const
{
    Q_D(const QQuickPopup);
    return d->effectiveY;
}

void QQuickPopup::setY(qreal y)
{
    Q_D(QQuickPopup);
    setPosition(QPointF(d->x, y));
}

QPointF QQuickPopup::position() const
{
    Q_D(const QQuickPopup);
    return QPointF(d->effectiveX, d->effectiveY);
}

void QQuickPopup::setPosition(const QPointF &pos)
{
    Q_D(QQuickPopup);
    const bool xChange = !qFuzzyCompare(d->x, pos.x());
    const bool yChange = !qFuzzyCompare(d->y, pos.y());
    if (!xChange && !yChange)
        return;

    d->x = pos.x();
    d->y = pos.y();
    if (d->popupItem->isVisible()) {
        d->reposition();
    } else {
        if (xChange)
            emit xChanged();
        if (yChange)
            emit yChanged();
    }
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::z

    This property holds the z-value of the popup. Z-value determines
    the stacking order of popups.

    If two visible popups have the same z-value, the last one that
    was opened will be on top.

    The default z-value is \c 0.

    \sa x, y
*/
qreal QQuickPopup::z() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->z();
}

void QQuickPopup::setZ(qreal z)
{
    Q_D(QQuickPopup);
    if (qFuzzyCompare(z, d->popupItem->z()))
        return;
    d->popupItem->setZ(z);
    emit zChanged();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::width

    This property holds the width of the popup.
*/
qreal QQuickPopup::width() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->width();
}

void QQuickPopup::setWidth(qreal width)
{
    Q_D(QQuickPopup);
    d->hasWidth = true;
    d->popupItem->setWidth(width);
}

void QQuickPopup::resetWidth()
{
    Q_D(QQuickPopup);
    if (!d->hasWidth)
        return;

    d->hasWidth = false;
    d->popupItem->resetWidth();
    if (d->popupItem->isVisible())
        d->reposition();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::height

    This property holds the height of the popup.
*/
qreal QQuickPopup::height() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->height();
}

void QQuickPopup::setHeight(qreal height)
{
    Q_D(QQuickPopup);
    d->hasHeight = true;
    d->popupItem->setHeight(height);
}

void QQuickPopup::resetHeight()
{
    Q_D(QQuickPopup);
    if (!d->hasHeight)
        return;

    d->hasHeight = false;
    d->popupItem->resetHeight();
    if (d->popupItem->isVisible())
        d->reposition();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::implicitWidth

    This property holds the implicit width of the popup.
*/
qreal QQuickPopup::implicitWidth() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->implicitWidth();
}

void QQuickPopup::setImplicitWidth(qreal width)
{
    Q_D(QQuickPopup);
    d->popupItem->setImplicitWidth(width);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::implicitHeight

    This property holds the implicit height of the popup.
*/
qreal QQuickPopup::implicitHeight() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->implicitHeight();
}

void QQuickPopup::setImplicitHeight(qreal height)
{
    Q_D(QQuickPopup);
    d->popupItem->setImplicitHeight(height);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::contentWidth

    This property holds the content width. It is used for calculating the
    total implicit width of the Popup.

    For more information, see \l {Popup Sizing}.

    \sa contentHeight
*/
qreal QQuickPopup::contentWidth() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->contentWidth();
}

void QQuickPopup::setContentWidth(qreal width)
{
    Q_D(QQuickPopup);
    d->popupItem->setContentWidth(width);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::contentHeight

    This property holds the content height. It is used for calculating the
    total implicit height of the Popup.

    For more information, see \l {Popup Sizing}.

    \sa contentWidth
*/
qreal QQuickPopup::contentHeight() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->contentHeight();
}

void QQuickPopup::setContentHeight(qreal height)
{
    Q_D(QQuickPopup);
    d->popupItem->setContentHeight(height);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::availableWidth
    \readonly

    This property holds the width available to the \l contentItem after
    deducting horizontal padding from the \l {Item::}{width} of the popup.

    \sa padding, leftPadding, rightPadding
*/
qreal QQuickPopup::availableWidth() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->availableWidth();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::availableHeight
    \readonly

    This property holds the height available to the \l contentItem after
    deducting vertical padding from the \l {Item::}{height} of the popup.

    \sa padding, topPadding, bottomPadding
*/
qreal QQuickPopup::availableHeight() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->availableHeight();
}

/*!
    \since QtQuick.Controls 2.1 (Qt 5.8)
    \qmlproperty real QtQuick.Controls::Popup::spacing

    This property holds the spacing.

    Spacing is useful for popups that have multiple or repetitive building
    blocks. For example, some styles use spacing to determine the distance
    between the header, content, and footer of \l Dialog. Spacing is not
    enforced by Popup, so each style may interpret it differently, and some
    may ignore it altogether.
*/
qreal QQuickPopup::spacing() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->spacing();
}

void QQuickPopup::setSpacing(qreal spacing)
{
    Q_D(QQuickPopup);
    d->popupItem->setSpacing(spacing);
}

void QQuickPopup::resetSpacing()
{
    setSpacing(0);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::margins

    This property holds the distance between the edges of the popup and the
    edges of its window.

    A popup with negative margins is not pushed within the bounds
    of the enclosing window. The default value is \c -1.

    \sa topMargin, leftMargin, rightMargin, bottomMargin, {Popup Layout}
*/
qreal QQuickPopup::margins() const
{
    Q_D(const QQuickPopup);
    return d->margins;
}

void QQuickPopup::setMargins(qreal margins)
{
    Q_D(QQuickPopup);
    if (qFuzzyCompare(d->margins, margins))
        return;
    QMarginsF oldMargins(leftMargin(), topMargin(), rightMargin(), bottomMargin());
    d->margins = margins;
    emit marginsChanged();
    QMarginsF newMargins(leftMargin(), topMargin(), rightMargin(), bottomMargin());
    if (!qFuzzyCompare(newMargins.top(), oldMargins.top()))
        emit topMarginChanged();
    if (!qFuzzyCompare(newMargins.left(), oldMargins.left()))
        emit leftMarginChanged();
    if (!qFuzzyCompare(newMargins.right(), oldMargins.right()))
        emit rightMarginChanged();
    if (!qFuzzyCompare(newMargins.bottom(), oldMargins.bottom()))
        emit bottomMarginChanged();
    marginsChange(newMargins, oldMargins);
}

void QQuickPopup::resetMargins()
{
    setMargins(-1);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::topMargin

    This property holds the distance between the top edge of the popup and
    the top edge of its window.

    A popup with a negative top margin is not pushed within the top edge
    of the enclosing window. The default value is \c -1.

    \sa margins, bottomMargin, {Popup Layout}
*/
qreal QQuickPopup::topMargin() const
{
    Q_D(const QQuickPopup);
    if (d->hasTopMargin)
        return d->topMargin;
    return d->margins;
}

void QQuickPopup::setTopMargin(qreal margin)
{
    Q_D(QQuickPopup);
    d->setTopMargin(margin);
}

void QQuickPopup::resetTopMargin()
{
    Q_D(QQuickPopup);
    d->setTopMargin(-1, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::leftMargin

    This property holds the distance between the left edge of the popup and
    the left edge of its window.

    A popup with a negative left margin is not pushed within the left edge
    of the enclosing window. The default value is \c -1.

    \sa margins, rightMargin, {Popup Layout}
*/
qreal QQuickPopup::leftMargin() const
{
    Q_D(const QQuickPopup);
    if (d->hasLeftMargin)
        return d->leftMargin;
    return d->margins;
}

void QQuickPopup::setLeftMargin(qreal margin)
{
    Q_D(QQuickPopup);
    d->setLeftMargin(margin);
}

void QQuickPopup::resetLeftMargin()
{
    Q_D(QQuickPopup);
    d->setLeftMargin(-1, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::rightMargin

    This property holds the distance between the right edge of the popup and
    the right edge of its window.

    A popup with a negative right margin is not pushed within the right edge
    of the enclosing window. The default value is \c -1.

    \sa margins, leftMargin, {Popup Layout}
*/
qreal QQuickPopup::rightMargin() const
{
    Q_D(const QQuickPopup);
    if (d->hasRightMargin)
        return d->rightMargin;
    return d->margins;
}

void QQuickPopup::setRightMargin(qreal margin)
{
    Q_D(QQuickPopup);
    d->setRightMargin(margin);
}

void QQuickPopup::resetRightMargin()
{
    Q_D(QQuickPopup);
    d->setRightMargin(-1, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::bottomMargin

    This property holds the distance between the bottom edge of the popup and
    the bottom edge of its window.

    A popup with a negative bottom margin is not pushed within the bottom edge
    of the enclosing window. The default value is \c -1.

    \sa margins, topMargin, {Popup Layout}
*/
qreal QQuickPopup::bottomMargin() const
{
    Q_D(const QQuickPopup);
    if (d->hasBottomMargin)
        return d->bottomMargin;
    return d->margins;
}

void QQuickPopup::setBottomMargin(qreal margin)
{
    Q_D(QQuickPopup);
    d->setBottomMargin(margin);
}

void QQuickPopup::resetBottomMargin()
{
    Q_D(QQuickPopup);
    d->setBottomMargin(-1, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::padding

    This property holds the default padding.

    \include qquickpopup-padding.qdocinc

    \sa availableWidth, availableHeight, topPadding, leftPadding, rightPadding, bottomPadding
*/
qreal QQuickPopup::padding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->padding();
}

void QQuickPopup::setPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setPadding(padding);
}

void QQuickPopup::resetPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetPadding();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::topPadding

    This property holds the top padding. Unless explicitly set, the value
    is equal to \c verticalPadding.

    \include qquickpopup-padding.qdocinc

    \sa padding, bottomPadding, verticalPadding, availableHeight
*/
qreal QQuickPopup::topPadding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->topPadding();
}

void QQuickPopup::setTopPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setTopPadding(padding);
}

void QQuickPopup::resetTopPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetTopPadding();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::leftPadding

    This property holds the left padding. Unless explicitly set, the value
    is equal to \c horizontalPadding.

    \include qquickpopup-padding.qdocinc

    \sa padding, rightPadding, horizontalPadding, availableWidth
*/
qreal QQuickPopup::leftPadding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->leftPadding();
}

void QQuickPopup::setLeftPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setLeftPadding(padding);
}

void QQuickPopup::resetLeftPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetLeftPadding();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::rightPadding

    This property holds the right padding. Unless explicitly set, the value
    is equal to \c horizontalPadding.

    \include qquickpopup-padding.qdocinc

    \sa padding, leftPadding, horizontalPadding, availableWidth
*/
qreal QQuickPopup::rightPadding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->rightPadding();
}

void QQuickPopup::setRightPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setRightPadding(padding);
}

void QQuickPopup::resetRightPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetRightPadding();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::bottomPadding

    This property holds the bottom padding. Unless explicitly set, the value
    is equal to \c verticalPadding.

    \include qquickpopup-padding.qdocinc

    \sa padding, topPadding, verticalPadding, availableHeight
*/
qreal QQuickPopup::bottomPadding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->bottomPadding();
}

void QQuickPopup::setBottomPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setBottomPadding(padding);
}

void QQuickPopup::resetBottomPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetBottomPadding();
}

/*!
    \qmlproperty Locale QtQuick.Controls::Popup::locale

    This property holds the locale of the popup.

    \sa mirrored, {LayoutMirroring}{LayoutMirroring}
*/
QLocale QQuickPopup::locale() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->locale();
}

void QQuickPopup::setLocale(const QLocale &locale)
{
    Q_D(QQuickPopup);
    d->popupItem->setLocale(locale);
}

void QQuickPopup::resetLocale()
{
    Q_D(QQuickPopup);
    d->popupItem->resetLocale();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::Popup::mirrored
    \readonly

    This property holds whether the popup is mirrored.

    This property is provided for convenience. A popup is considered mirrored
    when its visual layout direction is right-to-left; that is, when using a
    right-to-left locale.

    \sa locale, {Right-to-left User Interfaces}
*/
bool QQuickPopup::isMirrored() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->isMirrored();
}

/*!
    \qmlproperty font QtQuick.Controls::Popup::font

    This property holds the font currently set for the popup.

    Popup propagates explicit font properties to its children. If you change a specific
    property on a popup's font, that property propagates to all of the popup's children,
    overriding any system defaults for that property.

    \code
    Popup {
        font.family: "Courier"

        Column {
            Label {
                text: qsTr("This will use Courier...")
            }

            Switch {
                text: qsTr("... and so will this")
            }
        }
    }
    \endcode

    \sa Control::font, ApplicationWindow::font
*/
QFont QQuickPopup::font() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->font();
}

void QQuickPopup::setFont(const QFont &font)
{
    Q_D(QQuickPopup);
    d->popupItem->setFont(font);
}

void QQuickPopup::resetFont()
{
    Q_D(QQuickPopup);
    d->popupItem->resetFont();
}


/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty palette QtQuick.Controls::Popup::palette

    This property holds the palette currently set for the popup.

    Popup propagates explicit palette properties to its children. If you change a specific
    property on a popup's palette, that property propagates to all of the popup's children,
    overriding any system defaults for that property.

    \code
    Popup {
        palette.text: "red"

        Column {
            Label {
                text: qsTr("This will use red color...")
            }

            Switch {
                text: qsTr("... and so will this")
            }
        }
    }
    \endcode

    \sa Control::palette, ApplicationWindow::palette, {qtquickcontrols2-palette}{palette QML Basic Type}
*/
QPalette QQuickPopup::palette() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->palette();
}

void QQuickPopup::setPalette(const QPalette &palette)
{
    Q_D(QQuickPopup);
    d->popupItem->setPalette(palette);
}

void QQuickPopup::resetPalette()
{
    Q_D(QQuickPopup);
    d->popupItem->resetPalette();
}

QQuickWindow *QQuickPopup::window() const
{
    Q_D(const QQuickPopup);
    return d->window;
}

QQuickItem *QQuickPopup::popupItem() const
{
    Q_D(const QQuickPopup);
    return d->popupItem;
}

/*!
    \qmlproperty Item QtQuick.Controls::Popup::parent

    This property holds the parent item.
*/
QQuickItem *QQuickPopup::parentItem() const
{
    Q_D(const QQuickPopup);
    return d->parentItem;
}

void QQuickPopup::setParentItem(QQuickItem *parent)
{
    Q_D(QQuickPopup);
    if (d->parentItem == parent)
        return;

    if (d->parentItem) {
        QObjectPrivate::disconnect(d->parentItem, &QQuickItem::windowChanged, d, &QQuickPopupPrivate::setWindow);
        QQuickItemPrivate::get(d->parentItem)->removeItemChangeListener(d, QQuickItemPrivate::Destroyed);
    }
    d->parentItem = parent;
    QQuickPopupPositioner *positioner = d->getPositioner();
    if (positioner->parentItem())
        positioner->setParentItem(parent);
    if (parent) {
        QObjectPrivate::connect(parent, &QQuickItem::windowChanged, d, &QQuickPopupPrivate::setWindow);
        QQuickItemPrivate::get(d->parentItem)->addItemChangeListener(d, QQuickItemPrivate::Destroyed);
    } else {
        close();
    }
    d->setWindow(parent ? parent->window() : nullptr);
    emit parentChanged();
}

void QQuickPopup::resetParentItem()
{
    if (QQuickWindow *window = qobject_cast<QQuickWindow *>(parent()))
        setParentItem(window->contentItem());
    else
        setParentItem(qobject_cast<QQuickItem *>(parent()));
}

/*!
    \qmlproperty Item QtQuick.Controls::Popup::background

    This property holds the background item.

    \note If the background item has no explicit size specified, it automatically
          follows the popup's size. In most cases, there is no need to specify
          width or height for a background item.

    \note Most popups use the implicit size of the background item to calculate
    the implicit size of the popup itself. If you replace the background item
    with a custom one, you should also consider providing a sensible implicit
    size for it (unless it is an item like \l Image which has its own implicit
    size).

    \sa {Customizing Popup}
*/
QQuickItem *QQuickPopup::background() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->background();
}

void QQuickPopup::setBackground(QQuickItem *background)
{
    Q_D(QQuickPopup);
    d->popupItem->setBackground(background);
}

/*!
    \qmlproperty Item QtQuick.Controls::Popup::contentItem

    This property holds the content item of the popup.

    The content item is the visual implementation of the popup. When the
    popup is made visible, the content item is automatically reparented to
    the \l {Overlay::overlay}{overlay item}.

    \note The content item is automatically resized to fit within the
    \l padding of the popup.

    \note Most popups use the implicit size of the content item to calculate
    the implicit size of the popup itself. If you replace the content item
    with a custom one, you should also consider providing a sensible implicit
    size for it (unless it is an item like \l Text which has its own implicit
    size).

    \sa {Customizing Popup}
*/
QQuickItem *QQuickPopup::contentItem() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->contentItem();
}

void QQuickPopup::setContentItem(QQuickItem *item)
{
    Q_D(QQuickPopup);
    d->popupItem->setContentItem(item);
}

/*!
    \qmlproperty list<Object> QtQuick.Controls::Popup::contentData
    \default

    This property holds the list of content data.

    The list contains all objects that have been declared in QML as children
    of the popup.

    \note Unlike \c contentChildren, \c contentData does include non-visual QML
    objects.

    \sa Item::data, contentChildren
*/
QQmlListProperty<QObject> QQuickPopupPrivate::contentData()
{
    QQuickControlPrivate *p = QQuickControlPrivate::get(popupItem);
    if (!p->contentItem)
        p->executeContentItem();
    return QQmlListProperty<QObject>(popupItem->contentItem(), nullptr,
                                     QQuickItemPrivate::data_append,
                                     QQuickItemPrivate::data_count,
                                     QQuickItemPrivate::data_at,
                                     QQuickItemPrivate::data_clear);
}

/*!
    \qmlproperty list<Item> QtQuick.Controls::Popup::contentChildren

    This property holds the list of content children.

    The list contains all items that have been declared in QML as children
    of the popup.

    \note Unlike \c contentData, \c contentChildren does not include non-visual
    QML objects.

    \sa Item::children, contentData
*/
QQmlListProperty<QQuickItem> QQuickPopupPrivate::contentChildren()
{
    return QQmlListProperty<QQuickItem>(popupItem->contentItem(), nullptr,
                                        QQuickItemPrivate::children_append,
                                        QQuickItemPrivate::children_count,
                                        QQuickItemPrivate::children_at,
                                        QQuickItemPrivate::children_clear);
}

/*!
    \qmlproperty bool QtQuick.Controls::Popup::clip

    This property holds whether clipping is enabled. The default value is \c false.
*/
bool QQuickPopup::clip() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->clip();
}

void QQuickPopup::setClip(bool clip)
{
    Q_D(QQuickPopup);
    if (clip == d->popupItem->clip())
        return;
    d->popupItem->setClip(clip);
    emit clipChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::Popup::focus

    This property holds whether the popup wants focus.

    When the popup actually receives focus, \l activeFocus will be \c true.
    For more information, see \l {Keyboard Focus in Qt Quick}.

    The default value is \c false.

    \sa activeFocus
*/
bool QQuickPopup::hasFocus() const
{
    Q_D(const QQuickPopup);
    return d->focus;
}

void QQuickPopup::setFocus(bool focus)
{
    Q_D(QQuickPopup);
    if (d->focus == focus)
        return;
    d->focus = focus;
    emit focusChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::Popup::activeFocus
    \readonly

    This property holds whether the popup has active focus.

    \sa focus, {Keyboard Focus in Qt Quick}
*/
bool QQuickPopup::hasActiveFocus() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->hasActiveFocus();
}

/*!
    \qmlproperty bool QtQuick.Controls::Popup::modal

    This property holds whether the popup is modal.

    Modal popups often have a distinctive background dimming effect defined
    in \l {Overlay::modal}{Overlay.modal}, and do not allow press
    or release events through to items beneath them. For example, if the user
    accidentally clicks outside of a popup, any item beneath that popup at
    the location of the click will not receive the event.

    On desktop platforms, it is common for modal popups to be closed only when
    the escape key is pressed. To achieve this behavior, set
    \l closePolicy to \c Popup.CloseOnEscape. By default, \c closePolicy
    is set to \c {Popup.CloseOnEscape | Popup.CloseOnPressOutside}, which
    means that clicking outside of a modal popup will close it.

    The default value is \c false.

    \sa dim
*/
bool QQuickPopup::isModal() const
{
    Q_D(const QQuickPopup);
    return d->modal;
}

void QQuickPopup::setModal(bool modal)
{
    Q_D(QQuickPopup);
    if (d->modal == modal)
        return;
    d->modal = modal;
    if (d->complete && d->visible)
        d->toggleOverlay();
    emit modalChanged();

    if (!d->hasDim) {
        setDim(modal);
        d->hasDim = false;
    }
}

/*!
    \qmlproperty bool QtQuick.Controls::Popup::dim

    This property holds whether the popup dims the background.

    Unless explicitly set, this property follows the value of \l modal. To
    return to the default value, set this property to \c undefined.

    \sa modal, {Overlay::modeless}{Overlay.modeless}
*/
bool QQuickPopup::dim() const
{
    Q_D(const QQuickPopup);
    return d->dim;
}

void QQuickPopup::setDim(bool dim)
{
    Q_D(QQuickPopup);
    d->hasDim = true;

    if (d->dim == dim)
        return;

    d->dim = dim;
    if (d->complete && d->visible)
        d->toggleOverlay();
    emit dimChanged();
}

void QQuickPopup::resetDim()
{
    Q_D(QQuickPopup);
    if (!d->hasDim)
        return;

    setDim(d->modal);
    d->hasDim = false;
}

/*!
    \qmlproperty bool QtQuick.Controls::Popup::visible

    This property holds whether the popup is visible. The default value is \c false.

    \sa open(), close(), opened
*/
bool QQuickPopup::isVisible() const
{
    Q_D(const QQuickPopup);
    return d->visible && d->popupItem->isVisible();
}

void QQuickPopup::setVisible(bool visible)
{
    Q_D(QQuickPopup);
    if (d->visible == visible && d->transitionState != QQuickPopupPrivate::ExitTransition)
        return;

    if (d->complete) {
        if (visible)
            d->transitionManager.transitionEnter();
        else
            d->transitionManager.transitionExit();
    } else {
        d->visible = visible;
    }
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::Popup::enabled

    This property holds whether the popup is enabled. The default value is \c true.

    \sa visible, Item::enabled
*/
bool QQuickPopup::isEnabled() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->isEnabled();
}

void QQuickPopup::setEnabled(bool enabled)
{
    Q_D(QQuickPopup);
    d->popupItem->setEnabled(enabled);
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::Popup::opened

    This property holds whether the popup is fully open. The popup is considered opened
    when it's visible and neither the \l enter nor \l exit transitions are running.

    \sa open(), close(), visible
*/
bool QQuickPopup::isOpened() const
{
    Q_D(const QQuickPopup);
    return d->transitionState == QQuickPopupPrivate::NoTransition && isVisible();
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::opacity

    This property holds the opacity of the popup. Opacity is specified as a number between
    \c 0.0 (fully transparent) and \c 1.0 (fully opaque). The default value is \c 1.0.

    \sa visible
*/
qreal QQuickPopup::opacity() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->opacity();
}

void QQuickPopup::setOpacity(qreal opacity)
{
    Q_D(QQuickPopup);
    d->popupItem->setOpacity(opacity);
}

/*!
    \qmlproperty real QtQuick.Controls::Popup::scale

    This property holds the scale factor of the popup. The default value is \c 1.0.

    A scale of less than \c 1.0 causes the popup to be rendered at a smaller size,
    and a scale greater than \c 1.0 renders the popup at a larger size. Negative
    scales are not supported.
*/
qreal QQuickPopup::scale() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->scale();
}

void QQuickPopup::setScale(qreal scale)
{
    Q_D(QQuickPopup);
    if (qFuzzyCompare(scale, d->popupItem->scale()))
        return;
    d->popupItem->setScale(scale);
    emit scaleChanged();
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Popup::closePolicy

    This property determines the circumstances under which the popup closes.
    The flags can be combined to allow several ways of closing the popup.

    The available values are:
    \value Popup.NoAutoClose The popup will only close when manually instructed to do so.
    \value Popup.CloseOnPressOutside The popup will close when the mouse is pressed outside of it.
    \value Popup.CloseOnPressOutsideParent The popup will close when the mouse is pressed outside of its parent.
    \value Popup.CloseOnReleaseOutside The popup will close when the mouse is released outside of it.
    \value Popup.CloseOnReleaseOutsideParent The popup will close when the mouse is released outside of its parent.
    \value Popup.CloseOnEscape The popup will close when the escape key is pressed while the popup
        has active focus.

    The default value is \c {Popup.CloseOnEscape | Popup.CloseOnPressOutside}.

    \note There is a known limitation that the \c Popup.CloseOnReleaseOutside
        and \c Popup.CloseOnReleaseOutsideParent policies only work with
        \l modal popups.
*/
QQuickPopup::ClosePolicy QQuickPopup::closePolicy() const
{
    Q_D(const QQuickPopup);
    return d->closePolicy;
}

void QQuickPopup::setClosePolicy(ClosePolicy policy)
{
    Q_D(QQuickPopup);
    d->hasClosePolicy = true;
    if (d->closePolicy == policy)
        return;
    d->closePolicy = policy;
    if (isVisible()) {
        if (policy & QQuickPopup::CloseOnEscape)
            d->popupItem->grabShortcut();
        else
            d->popupItem->ungrabShortcut();
    }
    emit closePolicyChanged();
}

void QQuickPopup::resetClosePolicy()
{
    Q_D(QQuickPopup);
    setClosePolicy(QQuickPopupPrivate::DefaultClosePolicy);
    d->hasClosePolicy = false;
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Popup::transformOrigin

    This property holds the origin point for transformations in enter and exit transitions.

    Nine transform origins are available, as shown in the image below.
    The default transform origin is \c Popup.Center.

    \image qtquickcontrols2-popup-transformorigin.png

    \sa enter, exit, Item::transformOrigin
*/
QQuickPopup::TransformOrigin QQuickPopup::transformOrigin() const
{
    Q_D(const QQuickPopup);
    return static_cast<TransformOrigin>(d->popupItem->transformOrigin());
}

void QQuickPopup::setTransformOrigin(TransformOrigin origin)
{
    Q_D(QQuickPopup);
    d->popupItem->setTransformOrigin(static_cast<QQuickItem::TransformOrigin>(origin));
}

/*!
    \qmlproperty Transition QtQuick.Controls::Popup::enter

    This property holds the transition that is applied to the popup item
    when the popup is opened and enters the screen.

    The following example animates the opacity of the popup when it enters
    the screen:
    \code
    Popup {
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }
    }
    \endcode

    \sa exit
*/
QQuickTransition *QQuickPopup::enter() const
{
    Q_D(const QQuickPopup);
    return d->enter;
}

void QQuickPopup::setEnter(QQuickTransition *transition)
{
    Q_D(QQuickPopup);
    if (d->enter == transition)
        return;
    d->enter = transition;
    emit enterChanged();
}

/*!
    \qmlproperty Transition QtQuick.Controls::Popup::exit

    This property holds the transition that is applied to the popup item
    when the popup is closed and exits the screen.

    The following example animates the opacity of the popup when it exits
    the screen:
    \code
    Popup {
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }
    }
    \endcode

    \sa enter
*/
QQuickTransition *QQuickPopup::exit() const
{
    Q_D(const QQuickPopup);
    return d->exit;
}

void QQuickPopup::setExit(QQuickTransition *transition)
{
    Q_D(QQuickPopup);
    if (d->exit == transition)
        return;
    d->exit = transition;
    emit exitChanged();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::horizontalPadding

    This property holds the horizontal padding. Unless explicitly set, the value
    is equal to \c padding.

    \include qquickpopup-padding.qdocinc

    \sa padding, leftPadding, rightPadding, verticalPadding
*/
qreal QQuickPopup::horizontalPadding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->horizontalPadding();
}

void QQuickPopup::setHorizontalPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setHorizontalPadding(padding);
}

void QQuickPopup::resetHorizontalPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetHorizontalPadding();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::verticalPadding

    This property holds the vertical padding. Unless explicitly set, the value
    is equal to \c padding.

    \include qquickpopup-padding.qdocinc

    \sa padding, topPadding, bottomPadding, horizontalPadding
*/
qreal QQuickPopup::verticalPadding() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->verticalPadding();
}

void QQuickPopup::setVerticalPadding(qreal padding)
{
    Q_D(QQuickPopup);
    d->popupItem->setVerticalPadding(padding);
}

void QQuickPopup::resetVerticalPadding()
{
    Q_D(QQuickPopup);
    d->popupItem->resetVerticalPadding();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::implicitContentWidth
    \readonly

    This property holds the implicit content width.

    The value is calculated based on the content children.

    \sa implicitContentHeight, implicitBackgroundWidth
*/
qreal QQuickPopup::implicitContentWidth() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->implicitContentWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::implicitContentHeight
    \readonly

    This property holds the implicit content height.

    The value is calculated based on the content children.

    \sa implicitContentWidth, implicitBackgroundHeight
*/
qreal QQuickPopup::implicitContentHeight() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->implicitContentHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::implicitBackgroundWidth
    \readonly

    This property holds the implicit background width.

    The value is equal to \c {background ? background.implicitWidth : 0}.

    \sa implicitBackgroundHeight, implicitContentWidth
*/
qreal QQuickPopup::implicitBackgroundWidth() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->implicitBackgroundWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::implicitBackgroundHeight
    \readonly

    This property holds the implicit background height.

    The value is equal to \c {background ? background.implicitHeight : 0}.

    \sa implicitBackgroundWidth, implicitContentHeight
*/
qreal QQuickPopup::implicitBackgroundHeight() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->implicitBackgroundHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::topInset

    This property holds the top inset for the background.

    \sa {Popup Layout}, bottomInset
*/
qreal QQuickPopup::topInset() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->topInset();
}

void QQuickPopup::setTopInset(qreal inset)
{
    Q_D(QQuickPopup);
    d->popupItem->setTopInset(inset);
}

void QQuickPopup::resetTopInset()
{
    Q_D(QQuickPopup);
    d->popupItem->resetTopInset();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::leftInset

    This property holds the left inset for the background.

    \sa {Popup Layout}, rightInset
*/
qreal QQuickPopup::leftInset() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->leftInset();
}

void QQuickPopup::setLeftInset(qreal inset)
{
    Q_D(QQuickPopup);
    d->popupItem->setLeftInset(inset);
}

void QQuickPopup::resetLeftInset()
{
    Q_D(QQuickPopup);
    d->popupItem->resetLeftInset();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::rightInset

    This property holds the right inset for the background.

    \sa {Popup Layout}, leftInset
*/
qreal QQuickPopup::rightInset() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->rightInset();
}

void QQuickPopup::setRightInset(qreal inset)
{
    Q_D(QQuickPopup);
    d->popupItem->setRightInset(inset);
}

void QQuickPopup::resetRightInset()
{
    Q_D(QQuickPopup);
    d->popupItem->resetRightInset();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Popup::bottomInset

    This property holds the bottom inset for the background.

    \sa {Popup Layout}, topInset
*/
qreal QQuickPopup::bottomInset() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->bottomInset();
}

void QQuickPopup::setBottomInset(qreal inset)
{
    Q_D(QQuickPopup);
    d->popupItem->setBottomInset(inset);
}

void QQuickPopup::resetBottomInset()
{
    Q_D(QQuickPopup);
    d->popupItem->resetBottomInset();
}

bool QQuickPopup::filtersChildMouseEvents() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->filtersChildMouseEvents();
}

void QQuickPopup::setFiltersChildMouseEvents(bool filter)
{
    Q_D(QQuickPopup);
    d->popupItem->setFiltersChildMouseEvents(filter);
}

/*!
    \qmlmethod QtQuick.Controls::Popup::forceActiveFocus(reason = Qt.OtherFocusReason)

    Forces active focus on the popup with the given \a reason.

    This method sets focus on the popup and ensures that all ancestor
    \l FocusScope objects in the object hierarchy are also given \l focus.

    \sa activeFocus, Qt::FocusReason
*/
void QQuickPopup::forceActiveFocus(Qt::FocusReason reason)
{
    Q_D(QQuickPopup);
    d->popupItem->forceActiveFocus(reason);
}

void QQuickPopup::classBegin()
{
    Q_D(QQuickPopup);
    d->complete = false;
    QQmlContext *context = qmlContext(this);
    if (context)
        QQmlEngine::setContextForObject(d->popupItem, context);
    d->popupItem->classBegin();
}

void QQuickPopup::componentComplete()
{
    Q_D(QQuickPopup);
    if (!parentItem())
        resetParentItem();

    if (d->visible && d->window)
        d->transitionManager.transitionEnter();

    d->complete = true;
    d->popupItem->componentComplete();
}

bool QQuickPopup::isComponentComplete() const
{
    Q_D(const QQuickPopup);
    return d->complete;
}

bool QQuickPopup::childMouseEventFilter(QQuickItem *child, QEvent *event)
{
    Q_UNUSED(child);
    Q_UNUSED(event);
    return false;
}

void QQuickPopup::focusInEvent(QFocusEvent *event)
{
    event->accept();
}

void QQuickPopup::focusOutEvent(QFocusEvent *event)
{
    event->accept();
}

void QQuickPopup::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickPopup);
    event->accept();

    if (hasActiveFocus() && (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab))
        QQuickItemPrivate::focusNextPrev(d->popupItem, event->key() == Qt::Key_Tab);
}

void QQuickPopup::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();
}

void QQuickPopup::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickPopup);
    d->handleMouseEvent(d->popupItem, event);
    event->accept();
}

void QQuickPopup::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QQuickPopup);
    d->handleMouseEvent(d->popupItem, event);
    event->accept();
}

void QQuickPopup::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QQuickPopup);
    d->handleMouseEvent(d->popupItem, event);
    event->accept();
}

void QQuickPopup::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();
}

void QQuickPopup::mouseUngrabEvent()
{
    Q_D(QQuickPopup);
    d->handleUngrab();
}

bool QQuickPopup::overlayEvent(QQuickItem *item, QEvent *event)
{
    Q_D(QQuickPopup);
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
        if (d->modal)
            event->accept();
        return d->modal;

#if QT_CONFIG(quicktemplates2_multitouch)
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        return d->handleTouchEvent(item, static_cast<QTouchEvent *>(event));
#endif

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        return d->handleMouseEvent(item, static_cast<QMouseEvent *>(event));

    default:
        return false;
    }
}

#if QT_CONFIG(quicktemplates2_multitouch)
void QQuickPopup::touchEvent(QTouchEvent *event)
{
    Q_D(QQuickPopup);
    d->handleTouchEvent(d->popupItem, event);
}

void QQuickPopup::touchUngrabEvent()
{
    Q_D(QQuickPopup);
    d->handleUngrab();
}
#endif

#if QT_CONFIG(wheelevent)
void QQuickPopup::wheelEvent(QWheelEvent *event)
{
    event->accept();
}
#endif

void QQuickPopup::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_UNUSED(newItem);
    Q_UNUSED(oldItem);
}

void QQuickPopup::contentSizeChange(const QSizeF &newSize, const QSizeF &oldSize)
{
    if (!qFuzzyCompare(newSize.width(), oldSize.width()))
        emit contentWidthChanged();
    if (!qFuzzyCompare(newSize.height(), oldSize.height()))
        emit contentHeightChanged();
}

void QQuickPopup::fontChange(const QFont &newFont, const QFont &oldFont)
{
    Q_UNUSED(newFont);
    Q_UNUSED(oldFont);
    emit fontChanged();
}

void QQuickPopup::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickPopup);
    d->reposition();
    if (!qFuzzyCompare(newGeometry.width(), oldGeometry.width())) {
        emit widthChanged();
        emit availableWidthChanged();
    }
    if (!qFuzzyCompare(newGeometry.height(), oldGeometry.height())) {
        emit heightChanged();
        emit availableHeightChanged();
    }
}

void QQuickPopup::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    Q_D(QQuickPopup);

    switch (change) {
    case QQuickItem::ItemActiveFocusHasChanged:
        emit activeFocusChanged();
        break;
    case QQuickItem::ItemOpacityHasChanged:
        emit opacityChanged();
        break;
    case QQuickItem::ItemVisibleHasChanged:
        if (isComponentComplete() && d->closePolicy & CloseOnEscape) {
            if (data.boolValue)
                d->popupItem->grabShortcut();
            else
                d->popupItem->ungrabShortcut();
        }
        break;
    default:
        break;
    }
}

void QQuickPopup::localeChange(const QLocale &newLocale, const QLocale &oldLocale)
{
    Q_UNUSED(newLocale);
    Q_UNUSED(oldLocale);
    emit localeChanged();
}

void QQuickPopup::marginsChange(const QMarginsF &newMargins, const QMarginsF &oldMargins)
{
    Q_D(QQuickPopup);
    Q_UNUSED(newMargins);
    Q_UNUSED(oldMargins);
    d->reposition();
}

void QQuickPopup::paddingChange(const QMarginsF &newPadding, const QMarginsF &oldPadding)
{
    const bool tp = !qFuzzyCompare(newPadding.top(), oldPadding.top());
    const bool lp = !qFuzzyCompare(newPadding.left(), oldPadding.left());
    const bool rp = !qFuzzyCompare(newPadding.right(), oldPadding.right());
    const bool bp = !qFuzzyCompare(newPadding.bottom(), oldPadding.bottom());

    if (tp)
        emit topPaddingChanged();
    if (lp)
        emit leftPaddingChanged();
    if (rp)
        emit rightPaddingChanged();
    if (bp)
        emit bottomPaddingChanged();

    if (lp || rp) {
        emit horizontalPaddingChanged();
        emit availableWidthChanged();
    }
    if (tp || bp) {
        emit verticalPaddingChanged();
        emit availableHeightChanged();
    }
}

void QQuickPopup::paletteChange(const QPalette &newPalette, const QPalette &oldPalette)
{
    Q_UNUSED(newPalette);
    Q_UNUSED(oldPalette);
    emit paletteChanged();
}

void QQuickPopup::spacingChange(qreal newSpacing, qreal oldSpacing)
{
    Q_UNUSED(newSpacing);
    Q_UNUSED(oldSpacing);
    emit spacingChanged();
}

void QQuickPopup::insetChange(const QMarginsF &newInset, const QMarginsF &oldInset)
{
    if (!qFuzzyCompare(newInset.top(), oldInset.top()))
        emit topInsetChanged();
    if (!qFuzzyCompare(newInset.left(), oldInset.left()))
        emit leftInsetChanged();
    if (!qFuzzyCompare(newInset.right(), oldInset.right()))
        emit rightInsetChanged();
    if (!qFuzzyCompare(newInset.bottom(), oldInset.bottom()))
        emit bottomInsetChanged();
}

QFont QQuickPopup::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::System);
}

QPalette QQuickPopup::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::System);
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickPopup::accessibleRole() const
{
    return QAccessible::Dialog;
}

void QQuickPopup::accessibilityActiveChanged(bool active)
{
    Q_UNUSED(active);
}
#endif

QString QQuickPopup::accessibleName() const
{
    Q_D(const QQuickPopup);
    return d->popupItem->accessibleName();
}

void QQuickPopup::setAccessibleName(const QString &name)
{
    Q_D(QQuickPopup);
    d->popupItem->setAccessibleName(name);
}

QVariant QQuickPopup::accessibleProperty(const char *propertyName)
{
    Q_D(const QQuickPopup);
    return d->popupItem->accessibleProperty(propertyName);
}

bool QQuickPopup::setAccessibleProperty(const char *propertyName, const QVariant &value)
{
    Q_D(QQuickPopup);
    return d->popupItem->setAccessibleProperty(propertyName, value);
}

QT_END_NAMESPACE

#include "moc_qquickpopup_p.cpp"
