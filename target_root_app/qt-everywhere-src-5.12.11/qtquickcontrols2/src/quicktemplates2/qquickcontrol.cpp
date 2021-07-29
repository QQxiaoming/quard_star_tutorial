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

#include "qquickcontrol_p.h"
#include "qquickcontrol_p_p.h"

#include <QtGui/qstylehints.h>
#include <QtGui/qguiapplication.h>
#include "qquicklabel_p.h"
#include "qquicklabel_p_p.h"
#include "qquicktextarea_p.h"
#include "qquicktextarea_p_p.h"
#include "qquicktextfield_p.h"
#include "qquicktextfield_p_p.h"
#include "qquickpopup_p.h"
#include "qquickpopupitem_p_p.h"
#include "qquickapplicationwindow_p.h"
#include "qquickdeferredexecute_p_p.h"

#if QT_CONFIG(accessibility)
#include <QtQuick/private/qquickaccessibleattached_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype Control
    \inherits Item
    \instantiates QQuickControl
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \brief Abstract base type providing functionality common to all controls.

    Control is the base type of user interface controls.  It receives input
    events from the window system, and paints a representation of itself on
    the screen.

    \section1 Control Layout

    The following diagram illustrates the layout of a typical control:

    \image qtquickcontrols2-control.png

    The \l {Item::}{implicitWidth} and \l {Item::}{implicitHeight} of a control
    are typically based on the implicit sizes of the background and the content
    item plus any insets and paddings. These properties determine how large
    the control will be when no explicit \l {Item::}{width} or
    \l {Item::}{height} is specified.

    The geometry of the \l {Control::}{contentItem} is determined by the padding.
    The following example reserves 10px padding between the boundaries of the
    control and its content:

    \code
    Control {
        padding: 10

        contentItem: Text {
            text: "Content"
        }
    }
    \endcode

    The \l {Control::}{background} item fills the entire width and height of the
    control, unless insets or an explicit size have been given for it. Background
    insets are useful for extending the touchable/interactive area of a control
    without affecting its visual size. This is often used on touch devices to
    ensure that a control is not too small to be interacted with by the user.
    Insets affect the size of the control, and hence will affect how much space
    they take up in a layout, for example.

    Negative insets can be used to make the background larger than the control.
    The following example uses negative insets to place a shadow outside the
    control's boundaries:

    \code
    Control {
        topInset: -2
        leftInset: -2
        rightInset: -6
        bottomInset: -6

        background: BorderImage {
            source: ":/images/shadowed-background.png"
        }
    }
    \endcode

    \section1 Event Handling

    All controls, except non-interactive indicators, do not let clicks and
    touches through to items below them. For example, the \c console.log()
    call in the example below will never be executed when clicking on the
    Pane, because the \l MouseArea is below it in the scene:

    \code
    MouseArea {
        anchors.fill: parent
        onClicked: console.log("MouseArea was clicked")

        Pane {
            anchors.fill: parent
        }
    }
    \endcode

    \sa ApplicationWindow, Container
*/

const QQuickItemPrivate::ChangeTypes QQuickControlPrivate::ImplicitSizeChanges = QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight | QQuickItemPrivate::Destroyed;

static bool isKeyFocusReason(Qt::FocusReason reason)
{
    return reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason || reason == Qt::ShortcutFocusReason;
}

QQuickControlPrivate::QQuickControlPrivate()
{
#if QT_CONFIG(accessibility)
    QAccessible::installActivationObserver(this);
#endif
}

QQuickControlPrivate::~QQuickControlPrivate()
{
#if QT_CONFIG(accessibility)
    QAccessible::removeActivationObserver(this);
#endif
}

void QQuickControlPrivate::init()
{
    Q_Q(QQuickControl);
    QObject::connect(q, &QQuickItem::baselineOffsetChanged, q, &QQuickControl::baselineOffsetChanged);
}

#if QT_CONFIG(quicktemplates2_multitouch)
bool QQuickControlPrivate::acceptTouch(const QTouchEvent::TouchPoint &point)
{
    if (point.id() == touchId)
        return true;

    if (touchId == -1 && point.state() == Qt::TouchPointPressed) {
        touchId = point.id();
        return true;
    }

    return false;
}
#endif

static void setActiveFocus(QQuickControl *control, Qt::FocusReason reason)
{
    QQuickControlPrivate *d = QQuickControlPrivate::get(control);
    if (d->subFocusItem && d->window && d->flags & QQuickItem::ItemIsFocusScope)
        QQuickWindowPrivate::get(d->window)->clearFocusInScope(control, d->subFocusItem, reason);
    control->forceActiveFocus(reason);
}

void QQuickControlPrivate::handlePress(const QPointF &)
{
    Q_Q(QQuickControl);
    if ((focusPolicy & Qt::ClickFocus) == Qt::ClickFocus && !QGuiApplication::styleHints()->setFocusOnTouchRelease())
        setActiveFocus(q, Qt::MouseFocusReason);
}

void QQuickControlPrivate::handleMove(const QPointF &point)
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_Q(QQuickControl);
    q->setHovered(hoverEnabled && q->contains(point));
#else
    Q_UNUSED(point);
#endif
}

void QQuickControlPrivate::handleRelease(const QPointF &)
{
    Q_Q(QQuickControl);
    if ((focusPolicy & Qt::ClickFocus) == Qt::ClickFocus && QGuiApplication::styleHints()->setFocusOnTouchRelease())
        setActiveFocus(q, Qt::MouseFocusReason);
    touchId = -1;
}

void QQuickControlPrivate::handleUngrab()
{
    touchId = -1;
}

void QQuickControlPrivate::mirrorChange()
{
    Q_Q(QQuickControl);
    if (locale.textDirection() == Qt::LeftToRight)
        q->mirrorChange();
}

void QQuickControlPrivate::setTopPadding(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldPadding = getPadding();
    extra.value().topPadding = value;
    extra.value().hasTopPadding = !reset;
    if ((!reset && !qFuzzyCompare(oldPadding.top(), value)) || (reset && !qFuzzyCompare(oldPadding.top(), getVerticalPadding()))) {
        emit q->topPaddingChanged();
        emit q->availableHeightChanged();
        q->paddingChange(getPadding(), oldPadding);
    }
}

void QQuickControlPrivate::setLeftPadding(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldPadding = getPadding();
    extra.value().leftPadding = value;
    extra.value().hasLeftPadding = !reset;
    if ((!reset && !qFuzzyCompare(oldPadding.left(), value)) || (reset && !qFuzzyCompare(oldPadding.left(), getHorizontalPadding()))) {
        emit q->leftPaddingChanged();
        emit q->availableWidthChanged();
        q->paddingChange(getPadding(), oldPadding);
    }
}

void QQuickControlPrivate::setRightPadding(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldPadding = getPadding();
    extra.value().rightPadding = value;
    extra.value().hasRightPadding = !reset;
    if ((!reset && !qFuzzyCompare(oldPadding.right(), value)) || (reset && !qFuzzyCompare(oldPadding.right(), getHorizontalPadding()))) {
        emit q->rightPaddingChanged();
        emit q->availableWidthChanged();
        q->paddingChange(getPadding(), oldPadding);
    }
}

void QQuickControlPrivate::setBottomPadding(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldPadding = getPadding();
    extra.value().bottomPadding = value;
    extra.value().hasBottomPadding = !reset;
    if ((!reset && !qFuzzyCompare(oldPadding.bottom(), value)) || (reset && !qFuzzyCompare(oldPadding.bottom(), getVerticalPadding()))) {
        emit q->bottomPaddingChanged();
        emit q->availableHeightChanged();
        q->paddingChange(getPadding(), oldPadding);
    }
}

