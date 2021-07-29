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

#include "qquickaction_p.h"
#include "qquickaction_p_p.h"
#include "qquickactiongroup_p.h"
#include "qquickshortcutcontext_p_p.h"

#include <QtGui/qevent.h>
#include <QtGui/private/qshortcutmap_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Action
    \inherits QtObject
    \instantiates QQuickAction
    \inqmlmodule QtQuick.Controls
    \since 5.10
    \ingroup utilities
    \brief Abstract user interface action.

    Action represents an abstract user interface action that can have shortcuts
    and can be assigned to menu items and toolbar buttons.

    Actions may contain \l text, an \l icon, and a \l shortcut. Actions are normally
    \l triggered by the user via menu items, toolbar buttons, or keyboard shortcuts.
    A \l checkable Action toggles its \l checked state when triggered.

    \snippet qtquickcontrols2-action.qml action

    Action is commonly used to implement application commands that can be invoked
    via menu items, toolbar buttons, and keyboard shortcuts. Since the user expects
    the commands to be performed in the same way, regardless of the user interface
    used, it is useful to represent the commands as shareable actions.

    Action can be also used to separate the logic and the visual presentation. For
    example, when declaring buttons and menu items in \c .ui.qml files, actions can
    be declared elsewhere and assigned from the outside.

    \snippet qtquickcontrols2-action.qml toolbutton

    When an action is paired with buttons and menu items, the \c enabled, \c checkable,
    and \c checked states are synced automatically. For example, in a word processor,
    if the user clicks a "Bold" toolbar button, the "Bold" menu item will automatically
    be checked. Buttons and menu items get their \c text and \c icon from the action by
    default. An action-specific \c text or \c icon can be overridden for a specific
    control by specifying \c text or \c icon directly on the control.

    \snippet qtquickcontrols2-action.qml menuitem

    Since Action presents a user interface action, it is intended to be assigned to
    a \l MenuItem, \l ToolButton, or any other control that inherits \l AbstractButton.
    For keyboard shortcuts, the simpler \l Shortcut type is more appropriate.

    \sa MenuItem, ToolButton, Shortcut
*/

/*!
    \qmlsignal QtQuick.Controls::Action::toggled(QtObject source)

    This signal is emitted when the action is toggled. The \a source argument
    identifies the object that toggled the action.

    For example, if the action is assigned to a menu item and a toolbar button, the
    action is toggled when the control is toggled, the shortcut is activated, or
    when \l toggle() is called directly.
*/

/*!
    \qmlsignal QtQuick.Controls::Action::triggered(QtObject source)

    This signal is emitted when the action is triggered. The \a source argument
    identifies the object that triggered the action.

    For example, if the action is assigned to a menu item and a toolbar button, the
    action is triggered when the control is clicked, the shortcut is activated, or
    when \l trigger() is called directly.
*/

#if QT_CONFIG(shortcut)
static QKeySequence variantToKeySequence(const QVariant &var)
{
    if (var.type() == QVariant::Int)
        return QKeySequence(static_cast<QKeySequence::StandardKey>(var.toInt()));
    return QKeySequence::fromString(var.toString());
}

QQuickActionPrivate::ShortcutEntry::ShortcutEntry(QObject *target)
    : m_target(target)
{
}

QQuickActionPrivate::ShortcutEntry::~ShortcutEntry()
{
    ungrab();
}

QObject *QQuickActionPrivate::ShortcutEntry::target() const
{
    return m_target;
}

int QQuickActionPrivate::ShortcutEntry::shortcutId() const
{
    return m_shortcutId;
}

void QQuickActionPrivate::ShortcutEntry::grab(const QKeySequence &shortcut, bool enabled)
{
    if (shortcut.isEmpty())
        return;

    Qt::ShortcutContext context = Qt::WindowShortcut; // TODO
    m_shortcutId = QGuiApplicationPrivate::instance()->shortcutMap.addShortcut(m_target, shortcut, context, QQuickShortcutContext::matcher);

    if (!enabled)
        QGuiApplicationPrivate::instance()->shortcutMap.setShortcutEnabled(false, m_shortcutId, m_target);
}

