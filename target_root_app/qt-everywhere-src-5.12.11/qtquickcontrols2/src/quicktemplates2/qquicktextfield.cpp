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

#include "qquicktextfield_p.h"
#include "qquicktextfield_p_p.h"
#include "qquickcontrol_p.h"
#include "qquickcontrol_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquicktextinput_p.h>
#include <QtQuick/private/qquickclipnode_p.h>

#if QT_CONFIG(accessibility)
#include <QtQuick/private/qquickaccessibleattached_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype TextField
    \inherits TextInput
    \instantiates QQuickTextField
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup qtquickcontrols2-input
    \brief Single-line text input field.

    TextField is a single line text editor. TextField extends TextInput with
    a \l {placeholderText}{placeholder text} functionality, and adds decoration.

    \table
    \row \li \image qtquickcontrols2-textfield-normal.png
         \li A text field in its normal state.
    \row \li \image qtquickcontrols2-textfield-focused.png
         \li A text field that has active focus.
    \row \li \image qtquickcontrols2-textfield-disabled.png
         \li A text field that is disabled.
    \endtable

    \code
    TextField {
        placeholderText: qsTr("Enter name")
    }
    \endcode

    \sa TextArea, {Customizing TextField}, {Input Controls}
*/

/*!
    \qmlsignal QtQuick.Controls::TextField::pressAndHold(MouseEvent event)

    This signal is emitted when there is a long press (the delay depends on the platform plugin).
    The \l {MouseEvent}{event} parameter provides information about the press, including the x and y
    position of the press, and which button is pressed.

    \sa pressed, released
*/

/*!
    \qmlsignal QtQuick.Controls::TextField::pressed(MouseEvent event)
    \since QtQuick.Controls 2.1 (Qt 5.8)

    This signal is emitted when the text field is pressed by the user.
    The \l {MouseEvent}{event} parameter provides information about the press,
    including the x and y position of the press, and which button is pressed.

    \sa released, pressAndHold
*/

/*!
    \qmlsignal QtQuick.Controls::TextField::released(MouseEvent event)
    \since QtQuick.Controls 2.1 (Qt 5.8)

    This signal is emitted when the text field is released by the user.
    The \l {MouseEvent}{event} parameter provides information about the release,
    including the x and y position of the press, and which button is pressed.

    \sa pressed, pressAndHold
*/

QQuickTextFieldPrivate::QQuickTextFieldPrivate()
{
#if QT_CONFIG(accessibility)
    QAccessible::installActivationObserver(this);
#endif
}

QQuickTextFieldPrivate::~QQuickTextFieldPrivate()
{
#if QT_CONFIG(accessibility)
    QAccessible::removeActivationObserver(this);
#endif
}

