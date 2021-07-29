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

#include "qquickabstractbutton_p.h"
#include "qquickabstractbutton_p_p.h"
#include "qquickbuttongroup_p.h"
#include "qquickaction_p.h"
#include "qquickaction_p_p.h"
#include "qquickshortcutcontext_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtGui/qstylehints.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/private/qshortcutmap_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/private/qquickevents_p_p.h>
#include <QtQml/qqmllist.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype AbstractButton
    \inherits Control
    \instantiates QQuickAbstractButton
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-buttons
    \brief Abstract base type providing functionality common to buttons.

    AbstractButton provides the interface for controls with button-like
    behavior; for example, push buttons and checkable controls like
    radio buttons and check boxes. As an abstract control, it has no delegate
    implementations, leaving them to the types that derive from it.

    \sa ButtonGroup, {Button Controls}
*/

/*!
    \qmlsignal QtQuick.Controls::AbstractButton::pressed()

    This signal is emitted when the button is interactively pressed by the user via touch, mouse, or keyboard.
*/

/*!
    \qmlsignal QtQuick.Controls::AbstractButton::released()

    This signal is emitted when the button is interactively released by the user via touch, mouse, or keyboard.
*/

/*!
    \qmlsignal QtQuick.Controls::AbstractButton::canceled()

    This signal is emitted when the button loses mouse grab
    while being pressed, or when it would emit the \l released
    signal but the mouse cursor is not inside the button.
*/

/*!
    \qmlsignal QtQuick.Controls::AbstractButton::clicked()

    This signal is emitted when the button is interactively clicked by the user via touch, mouse, or keyboard.
*/

/*!
    \since QtQuick.Controls 2.2 (Qt 5.9)
    \qmlsignal QtQuick.Controls::AbstractButton::toggled()

    This signal is emitted when a checkable button is interactively toggled by the user via touch, mouse, or keyboard.
*/

/*!
    \qmlsignal QtQuick.Controls::AbstractButton::pressAndHold()

    This signal is emitted when the button is interactively pressed and held down by the user via touch or mouse.
*/

/*!
    \qmlsignal QtQuick.Controls::AbstractButton::doubleClicked()

    This signal is emitted when the button is interactively double clicked by the user via touch or mouse.
*/

void QQuickAbstractButtonPrivate::setPressPoint(const QPointF &point)
{
    pressPoint = point;
    setMovePoint(point);
}

void QQuickAbstractButtonPrivate::setMovePoint(const QPointF &point)
{
    Q_Q(QQuickAbstractButton);
    bool xChange = !qFuzzyCompare(point.x(), movePoint.x());
    bool yChange = !qFuzzyCompare(point.y(), movePoint.y());
    movePoint = point;
    if (xChange)
        emit q->pressXChanged();
    if (yChange)
        emit q->pressYChanged();
}

void QQuickAbstractButtonPrivate::handlePress(const QPointF &point)
{
    Q_Q(QQuickAbstractButton);
    QQuickControlPrivate::handlePress(point);
    setPressPoint(point);
    q->setPressed(true);

    emit q->pressed();

    if (autoRepeat)
        startRepeatDelay();
    else if (touchId != -1 || Qt::LeftButton == (pressButtons & Qt::LeftButton))
        startPressAndHold();
    else
        stopPressAndHold();
}

void QQuickAbstractButtonPrivate::handleMove(const QPointF &point)
{
    Q_Q(QQuickAbstractButton);
    QQuickControlPrivate::handleMove(point);
    setMovePoint(point);
    q->setPressed(keepPressed || q->contains(point));

    if (!pressed && autoRepeat)
        stopPressRepeat();
    else if (holdTimer > 0 && (!pressed || QLineF(pressPoint, point).length() > QGuiApplication::styleHints()->startDragDistance()))
        stopPressAndHold();
}

void QQuickAbstractButtonPrivate::handleRelease(const QPointF &point)
{
    Q_Q(QQuickAbstractButton);
    QQuickControlPrivate::handleRelease(point);
    bool wasPressed = pressed;
    setPressPoint(point);
    q->setPressed(false);
    pressButtons = Qt::NoButton;

    if (!wasHeld && (keepPressed || q->contains(point)))
        q->nextCheckState();

    if (wasPressed) {
        emit q->released();
        if (!wasHeld)
            trigger();
    } else {
        emit q->canceled();
    }

    if (autoRepeat)
        stopPressRepeat();
    else
        stopPressAndHold();
}

