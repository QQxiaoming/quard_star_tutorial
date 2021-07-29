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

#include "qquickdialog_p.h"
#include "qquickdialog_p_p.h"
#include "qquickdialogbuttonbox_p.h"
#include "qquickabstractbutton_p.h"
#include "qquickpopupitem_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Dialog
    \inherits Popup
    \instantiates QQuickDialog
    \inqmlmodule QtQuick.Controls
    \ingroup qtquickcontrols2-dialogs
    \ingroup qtquickcontrols2-popups
    \brief Popup dialog with standard buttons and a title, used for short-term interaction with the user.
    \since 5.8

    A dialog is a popup mostly used for short-term tasks and brief communications
    with the user. Similarly to \l ApplicationWindow and \l Page, Dialog is organized
    into three sections: \l header, \l {Popup::}{contentItem}, and \l footer.

    \image qtquickcontrols2-page-wireframe.png

    \section1 Dialog Title and Buttons

    Dialog's \l title is displayed by a style-specific title bar that is assigned
    as a dialog \l header by default.

    Dialog's standard buttons are managed by a \l DialogButtonBox that is assigned
    as a dialog \l footer by default. The dialog's \l standardButtons property is
    forwarded to the respective property of the button box. Furthermore, the
    \l {DialogButtonBox::}{accepted()} and \l {DialogButtonBox::}{rejected()}
    signals of the button box are connected to the respective signals in Dialog.

    \snippet qtquickcontrols2-dialog.qml 1

    \section1 Modal Dialogs

    A \l {Popup::}{modal} dialog blocks input to other content beneath
    the dialog. When a modal dialog is opened, the user must finish
    interacting with the dialog and close it before they can access any
    other content in the same window.

    \snippet qtquickcontrols2-dialog-modal.qml 1

    \section1 Modeless Dialogs

    A modeless dialog is a dialog that operates independently of other
    content around the dialog. When a modeless dialog is opened, the user
    is allowed to interact with both the dialog and the other content in
    the same window.

    \snippet qtquickcontrols2-dialog-modeless.qml 1

    \sa DialogButtonBox, {Popup Controls}
*/

/*!
    \qmlsignal QtQuick.Controls::Dialog::accepted()

    This signal is emitted when the dialog has been accepted either
    interactively or by calling \l accept().

    \note This signal is \e not emitted when closing the dialog with
    \l {Popup::}{close()} or setting \l {Popup::}{visible} to \c false.

    \sa rejected()
*/

/*!
    \qmlsignal QtQuick.Controls::Dialog::rejected()

    This signal is emitted when the dialog has been rejected either
    interactively or by calling \l reject().

    \note This signal is \e not emitted when closing the dialog with
    \l {Popup::}{close()} or setting \l {Popup::}{visible} to \c false.

    \sa accepted()
*/

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlsignal QtQuick.Controls::Dialog::applied()

    This signal is emitted when the \c Dialog.Apply standard button is clicked.

    \sa discarded(), reset()
*/

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlsignal QtQuick.Controls::Dialog::reset()

    This signal is emitted when the \c Dialog.Reset standard button is clicked.

    \sa discarded(), applied()
*/

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlsignal QtQuick.Controls::Dialog::discarded()

    This signal is emitted when the \c Dialog.Discard standard button is clicked.

    \sa reset(), applied()
*/

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlsignal QtQuick.Controls::Dialog::helpRequested()

    This signal is emitted when the \c Dialog.Help standard button is clicked.

    \sa accepted(), rejected()
*/

QPlatformDialogHelper::ButtonRole QQuickDialogPrivate::buttonRole(QQuickAbstractButton *button)
{
    const QQuickDialogButtonBoxAttached *attached = qobject_cast<QQuickDialogButtonBoxAttached *>(qmlAttachedPropertiesObject<QQuickDialogButtonBox>(button, false));
    return attached ? attached->buttonRole() : QPlatformDialogHelper::InvalidRole;
}

void QQuickDialogPrivate::handleClick(QQuickAbstractButton *button)
{
    Q_Q(QQuickDialog);
    switch (buttonRole(button)) {
    case QPlatformDialogHelper::ApplyRole:
        emit q->applied();
        break;
    case QPlatformDialogHelper::ResetRole:
        emit q->reset();
        break;
    case QPlatformDialogHelper::DestructiveRole:
        emit q->discarded();
        break;
    case QPlatformDialogHelper::HelpRole:
        emit q->helpRequested();
        break;
    default:
        break;
    }
}

QQuickDialog::QQuickDialog(QObject *parent)
    : QQuickPopup(*(new QQuickDialogPrivate), parent)
{
    Q_D(QQuickDialog);
    connect(d->popupItem, &QQuickPopupItem::titleChanged, this, &QQuickDialog::titleChanged);
    connect(d->popupItem, &QQuickPopupItem::headerChanged, this, &QQuickDialog::headerChanged);
    connect(d->popupItem, &QQuickPopupItem::footerChanged, this, &QQuickDialog::footerChanged);
    connect(d->popupItem, &QQuickPopupItem::implicitHeaderWidthChanged, this, &QQuickDialog::implicitHeaderWidthChanged);
    connect(d->popupItem, &QQuickPopupItem::implicitHeaderHeightChanged, this, &QQuickDialog::implicitHeaderHeightChanged);
    connect(d->popupItem, &QQuickPopupItem::implicitFooterWidthChanged, this, &QQuickDialog::implicitFooterWidthChanged);
    connect(d->popupItem, &QQuickPopupItem::implicitFooterHeightChanged, this, &QQuickDialog::implicitFooterHeightChanged);
}