void QQuickActionPrivate::ShortcutEntry::ungrab()
{
    if (!m_shortcutId)
        return;

    QGuiApplicationPrivate::instance()->shortcutMap.removeShortcut(m_shortcutId, m_target);
    m_shortcutId = 0;
}

void QQuickActionPrivate::ShortcutEntry::setEnabled(bool enabled)
{
    if (!m_shortcutId)
        return;

    QGuiApplicationPrivate::instance()->shortcutMap.setShortcutEnabled(enabled, m_shortcutId, m_target);
}

QVariant QQuickActionPrivate::shortcut() const
{
    return vshortcut;
}

void QQuickActionPrivate::setShortcut(const QVariant &var)
{
    Q_Q(QQuickAction);
    if (vshortcut == var)
        return;

    defaultShortcutEntry->ungrab();
    for (QQuickActionPrivate::ShortcutEntry *entry : qAsConst(shortcutEntries))
        entry->ungrab();

    vshortcut = var;
    keySequence = variantToKeySequence(var);

    defaultShortcutEntry->grab(keySequence, enabled);
    for (QQuickActionPrivate::ShortcutEntry *entry : qAsConst(shortcutEntries))
        entry->grab(keySequence, enabled);

    emit q->shortcutChanged(keySequence);
}
#endif // QT_CONFIG(shortcut)

void QQuickActionPrivate::setEnabled(bool enable)
{
    Q_Q(QQuickAction);
    if (enabled == enable)
        return;

    enabled = enable;

#if QT_CONFIG(shortcut)
    defaultShortcutEntry->setEnabled(enable);
    for (QQuickActionPrivate::ShortcutEntry *entry : qAsConst(shortcutEntries))
        entry->setEnabled(enable);
#endif

    emit q->enabledChanged(enable);
}

bool QQuickActionPrivate::watchItem(QQuickItem *item)
{
    Q_Q(QQuickAction);
    if (!item)
        return false;

    item->installEventFilter(q);
    QQuickItemPrivate::get(item)->addItemChangeListener(this, QQuickItemPrivate::Visibility | QQuickItemPrivate::Destroyed);
    return true;
}

bool QQuickActionPrivate::unwatchItem(QQuickItem *item)
{
    Q_Q(QQuickAction);
    if (!item)
        return false;

    item->removeEventFilter(q);
    QQuickItemPrivate::get(item)->removeItemChangeListener(this, QQuickItemPrivate::Visibility | QQuickItemPrivate::Destroyed);
    return true;
}

void QQuickActionPrivate::registerItem(QQuickItem *item)
{
    if (!watchItem(item))
        return;

#if QT_CONFIG(shortcut)
    QQuickActionPrivate::ShortcutEntry *entry = new QQuickActionPrivate::ShortcutEntry(item);
    if (item->isVisible())
        entry->grab(keySequence, enabled);
    shortcutEntries += entry;

    updateDefaultShortcutEntry();
#endif
}

void QQuickActionPrivate::unregisterItem(QQuickItem *item)
{
#if QT_CONFIG(shortcut)
    QQuickActionPrivate::ShortcutEntry *entry = findShortcutEntry(item);
    if (!entry || !unwatchItem(item))
        return;

    shortcutEntries.removeOne(entry);
    delete entry;

    updateDefaultShortcutEntry();
#endif
}

void QQuickActionPrivate::itemVisibilityChanged(QQuickItem *item)
{
#if QT_CONFIG(shortcut)
    QQuickActionPrivate::ShortcutEntry *entry = findShortcutEntry(item);
    if (!entry)
        return;

    if (item->isVisible())
        entry->grab(keySequence, enabled);
    else
        entry->ungrab();

    updateDefaultShortcutEntry();
#endif
}

void QQuickActionPrivate::itemDestroyed(QQuickItem *item)
{
    unregisterItem(item);
}