void QQuickControlPrivate::setHorizontalPadding(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldPadding = getPadding();
    const qreal oldHorizontalPadding = getHorizontalPadding();
    horizontalPadding = value;
    hasHorizontalPadding = !reset;
    if ((!reset && !qFuzzyCompare(oldHorizontalPadding, value)) || (reset && !qFuzzyCompare(oldHorizontalPadding, padding))) {
        const QMarginsF newPadding = getPadding();
        if (!qFuzzyCompare(newPadding.left(), oldPadding.left()))
            emit q->leftPaddingChanged();
        if (!qFuzzyCompare(newPadding.right(), oldPadding.right()))
            emit q->rightPaddingChanged();
        emit q->horizontalPaddingChanged();
        emit q->availableWidthChanged();
        q->paddingChange(newPadding, oldPadding);
    }
}

void QQuickControlPrivate::setVerticalPadding(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldPadding = getPadding();
    const qreal oldVerticalPadding = getVerticalPadding();
    verticalPadding = value;
    hasVerticalPadding = !reset;
    if ((!reset && !qFuzzyCompare(oldVerticalPadding, value)) || (reset && !qFuzzyCompare(oldVerticalPadding, padding))) {
        const QMarginsF newPadding = getPadding();
        if (!qFuzzyCompare(newPadding.top(), oldPadding.top()))
            emit q->topPaddingChanged();
        if (!qFuzzyCompare(newPadding.bottom(), oldPadding.bottom()))
            emit q->bottomPaddingChanged();
        emit q->verticalPaddingChanged();
        emit q->availableHeightChanged();
        q->paddingChange(newPadding, oldPadding);
    }
}

