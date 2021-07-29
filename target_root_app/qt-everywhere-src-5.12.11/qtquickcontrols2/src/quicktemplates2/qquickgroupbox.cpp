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

#include "qquickgroupbox_p.h"
#include "qquickframe_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtGui/qpa/qplatformtheme.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype GroupBox
    \inherits Frame
    \instantiates QQuickGroupBox
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-containers
    \brief Visual frame and title for a logical group of controls.

    GroupBox is used to layout a logical group of controls together, within
    a \l {title}{titled} visual frame. GroupBox does not provide a layout of its own, but
    requires you to position its contents, for instance by creating a \l RowLayout
    or a \l ColumnLayout.

    Items declared as children of a GroupBox are automatically parented to the
    GroupBox's \l {Control::}{contentItem}. Items created dynamically need to be
    explicitly parented to the contentItem.

    If only a single item is used within a GroupBox, it will resize to fit the
    implicit size of its contained item. This makes it particularly suitable
    for use together with layouts.

    \image qtquickcontrols2-groupbox.png

    \snippet qtquickcontrols2-groupbox.qml 1

    \section2 Checkable GroupBox

    Even though GroupBox has no built-in check box, it is straightforward
    to create a checkable GroupBox by pairing it with a CheckBox.

    \image qtquickcontrols2-groupbox-checkable.png

    It is a common pattern to enable or disable the groupbox's children when
    its checkbox is toggled on or off, but it is up to the application to decide
    on the behavior of the checkbox.

    \snippet qtquickcontrols2-groupbox-checkable.qml 1

    \sa CheckBox, {Customizing GroupBox}, {Container Controls}
*/

class QQuickGroupBoxPrivate : public QQuickFramePrivate
{
    Q_DECLARE_PUBLIC(QQuickGroupBox)

public:
    void cancelLabel();
    void executeLabel(bool complete = false);

    void itemImplicitWidthChanged(QQuickItem *item) override;
    void itemImplicitHeightChanged(QQuickItem *item) override;

    QString title;
    QQuickDeferredPointer<QQuickItem> label;
};

static inline QString labelName() { return QStringLiteral("label"); }

void QQuickGroupBoxPrivate::cancelLabel()
{
    Q_Q(QQuickGroupBox);
    quickCancelDeferred(q, labelName());
}

void QQuickGroupBoxPrivate::executeLabel(bool complete)
{
    Q_Q(QQuickGroupBox);
    if (label.wasExecuted())
        return;

    if (!label || complete)
        quickBeginDeferred(q, labelName(), label);
    if (complete)
        quickCompleteDeferred(q, labelName(), label);
}

void QQuickGroupBoxPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickGroupBox);
    QQuickFramePrivate::itemImplicitWidthChanged(item);
    if (item == label)
        emit q->implicitLabelWidthChanged();
}

void QQuickGroupBoxPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickGroupBox);
    QQuickFramePrivate::itemImplicitHeightChanged(item);
    if (item == label)
        emit q->implicitLabelHeightChanged();
}

QQuickGroupBox::QQuickGroupBox(QQuickItem *parent)
    : QQuickFrame(*(new QQuickGroupBoxPrivate), parent)
{
}

QQuickGroupBox::~QQuickGroupBox()
{
    Q_D(QQuickGroupBox);
    d->removeImplicitSizeListener(d->label);
}

/*!
    \qmlproperty string QtQuick.Controls::GroupBox::title

    This property holds the title.

    The title is typically displayed above the groupbox to
    summarize its contents.
*/
QString QQuickGroupBox::title() const
{
    Q_D(const QQuickGroupBox);
    return d->title;
}

void QQuickGroupBox::setTitle(const QString &title)
{
    Q_D(QQuickGroupBox);
    if (d->title == title)
        return;

    d->title = title;
    setAccessibleName(title);
    emit titleChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::GroupBox::label

    This property holds the label item that visualizes \l title.

    \sa {Customizing GroupBox}
*/
QQuickItem *QQuickGroupBox::label() const
{
    QQuickGroupBoxPrivate *d = const_cast<QQuickGroupBoxPrivate *>(d_func());
    if (!d->label)
        d->executeLabel();
    return d->label;
}

void QQuickGroupBox::setLabel(QQuickItem *label)
{
    Q_D(QQuickGroupBox);
    if (d->label == label)
        return;

    if (!d->label.isExecuting())
        d->cancelLabel();

    const qreal oldImplicitLabelWidth = implicitLabelWidth();
    const qreal oldImplicitLabelHeight = implicitLabelHeight();

    d->removeImplicitSizeListener(d->label);
    delete d->label;
    d->label = label;

    if (label) {
        if (!label->parentItem())
            label->setParentItem(this);
        d->addImplicitSizeListener(label);
    }

    if (!qFuzzyCompare(oldImplicitLabelWidth, implicitLabelWidth()))
        emit implicitLabelWidthChanged();
    if (!qFuzzyCompare(oldImplicitLabelHeight, implicitLabelHeight()))
        emit implicitLabelHeightChanged();
    if (!d->label.isExecuting())
        emit labelChanged();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::GroupBox::implicitLabelWidth
    \readonly

    This property holds the implicit label width.

    The value is equal to \c {label ? label.implicitWidth : 0}.

    \sa implicitLabelHeight
*/
qreal QQuickGroupBox::implicitLabelWidth() const
{
    Q_D(const QQuickGroupBox);
    if (!d->label)
        return 0;
    return d->label->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::GroupBox::implicitLabelHeight
    \readonly

    This property holds the implicit label height.

    The value is equal to \c {label ? label.implicitHeight : 0}.

    \sa implicitLabelWidth
*/
qreal QQuickGroupBox::implicitLabelHeight() const
{
    Q_D(const QQuickGroupBox);
    if (!d->label)
        return 0;
    return d->label->implicitHeight();
}

void QQuickGroupBox::componentComplete()
{
    Q_D(QQuickGroupBox);
    d->executeLabel(true);
    QQuickFrame::componentComplete();
}

QFont QQuickGroupBox::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::GroupBox);
}

QPalette QQuickGroupBox::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::GroupBox);
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickGroupBox::accessibleRole() const
{
    return QAccessible::Grouping;
}

void QQuickGroupBox::accessibilityActiveChanged(bool active)
{
    Q_D(QQuickGroupBox);
    QQuickFrame::accessibilityActiveChanged(active);

    if (active)
        setAccessibleName(d->title);
}
#endif

QT_END_NAMESPACE