#if QT_CONFIG(shortcut)
bool QQuickActionPrivate::handleShortcutEvent(QObject *object, QShortcutEvent *event)
{
    Q_Q(QQuickAction);
    if (event->key() != keySequence)
        return false;

    QQuickActionPrivate::ShortcutEntry *entry = findShortcutEntry(object);
    if (!entry || event->shortcutId() != entry->shortcutId())
        return false;

    q->trigger(entry->target());
    return true;
}

QQuickActionPrivate::ShortcutEntry *QQuickActionPrivate::findShortcutEntry(QObject *target) const
{
    Q_Q(const QQuickAction);
    if (target == q)
        return defaultShortcutEntry;
    for (QQuickActionPrivate::ShortcutEntry *entry : shortcutEntries) {
        if (entry->target() == target)
            return entry;
    }
    return nullptr;
}

void QQuickActionPrivate::updateDefaultShortcutEntry()
{
    bool hasActiveShortcutEntries = false;
    for (QQuickActionPrivate::ShortcutEntry *entry : qAsConst(shortcutEntries)) {
        if (entry->shortcutId()) {
            hasActiveShortcutEntries = true;
            break;
        }
    }

    if (hasActiveShortcutEntries)
        defaultShortcutEntry->ungrab();
    else if (!defaultShortcutEntry->shortcutId())
        defaultShortcutEntry->grab(keySequence, enabled);
}
#endif // QT_CONFIG(shortcut)

QQuickAction::QQuickAction(QObject *parent)
    : QObject(*(new QQuickActionPrivate), parent)
{
    Q_D(QQuickAction);
#if QT_CONFIG(shortcut)
    d->defaultShortcutEntry = new QQuickActionPrivate::ShortcutEntry(this);
#endif
}

QQuickAction::~QQuickAction()
{
    Q_D(QQuickAction);
    if (d->group)
        d->group->removeAction(this);

#if QT_CONFIG(shortcut)
    for (QQuickActionPrivate::ShortcutEntry *entry : qAsConst(d->shortcutEntries))
        d->unwatchItem(qobject_cast<QQuickItem *>(entry->target()));

    qDeleteAll(d->shortcutEntries);
    delete d->defaultShortcutEntry;
#endif
}

/*!
    \qmlproperty string QtQuick.Controls::Action::text

    This property holds a textual description of the action.
*/
QString QQuickAction::text() const
{
    Q_D(const QQuickAction);
    return d->text;
}

void QQuickAction::setText(const QString &text)
{
    Q_D(QQuickAction);
    if (d->text == text)
        return;

    d->text = text;
    emit textChanged(text);
}

/*!
    \qmlpropertygroup QtQuick.Controls::Action::icon
    \qmlproperty string QtQuick.Controls::Action::icon.name
    \qmlproperty url QtQuick.Controls::Action::icon.source
    \qmlproperty int QtQuick.Controls::Action::icon.width
    \qmlproperty int QtQuick.Controls::Action::icon.height
    \qmlproperty color QtQuick.Controls::Action::icon.color

    \include qquickicon.qdocinc grouped-properties
*/
QQuickIcon QQuickAction::icon() const
{
    Q_D(const QQuickAction);
    return d->icon;
}

void QQuickAction::setIcon(const QQuickIcon &icon)
{
    Q_D(QQuickAction);
    if (d->icon == icon)
        return;

    d->icon = icon;
    emit iconChanged(icon);
}

/*!
    \qmlproperty bool QtQuick.Controls::Action::enabled

    This property holds whether the action is enabled. The default value is \c true.
*/
bool QQuickAction::isEnabled() const
{
    Q_D(const QQuickAction);
    return d->enabled && (!d->group || d->group->isEnabled());
}

void QQuickAction::setEnabled(bool enabled)
{
    Q_D(QQuickAction);
    d->explicitEnabled = true;
    d->setEnabled(enabled);
}