void QQuickTextFieldPrivate::setTopInset(qreal value, bool reset)
{
    Q_Q(QQuickTextField);
    const QMarginsF oldInset = getInset();
    extra.value().topInset = value;
    extra.value().hasTopInset = !reset;
    if (!qFuzzyCompare(oldInset.top(), value)) {
        emit q->topInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickTextFieldPrivate::setLeftInset(qreal value, bool reset)
{
    Q_Q(QQuickTextField);
    const QMarginsF oldInset = getInset();
    extra.value().leftInset = value;
    extra.value().hasLeftInset = !reset;
    if (!qFuzzyCompare(oldInset.left(), value)) {
        emit q->leftInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickTextFieldPrivate::setRightInset(qreal value, bool reset)
{
    Q_Q(QQuickTextField);
    const QMarginsF oldInset = getInset();
    extra.value().rightInset = value;
    extra.value().hasRightInset = !reset;
    if (!qFuzzyCompare(oldInset.right(), value)) {
        emit q->rightInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickTextFieldPrivate::setBottomInset(qreal value, bool reset)
{
    Q_Q(QQuickTextField);
    const QMarginsF oldInset = getInset();
    extra.value().bottomInset = value;
    extra.value().hasBottomInset = !reset;
    if (!qFuzzyCompare(oldInset.bottom(), value)) {
        emit q->bottomInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickTextFieldPrivate::resizeBackground()
{
    if (!background)
        return;

    resizingBackground = true;

    QQuickItemPrivate *p = QQuickItemPrivate::get(background);
    if (((!p->widthValid || !extra.isAllocated() || !extra->hasBackgroundWidth) && qFuzzyIsNull(background->x()))
            || (extra.isAllocated() && (extra->hasLeftInset || extra->hasRightInset))) {
        const bool wasWidthValid = p->widthValid;
        background->setX(getLeftInset());
        background->setWidth(width - getLeftInset() - getRightInset());
        // If the user hadn't previously set the width, that shouldn't change when we set it for them.
        if (!wasWidthValid)
            p->widthValid = false;
    }
    if (((!p->heightValid || !extra.isAllocated() || !extra->hasBackgroundHeight) && qFuzzyIsNull(background->y()))
            || (extra.isAllocated() && (extra->hasTopInset || extra->hasBottomInset))) {
        const bool wasHeightValid = p->heightValid;
        background->setY(getTopInset());
        background->setHeight(height - getTopInset() - getBottomInset());
        if (!wasHeightValid)
            p->heightValid = false;
    }

    resizingBackground = false;
}

/*!
    \internal

    Determine which font is implicitly imposed on this control by its ancestors
    and QGuiApplication::font, resolve this against its own font (attributes from
    the implicit font are copied over). Then propagate this font to this
    control's children.
*/
void QQuickTextFieldPrivate::resolveFont()
{
    Q_Q(QQuickTextField);
    inheritFont(QQuickControlPrivate::parentFont(q));
}

void QQuickTextFieldPrivate::inheritFont(const QFont &font)
{
    QFont parentFont = extra.isAllocated() ? extra->requestedFont.resolve(font) : font;
    parentFont.resolve(extra.isAllocated() ? extra->requestedFont.resolve() | font.resolve() : font.resolve());

    const QFont defaultFont = QQuickTheme::font(QQuickTheme::TextField);
    const QFont resolvedFont = parentFont.resolve(defaultFont);

    setFont_helper(resolvedFont);
}

/*!
    \internal

    Assign \a font to this control, and propagate it to all children.
*/
void QQuickTextFieldPrivate::updateFont(const QFont &font)
{
    Q_Q(QQuickTextField);
    QFont oldFont = sourceFont;
    q->QQuickTextInput::setFont(font);

    QQuickControlPrivate::updateFontRecur(q, font);

    if (oldFont != font)
        emit q->fontChanged();
}

/*!
    \internal

    Determine which palette is implicitly imposed on this control by its ancestors
    and QGuiApplication::palette, resolve this against its own palette (attributes from
    the implicit palette are copied over). Then propagate this palette to this
    control's children.
*/
void QQuickTextFieldPrivate::resolvePalette()
{
    Q_Q(QQuickTextField);
    inheritPalette(QQuickControlPrivate::parentPalette(q));
}

void QQuickTextFieldPrivate::inheritPalette(const QPalette &palette)
{
    QPalette parentPalette = extra.isAllocated() ? extra->requestedPalette.resolve(palette) : palette;
    parentPalette.resolve(extra.isAllocated() ? extra->requestedPalette.resolve() | palette.resolve() : palette.resolve());

    const QPalette defaultPalette = QQuickTheme::palette(QQuickTheme::TextField);
    const QPalette resolvedPalette = parentPalette.resolve(defaultPalette);

    setPalette_helper(resolvedPalette);
}

void QQuickTextFieldPrivate::updatePalette(const QPalette &palette)
{
    Q_Q(QQuickTextField);
    QPalette oldPalette = resolvedPalette;
    resolvedPalette = palette;

    QQuickControlPrivate::updatePaletteRecur(q, palette);

    if (oldPalette != palette)
        emit q->paletteChanged();
}

#if QT_CONFIG(quicktemplates2_hover)
void QQuickTextFieldPrivate::updateHoverEnabled(bool enabled, bool xplicit)
{
    Q_Q(QQuickTextField);
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
#endif

qreal QQuickTextFieldPrivate::getImplicitWidth() const
{
    return QQuickItemPrivate::getImplicitWidth();
}

qreal QQuickTextFieldPrivate::getImplicitHeight() const
{
    return QQuickItemPrivate::getImplicitHeight();
}

void QQuickTextFieldPrivate::implicitWidthChanged()
{
    Q_Q(QQuickTextField);
    QQuickItemPrivate::implicitWidthChanged();
    emit q->implicitWidthChanged3();
}

void QQuickTextFieldPrivate::implicitHeightChanged()
{
    Q_Q(QQuickTextField);
    QQuickItemPrivate::implicitHeightChanged();
    emit q->implicitHeightChanged3();
}

void QQuickTextFieldPrivate::readOnlyChanged(bool isReadOnly)
{
    Q_UNUSED(isReadOnly);
#if QT_CONFIG(accessibility)
    if (QQuickAccessibleAttached *accessibleAttached = QQuickControlPrivate::accessibleAttached(q_func()))
        accessibleAttached->set_readOnly(isReadOnly);
#endif
#if QT_CONFIG(cursor)
    q_func()->setCursor(isReadOnly ? Qt::ArrowCursor : Qt::IBeamCursor);
#endif
}

void QQuickTextFieldPrivate::echoModeChanged(QQuickTextField::EchoMode echoMode)
{
#if QT_CONFIG(accessibility)
    if (QQuickAccessibleAttached *accessibleAttached = QQuickControlPrivate::accessibleAttached(q_func()))
        accessibleAttached->set_passwordEdit((echoMode == QQuickTextField::Password || echoMode == QQuickTextField::PasswordEchoOnEdit) ? true : false);
#else
    Q_UNUSED(echoMode)
#endif
}

#if QT_CONFIG(accessibility)
void QQuickTextFieldPrivate::accessibilityActiveChanged(bool active)
{
    if (!active)
        return;

    Q_Q(QQuickTextField);
    QQuickAccessibleAttached *accessibleAttached = qobject_cast<QQuickAccessibleAttached *>(qmlAttachedPropertiesObject<QQuickAccessibleAttached>(q, true));
    Q_ASSERT(accessibleAttached);
    accessibleAttached->setRole(accessibleRole());
    accessibleAttached->set_readOnly(m_readOnly);
    accessibleAttached->set_passwordEdit((m_echoMode == QQuickTextField::Password || m_echoMode == QQuickTextField::PasswordEchoOnEdit) ? true : false);
    accessibleAttached->setDescription(placeholder);
}

QAccessible::Role QQuickTextFieldPrivate::accessibleRole() const
{
    return QAccessible::EditableText;
}
#endif

static inline QString backgroundName() { return QStringLiteral("background"); }

void QQuickTextFieldPrivate::cancelBackground()
{
    Q_Q(QQuickTextField);
    quickCancelDeferred(q, backgroundName());
}

void QQuickTextFieldPrivate::executeBackground(bool complete)
{
    Q_Q(QQuickTextField);
    if (background.wasExecuted())
        return;

    if (!background || complete)
        quickBeginDeferred(q, backgroundName(), background);
    if (complete)
        quickCompleteDeferred(q, backgroundName(), background);
}

void QQuickTextFieldPrivate::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &diff)
{
    Q_UNUSED(diff);
    if (resizingBackground || item != background || !change.sizeChange())
        return;

    QQuickItemPrivate *p = QQuickItemPrivate::get(item);
    // QTBUG-71875: only allocate the extra data if we have to.
    // resizeBackground() relies on the value of extra.isAllocated()
    // as part of its checks to see whether it should resize the background or not.
    if (p->widthValid || extra.isAllocated())
        extra.value().hasBackgroundWidth = p->widthValid;
    if (p->heightValid || extra.isAllocated())
        extra.value().hasBackgroundHeight = p->heightValid;
    resizeBackground();
}

void QQuickTextFieldPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickTextField);
    if (item == background)
        emit q->implicitBackgroundWidthChanged();
}

void QQuickTextFieldPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickTextField);
    if (item == background)
        emit q->implicitBackgroundHeightChanged();
}

void QQuickTextFieldPrivate::itemDestroyed(QQuickItem *item)
{
    Q_Q(QQuickTextField);
    if (item == background) {
        background = nullptr;
        emit q->implicitBackgroundWidthChanged();
        emit q->implicitBackgroundHeightChanged();
    }
}

QQuickTextField::QQuickTextField(QQuickItem *parent)
    : QQuickTextInput(*(new QQuickTextFieldPrivate), parent)
{
    Q_D(QQuickTextField);
    d->pressHandler.control = this;
    d->setImplicitResizeEnabled(false);
    setAcceptedMouseButtons(Qt::AllButtons);
    setActiveFocusOnTab(true);
#if QT_CONFIG(cursor)
    setCursor(Qt::IBeamCursor);
#endif
    QObjectPrivate::connect(this, &QQuickTextInput::readOnlyChanged, d, &QQuickTextFieldPrivate::readOnlyChanged);
    QObjectPrivate::connect(this, &QQuickTextInput::echoModeChanged, d, &QQuickTextFieldPrivate::echoModeChanged);
}

QQuickTextField::~QQuickTextField()
{
    Q_D(QQuickTextField);
    QQuickControlPrivate::removeImplicitSizeListener(d->background, d, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
}

QFont QQuickTextField::font() const
{
    return QQuickTextInput::font();
}

void QQuickTextField::setFont(const QFont &font)
{
    Q_D(QQuickTextField);
    if (d->extra.value().requestedFont.resolve() == font.resolve() && d->extra.value().requestedFont == font)
        return;

    d->extra.value().requestedFont = font;
    d->resolveFont();
}

/*!
    \qmlproperty Item QtQuick.Controls::TextField::background

    This property holds the background item.

    \input qquickcontrol-background.qdocinc notes

    \sa {Customizing TextField}
*/
QQuickItem *QQuickTextField::background() const
{
    QQuickTextFieldPrivate *d = const_cast<QQuickTextFieldPrivate *>(d_func());
    if (!d->background)
        d->executeBackground();
    return d->background;
}

void QQuickTextField::setBackground(QQuickItem *background)
{
    Q_D(QQuickTextField);
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

    QQuickControlPrivate::removeImplicitSizeListener(d->background, d, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
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
        QQuickControlPrivate::addImplicitSizeListener(background, d, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
    }

    if (!qFuzzyCompare(oldImplicitBackgroundWidth, implicitBackgroundWidth()))
        emit implicitBackgroundWidthChanged();
    if (!qFuzzyCompare(oldImplicitBackgroundHeight, implicitBackgroundHeight()))
        emit implicitBackgroundHeightChanged();
    if (!d->background.isExecuting())
        emit backgroundChanged();
}

/*!
    \qmlproperty string QtQuick.Controls::TextField::placeholderText

    This property holds the hint that is displayed in the TextField before the user
    enters text.
*/
QString QQuickTextField::placeholderText() const
{
    Q_D(const QQuickTextField);
    return d->placeholder;
}

void QQuickTextField::setPlaceholderText(const QString &text)
{
    Q_D(QQuickTextField);
    if (d->placeholder == text)
        return;

    d->placeholder = text;
#if QT_CONFIG(accessibility)
    if (QQuickAccessibleAttached *accessibleAttached = QQuickControlPrivate::accessibleAttached(this))
        accessibleAttached->setDescription(text);
#endif
    emit placeholderTextChanged();
}

/*!
    \qmlproperty color QtQuick.Controls::TextField::placeholderTextColor
    \since QtQuick.Controls 2.5 (Qt 5.12)

    This property holds the color of placeholderText.

    \sa placeholderText
*/
QColor QQuickTextField::placeholderTextColor() const
{
    Q_D(const QQuickTextField);
    return d->placeholderColor;
}

void QQuickTextField::setPlaceholderTextColor(const QColor &color)
{
    Q_D(QQuickTextField);
    if (d->placeholderColor == color)
        return;

    d->placeholderColor = color;
    emit placeholderTextColorChanged();
}

/*!
    \qmlproperty enumeration QtQuick.Controls::TextField::focusReason

    \include qquickcontrol-focusreason.qdocinc
*/
Qt::FocusReason QQuickTextField::focusReason() const
{
    Q_D(const QQuickTextField);
    return d->focusReason;
}

void QQuickTextField::setFocusReason(Qt::FocusReason reason)
{
    Q_D(QQuickTextField);
    if (d->focusReason == reason)
        return;

    d->focusReason = reason;
    emit focusReasonChanged();
}

/*!
    \since QtQuick.Controls 2.1 (Qt 5.8)
    \qmlproperty bool QtQuick.Controls::TextField::hovered
    \readonly

    This property holds whether the text field is hovered.

    \sa hoverEnabled
*/
bool QQuickTextField::isHovered() const
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(const QQuickTextField);
    return d->hovered;
#else
    return false;
#endif
}

void QQuickTextField::setHovered(bool hovered)
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(QQuickTextField);
    if (hovered == d->hovered)
        return;

    d->hovered = hovered;
    emit hoveredChanged();
#else
    Q_UNUSED(hovered);
#endif
}

/*!
    \since QtQuick.Controls 2.1 (Qt 5.8)
    \qmlproperty bool QtQuick.Controls::TextField::hoverEnabled

    This property determines whether the text field accepts hover events. The default value is \c false.

    \sa hovered
*/
bool QQuickTextField::isHoverEnabled() const
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(const QQuickTextField);
    return d->hoverEnabled;
#else
    return false;
#endif
}

void QQuickTextField::setHoverEnabled(bool enabled)
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(QQuickTextField);
    if (d->explicitHoverEnabled && enabled == d->hoverEnabled)
        return;

    d->updateHoverEnabled(enabled, true); // explicit=true
#else
    Q_UNUSED(enabled);
#endif
}

void QQuickTextField::resetHoverEnabled()
{
#if QT_CONFIG(quicktemplates2_hover)
    Q_D(QQuickTextField);
    if (!d->explicitHoverEnabled)
        return;

    d->explicitHoverEnabled = false;
    d->updateHoverEnabled(QQuickControlPrivate::calcHoverEnabled(d->parentItem), false); // explicit=false
#endif
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty palette QtQuick.Controls::TextField::palette

    This property holds the palette currently set for the text field.

    \sa Control::palette
*/
QPalette QQuickTextField::palette() const
{
    Q_D(const QQuickTextField);
    QPalette palette = d->resolvedPalette;
    if (!isEnabled())
        palette.setCurrentColorGroup(QPalette::Disabled);
    return palette;
}

void QQuickTextField::setPalette(const QPalette &palette)
{
    Q_D(QQuickTextField);
    if (d->extra.value().requestedPalette.resolve() == palette.resolve() && d->extra.value().requestedPalette == palette)
        return;

    d->extra.value().requestedPalette = palette;
    d->resolvePalette();
}

void QQuickTextField::resetPalette()
{
    setPalette(QPalette());
}

void QQuickTextField::classBegin()
{
    Q_D(QQuickTextField);
    QQuickTextInput::classBegin();
    d->resolveFont();
    d->resolvePalette();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::TextField::implicitBackgroundWidth
    \readonly

    This property holds the implicit background width.

    The value is equal to \c {background ? background.implicitWidth : 0}.

    \sa implicitBackgroundHeight
*/
qreal QQuickTextField::implicitBackgroundWidth() const
{
    Q_D(const QQuickTextField);
    if (!d->background)
        return 0;
    return d->background->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::TextField::implicitBackgroundHeight
    \readonly

    This property holds the implicit background height.

    The value is equal to \c {background ? background.implicitHeight : 0}.

    \sa implicitBackgroundWidth
*/
qreal QQuickTextField::implicitBackgroundHeight() const
{
    Q_D(const QQuickTextField);
    if (!d->background)
        return 0;
    return d->background->implicitHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::TextField::topInset

    This property holds the top inset for the background.

    \sa {Control Layout}, bottomInset
*/
qreal QQuickTextField::topInset() const
{
    Q_D(const QQuickTextField);
    return d->getTopInset();
}

void QQuickTextField::setTopInset(qreal inset)
{
    Q_D(QQuickTextField);
    d->setTopInset(inset);
}

void QQuickTextField::resetTopInset()
{
    Q_D(QQuickTextField);
    d->setTopInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::TextField::leftInset

    This property holds the left inset for the background.

    \sa {Control Layout}, rightInset
*/
qreal QQuickTextField::leftInset() const
{
    Q_D(const QQuickTextField);
    return d->getLeftInset();
}

void QQuickTextField::setLeftInset(qreal inset)
{
    Q_D(QQuickTextField);
    d->setLeftInset(inset);
}

void QQuickTextField::resetLeftInset()
{
    Q_D(QQuickTextField);
    d->setLeftInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::TextField::rightInset

    This property holds the right inset for the background.

    \sa {Control Layout}, leftInset
*/
qreal QQuickTextField::rightInset() const
{
    Q_D(const QQuickTextField);
    return d->getRightInset();
}

void QQuickTextField::setRightInset(qreal inset)
{
    Q_D(QQuickTextField);
    d->setRightInset(inset);
}

void QQuickTextField::resetRightInset()
{
    Q_D(QQuickTextField);
    d->setRightInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::TextField::bottomInset

    This property holds the bottom inset for the background.

    \sa {Control Layout}, topInset
*/
qreal QQuickTextField::bottomInset() const
{
    Q_D(const QQuickTextField);
    return d->getBottomInset();
}

void QQuickTextField::setBottomInset(qreal inset)
{
    Q_D(QQuickTextField);
    d->setBottomInset(inset);
}

void QQuickTextField::resetBottomInset()
{
    Q_D(QQuickTextField);
    d->setBottomInset(0, true);
}

void QQuickTextField::componentComplete()
{
    Q_D(QQuickTextField);
    d->executeBackground(true);
    QQuickTextInput::componentComplete();
    d->resizeBackground();
#if QT_CONFIG(quicktemplates2_hover)
    if (!d->explicitHoverEnabled)
        setAcceptHoverEvents(QQuickControlPrivate::calcHoverEnabled(d->parentItem));
#endif
#if QT_CONFIG(accessibility)
    if (QAccessible::isActive())
        d->accessibilityActiveChanged(true);
#endif
}

void QQuickTextField::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    Q_D(QQuickTextField);
    QQuickTextInput::itemChange(change, value);
    switch (change) {
    case ItemEnabledHasChanged:
        emit paletteChanged();
        break;
    case ItemSceneChange:
    case ItemParentHasChanged:
        if ((change == ItemParentHasChanged && value.item) || (change == ItemSceneChange && value.window)) {
            d->resolveFont();
            d->resolvePalette();
#if QT_CONFIG(quicktemplates2_hover)
            if (!d->explicitHoverEnabled)
                d->updateHoverEnabled(QQuickControlPrivate::calcHoverEnabled(d->parentItem), false); // explicit=false
#endif
        }
        break;
    default:
        break;
    }
}

void QQuickTextField::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickTextField);
    QQuickTextInput::geometryChanged(newGeometry, oldGeometry);
    d->resizeBackground();
}

void QQuickTextField::insetChange(const QMarginsF &newInset, const QMarginsF &oldInset)
{
    Q_D(QQuickTextField);
    Q_UNUSED(newInset);
    Q_UNUSED(oldInset);
    d->resizeBackground();
}
QSGNode *QQuickTextField::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    QQuickDefaultClipNode *clipNode = static_cast<QQuickDefaultClipNode *>(oldNode);
    if (!clipNode)
        clipNode = new QQuickDefaultClipNode(QRectF());

    clipNode->setRect(clipRect().adjusted(leftPadding(), topPadding(), -rightPadding(), -bottomPadding()));
    clipNode->update();

    QSGNode *textNode = QQuickTextInput::updatePaintNode(clipNode->firstChild(), data);
    if (!textNode->parent())
        clipNode->appendChildNode(textNode);

    return clipNode;
}

void QQuickTextField::focusInEvent(QFocusEvent *event)
{
    QQuickTextInput::focusInEvent(event);
    setFocusReason(event->reason());
}

void QQuickTextField::focusOutEvent(QFocusEvent *event)
{
    QQuickTextInput::focusOutEvent(event);
    setFocusReason(event->reason());
}

#if QT_CONFIG(quicktemplates2_hover)
void QQuickTextField::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(QQuickTextField);
    QQuickTextInput::hoverEnterEvent(event);
    setHovered(d->hoverEnabled);
    event->setAccepted(d->hoverEnabled);
}

void QQuickTextField::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QQuickTextField);
    QQuickTextInput::hoverLeaveEvent(event);
    setHovered(false);
    event->setAccepted(d->hoverEnabled);
}
#endif

void QQuickTextField::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickTextField);
    d->pressHandler.mousePressEvent(event);
    if (d->pressHandler.isActive()) {
        if (d->pressHandler.delayedMousePressEvent) {
            QQuickTextInput::mousePressEvent(d->pressHandler.delayedMousePressEvent);
            d->pressHandler.clearDelayedMouseEvent();
        }
        if (event->buttons() != Qt::RightButton)
            QQuickTextInput::mousePressEvent(event);
    }
}