void QQuickControlPrivate::setTopInset(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldInset = getInset();
    extra.value().topInset = value;
    extra.value().hasTopInset = !reset;
    if (!qFuzzyCompare(oldInset.top(), value)) {
        emit q->topInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickControlPrivate::setLeftInset(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldInset = getInset();
    extra.value().leftInset = value;
    extra.value().hasLeftInset = !reset;
    if (!qFuzzyCompare(oldInset.left(), value)) {
        emit q->leftInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickControlPrivate::setRightInset(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldInset = getInset();
    extra.value().rightInset = value;
    extra.value().hasRightInset = !reset;
    if (!qFuzzyCompare(oldInset.right(), value)) {
        emit q->rightInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickControlPrivate::setBottomInset(qreal value, bool reset)
{
    Q_Q(QQuickControl);
    const QMarginsF oldInset = getInset();
    extra.value().bottomInset = value;
    extra.value().hasBottomInset = !reset;
    if (!qFuzzyCompare(oldInset.bottom(), value)) {
        emit q->bottomInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickControlPrivate::resizeBackground()
{
    if (!background)
        return;

    resizingBackground = true;

    QQuickItemPrivate *p = QQuickItemPrivate::get(background);
    if (((!p->widthValid || !extra.isAllocated() || !extra->hasBackgroundWidth) && qFuzzyIsNull(background->x()))
            || (extra.isAllocated() && (extra->hasLeftInset || extra->hasRightInset))) {
        background->setX(getLeftInset());
        background->setWidth(width - getLeftInset() - getRightInset());
    }
    if (((!p->heightValid || !extra.isAllocated() || !extra->hasBackgroundHeight) && qFuzzyIsNull(background->y()))
            || (extra.isAllocated() && (extra->hasTopInset || extra->hasBottomInset))) {
        background->setY(getTopInset());
        background->setHeight(height - getTopInset() - getBottomInset());
    }

    resizingBackground = false;
}

void QQuickControlPrivate::resizeContent()
{
    Q_Q(QQuickControl);
    if (contentItem) {
        contentItem->setPosition(QPointF(q->leftPadding(), q->topPadding()));
        contentItem->setSize(QSizeF(q->availableWidth(), q->availableHeight()));
    }
}

QQuickItem *QQuickControlPrivate::getContentItem()
{
    if (!contentItem)
        executeContentItem();
    return contentItem;
}

void QQuickControlPrivate::setContentItem_helper(QQuickItem *item, bool notify)
{
    Q_Q(QQuickControl);
    if (contentItem == item)
        return;

    if (!contentItem.isExecuting())
        cancelContentItem();

    QQuickItem *oldContentItem = contentItem;
    if (oldContentItem) {
        disconnect(oldContentItem, &QQuickItem::baselineOffsetChanged, this, &QQuickControlPrivate::updateBaselineOffset);
        removeImplicitSizeListener(oldContentItem);
    }

    contentItem = item;
    q->contentItemChange(item, oldContentItem);
    delete oldContentItem;

    if (item) {
        connect(contentItem.data(), &QQuickItem::baselineOffsetChanged, this, &QQuickControlPrivate::updateBaselineOffset);
        if (!item->parentItem())
            item->setParentItem(q);
        if (componentComplete)
            resizeContent();
        addImplicitSizeListener(contentItem);
    }

    updateImplicitContentSize();
    updateBaselineOffset();

    if (notify && !contentItem.isExecuting())
        emit q->contentItemChanged();
}

qreal QQuickControlPrivate::getContentWidth() const
{
    return contentItem ? contentItem->implicitWidth() : 0;
}

qreal QQuickControlPrivate::getContentHeight() const
{
    return contentItem ? contentItem->implicitHeight() : 0;
}

void QQuickControlPrivate::updateImplicitContentWidth()
{
    Q_Q(QQuickControl);
    const qreal oldWidth = implicitContentWidth;
    implicitContentWidth = getContentWidth();
    if (!qFuzzyCompare(implicitContentWidth, oldWidth))
        emit q->implicitContentWidthChanged();
}

void QQuickControlPrivate::updateImplicitContentHeight()
{
    Q_Q(QQuickControl);
    const qreal oldHeight = implicitContentHeight;
    implicitContentHeight = getContentHeight();
    if (!qFuzzyCompare(implicitContentHeight, oldHeight))
        emit q->implicitContentHeightChanged();
}

void QQuickControlPrivate::updateImplicitContentSize()
{
    Q_Q(QQuickControl);
    const qreal oldWidth = implicitContentWidth;
    const qreal oldHeight = implicitContentHeight;
    implicitContentWidth = getContentWidth();
    implicitContentHeight = getContentHeight();
    if (!qFuzzyCompare(implicitContentWidth, oldWidth))
        emit q->implicitContentWidthChanged();
    if (!qFuzzyCompare(implicitContentHeight, oldHeight))
        emit q->implicitContentHeightChanged();
}

#if QT_CONFIG(accessibility)
void QQuickControlPrivate::accessibilityActiveChanged(bool active)
{
    Q_Q(QQuickControl);
    return q->accessibilityActiveChanged(active);
}

QAccessible::Role QQuickControlPrivate::accessibleRole() const
{
    Q_Q(const QQuickControl);
    return q->accessibleRole();
}

QQuickAccessibleAttached *QQuickControlPrivate::accessibleAttached(const QObject *object)
{
    if (!QAccessible::isActive())
        return nullptr;
    return QQuickAccessibleAttached::attachedProperties(object);
}
#endif

/*!
    \internal

    Returns the font that the control w inherits from its ancestors and
    QGuiApplication::font.
*/
QFont QQuickControlPrivate::parentFont(const QQuickItem *item)
{
    QQuickItem *p = item->parentItem();
    while (p) {
        if (QQuickControl *control = qobject_cast<QQuickControl *>(p))
            return control->font();
        else if (QQuickLabel *label = qobject_cast<QQuickLabel *>(p))
            return label->font();
        else if (QQuickTextField *textField = qobject_cast<QQuickTextField *>(p))
            return textField->font();
        else if (QQuickTextArea *textArea = qobject_cast<QQuickTextArea *>(p))
            return textArea->font();

        p = p->parentItem();
    }

    if (QQuickApplicationWindow *window = qobject_cast<QQuickApplicationWindow *>(item->window()))
        return window->font();

    return QQuickTheme::font(QQuickTheme::System);
}

/*!
    \internal

    Determine which font is implicitly imposed on this control by its ancestors
    and QGuiApplication::font, resolve this against its own font (attributes from
    the implicit font are copied over). Then propagate this font to this
    control's children.
*/
void QQuickControlPrivate::resolveFont()
{
    Q_Q(QQuickControl);
    inheritFont(parentFont(q));
}

void QQuickControlPrivate::inheritFont(const QFont &font)
{
    Q_Q(QQuickControl);
    QFont parentFont = extra.isAllocated() ? extra->requestedFont.resolve(font) : font;
    parentFont.resolve(extra.isAllocated() ? extra->requestedFont.resolve() | font.resolve() : font.resolve());

    const QFont defaultFont = q->defaultFont();
    const QFont resolvedFont = parentFont.resolve(defaultFont);

    setFont_helper(resolvedFont);
}

/*!
    \internal

    Assign \a font to this control, and propagate it to all children.
*/
void QQuickControlPrivate::updateFont(const QFont &font)
{
    Q_Q(QQuickControl);
    QFont oldFont = resolvedFont;
    resolvedFont = font;

    if (oldFont != font)
        q->fontChange(font, oldFont);

    QQuickControlPrivate::updateFontRecur(q, font);

    if (oldFont != font)
        emit q->fontChanged();
}

void QQuickControlPrivate::updateFontRecur(QQuickItem *item, const QFont &font)
{
    const auto childItems = item->childItems();
    for (QQuickItem *child : childItems) {
        if (QQuickControl *control = qobject_cast<QQuickControl *>(child))
            QQuickControlPrivate::get(control)->inheritFont(font);
        else if (QQuickLabel *label = qobject_cast<QQuickLabel *>(child))
            QQuickLabelPrivate::get(label)->inheritFont(font);
        else if (QQuickTextArea *textArea = qobject_cast<QQuickTextArea *>(child))
            QQuickTextAreaPrivate::get(textArea)->inheritFont(font);
        else if (QQuickTextField *textField = qobject_cast<QQuickTextField *>(child))
            QQuickTextFieldPrivate::get(textField)->inheritFont(font);
        else
            QQuickControlPrivate::updateFontRecur(child, font);
    }
}

/*!
    \internal

    Returns the palette that the item inherits from its ancestors and
    QGuiApplication::palette.
*/
QPalette QQuickControlPrivate::parentPalette(const QQuickItem *item)
{
    QQuickItem *p = item->parentItem();
    while (p) {
        if (QQuickControl *control = qobject_cast<QQuickControl *>(p))
            return control->palette();
        else if (QQuickLabel *label = qobject_cast<QQuickLabel *>(p))
            return label->palette();
        else if (QQuickTextField *textField = qobject_cast<QQuickTextField *>(p))
            return textField->palette();
        else if (QQuickTextArea *textArea = qobject_cast<QQuickTextArea *>(p))
            return textArea->palette();

        p = p->parentItem();
    }

    if (QQuickApplicationWindow *window = qobject_cast<QQuickApplicationWindow *>(item->window()))
        return window->palette();

    return QQuickTheme::palette(QQuickTheme::System);
}

/*!
    \internal

    Determine which palette is implicitly imposed on this control by its ancestors
    and QGuiApplication::palette, resolve this against its own palette (attributes from
    the implicit palette are copied over). Then propagate this palette to this
    control's children.
*/
void QQuickControlPrivate::resolvePalette()
{
    Q_Q(QQuickControl);
    inheritPalette(parentPalette(q));
}

void QQuickControlPrivate::inheritPalette(const QPalette &palette)
{
    Q_Q(QQuickControl);
    QPalette parentPalette = extra.isAllocated() ? extra->requestedPalette.resolve(palette) : palette;
    parentPalette.resolve(extra.isAllocated() ? extra->requestedPalette.resolve() | palette.resolve() : palette.resolve());

    const QPalette defaultPalette = q->defaultPalette();
    const QPalette resolvedPalette = parentPalette.resolve(defaultPalette);

    setPalette_helper(resolvedPalette);
}

/*!
    \internal

    Assign \a palette to this control, and propagate it to all children.
*/
void QQuickControlPrivate::updatePalette(const QPalette &palette)
{
    Q_Q(QQuickControl);
    QPalette oldPalette = resolvedPalette;
    resolvedPalette = palette;

    if (oldPalette != palette)
        q->paletteChange(palette, oldPalette);

    QQuickControlPrivate::updatePaletteRecur(q, palette);

    if (oldPalette != palette)
        emit q->paletteChanged();
}

void QQuickControlPrivate::updatePaletteRecur(QQuickItem *item, const QPalette &palette)
{
    const auto childItems = item->childItems();
    for (QQuickItem *child : childItems) {
        if (QQuickControl *control = qobject_cast<QQuickControl *>(child))
            QQuickControlPrivate::get(control)->inheritPalette(palette);
        else if (QQuickLabel *label = qobject_cast<QQuickLabel *>(child))
            QQuickLabelPrivate::get(label)->inheritPalette(palette);
        else if (QQuickTextArea *textArea = qobject_cast<QQuickTextArea *>(child))
            QQuickTextAreaPrivate::get(textArea)->inheritPalette(palette);
        else if (QQuickTextField *textField = qobject_cast<QQuickTextField *>(child))
            QQuickTextFieldPrivate::get(textField)->inheritPalette(palette);
        else
            QQuickControlPrivate::updatePaletteRecur(child, palette);
    }
}

QLocale QQuickControlPrivate::calcLocale(const QQuickItem *item)
{
    const QQuickItem *p = item;
    while (p) {
        if (const QQuickControl *control = qobject_cast<const QQuickControl *>(p))
            return control->locale();

        QVariant v = p->property("locale");
        if (v.isValid() && v.userType() == QMetaType::QLocale)
            return v.toLocale();

        p = p->parentItem();
    }

    if (item) {
        if (QQuickApplicationWindow *window = qobject_cast<QQuickApplicationWindow *>(item->window()))
            return window->locale();
    }

    return QLocale();
}

void QQuickControlPrivate::updateLocale(const QLocale &l, bool e)
{
    Q_Q(QQuickControl);
    if (!e && hasLocale)
        return;

    QLocale old = q->locale();
    hasLocale = e;
    if (old != l) {
        bool wasMirrored = q->isMirrored();
        locale = l;
        q->localeChange(l, old);
        QQuickControlPrivate::updateLocaleRecur(q, l);
        emit q->localeChanged();
        if (wasMirrored != q->isMirrored())
            q->mirrorChange();
    }
}

void QQuickControlPrivate::updateLocaleRecur(QQuickItem *item, const QLocale &l)
{
    const auto childItems = item->childItems();
    for (QQuickItem *child : childItems) {
        if (QQuickControl *control = qobject_cast<QQuickControl *>(child))
            QQuickControlPrivate::get(control)->updateLocale(l, false);
        else
            updateLocaleRecur(child, l);
    }
}

#if QT_CONFIG(quicktemplates2_hover)
void QQuickControlPrivate::updateHoverEnabled(bool enabled, bool xplicit)
{
    Q_Q(QQuickControl);
    if (!xplicit && explicitHoverEnabled)
        return;

    bool wasEnabled = q->isHoverEnabled();
    explicitHoverEnabled = xplicit;
    if (wasEnabled != enabled) {
        q->setAcceptHoverEvents(enabled);
        QQuickControlPrivate::updateHoverEnabledRecur(q, enabled);
        emit q->hoverEnabledChanged();
    }
}

void QQuickControlPrivate::updateHoverEnabledRecur(QQuickItem *item, bool enabled)
{
    const auto childItems = item->childItems();
    for (QQuickItem *child : childItems) {
        if (QQuickControl *control = qobject_cast<QQuickControl *>(child))
            QQuickControlPrivate::get(control)->updateHoverEnabled(enabled, false);
        else
            updateHoverEnabledRecur(child, enabled);
    }
}

bool QQuickControlPrivate::calcHoverEnabled(const QQuickItem *item)
{
    const QQuickItem *p = item;
    while (p) {
        // QQuickPopupItem accepts hover events to avoid leaking them through.
        // Don't inherit that to the children of the popup, but fallback to the
        // environment variable or style hint.
        if (qobject_cast<const QQuickPopupItem *>(p))
            break;

        if (const QQuickControl *control = qobject_cast<const QQuickControl *>(p))
            return control->isHoverEnabled();

        QVariant v = p->property("hoverEnabled");
        if (v.isValid() && v.userType() == QMetaType::Bool)
            return v.toBool();

        p = p->parentItem();
    }

    bool ok = false;
    int env = qEnvironmentVariableIntValue("QT_QUICK_CONTROLS_HOVER_ENABLED", &ok);
    if (ok)
        return env != 0;

    // TODO: QQuickApplicationWindow::isHoverEnabled()

    return QGuiApplication::styleHints()->useHoverEffects();
}
#endif

static inline QString contentItemName() { return QStringLiteral("contentItem"); }

void QQuickControlPrivate::cancelContentItem()
{
    Q_Q(QQuickControl);
    quickCancelDeferred(q, contentItemName());
}

void QQuickControlPrivate::executeContentItem(bool complete)
{
    Q_Q(QQuickControl);
    if (contentItem.wasExecuted())
        return;

    if (!contentItem || complete)
        quickBeginDeferred(q, contentItemName(), contentItem);
    if (complete)
        quickCompleteDeferred(q, contentItemName(), contentItem);
}

static inline QString backgroundName() { return QStringLiteral("background"); }

void QQuickControlPrivate::cancelBackground()
{
    Q_Q(QQuickControl);
    quickCancelDeferred(q, backgroundName());
}

void QQuickControlPrivate::executeBackground(bool complete)
{
    Q_Q(QQuickControl);
    if (background.wasExecuted())
        return;

    if (!background || complete)
        quickBeginDeferred(q, backgroundName(), background);
    if (complete)
        quickCompleteDeferred(q, backgroundName(), background);
}

void QQuickControlPrivate::updateBaselineOffset()
{
    Q_Q(QQuickControl);
    if (extra.isAllocated() && extra.value().hasBaselineOffset)
        return;

    if (!contentItem)
        q->QQuickItem::setBaselineOffset(0);
    else
        q->QQuickItem::setBaselineOffset(getTopPadding() + contentItem->baselineOffset());
}

void QQuickControlPrivate::addImplicitSizeListener(QQuickItem *item, ChangeTypes changes)
{
    addImplicitSizeListener(item, this, changes);
}

void QQuickControlPrivate::removeImplicitSizeListener(QQuickItem *item, ChangeTypes changes)
{
    removeImplicitSizeListener(item, this, changes);
}

void QQuickControlPrivate::addImplicitSizeListener(QQuickItem *item, QQuickItemChangeListener *listener, ChangeTypes changes)
{
    if (!item || !listener)
        return;
    QQuickItemPrivate::get(item)->addItemChangeListener(listener, changes);
}

void QQuickControlPrivate::removeImplicitSizeListener(QQuickItem *item, QQuickItemChangeListener *listener, ChangeTypes changes)
{
    if (!item || !listener)
        return;
    QQuickItemPrivate::get(item)->removeItemChangeListener(listener, changes);
}

void QQuickControlPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickControl);
    if (item == background)
        emit q->implicitBackgroundWidthChanged();
    else if (item == contentItem)
        updateImplicitContentWidth();
}

void QQuickControlPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickControl);
    if (item == background)
        emit q->implicitBackgroundHeightChanged();
    else if (item == contentItem)
        updateImplicitContentHeight();
}

void QQuickControlPrivate::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &diff)
{
    Q_UNUSED(diff);
    if (resizingBackground || item != background || !change.sizeChange())
        return;

    QQuickItemPrivate *p = QQuickItemPrivate::get(item);
    // Only set hasBackgroundWidth/Height if it was a width/height change,
    // otherwise we're prevented from setting a width/height in the future.
    if (change.widthChange())
        extra.value().hasBackgroundWidth = p->widthValid;
    if (change.heightChange())
        extra.value().hasBackgroundHeight = p->heightValid;
    resizeBackground();
}

void QQuickControlPrivate::itemDestroyed(QQuickItem *item)
{
    Q_Q(QQuickControl);
    if (item == background) {
        background = nullptr;
        emit q->implicitBackgroundWidthChanged();
        emit q->implicitBackgroundHeightChanged();
    } else if (item == contentItem) {
        contentItem = nullptr;
        updateImplicitContentSize();
    }
}

QQuickControl::QQuickControl(QQuickItem *parent)
    : QQuickItem(*(new QQuickControlPrivate), parent)
{
    Q_D(QQuickControl);
    d->init();
}

QQuickControl::QQuickControl(QQuickControlPrivate &dd, QQuickItem *parent)
    : QQuickItem(dd, parent)
{
    Q_D(QQuickControl);
    d->init();
}

QQuickControl::~QQuickControl()
{
    Q_D(QQuickControl);
    d->removeImplicitSizeListener(d->background, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
    d->removeImplicitSizeListener(d->contentItem);
}

void QQuickControl::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    Q_D(QQuickControl);
    QQuickItem::itemChange(change, value);
    switch (change) {
    case ItemEnabledHasChanged:
        emit paletteChanged();
        enabledChange();
        break;
    case ItemVisibleHasChanged:
#if QT_CONFIG(quicktemplates2_hover)
        if (!value.boolValue)
            setHovered(false);
#endif
        break;
    case ItemSceneChange:
    case ItemParentHasChanged:
        if ((change == ItemParentHasChanged && value.item) || (change == ItemSceneChange && value.window)) {
            d->resolveFont();
            d->resolvePalette();
            if (!d->hasLocale)
                d->updateLocale(QQuickControlPrivate::calcLocale(d->parentItem), false); // explicit=false
#if QT_CONFIG(quicktemplates2_hover)
            if (!d->explicitHoverEnabled)
                d->updateHoverEnabled(QQuickControlPrivate::calcHoverEnabled(d->parentItem), false); // explicit=false
#endif
        }
        break;
    case ItemActiveFocusHasChanged:
        if (isKeyFocusReason(d->focusReason))
            emit visualFocusChanged();
        break;
    default:
        break;
    }
}

/*!
    \qmlproperty font QtQuick.Controls::Control::font

    This property holds the font currently set for the control.

    This property describes the control's requested font. The font is used by the control's
    style when rendering standard components, and is available as a means to ensure that custom
    controls can maintain consistency with the native platform's native look and feel. It's common
    that different platforms, or different styles, define different fonts for an application.

    The default font depends on the system environment. ApplicationWindow maintains a system/theme
    font which serves as a default for all controls. There may also be special font defaults for
    certain types of controls. You can also set the default font for controls by either:

    \list
    \li passing a custom font to QGuiApplication::setFont(), before loading the QML; or
    \li specifying the fonts in the \l {Qt Quick Controls 2 Configuration File}{qtquickcontrols2.conf file}.
    \endlist

    Finally, the font is matched against Qt's font database to find the best match.

    Control propagates explicit font properties from parent to children. If you change a specific
    property on a control's font, that property propagates to all of the control's children,
    overriding any system defaults for that property.

    \code
    Page {
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

    For the full list of available font properties, see the
    \l [QtQuick]{font}{font QML Basic Type} documentation.
*/
QFont QQuickControl::font() const
{
    Q_D(const QQuickControl);
    return d->resolvedFont;
}

void QQuickControl::setFont(const QFont &font)
{
    Q_D(QQuickControl);
    if (d->extra.value().requestedFont.resolve() == font.resolve() && d->extra.value().requestedFont == font)
        return;

    d->extra.value().requestedFont = font;
    d->resolveFont();
}

void QQuickControl::resetFont()
{
    setFont(QFont());
}

/*!
    \qmlproperty real QtQuick.Controls::Control::availableWidth
    \readonly

    This property holds the width available to the \l contentItem after
    deducting horizontal padding from the \l {Item::}{width} of the control.

    \sa {Control Layout}, padding, leftPadding, rightPadding
*/
qreal QQuickControl::availableWidth() const
{
    return qMax<qreal>(0.0, width() - leftPadding() - rightPadding());
}

/*!
    \qmlproperty real QtQuick.Controls::Control::availableHeight
    \readonly

    This property holds the height available to the \l contentItem after
    deducting vertical padding from the \l {Item::}{height} of the control.

    \sa {Control Layout}, padding, topPadding, bottomPadding
*/
qreal QQuickControl::availableHeight() const
{
    return qMax<qreal>(0.0, height() - topPadding() - bottomPadding());
}

/*!
    \qmlproperty real QtQuick.Controls::Control::padding

    This property holds the default padding.

    Padding adds a space between each edge of the content item and the
    background item, effectively controlling the size of the content item. To
    specify a padding value for a specific edge of the control, set its
    relevant property:

    \list
    \li \l {Control::}{leftPadding}
    \li \l {Control::}{rightPadding}
    \li \l {Control::}{topPadding}
    \li \l {Control::}{bottomPadding}
    \endlist

    \note Different styles may specify the default padding for certain controls
    in different ways, and these ways may change over time as the design
    guidelines that the style is based on evolve. To ensure that these changes
    don't affect the padding values you have specified, it is best to use the
    most specific properties available. For example, rather than setting
    the \l padding property:

    \code
    padding: 0
    \endcode

    set each specific property instead:

    \code
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    \endcode

    \sa {Control Layout}, availableWidth, availableHeight, topPadding, leftPadding, rightPadding, bottomPadding
*/
qreal QQuickControl::padding() const
{
    Q_D(const QQuickControl);
    return d->padding;
}

void QQuickControl::setPadding(qreal padding)
{
    Q_D(QQuickControl);
    if (qFuzzyCompare(d->padding, padding))
        return;

    const QMarginsF oldPadding = d->getPadding();
    const qreal oldVerticalPadding = d->getVerticalPadding();
    const qreal oldHorizontalPadding = d->getHorizontalPadding();

    d->padding = padding;
    emit paddingChanged();

    const QMarginsF newPadding = d->getPadding();
    const qreal newVerticalPadding = d->getVerticalPadding();
    const qreal newHorizontalPadding = d->getHorizontalPadding();

    if (!qFuzzyCompare(newPadding.top(), oldPadding.top()))
        emit topPaddingChanged();
    if (!qFuzzyCompare(newPadding.left(), oldPadding.left()))
        emit leftPaddingChanged();
    if (!qFuzzyCompare(newPadding.right(), oldPadding.right()))
        emit rightPaddingChanged();
    if (!qFuzzyCompare(newPadding.bottom(), oldPadding.bottom()))
        emit bottomPaddingChanged();
    if (!qFuzzyCompare(newVerticalPadding, oldVerticalPadding))
        emit verticalPaddingChanged();
    if (!qFuzzyCompare(newHorizontalPadding, oldHorizontalPadding))
        emit horizontalPaddingChanged();
    if (!qFuzzyCompare(newPadding.top(), oldPadding.top()) || !qFuzzyCompare(newPadding.bottom(), oldPadding.bottom()))
        emit availableHeightChanged();
    if (!qFuzzyCompare(newPadding.left(), oldPadding.left()) || !qFuzzyCompare(newPadding.right(), oldPadding.right()))
        emit availableWidthChanged();

    paddingChange(newPadding, oldPadding);
}

void QQuickControl::resetPadding()
{
    setPadding(0);
}

/*!
    \qmlproperty real QtQuick.Controls::Control::topPadding

    This property holds the top padding. Unless explicitly set, the value
    is equal to \c verticalPadding.

    \sa {Control Layout}, padding, bottomPadding, verticalPadding, availableHeight
*/
qreal QQuickControl::topPadding() const
{
    Q_D(const QQuickControl);
    return d->getTopPadding();
}

void QQuickControl::setTopPadding(qreal padding)
{
    Q_D(QQuickControl);
    d->setTopPadding(padding);
}

void QQuickControl::resetTopPadding()
{
    Q_D(QQuickControl);
    d->setTopPadding(0, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Control::leftPadding

    This property holds the left padding. Unless explicitly set, the value
    is equal to \c horizontalPadding.

    \sa {Control Layout}, padding, rightPadding, horizontalPadding, availableWidth
*/
qreal QQuickControl::leftPadding() const
{
    Q_D(const QQuickControl);
    return d->getLeftPadding();
}

void QQuickControl::setLeftPadding(qreal padding)
{
    Q_D(QQuickControl);
    d->setLeftPadding(padding);
}

void QQuickControl::resetLeftPadding()
{
    Q_D(QQuickControl);
    d->setLeftPadding(0, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Control::rightPadding

    This property holds the right padding. Unless explicitly set, the value
    is equal to \c horizontalPadding.

    \sa {Control Layout}, padding, leftPadding, horizontalPadding, availableWidth
*/
qreal QQuickControl::rightPadding() const
{
    Q_D(const QQuickControl);
    return d->getRightPadding();
}

void QQuickControl::setRightPadding(qreal padding)
{
    Q_D(QQuickControl);
    d->setRightPadding(padding);
}

void QQuickControl::resetRightPadding()
{
    Q_D(QQuickControl);
    d->setRightPadding(0, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Control::bottomPadding

    This property holds the bottom padding. Unless explicitly set, the value
    is equal to \c verticalPadding.

    \sa {Control Layout}, padding, topPadding, verticalPadding, availableHeight
*/
qreal QQuickControl::bottomPadding() const
{
    Q_D(const QQuickControl);
    return d->getBottomPadding();
}

void QQuickControl::setBottomPadding(qreal padding)
{
    Q_D(QQuickControl);
    d->setBottomPadding(padding);
}

void QQuickControl::resetBottomPadding()
{
    Q_D(QQuickControl);
    d->setBottomPadding(0, true);
}

/*!
    \qmlproperty real QtQuick.Controls::Control::spacing

    This property holds the spacing.

    Spacing is useful for controls that have multiple or repetitive building
    blocks. For example, some styles use spacing to determine the distance
    between the text and indicator of \l CheckBox. Spacing is not enforced by
    Control, so each style may interpret it differently, and some may ignore it
    altogether.
*/
qreal QQuickControl::spacing() const
{
    Q_D(const QQuickControl);
    return d->spacing;
}

void QQuickControl::setSpacing(qreal spacing)
{
    Q_D(QQuickControl);
    if (qFuzzyCompare(d->spacing, spacing))
        return;

    qreal oldSpacing = d->spacing;
    d->spacing = spacing;
    emit spacingChanged();
    spacingChange(spacing, oldSpacing);
}

void QQuickControl::resetSpacing()
{
    setSpacing(0);
}

/*!
    \qmlproperty Locale QtQuick.Controls::Control::locale

    This property holds the locale of the control.

    It contains locale specific properties for formatting data and numbers.
    Unless a special locale has been set, this is either the parent's locale
    or the default locale.

    Control propagates the locale from parent to children. If you change the
    control's locale, that locale propagates to all of the control's children,
    overriding the system default locale.

    \sa mirrored, {LayoutMirroring}{LayoutMirroring}
*/
QLocale QQuickControl::locale() const
{
    Q_D(const QQuickControl);
    return d->locale;
}

void QQuickControl::setLocale(const QLocale &locale)
{
    Q_D(QQuickControl);
    if (d->hasLocale && d->locale == locale)
        return;

    d->updateLocale(locale, true); // explicit=true
}

void QQuickControl::resetLocale()
{
    Q_D(QQuickControl);
    if (!d->hasLocale)
        return;

    d->hasLocale = false;
    d->updateLocale(QQuickControlPrivate::calcLocale(d->parentItem), false); // explicit=false
}

/*!
    \qmlproperty bool QtQuick.Controls::Control::mirrored
    \readonly

    This property holds whether the control is mirrored.

    This property is provided for convenience. A control is considered mirrored
    when its visual layout direction is right-to-left; that is, when using a
    right-to-left locale or when \l {LayoutMirroring::enabled}{LayoutMirroring.enabled}
    is \c true.

    \sa locale, {LayoutMirroring}{LayoutMirroring}, {Right-to-left User Interfaces}
*/
bool QQuickControl::isMirrored() const
{
    Q_D(const QQuickControl);
    return d->isMirrored() || d->locale.textDirection() == Qt::RightToLeft;
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Control::focusPolicy

    This property determines the way the control accepts focus.

    \value Qt.TabFocus    The control accepts focus by tabbing.
    \value Qt.ClickFocus  The control accepts focus by clicking.
    \value Qt.StrongFocus The control accepts focus by both tabbing and clicking.
    \value Qt.WheelFocus  The control accepts focus by tabbing, clicking, and using the mouse wheel.
    \value Qt.NoFocus     The control does not accept focus.
*/
Qt::FocusPolicy QQuickControl::focusPolicy() const
{
    Q_D(const QQuickControl);
    uint policy = d->focusPolicy;
    if (activeFocusOnTab())
        policy |= Qt::TabFocus;
    return static_cast<Qt::FocusPolicy>(policy);
}

void QQuickControl::setFocusPolicy(Qt::FocusPolicy policy)
{
    Q_D(QQuickControl);
    if (d->focusPolicy == policy)
        return;

    d->focusPolicy = policy;
    setActiveFocusOnTab(policy & Qt::TabFocus);
    emit focusPolicyChanged();
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Control::focusReason
    \readonly

    \include qquickcontrol-focusreason.qdocinc

    \sa visualFocus
*/
Qt::FocusReason QQuickControl::focusReason() const
{
    Q_D(const QQuickControl);
    return d->focusReason;
}

void QQuickControl::setFocusReason(Qt::FocusReason reason)
{
    Q_D(QQuickControl);
    if (d->focusReason == reason)
        return;

    Qt::FocusReason oldReason = d->focusReason;
    d->focusReason = reason;
    emit focusReasonChanged();
    if (isKeyFocusReason(oldReason) != isKeyFocusReason(reason))
        emit visualFocusChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::Control::visualFocus
    \readonly

    This property holds whether the control has visual focus. This property
    is \c true when the control has active focus and the focus reason is either
    \c Qt.TabFocusReason, \c Qt.BacktabFocusReason, or \c Qt.ShortcutFocusReason.

    In general, for visualizing key focus, this property is preferred over
    \l Item::activeFocus. This ensures that key focus is only visualized when
    interacting with keys - not when interacting via touch or mouse.

    \sa focusReason, Item::activeFocus
*/
bool QQuickControl::hasVisualFocus() const
{
    Q_D(const QQuickControl);
    return d->activeFocus && isKeyFocusReason(d->focusReason);
}

/*!
    \qmlproperty bool QtQuick.Controls::Control::hovered
    \readonly

    This property holds whether the control is hovered.

    \sa hoverEnabled
*/
bool QQuickControl::isHovered() const
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(const QQuickControl);
    return d->hovered;
#else
    return false;
#endif
}

void QQuickControl::setHovered(bool hovered)
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(QQuickControl);
    if (hovered == d->hovered)
        return;

    d->hovered = hovered;
    emit hoveredChanged();
    hoverChange();
#else
    Q_UNUSED(hovered);
#endif
}

/*!
    \qmlproperty bool QtQuick.Controls::Control::hoverEnabled

    This property determines whether the control accepts hover events. The default value
    is \c Qt.styleHints.useHoverEffects.

    Setting this property propagates the value to all child controls that do not have
    \c hoverEnabled explicitly set.

    You can also enable or disable hover effects for all Qt Quick Controls 2 applications
    by setting the \c QT_QUICK_CONTROLS_HOVER_ENABLED \l {Supported Environment Variables
    in Qt Quick Controls 2}{environment variable}.

    \sa hovered
*/
bool QQuickControl::isHoverEnabled() const
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(const QQuickControl);
    return d->hoverEnabled;
#else
    return false;
#endif
}

void QQuickControl::setHoverEnabled(bool enabled)
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(QQuickControl);
    if (d->explicitHoverEnabled && enabled == d->hoverEnabled)
        return;

    d->updateHoverEnabled(enabled, true); // explicit=true
#else
    Q_UNUSED(enabled)
#endif
}

void QQuickControl::resetHoverEnabled()
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(QQuickControl);
    if (!d->explicitHoverEnabled)
        return;

    d->explicitHoverEnabled = false;
    d->updateHoverEnabled(QQuickControlPrivate::calcHoverEnabled(d->parentItem), false); // explicit=false
#endif
}

/*!
    \qmlproperty bool QtQuick.Controls::Control::wheelEnabled

    This property determines whether the control handles wheel events. The default value is \c false.

    \note Care must be taken when enabling wheel events for controls within scrollable items such
    as \l Flickable, as the control will consume the events and hence interrupt scrolling of the
    Flickable.
*/
bool QQuickControl::isWheelEnabled() const
{
    Q_D(const QQuickControl);
    return d->wheelEnabled;
}

void QQuickControl::setWheelEnabled(bool enabled)
{
    Q_D(QQuickControl);
    if (d->wheelEnabled == enabled)
        return;

    d->wheelEnabled = enabled;
    emit wheelEnabledChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::Control::background

    This property holds the background item.

    \code
    Button {
        id: control
        text: qsTr("Button")
        background: Rectangle {
            implicitWidth: 100
            implicitHeight: 40
            opacity: enabled ? 1 : 0.3
            color: control.down ? "#d0d0d0" : "#e0e0e0"
        }
    }
    \endcode

    \input qquickcontrol-background.qdocinc notes

    \sa {Control Layout}
*/
QQuickItem *QQuickControl::background() const
{
    QQuickControlPrivate *d = const_cast<QQuickControlPrivate *>(d_func());
    if (!d->background)
        d->executeBackground();
    return d->background;
}

void QQuickControl::setBackground(QQuickItem *background)
{
    Q_D(QQuickControl);
    if (d->background == background)
        return;

    if (!d->background.isExecuting())
        d->cancelBackground();

    const qreal oldImplicitBackgroundWidth = implicitBackgroundWidth();
    const qreal oldImplicitBackgroundHeight = implicitBackgroundHeight();

    if (d->extra.isAllocated()) {
        d->extra.value().hasBackgroundWidth = false;
        d->extra.value().hasBackgroundHeight = false;
    }

    d->removeImplicitSizeListener(d->background, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
    delete d->background;
    d->background = background;

    if (background) {
        background->setParentItem(this);
        if (qFuzzyIsNull(background->z()))
            background->setZ(-1);
        QQuickItemPrivate *p = QQuickItemPrivate::get(background);
        if (p->widthValid || p->heightValid) {
            d->extra.value().hasBackgroundWidth = p->widthValid;
            d->extra.value().hasBackgroundHeight = p->heightValid;
        }
        if (isComponentComplete())
            d->resizeBackground();
        d->addImplicitSizeListener(background, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
    }

    if (!qFuzzyCompare(oldImplicitBackgroundWidth, implicitBackgroundWidth()))
        emit implicitBackgroundWidthChanged();
    if (!qFuzzyCompare(oldImplicitBackgroundHeight, implicitBackgroundHeight()))
        emit implicitBackgroundHeightChanged();
    if (!d->background.isExecuting())
        emit backgroundChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::Control::contentItem

    This property holds the visual content item.

    \code
    Button {
        id: control
        text: qsTr("Button")
        contentItem: Label {
            text: control.text
            font: control.font
            verticalAlignment: Text.AlignVCenter
        }
    }
    \endcode

    \note The content item is automatically positioned and resized to fit
    within the \l padding of the control. Bindings to the \l x, \l y, \l width,
    and \l height properties of the contentItem are not respected.

    \note Most controls use the implicit size of the content item to calculate
    the implicit size of the control itself. If you replace the content item
    with a custom one, you should also consider providing a sensible implicit
    size for it (unless it is an item like \l Text which has its own implicit
    size).

    \sa {Control Layout}, padding
*/
QQuickItem *QQuickControl::contentItem() const
{
    QQuickControlPrivate *d = const_cast<QQuickControlPrivate *>(d_func());
    if (!d->contentItem)
        d->setContentItem_helper(d->getContentItem(), false);
    return d->contentItem;
}

void QQuickControl::setContentItem(QQuickItem *item)
{
    Q_D(QQuickControl);
    d->setContentItem_helper(item, true);
}

qreal QQuickControl::baselineOffset() const
{
    Q_D(const QQuickControl);
    return d->baselineOffset;
}

void QQuickControl::setBaselineOffset(qreal offset)
{
    Q_D(QQuickControl);
    d->extra.value().hasBaselineOffset = true;
    QQuickItem::setBaselineOffset(offset);
}

void QQuickControl::resetBaselineOffset()
{
    Q_D(QQuickControl);
    if (!d->extra.isAllocated() || !d->extra.value().hasBaselineOffset)
        return;

    if (d->extra.isAllocated())
        d->extra.value().hasBaselineOffset = false;
    d->updateBaselineOffset();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty palette QtQuick.Controls::Control::palette

    This property holds the palette currently set for the control.

    This property describes the control's requested palette. The palette is used by the control's
    style when rendering standard components, and is available as a means to ensure that custom
    controls can maintain consistency with the native platform's native look and feel. It's common
    that different platforms, or different styles, define different palettes for an application.

    The default palette depends on the system environment. ApplicationWindow maintains a system/theme
    palette which serves as a default for all controls. There may also be special palette defaults for
    certain types of controls. You can also set the default palette for controls by either:

    \list
    \li passing a custom palette to QGuiApplication::setPalette(), before loading any QML; or
    \li specifying the colors in the \l {Qt Quick Controls 2 Configuration File}{qtquickcontrols2.conf file}.
    \endlist

    Control propagates explicit palette properties from parent to children. If you change a specific
    property on a control's palette, that property propagates to all of the control's children,
    overriding any system defaults for that property.

    \code
    Page {
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

    For the full list of available palette colors, see the
    \l {qtquickcontrols2-palette}{palette QML Basic Type} documentation.

    \sa ApplicationWindow::palette, Popup::palette
*/
QPalette QQuickControl::palette() const
{
    Q_D(const QQuickControl);
    QPalette palette = d->resolvedPalette;
    if (!isEnabled())
        palette.setCurrentColorGroup(QPalette::Disabled);
    return palette;
}

void QQuickControl::setPalette(const QPalette &palette)
{
    Q_D(QQuickControl);
    if (d->extra.value().requestedPalette.resolve() == palette.resolve() && d->extra.value().requestedPalette == palette)
        return;

    d->extra.value().requestedPalette = palette;
    d->resolvePalette();
}

void QQuickControl::resetPalette()
{
    setPalette(QPalette());
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::horizontalPadding

    This property holds the horizontal padding. Unless explicitly set, the value
    is equal to \c padding.

    \sa {Control Layout}, padding, leftPadding, rightPadding, verticalPadding
*/
qreal QQuickControl::horizontalPadding() const
{
    Q_D(const QQuickControl);
    return d->getHorizontalPadding();
}

void QQuickControl::setHorizontalPadding(qreal padding)
{
    Q_D(QQuickControl);
    d->setHorizontalPadding(padding);
}

void QQuickControl::resetHorizontalPadding()
{
    Q_D(QQuickControl);
    d->setHorizontalPadding(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::verticalPadding

    This property holds the vertical padding. Unless explicitly set, the value
    is equal to \c padding.

    \sa {Control Layout}, padding, topPadding, bottomPadding, horizontalPadding
*/
qreal QQuickControl::verticalPadding() const
{
    Q_D(const QQuickControl);
    return d->getVerticalPadding();
}

void QQuickControl::setVerticalPadding(qreal padding)
{
    Q_D(QQuickControl);
    d->setVerticalPadding(padding);
}

void QQuickControl::resetVerticalPadding()
{
    Q_D(QQuickControl);
    d->setVerticalPadding(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::implicitContentWidth
    \readonly

    This property holds the implicit content width.

    For basic controls, the value is equal to \c {contentItem ? contentItem.implicitWidth : 0}.
    For types that inherit Container or Pane, the value is calculated based on the content children.

    This is typically used, together with \l implicitBackgroundWidth, to calculate
    the \l {Item::}{implicitWidth}:

    \code
    Control {
        implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
    }
    \endcode

    \sa implicitContentHeight, implicitBackgroundWidth
*/
qreal QQuickControl::implicitContentWidth() const
{
    Q_D(const QQuickControl);
    return d->implicitContentWidth;
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::implicitContentHeight
    \readonly

    This property holds the implicit content height.

    For basic controls, the value is equal to \c {contentItem ? contentItem.implicitHeight : 0}.
    For types that inherit Container or Pane, the value is calculated based on the content children.

    This is typically used, together with \l implicitBackgroundHeight, to calculate
    the \l {Item::}{implicitHeight}:

    \code
    Control {
        implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 implicitContentHeight + topPadding + bottomPadding)
    }
    \endcode

    \sa implicitContentWidth, implicitBackgroundHeight
*/
qreal QQuickControl::implicitContentHeight() const
{
    Q_D(const QQuickControl);
    return d->implicitContentHeight;
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::implicitBackgroundWidth
    \readonly

    This property holds the implicit background width.

    The value is equal to \c {background ? background.implicitWidth : 0}.

    This is typically used, together with \l implicitContentWidth, to calculate
    the \l {Item::}{implicitWidth}:

    \code
    Control {
        implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                                implicitContentWidth + leftPadding + rightPadding)
    }
    \endcode

    \sa implicitBackgroundHeight, implicitContentWidth
*/
qreal QQuickControl::implicitBackgroundWidth() const
{
    Q_D(const QQuickControl);
    if (!d->background)
        return 0;
    return d->background->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::implicitBackgroundHeight
    \readonly

    This property holds the implicit background height.

    The value is equal to \c {background ? background.implicitHeight : 0}.

    This is typically used, together with \l implicitContentHeight, to calculate
    the \l {Item::}{implicitHeight}:

    \code
    Control {
        implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 implicitContentHeight + topPadding + bottomPadding)
    }
    \endcode

    \sa implicitBackgroundWidth, implicitContentHeight
*/
qreal QQuickControl::implicitBackgroundHeight() const
{
    Q_D(const QQuickControl);
    if (!d->background)
        return 0;
    return d->background->implicitHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::topInset

    This property holds the top inset for the background.

    \sa {Control Layout}, bottomInset
*/
qreal QQuickControl::topInset() const
{
    Q_D(const QQuickControl);
    return d->getTopInset();
}

void QQuickControl::setTopInset(qreal inset)
{
    Q_D(QQuickControl);
    d->setTopInset(inset);
}

void QQuickControl::resetTopInset()
{
    Q_D(QQuickControl);
    d->setTopInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::leftInset

    This property holds the left inset for the background.

    \sa {Control Layout}, rightInset
*/
qreal QQuickControl::leftInset() const
{
    Q_D(const QQuickControl);
    return d->getLeftInset();
}

void QQuickControl::setLeftInset(qreal inset)
{
    Q_D(QQuickControl);
    d->setLeftInset(inset);
}

void QQuickControl::resetLeftInset()
{
    Q_D(QQuickControl);
    d->setLeftInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::rightInset

    This property holds the right inset for the background.

    \sa {Control Layout}, leftInset
*/
qreal QQuickControl::rightInset() const
{
    Q_D(const QQuickControl);
    return d->getRightInset();
}

void QQuickControl::setRightInset(qreal inset)
{
    Q_D(QQuickControl);
    d->setRightInset(inset);
}

void QQuickControl::resetRightInset()
{
    Q_D(QQuickControl);
    d->setRightInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Control::bottomInset

    This property holds the bottom inset for the background.

    \sa {Control Layout}, topInset
*/
qreal QQuickControl::bottomInset() const
{
    Q_D(const QQuickControl);
    return d->getBottomInset();
}

void QQuickControl::setBottomInset(qreal inset)
{
    Q_D(QQuickControl);
    d->setBottomInset(inset);
}

void QQuickControl::resetBottomInset()
{
    Q_D(QQuickControl);
    d->setBottomInset(0, true);
}

void QQuickControl::classBegin()
{
    Q_D(QQuickControl);
    QQuickItem::classBegin();
    d->resolveFont();
    d->resolvePalette();
}

void QQuickControl::componentComplete()
{
    Q_D(QQuickControl);
    d->executeBackground(true);
    d->executeContentItem(true);
    QQuickItem::componentComplete();
    d->resizeBackground();
    d->resizeContent();
    d->updateBaselineOffset();
    if (!d->hasLocale)
        d->locale = QQuickControlPrivate::calcLocale(d->parentItem);
#if QT_CONFIG(quicktemplates2_hover)
    if (!d->explicitHoverEnabled)
        setAcceptHoverEvents(QQuickControlPrivate::calcHoverEnabled(d->parentItem));
#endif
#if QT_CONFIG(accessibility)
    if (QAccessible::isActive())
        accessibilityActiveChanged(true);
#endif
}

QFont QQuickControl::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::System);
}

QPalette QQuickControl::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::System);
}

void QQuickControl::focusInEvent(QFocusEvent *event)
{
    QQuickItem::focusInEvent(event);
    setFocusReason(event->reason());
}

void QQuickControl::focusOutEvent(QFocusEvent *event)
{
    QQuickItem::focusOutEvent(event);
    setFocusReason(event->reason());
}

#if QT_CONFIG(quicktemplates2_hover)
void QQuickControl::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(QQuickControl);
    setHovered(d->hoverEnabled);
    event->setAccepted(d->hoverEnabled);
}

void QQuickControl::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(QQuickControl);
    setHovered(d->hoverEnabled && contains(event->pos()));
    event->setAccepted(d->hoverEnabled);
}

void QQuickControl::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QQuickControl);
    setHovered(false);
    event->setAccepted(d->hoverEnabled);
}
#endif

void QQuickControl::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickControl);
    d->handlePress(event->localPos());
    event->accept();
}

void QQuickControl::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QQuickControl);
    d->handleMove(event->localPos());
    event->accept();
}

void QQuickControl::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QQuickControl);
    d->handleRelease(event->localPos());
    event->accept();
}

void QQuickControl::mouseUngrabEvent()
{
    Q_D(QQuickControl);
    d->handleUngrab();
}

#if QT_CONFIG(quicktemplates2_multitouch)
void QQuickControl::touchEvent(QTouchEvent *event)
{
    Q_D(QQuickControl);
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        for (const QTouchEvent::TouchPoint &point : event->touchPoints()) {
            if (!d->acceptTouch(point))
                continue;

            switch (point.state()) {
            case Qt::TouchPointPressed:
                d->handlePress(point.pos());
                break;
            case Qt::TouchPointMoved:
                d->handleMove(point.pos());
                break;
            case Qt::TouchPointReleased:
                d->handleRelease(point.pos());
                break;
            default:
                break;
            }
        }
        break;

    case QEvent::TouchCancel:
        d->handleUngrab();
        break;

    default:
        QQuickItem::touchEvent(event);
        break;
    }
}

void QQuickControl::touchUngrabEvent()
{
    Q_D(QQuickControl);
    d->handleUngrab();
}
#endif

#if QT_CONFIG(wheelevent)
void QQuickControl::wheelEvent(QWheelEvent *event)
{
    Q_D(QQuickControl);
    if ((d->focusPolicy & Qt::WheelFocus) == Qt::WheelFocus)
        setActiveFocus(this, Qt::MouseFocusReason);

    event->setAccepted(d->wheelEnabled);
}
#endif

void QQuickControl::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickControl);
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->resizeBackground();
    d->resizeContent();
    if (!qFuzzyCompare(newGeometry.width(), oldGeometry.width()))
        emit availableWidthChanged();
    if (!qFuzzyCompare(newGeometry.height(), oldGeometry.height()))
        emit availableHeightChanged();
}

void QQuickControl::enabledChange()
{
}

void QQuickControl::fontChange(const QFont &newFont, const QFont &oldFont)
{
    Q_UNUSED(newFont);
    Q_UNUSED(oldFont);
}

#if QT_CONFIG(quicktemplates2_hover)
void QQuickControl::hoverChange()
{
}
#endif

void QQuickControl::mirrorChange()
{
    emit mirroredChanged();
}

void QQuickControl::spacingChange(qreal newSpacing, qreal oldSpacing)
{
    Q_UNUSED(newSpacing);
    Q_UNUSED(oldSpacing);
}

void QQuickControl::paddingChange(const QMarginsF &newPadding, const QMarginsF &oldPadding)
{
    Q_D(QQuickControl);
    Q_UNUSED(newPadding);
    Q_UNUSED(oldPadding);
    d->resizeContent();
    d->updateBaselineOffset();
}

void QQuickControl::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_UNUSED(newItem);
    Q_UNUSED(oldItem);
}

void QQuickControl::localeChange(const QLocale &newLocale, const QLocale &oldLocale)
{
    Q_UNUSED(newLocale);
    Q_UNUSED(oldLocale);
}

void QQuickControl::paletteChange(const QPalette &newPalette, const QPalette &oldPalette)
{
    Q_UNUSED(newPalette);
    Q_UNUSED(oldPalette);
}

void QQuickControl::insetChange(const QMarginsF &newInset, const QMarginsF &oldInset)
{
    Q_D(QQuickControl);
    Q_UNUSED(newInset);
    Q_UNUSED(oldInset);
    d->resizeBackground();
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickControl::accessibleRole() const
{
    return QAccessible::NoRole;
}

void QQuickControl::accessibilityActiveChanged(bool active)
{
    if (!active)
        return;

    QQuickAccessibleAttached *accessibleAttached = qobject_cast<QQuickAccessibleAttached *>(qmlAttachedPropertiesObject<QQuickAccessibleAttached>(this, true));
    Q_ASSERT(accessibleAttached);
    accessibleAttached->setRole(accessibleRole());
}
#endif

QString QQuickControl::accessibleName() const
{
#if QT_CONFIG(accessibility)
    if (QQuickAccessibleAttached *accessibleAttached = QQuickControlPrivate::accessibleAttached(this))
        return accessibleAttached->name();
#endif
    return QString();
}

void QQuickControl::setAccessibleName(const QString &name)
{
#if QT_CONFIG(accessibility)
    if (QQuickAccessibleAttached *accessibleAttached = QQuickControlPrivate::accessibleAttached(this))
        accessibleAttached->setName(name);
#else
    Q_UNUSED(name)
#endif
}

QVariant QQuickControl::accessibleProperty(const char *propertyName)
{
#if QT_CONFIG(accessibility)
    if (QAccessible::isActive())
        return QQuickAccessibleAttached::property(this, propertyName);
#endif
    Q_UNUSED(propertyName)
    return QVariant();
}

bool QQuickControl::setAccessibleProperty(const char *propertyName, const QVariant &value)
{
#if QT_CONFIG(accessibility)
    if (QAccessible::isActive())
        return QQuickAccessibleAttached::setProperty(this, propertyName, value);
#endif
    Q_UNUSED(propertyName)
    Q_UNUSED(value)
    return false;
}

QT_END_NAMESPACE