void QQuickAbstractButtonPrivate::handleUngrab()
{
    Q_Q(QQuickAbstractButton);
    QQuickControlPrivate::handleUngrab();
    pressButtons = Qt::NoButton;
    if (!pressed)
        return;

    q->setPressed(false);
    stopPressRepeat();
    stopPressAndHold();
    emit q->canceled();
}

bool QQuickAbstractButtonPrivate::acceptKeyClick(Qt::Key key) const
{
    return key == Qt::Key_Space;
}

bool QQuickAbstractButtonPrivate::isPressAndHoldConnected()
{
    Q_Q(QQuickAbstractButton);
    const auto signal = &QQuickAbstractButton::pressAndHold;
    const QMetaMethod method = QMetaMethod::fromSignal(signal);
    return q->isSignalConnected(method);
}

void QQuickAbstractButtonPrivate::startPressAndHold()
{
    Q_Q(QQuickAbstractButton);
    wasHeld = false;
    stopPressAndHold();
    if (isPressAndHoldConnected())
        holdTimer = q->startTimer(QGuiApplication::styleHints()->mousePressAndHoldInterval());
}

void QQuickAbstractButtonPrivate::stopPressAndHold()
{
    Q_Q(QQuickAbstractButton);
    if (holdTimer > 0) {
        q->killTimer(holdTimer);
        holdTimer = 0;
    }
}

void QQuickAbstractButtonPrivate::startRepeatDelay()
{
    Q_Q(QQuickAbstractButton);
    stopPressRepeat();
    delayTimer = q->startTimer(repeatDelay);
}

void QQuickAbstractButtonPrivate::startPressRepeat()
{
    Q_Q(QQuickAbstractButton);
    stopPressRepeat();
    repeatTimer = q->startTimer(repeatInterval);
}

void QQuickAbstractButtonPrivate::stopPressRepeat()
{
    Q_Q(QQuickAbstractButton);
    if (delayTimer > 0) {
        q->killTimer(delayTimer);
        delayTimer = 0;
    }
    if (repeatTimer > 0) {
        q->killTimer(repeatTimer);
        repeatTimer = 0;
    }
}

#if QT_CONFIG(shortcut)
void QQuickAbstractButtonPrivate::grabShortcut()
{
    Q_Q(QQuickAbstractButton);
    if (shortcut.isEmpty())
        return;

    shortcutId = QGuiApplicationPrivate::instance()->shortcutMap.addShortcut(q, shortcut, Qt::WindowShortcut, QQuickShortcutContext::matcher);

    if (!q->isEnabled())
        QGuiApplicationPrivate::instance()->shortcutMap.setShortcutEnabled(false, shortcutId, q);
}

void QQuickAbstractButtonPrivate::ungrabShortcut()
{
    Q_Q(QQuickAbstractButton);
    if (!shortcutId)
        return;

    QGuiApplicationPrivate::instance()->shortcutMap.removeShortcut(shortcutId, q);
    shortcutId = 0;
}
#endif

void QQuickAbstractButtonPrivate::actionTextChange()
{
    Q_Q(QQuickAbstractButton);
    if (explicitText)
        return;

    q->buttonChange(QQuickAbstractButton::ButtonTextChange);
}

void QQuickAbstractButtonPrivate::setText(const QString &newText, bool isExplicit)
{
    Q_Q(QQuickAbstractButton);
    const QString oldText = q->text();
    explicitText = isExplicit;
    text = newText;
    if (oldText == q->text())
        return;

    q->buttonChange(QQuickAbstractButton::ButtonTextChange);
}

void QQuickAbstractButtonPrivate::updateEffectiveIcon()
{
    Q_Q(QQuickAbstractButton);
    // We store effectiveIcon because we need to be able to tell if the icon has actually changed.
    // If we only stored our icon and the action's icon, and resolved in the getter, we'd have
    // no way of knowing what the old value was here. As an added benefit, we only resolve when
    // something has changed, as opposed to doing it unconditionally in the icon() getter.
    const QQuickIcon newEffectiveIcon = action ? icon.resolve(action->icon()) : icon;
    if (newEffectiveIcon == effectiveIcon)
        return;

    effectiveIcon = newEffectiveIcon;
    emit q->iconChanged();
}