void QQuickAction::resetEnabled()
{
    Q_D(QQuickAction);
    if (!d->explicitEnabled)
        return;

    d->explicitEnabled = false;
    d->setEnabled(true);
}

/*!
    \qmlproperty bool QtQuick.Controls::Action::checked

    This property holds whether the action is checked.

    \sa checkable
*/
bool QQuickAction::isChecked() const
{
    Q_D(const QQuickAction);
    return d->checked;
}

void QQuickAction::setChecked(bool checked)
{
    Q_D(QQuickAction);
    if (d->checked == checked)
        return;

    d->checked = checked;
    emit checkedChanged(checked);
}

/*!
    \qmlproperty bool QtQuick.Controls::Action::checkable

    This property holds whether the action is checkable. The default value is \c false.

    A checkable action toggles between checked (on) and unchecked (off) when triggered.

    \sa checked
*/
bool QQuickAction::isCheckable() const
{
    Q_D(const QQuickAction);
    return d->checkable;
}

void QQuickAction::setCheckable(bool checkable)
{
    Q_D(QQuickAction);
    if (d->checkable == checkable)
        return;

    d->checkable = checkable;
    emit checkableChanged(checkable);
}

#if QT_CONFIG(shortcut)
/*!
    \qmlproperty keysequence QtQuick.Controls::Action::shortcut

    This property holds the action's shortcut. The key sequence can be set
    to one of the \l{QKeySequence::StandardKey}{standard keyboard shortcuts},
    or it can be described with a string containing a sequence of up to four
    key presses that are needed to trigger the shortcut.

    \code
    Action {
        shortcut: "Ctrl+E,Ctrl+W"
        onTriggered: edit.wrapMode = TextEdit.Wrap
    }
    \endcode
*/
QKeySequence QQuickAction::shortcut() const
{
    Q_D(const QQuickAction);
    return d->keySequence;
}

void QQuickAction::setShortcut(const QKeySequence &shortcut)
{
    Q_D(QQuickAction);
    d->setShortcut(shortcut.toString());
}
#endif // QT_CONFIG(shortcut)

/*!
    \qmlmethod void QtQuick.Controls::Action::toggle(QtObject source = null)

    Toggles the action and emits \l toggled() if enabled, with an optional \a source object defined.
*/
void QQuickAction::toggle(QObject *source)
{
    Q_D(QQuickAction);
    if (!d->enabled)
        return;

    if (d->checkable)
        setChecked(!d->checked);

    emit toggled(source);
}

/*!
    \qmlmethod void QtQuick.Controls::Action::trigger(QtObject source = null)

    Triggers the action and emits \l triggered() if enabled, with an optional \a source object defined.
*/
void QQuickAction::trigger(QObject *source)
{
    Q_D(QQuickAction);
    d->trigger(source, true);
}

void QQuickActionPrivate::trigger(QObject* source, bool doToggle)
{
    Q_Q(QQuickAction);
    if (!enabled)
        return;

    QPointer<QObject> guard = q;
    // the checked action of an exclusive group cannot be unchecked
    if (checkable && (!checked || !group || !group->isExclusive() || group->checkedAction() != q)) {
        if (doToggle)
            q->toggle(source);
        else
            emit q->toggled(source);
    }

    if (!guard.isNull())
        emit q->triggered(source);
}

bool QQuickAction::event(QEvent *event)
{
    Q_D(QQuickAction);
#if QT_CONFIG(shortcut)
    if (event->type() == QEvent::Shortcut)
        return d->handleShortcutEvent(this, static_cast<QShortcutEvent *>(event));
#endif
    return QObject::event(event);
}

bool QQuickAction::eventFilter(QObject *object, QEvent *event)
{
    Q_D(QQuickAction);
#if QT_CONFIG(shortcut)
    if (event->type() == QEvent::Shortcut)
        return d->handleShortcutEvent(object, static_cast<QShortcutEvent *>(event));
#endif
    return false;
}

QT_END_NAMESPACE

#include "moc_qquickaction_p.cpp"