void QQuickTextField::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QQuickTextField);
    d->pressHandler.mouseMoveEvent(event);
    if (d->pressHandler.isActive()) {
        if (d->pressHandler.delayedMousePressEvent) {
            QQuickTextInput::mousePressEvent(d->pressHandler.delayedMousePressEvent);
            d->pressHandler.clearDelayedMouseEvent();
        }
        if (event->buttons() != Qt::RightButton)
            QQuickTextInput::mouseMoveEvent(event);
    }
}

void QQuickTextField::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QQuickTextField);
    d->pressHandler.mouseReleaseEvent(event);
    if (d->pressHandler.isActive()) {
        if (d->pressHandler.delayedMousePressEvent) {
            QQuickTextInput::mousePressEvent(d->pressHandler.delayedMousePressEvent);
            d->pressHandler.clearDelayedMouseEvent();
        }
        if (event->buttons() != Qt::RightButton)
            QQuickTextInput::mouseReleaseEvent(event);
    }
}

void QQuickTextField::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(QQuickTextField);
    if (d->pressHandler.delayedMousePressEvent) {
        QQuickTextInput::mousePressEvent(d->pressHandler.delayedMousePressEvent);
        d->pressHandler.clearDelayedMouseEvent();
    }
    if (event->buttons() != Qt::RightButton)
        QQuickTextInput::mouseDoubleClickEvent(event);
}

void QQuickTextField::timerEvent(QTimerEvent *event)
{
    Q_D(QQuickTextField);
    if (event->timerId() == d->pressHandler.timer.timerId())
        d->pressHandler.timerEvent(event);
    else
        QQuickTextInput::timerEvent(event);
}

QT_END_NAMESPACE