void QQuickAbstractButtonPrivate::click()
{
    Q_Q(QQuickAbstractButton);
    if (effectiveEnable)
        emit q->clicked();
}

void QQuickAbstractButtonPrivate::trigger()
{
    Q_Q(QQuickAbstractButton);
    const bool wasEnabled = effectiveEnable;
    if (action && action->isEnabled())
        QQuickActionPrivate::get(action)->trigger(q, false);
    if (wasEnabled && (!action || !action->isEnabled()))
        emit q->clicked();
}

void QQuickAbstractButtonPrivate::toggle(bool value)
{
    Q_Q(QQuickAbstractButton);
    const bool wasChecked = checked;
    q->setChecked(value);
    if (wasChecked != checked)
        emit q->toggled();
}

static inline QString indicatorName() { return QStringLiteral("indicator"); }

void QQuickAbstractButtonPrivate::cancelIndicator()
{
    Q_Q(QQuickAbstractButton);
    quickCancelDeferred(q, indicatorName());
}

void QQuickAbstractButtonPrivate::executeIndicator(bool complete)
{
    Q_Q(QQuickAbstractButton);
    if (indicator.wasExecuted())
        return;

    if (!indicator || complete)
        quickBeginDeferred(q, indicatorName(), indicator);
    if (complete)
        quickCompleteDeferred(q, indicatorName(), indicator);
}

void QQuickAbstractButtonPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickAbstractButton);
    QQuickControlPrivate::itemImplicitWidthChanged(item);
    if (item == indicator)
        emit q->implicitIndicatorWidthChanged();
}

void QQuickAbstractButtonPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickAbstractButton);
    QQuickControlPrivate::itemImplicitHeightChanged(item);
    if (item == indicator)
        emit q->implicitIndicatorHeightChanged();
}

QQuickAbstractButton *QQuickAbstractButtonPrivate::findCheckedButton() const
{
    Q_Q(const QQuickAbstractButton);
    if (group)
        return qobject_cast<QQuickAbstractButton *>(group->checkedButton());

    const QList<QQuickAbstractButton *> buttons = findExclusiveButtons();
    // TODO: A singular QRadioButton can be unchecked, which seems logical,
    // because there's nothing to be exclusive with. However, a RadioButton
    // from QtQuick.Controls 1.x can never be unchecked, which is the behavior
    // that QQuickRadioButton adopted. Uncommenting the following count check
    // gives the QRadioButton behavior. Notice that tst_radiobutton.qml needs
    // to be updated.
    if (!autoExclusive /*|| buttons.count() == 1*/)
        return nullptr;

    for (QQuickAbstractButton *button : buttons) {
        if (button->isChecked() && button != q)
            return button;
    }
    return checked ? const_cast<QQuickAbstractButton *>(q) : nullptr;
}

QList<QQuickAbstractButton *> QQuickAbstractButtonPrivate::findExclusiveButtons() const
{
    QList<QQuickAbstractButton *> buttons;
    if (group) {
        QQmlListProperty<QQuickAbstractButton> groupButtons = group->buttons();
        int count = groupButtons.count(&groupButtons);
        for (int i = 0; i < count; ++i) {
            QQuickAbstractButton *button = qobject_cast<QQuickAbstractButton *>(groupButtons.at(&groupButtons, i));
            if (button)
                buttons += button;
        }
    } else if (parentItem) {
        const auto childItems = parentItem->childItems();
        for (QQuickItem *child : childItems) {
            QQuickAbstractButton *button = qobject_cast<QQuickAbstractButton *>(child);
            if (button && button->autoExclusive() && !QQuickAbstractButtonPrivate::get(button)->group)
                buttons += button;
        }
    }
    return buttons;
}