/*!
    \qmlproperty string QtQuick.Controls::Dialog::title

    This property holds the dialog title.

    The title is displayed in the dialog header.

    \code
    Dialog {
        title: qsTr("About")

        Label {
            text: "Lorem ipsum..."
        }
    }
    \endcode
*/
QString QQuickDialog::title() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->title();
}

void QQuickDialog::setTitle(const QString &title)
{
    Q_D(QQuickDialog);
    d->popupItem->setTitle(title);
    setAccessibleName(title);
}

/*!
    \qmlproperty Item QtQuick.Controls::Dialog::header

    This property holds the dialog header item. The header item is positioned to
    the top, and resized to the width of the dialog. The default value is \c null.

    \note Assigning a \l DialogButtonBox as a dialog header automatically connects
    its \l {DialogButtonBox::}{accepted()} and \l {DialogButtonBox::}{rejected()}
    signals to the respective signals in Dialog.

    \note Assigning a \l DialogButtonBox, \l ToolBar, or \l TabBar as a dialog
    header automatically sets the respective \l DialogButtonBox::position,
    \l ToolBar::position, or \l TabBar::position property to \c Header.

    \sa footer
*/
QQuickItem *QQuickDialog::header() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->header();
}

void QQuickDialog::setHeader(QQuickItem *header)
{
    Q_D(QQuickDialog);
    QQuickItem *oldHeader = d->popupItem->header();
    if (oldHeader == header)
        return;

    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(oldHeader)) {
        disconnect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        disconnect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        QObjectPrivate::disconnect(buttonBox, &QQuickDialogButtonBox::clicked, d, &QQuickDialogPrivate::handleClick);
        if (d->buttonBox == buttonBox)
            d->buttonBox = nullptr;
    }

    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(header)) {
        connect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        connect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        QObjectPrivate::connect(buttonBox, &QQuickDialogButtonBox::clicked, d, &QQuickDialogPrivate::handleClick);
        d->buttonBox = buttonBox;
        buttonBox->setStandardButtons(d->standardButtons);
    }

    d->popupItem->setHeader(header);
}

/*!
    \qmlproperty Item QtQuick.Controls::Dialog::footer

    This property holds the dialog footer item. The footer item is positioned to
    the bottom, and resized to the width of the dialog. The default value is \c null.

    \note Assigning a \l DialogButtonBox as a dialog footer automatically connects
    its \l {DialogButtonBox::}{accepted()} and \l {DialogButtonBox::}{rejected()}
    signals to the respective signals in Dialog.

    \note Assigning a \l DialogButtonBox, \l ToolBar, or \l TabBar as a dialog
    footer automatically sets the respective \l DialogButtonBox::position,
    \l ToolBar::position, or \l TabBar::position property to \c Footer.

    \sa header
*/
QQuickItem *QQuickDialog::footer() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->footer();
}

void QQuickDialog::setFooter(QQuickItem *footer)
{
    Q_D(QQuickDialog);
    QQuickItem *oldFooter = d->popupItem->footer();
    if (oldFooter == footer)
        return;

    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(oldFooter)) {
        disconnect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        disconnect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        QObjectPrivate::disconnect(buttonBox, &QQuickDialogButtonBox::clicked, d, &QQuickDialogPrivate::handleClick);
        if (d->buttonBox == buttonBox)
            d->buttonBox = nullptr;
    }
    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(footer)) {
        connect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        connect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        QObjectPrivate::connect(buttonBox, &QQuickDialogButtonBox::clicked, d, &QQuickDialogPrivate::handleClick);
        d->buttonBox = buttonBox;
        buttonBox->setStandardButtons(d->standardButtons);
    }

    d->popupItem->setFooter(footer);
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Dialog::standardButtons

    This property holds a combination of standard buttons that are used by the dialog.

    \snippet qtquickcontrols2-dialog.qml 1

    The buttons will be positioned in the appropriate order for the user's platform.

    Possible flags:
    \value Dialog.Ok An "OK" button defined with the \c AcceptRole.
    \value Dialog.Open An "Open" button defined with the \c AcceptRole.
    \value Dialog.Save A "Save" button defined with the \c AcceptRole.
    \value Dialog.Cancel A "Cancel" button defined with the \c RejectRole.
    \value Dialog.Close A "Close" button defined with the \c RejectRole.
    \value Dialog.Discard A "Discard" or "Don't Save" button, depending on the platform, defined with the \c DestructiveRole.
    \value Dialog.Apply An "Apply" button defined with the \c ApplyRole.
    \value Dialog.Reset A "Reset" button defined with the \c ResetRole.
    \value Dialog.RestoreDefaults A "Restore Defaults" button defined with the \c ResetRole.
    \value Dialog.Help A "Help" button defined with the \c HelpRole.
    \value Dialog.SaveAll A "Save All" button defined with the \c AcceptRole.
    \value Dialog.Yes A "Yes" button defined with the \c YesRole.
    \value Dialog.YesToAll A "Yes to All" button defined with the \c YesRole.
    \value Dialog.No A "No" button defined with the \c NoRole.
    \value Dialog.NoToAll A "No to All" button defined with the \c NoRole.
    \value Dialog.Abort An "Abort" button defined with the \c RejectRole.
    \value Dialog.Retry A "Retry" button defined with the \c AcceptRole.
    \value Dialog.Ignore An "Ignore" button defined with the \c AcceptRole.
    \value Dialog.NoButton An invalid button.

    \sa DialogButtonBox