QQuickAbstractButton::QQuickAbstractButton(QQuickItem *parent)
    : QQuickControl(*(new QQuickAbstractButtonPrivate), parent)
{
    setActiveFocusOnTab(true);
    setFocusPolicy(Qt::StrongFocus);
    setAcceptedMouseButtons(Qt::LeftButton);
#if QT_CONFIG(cursor)
    setCursor(Qt::ArrowCursor);
#endif
}

QQuickAbstractButton::QQuickAbstractButton(QQuickAbstractButtonPrivate &dd, QQuickItem *parent)
    : QQuickControl(dd, parent)
{
    setActiveFocusOnTab(true);
    setFocusPolicy(Qt::StrongFocus);
    setAcceptedMouseButtons(Qt::LeftButton);
#if QT_CONFIG(cursor)
    setCursor(Qt::ArrowCursor);
#endif
}

QQuickAbstractButton::~QQuickAbstractButton()
{
    Q_D(QQuickAbstractButton);
    d->removeImplicitSizeListener(d->indicator);
    if (d->group)
        d->group->removeButton(this);
#if QT_CONFIG(shortcut)
    d->ungrabShortcut();
#endif
}

/*!
    \qmlproperty string QtQuick.Controls::AbstractButton::text

    This property holds a textual description of the button.

    \note The text is used for accessibility purposes, so it makes sense to
          set a textual description even if the content item is an image.

    \sa icon, display, {Control::contentItem}{contentItem}
*/
QString QQuickAbstractButton::text() const
{
    Q_D(const QQuickAbstractButton);
    return d->explicitText || !d->action ? d->text : d->action->text();
}

void QQuickAbstractButton::setText(const QString &text)
{
    Q_D(QQuickAbstractButton);
    d->setText(text, true);
}

void QQuickAbstractButton::resetText()
{
    Q_D(QQuickAbstractButton);
    d->setText(QString(), false);
}

/*!
    \qmlproperty bool QtQuick.Controls::AbstractButton::down

    This property holds whether the button is visually down.

    Unless explicitly set, this property follows the value of \l pressed. To
    return to the default value, set this property to \c undefined.

    \sa pressed
*/
bool QQuickAbstractButton::isDown() const
{
    Q_D(const QQuickAbstractButton);
    return d->down;
}

void QQuickAbstractButton::setDown(bool down)
{
    Q_D(QQuickAbstractButton);
    d->explicitDown = true;

    if (d->down == down)
        return;

    d->down = down;
    emit downChanged();
}

void QQuickAbstractButton::resetDown()
{
    Q_D(QQuickAbstractButton);
    if (!d->explicitDown)
        return;

    setDown(d->pressed);
    d->explicitDown = false;
}

/*!
    \qmlproperty bool QtQuick.Controls::AbstractButton::pressed
    \readonly

    This property holds whether the button is physically pressed. A button can
    be pressed by either touch or key events.

    \sa down
*/
bool QQuickAbstractButton::isPressed() const
{
    Q_D(const QQuickAbstractButton);
    return d->pressed;
}

void QQuickAbstractButton::setPressed(bool isPressed)
{
    Q_D(QQuickAbstractButton);
    if (d->pressed == isPressed)
        return;

    d->pressed = isPressed;
    setAccessibleProperty("pressed", isPressed);
    emit pressedChanged();
    buttonChange(ButtonPressedChanged);

    if (!d->explicitDown) {
        setDown(d->pressed);
        d->explicitDown = false;
    }
}

/*!
    \qmlproperty bool QtQuick.Controls::AbstractButton::checked

    This property holds whether the button is checked.

    \sa checkable
*/
bool QQuickAbstractButton::isChecked() const
{
    Q_D(const QQuickAbstractButton);
    return d->checked;
}

void QQuickAbstractButton::setChecked(bool checked)
{
    Q_D(QQuickAbstractButton);
    if (d->checked == checked)
        return;

    if (checked && !d->checkable)
        setCheckable(true);

    d->checked = checked;
    if (d->action)
        d->action->setChecked(checked);
    setAccessibleProperty("checked", checked);
    buttonChange(ButtonCheckedChange);
    emit checkedChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::AbstractButton::checkable

    This property holds whether the button is checkable.

    A checkable button toggles between checked (on) and unchecked (off) when
    the user clicks on it or presses the space bar while the button has active
    focus.

    Setting \l checked to \c true forces this property to \c true.

    The default value is \c false.

    \sa checked
*/
bool QQuickAbstractButton::isCheckable() const
{
    Q_D(const QQuickAbstractButton);
    return d->checkable;
}

void QQuickAbstractButton::setCheckable(bool checkable)
{
    Q_D(QQuickAbstractButton);
    if (d->checkable == checkable)
        return;

    d->checkable = checkable;
    if (d->action)
        d->action->setCheckable(checkable);
    setAccessibleProperty("checkable", checkable);
    buttonChange(ButtonCheckableChange);
    emit checkableChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::AbstractButton::autoExclusive

    This property holds whether auto-exclusivity is enabled.

    If auto-exclusivity is enabled, checkable buttons that belong to the same
    parent item behave as if they were part of the same ButtonGroup. Only
    one button can be checked at any time; checking another button automatically
    unchecks the previously checked one.

    \note The property has no effect on buttons that belong to a ButtonGroup.

    RadioButton and TabButton are auto-exclusive by default.
*/
bool QQuickAbstractButton::autoExclusive() const
{
    Q_D(const QQuickAbstractButton);
    return d->autoExclusive;
}

void QQuickAbstractButton::setAutoExclusive(bool exclusive)
{
    Q_D(QQuickAbstractButton);
    if (d->autoExclusive == exclusive)
        return;

    d->autoExclusive = exclusive;
    emit autoExclusiveChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::AbstractButton::autoRepeat

    This property holds whether the button repeats \l pressed(), \l released()
    and \l clicked() signals while the button is pressed and held down.

    The default value is \c false.

    The initial delay and the repetition interval are defined in milliseconds
    by \l autoRepeatDelay and \l autoRepeatInterval.
*/
bool QQuickAbstractButton::autoRepeat() const
{
    Q_D(const QQuickAbstractButton);
    return d->autoRepeat;
}

void QQuickAbstractButton::setAutoRepeat(bool repeat)
{
    Q_D(QQuickAbstractButton);
    if (d->autoRepeat == repeat)
        return;

    d->stopPressRepeat();
    d->autoRepeat = repeat;
    emit autoRepeatChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::AbstractButton::indicator

    This property holds the indicator item.
*/
QQuickItem *QQuickAbstractButton::indicator() const
{
    QQuickAbstractButtonPrivate *d = const_cast<QQuickAbstractButtonPrivate *>(d_func());
    if (!d->indicator)
        d->executeIndicator();
    return d->indicator;
}

void QQuickAbstractButton::setIndicator(QQuickItem *indicator)
{
    Q_D(QQuickAbstractButton);
    if (d->indicator == indicator)
        return;

    if (!d->indicator.isExecuting())
        d->cancelIndicator();

    const qreal oldImplicitIndicatorWidth = implicitIndicatorWidth();
    const qreal oldImplicitIndicatorHeight = implicitIndicatorHeight();

    d->removeImplicitSizeListener(d->indicator);
    delete d->indicator;
    d->indicator = indicator;

    if (indicator) {
        if (!indicator->parentItem())
            indicator->setParentItem(this);
        indicator->setAcceptedMouseButtons(Qt::LeftButton);
        d->addImplicitSizeListener(indicator);
    }

    if (!qFuzzyCompare(oldImplicitIndicatorWidth, implicitIndicatorWidth()))
        emit implicitIndicatorWidthChanged();
    if (!qFuzzyCompare(oldImplicitIndicatorHeight, implicitIndicatorHeight()))
        emit implicitIndicatorHeightChanged();
    if (!d->indicator.isExecuting())
        emit indicatorChanged();
}

/*!
    \qmlpropertygroup QtQuick.Controls::AbstractButton::icon
    \qmlproperty string QtQuick.Controls::AbstractButton::icon.name
    \qmlproperty url QtQuick.Controls::AbstractButton::icon.source
    \qmlproperty int QtQuick.Controls::AbstractButton::icon.width
    \qmlproperty int QtQuick.Controls::AbstractButton::icon.height
    \qmlproperty color QtQuick.Controls::AbstractButton::icon.color

    This property group was added in QtQuick.Controls 2.3.

    \include qquickicon.qdocinc grouped-properties

    \sa text, display, {Icons in Qt Quick Controls 2}
*/

QQuickIcon QQuickAbstractButton::icon() const
{
    Q_D(const QQuickAbstractButton);
    return d->effectiveIcon;
}

void QQuickAbstractButton::setIcon(const QQuickIcon &icon)
{
    Q_D(QQuickAbstractButton);
    d->icon = icon;
    d->updateEffectiveIcon();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty enumeration QtQuick.Controls::AbstractButton::display

    This property determines how the \l icon and \l text are displayed within
    the button.

    \table
    \header \li Display \li Result
    \row \li \c AbstractButton.IconOnly \li \image qtquickcontrols2-button-icononly.png
    \row \li \c AbstractButton.TextOnly \li \image qtquickcontrols2-button-textonly.png
    \row \li \c AbstractButton.TextBesideIcon \li \image qtquickcontrols2-button-textbesideicon.png
    \row \li \c AbstractButton.TextUnderIcon \li \image qtquickcontrols2-button-textundericon.png
    \endtable

    \sa {Control::}{spacing}, {Control::}{padding}
*/
QQuickAbstractButton::Display QQuickAbstractButton::display() const
{
    Q_D(const QQuickAbstractButton);
    return d->display;
}

void QQuickAbstractButton::setDisplay(Display display)
{
    Q_D(QQuickAbstractButton);
    if (display == d->display)
        return;

    d->display = display;
    emit displayChanged();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty Action QtQuick.Controls::AbstractButton::action

    This property holds the button action.

    \sa Action
*/
QQuickAction *QQuickAbstractButton::action() const
{
    Q_D(const QQuickAbstractButton);
    return d->action;
}

void QQuickAbstractButton::setAction(QQuickAction *action)
{
    Q_D(QQuickAbstractButton);
    if (d->action == action)
        return;

    const QString oldText = text();

    if (QQuickAction *oldAction = d->action.data()) {
        QQuickActionPrivate::get(oldAction)->unregisterItem(this);
        QObjectPrivate::disconnect(oldAction, &QQuickAction::triggered, d, &QQuickAbstractButtonPrivate::click);
        QObjectPrivate::disconnect(oldAction, &QQuickAction::textChanged, d, &QQuickAbstractButtonPrivate::actionTextChange);

        QObjectPrivate::disconnect(oldAction, &QQuickAction::iconChanged, d, &QQuickAbstractButtonPrivate::updateEffectiveIcon);
        disconnect(oldAction, &QQuickAction::checkedChanged, this, &QQuickAbstractButton::setChecked);
        disconnect(oldAction, &QQuickAction::checkableChanged, this, &QQuickAbstractButton::setCheckable);
        disconnect(oldAction, &QQuickAction::enabledChanged, this, &QQuickItem::setEnabled);
    }

    if (action) {
        QQuickActionPrivate::get(action)->registerItem(this);
        QObjectPrivate::connect(action, &QQuickAction::triggered, d, &QQuickAbstractButtonPrivate::click);
        QObjectPrivate::connect(action, &QQuickAction::textChanged, d, &QQuickAbstractButtonPrivate::actionTextChange);

        QObjectPrivate::connect(action, &QQuickAction::iconChanged, d, &QQuickAbstractButtonPrivate::updateEffectiveIcon);
        connect(action, &QQuickAction::checkedChanged, this, &QQuickAbstractButton::setChecked);
        connect(action, &QQuickAction::checkableChanged, this, &QQuickAbstractButton::setCheckable);
        connect(action, &QQuickAction::enabledChanged, this, &QQuickItem::setEnabled);

        setChecked(action->isChecked());
        setCheckable(action->isCheckable());
        setEnabled(action->isEnabled());
    }

    d->action = action;

    if (oldText != text())
        buttonChange(ButtonTextChange);

    d->updateEffectiveIcon();

    emit actionChanged();
}

/*!
    \since QtQuick.Controls 2.4 (Qt 5.11)
    \qmlproperty int QtQuick.Controls::AbstractButton::autoRepeatDelay

    This property holds the initial delay of auto-repetition in milliseconds.
    The default value is \c 300 ms.

    \sa autoRepeat, autoRepeatInterval
*/
int QQuickAbstractButton::autoRepeatDelay() const
{
    Q_D(const QQuickAbstractButton);
    return d->repeatDelay;
}

void QQuickAbstractButton::setAutoRepeatDelay(int delay)
{
    Q_D(QQuickAbstractButton);
    if (d->repeatDelay == delay)
        return;

    d->repeatDelay = delay;
    emit autoRepeatDelayChanged();
}

/*!
    \since QtQuick.Controls 2.4 (Qt 5.11)
    \qmlproperty int QtQuick.Controls::AbstractButton::autoRepeatInterval

    This property holds the interval of auto-repetition in milliseconds.
    The default value is \c 100 ms.

    \sa autoRepeat, autoRepeatDelay
*/
int QQuickAbstractButton::autoRepeatInterval() const
{
    Q_D(const QQuickAbstractButton);
    return d->repeatInterval;
}

void QQuickAbstractButton::setAutoRepeatInterval(int interval)
{
    Q_D(QQuickAbstractButton);
    if (d->repeatInterval == interval)
        return;

    d->repeatInterval = interval;
    emit autoRepeatIntervalChanged();
}

#if QT_CONFIG(shortcut)
QKeySequence QQuickAbstractButton::shortcut() const
{
    Q_D(const QQuickAbstractButton);
    return d->shortcut;
}

void QQuickAbstractButton::setShortcut(const QKeySequence &shortcut)
{
    Q_D(QQuickAbstractButton);
    if (d->shortcut == shortcut)
        return;

    d->ungrabShortcut();
    d->shortcut = shortcut;
    if (isVisible())
        d->grabShortcut();
}
#endif

/*!
    \readonly
    \since QtQuick.Controls 2.4 (Qt 5.11)
    \qmlproperty real QtQuick.Controls::AbstractButton::pressX

    This property holds the x-coordinate of the last press.

    \note The value is updated on touch moves, but left intact after touch release.

    \sa pressY
*/
qreal QQuickAbstractButton::pressX() const
{
    Q_D(const QQuickAbstractButton);
    return d->movePoint.x();
}

/*!
    \readonly
    \since QtQuick.Controls 2.4 (Qt 5.11)
    \qmlproperty real QtQuick.Controls::AbstractButton::pressY

    This property holds the y-coordinate of the last press.

    \note The value is updated on touch moves, but left intact after touch release.

    \sa pressX
*/
qreal QQuickAbstractButton::pressY() const
{
    Q_D(const QQuickAbstractButton);
    return d->movePoint.y();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::AbstractButton::implicitIndicatorWidth
    \readonly

    This property holds the implicit indicator width.

    The value is equal to \c {indicator ? indicator.implicitWidth : 0}.

    This is typically used, together with \l {Control::}{implicitContentWidth} and
    \l {Control::}{implicitBackgroundWidth}, to calculate the \l {Item::}{implicitWidth}.

    \sa implicitIndicatorHeight
*/
qreal QQuickAbstractButton::implicitIndicatorWidth() const
{
    Q_D(const QQuickAbstractButton);
    if (!d->indicator)
        return 0;
    return d->indicator->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::AbstractButton::implicitIndicatorHeight
    \readonly

    This property holds the implicit indicator height.

    The value is equal to \c {indicator ? indicator.implicitHeight : 0}.

    This is typically used, together with \l {Control::}{implicitContentHeight} and
    \l {Control::}{implicitBackgroundHeight}, to calculate the \l {Item::}{implicitHeight}.

    \sa implicitIndicatorWidth
*/
qreal QQuickAbstractButton::implicitIndicatorHeight() const
{
    Q_D(const QQuickAbstractButton);
    if (!d->indicator)
        return 0;
    return d->indicator->implicitHeight();
}

/*!
    \qmlmethod void QtQuick.Controls::AbstractButton::toggle()

    Toggles the checked state of the button.
*/
void QQuickAbstractButton::toggle()
{
    Q_D(QQuickAbstractButton);
    setChecked(!d->checked);
}

void QQuickAbstractButton::componentComplete()
{
    Q_D(QQuickAbstractButton);
    d->executeIndicator(true);
    QQuickControl::componentComplete();
}

bool QQuickAbstractButton::event(QEvent *event)
{
    Q_D(QQuickAbstractButton);
#if QT_CONFIG(shortcut)
    if (event->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(event);
        if (se->shortcutId() == d->shortcutId) {
            d->trigger();
            return true;
        }
    }
#endif
    return QQuickControl::event(event);
}

void QQuickAbstractButton::focusOutEvent(QFocusEvent *event)
{
    Q_D(QQuickAbstractButton);
    QQuickControl::focusOutEvent(event);
    if (d->touchId == -1) // don't ungrab on multi-touch if another control gets focused
        d->handleUngrab();
}

void QQuickAbstractButton::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickAbstractButton);
    QQuickControl::keyPressEvent(event);
    if (d->acceptKeyClick(static_cast<Qt::Key>(event->key()))) {
        d->setPressPoint(QPoint(qRound(width() / 2), qRound(height() / 2)));
        setPressed(true);

        if (d->autoRepeat)
            d->startRepeatDelay();

        emit pressed();
        event->accept();
    }
}

void QQuickAbstractButton::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QQuickAbstractButton);
    QQuickControl::keyReleaseEvent(event);
    if (d->pressed && d->acceptKeyClick(static_cast<Qt::Key>(event->key()))) {
        setPressed(false);

        nextCheckState();
        emit released();
        d->trigger();

        if (d->autoRepeat)
            d->stopPressRepeat();
        event->accept();
    }
}

void QQuickAbstractButton::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickAbstractButton);
    d->pressButtons = event->buttons();
    QQuickControl::mousePressEvent(event);
}

void QQuickAbstractButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    QQuickControl::mouseDoubleClickEvent(event);
    emit doubleClicked();
}

void QQuickAbstractButton::timerEvent(QTimerEvent *event)
{
    Q_D(QQuickAbstractButton);
    QQuickControl::timerEvent(event);
    if (event->timerId() == d->holdTimer) {
        d->stopPressAndHold();
        d->wasHeld = true;
        emit pressAndHold();
    } else if (event->timerId() == d->delayTimer) {
        d->startPressRepeat();
    } else if (event->timerId() == d->repeatTimer) {
        emit released();
        d->trigger();
        emit pressed();
    }
}

void QQuickAbstractButton::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QQuickAbstractButton);
    QQuickControl::itemChange(change, value);