*/
QPlatformDialogHelper::StandardButtons QQuickDialog::standardButtons() const
{
    Q_D(const QQuickDialog);
    return d->standardButtons;
}

void QQuickDialog::setStandardButtons(QPlatformDialogHelper::StandardButtons buttons)
{
    Q_D(QQuickDialog);
    if (d->standardButtons == buttons)
        return;

    d->standardButtons = buttons;
    if (d->buttonBox)
        d->buttonBox->setStandardButtons(buttons);
    emit standardButtonsChanged();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlmethod AbstractButton QtQuick.Controls::Dialog::standardButton(StandardButton button)

    Returns the specified standard \a button, or \c null if it does not exist.

    \sa standardButtons
*/
QQuickAbstractButton *QQuickDialog::standardButton(QPlatformDialogHelper::StandardButton button) const
{
    Q_D(const QQuickDialog);
    if (!d->buttonBox)
        return nullptr;
    return d->buttonBox->standardButton(button);
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty int QtQuick.Controls::Dialog::result

    This property holds the result code.

    Standard result codes:
    \value Dialog.Accepted The dialog was accepted.
    \value Dialog.Rejected The dialog was rejected.

    \sa accept(), reject(), done()
*/
int QQuickDialog::result() const
{
    Q_D(const QQuickDialog);
    return d->result;
}

void QQuickDialog::setResult(int result)
{
    Q_D(QQuickDialog);
    if (d->result == result)
        return;

    d->result = result;
    emit resultChanged();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Dialog::implicitHeaderWidth
    \readonly

    This property holds the implicit header width.

    The value is equal to \c {header && header.visible ? header.implicitWidth : 0}.

    \sa implicitHeaderHeight, implicitFooterWidth
*/
qreal QQuickDialog::implicitHeaderWidth() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->implicitHeaderWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Dialog::implicitHeaderHeight
    \readonly

    This property holds the implicit header height.

    The value is equal to \c {header && header.visible ? header.implicitHeight : 0}.

    \sa implicitHeaderWidth, implicitFooterHeight
*/
qreal QQuickDialog::implicitHeaderHeight() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->implicitHeaderHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Dialog::implicitFooterWidth
    \readonly

    This property holds the implicit footer width.

    The value is equal to \c {footer && footer.visible ? footer.implicitWidth : 0}.

    \sa implicitFooterHeight, implicitHeaderWidth
*/
qreal QQuickDialog::implicitFooterWidth() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->implicitFooterWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Dialog::implicitFooterHeight
    \readonly

    This property holds the implicit footer height.

    The value is equal to \c {footer && footer.visible ? footer.implicitHeight : 0}.

    \sa implicitFooterWidth, implicitHeaderHeight
*/
qreal QQuickDialog::implicitFooterHeight() const
{
    Q_D(const QQuickDialog);
    return d->popupItem->implicitFooterHeight();
}

/*!
    \qmlmethod void QtQuick.Controls::Dialog::accept()

    Closes the dialog and emits the \l accepted() signal.

    \sa reject(), done()
*/
void QQuickDialog::accept()
{
    done(Accepted);
}

/*!
    \qmlmethod void QtQuick.Controls::Dialog::reject()

    Closes the dialog and emits the \l rejected() signal.

    \sa accept(), done()
*/
void QQuickDialog::reject()
{
    done(Rejected);
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlmethod void QtQuick.Controls::Dialog::done(int result)

    Closes the dialog, sets the \a result, and emits \l accepted() or
    \l rejected() depending on whether the result is \c Dialog.Accepted
    or \c Dialog.Rejected, respectively.

    \sa accept(), reject(), result
*/
void QQuickDialog::done(int result)
{
    close();
    setResult(result);

    if (result == Accepted)
        emit accepted();
    else if (result == Rejected)
        emit rejected();
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickDialog::accessibleRole() const
{
    return QAccessible::Dialog;
}

void QQuickDialog::accessibilityActiveChanged(bool active)
{
    Q_D(QQuickDialog);
    QQuickPopup::accessibilityActiveChanged(active);

    if (active)
        setAccessibleName(d->popupItem->title());
}
#endif

QT_END_NAMESPACE