#if QT_CONFIG(shortcut)
    if (change == ItemVisibleHasChanged) {
        if (value.boolValue)
            d->grabShortcut();
        else
            d->ungrabShortcut();
    }
#endif
}

void QQuickAbstractButton::buttonChange(ButtonChange change)
{
    Q_D(QQuickAbstractButton);
    switch (change) {
    case ButtonCheckedChange:
        if (d->checked) {
            QQuickAbstractButton *button = d->findCheckedButton();
            if (button && button != this)
                button->setChecked(false);
        }
        break;
    case ButtonTextChange: {
        const QString txt = text();
        setAccessibleName(txt);
#if QT_CONFIG(shortcut)
        setShortcut(QKeySequence::mnemonic(txt));
#endif
        emit textChanged();
        break;
    }
    default:
        break;
    }
}

void QQuickAbstractButton::nextCheckState()
{
    Q_D(QQuickAbstractButton);
    if (d->checkable && (!d->checked || d->findCheckedButton() != this))
        d->toggle(!d->checked);
}

#if QT_CONFIG(accessibility)
void QQuickAbstractButton::accessibilityActiveChanged(bool active)
{
    QQuickControl::accessibilityActiveChanged(active);

    Q_D(QQuickAbstractButton);
    if (active) {
        setAccessibleName(text());
        setAccessibleProperty("pressed", d->pressed);
        setAccessibleProperty("checked", d->checked);
        setAccessibleProperty("checkable", d->checkable);
    }
}

QAccessible::Role QQuickAbstractButton::accessibleRole() const
{
    Q_D(const QQuickAbstractButton);
    if (d->checkable) {
        return QAccessible::CheckBox;
    }
    return QAccessible::Button;
}
#endif

QT_END_NAMESPACE
