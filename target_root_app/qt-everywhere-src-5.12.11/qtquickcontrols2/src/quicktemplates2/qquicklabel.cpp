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

#include "qquicklabel_p.h"
#include "qquicklabel_p_p.h"
#include "qquickcontrol_p.h"
#include "qquickcontrol_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquicktext_p.h>

#if QT_CONFIG(accessibility)
#include <QtQuick/private/qquickaccessibleattached_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype Label
    \inherits Text
    \instantiates QQuickLabel
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup text
    \brief Styled text label with inherited font.

    Label extends \l Text with styling and \l {Control::font}{font}
    inheritance. The default colors and font are style specific. Label
    can also have a visual \l background item.

    \image qtquickcontrols2-label.png

    \snippet qtquickcontrols2-label.qml 1

    You can use the properties of \l Text to change the appearance of the text as desired:

    \qml
     Label {
         text: "Hello world"
         font.pixelSize: 22
         font.italic: true
     }
    \endqml

    \sa {Customizing Label}
*/

QQuickLabelPrivate::QQuickLabelPrivate()
{
#if QT_CONFIG(accessibility)
    QAccessible::installActivationObserver(this);
#endif
}

QQuickLabelPrivate::~QQuickLabelPrivate()
{
#if QT_CONFIG(accessibility)
    QAccessible::removeActivationObserver(this);
#endif
}

void QQuickLabelPrivate::setTopInset(qreal value, bool reset)
{
    Q_Q(QQuickLabel);
    const QMarginsF oldInset = getInset();
    extra.value().topInset = value;
    extra.value().hasTopInset = !reset;
    if (!qFuzzyCompare(oldInset.top(), value)) {
        emit q->topInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickLabelPrivate::setLeftInset(qreal value, bool reset)
{
    Q_Q(QQuickLabel);
    const QMarginsF oldInset = getInset();
    extra.value().leftInset = value;
    extra.value().hasLeftInset = !reset;
    if (!qFuzzyCompare(oldInset.left(), value)) {
        emit q->leftInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickLabelPrivate::setRightInset(qreal value, bool reset)
{
    Q_Q(QQuickLabel);
    const QMarginsF oldInset = getInset();
    extra.value().rightInset = value;
    extra.value().hasRightInset = !reset;
    if (!qFuzzyCompare(oldInset.right(), value)) {
        emit q->rightInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickLabelPrivate::setBottomInset(qreal value, bool reset)
{
    Q_Q(QQuickLabel);
    const QMarginsF oldInset = getInset();
    extra.value().bottomInset = value;
    extra.value().hasBottomInset = !reset;
    if (!qFuzzyCompare(oldInset.bottom(), value)) {
        emit q->bottomInsetChanged();
        q->insetChange(getInset(), oldInset);
    }
}

void QQuickLabelPrivate::resizeBackground()
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

/*!
    \internal

    Determine which font is implicitly imposed on this control by its ancestors
    and QGuiApplication::font, resolve this against its own font (attributes from
    the implicit font are copied over). Then propagate this font to this
    control's children.
*/
void QQuickLabelPrivate::resolveFont()
{
    Q_Q(QQuickLabel);
    inheritFont(QQuickControlPrivate::parentFont(q));
}

void QQuickLabelPrivate::inheritFont(const QFont &font)
{
    QFont parentFont = extra.isAllocated() ? extra->requestedFont.resolve(font) : font;
    parentFont.resolve(extra.isAllocated() ? extra->requestedFont.resolve() | font.resolve() : font.resolve());

    const QFont defaultFont = QQuickTheme::font(QQuickTheme::Label);
    const QFont resolvedFont = parentFont.resolve(defaultFont);

    setFont_helper(resolvedFont);
}

/*!
    \internal

    Assign \a font to this control, and propagate it to all children.
*/
void QQuickLabelPrivate::updateFont(const QFont &font)
{
    Q_Q(QQuickLabel);
    QFont oldFont = sourceFont;
    q->QQuickText::setFont(font);

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
void QQuickLabelPrivate::resolvePalette()
{
    Q_Q(QQuickLabel);
    inheritPalette(QQuickControlPrivate::parentPalette(q));
}

void QQuickLabelPrivate::inheritPalette(const QPalette &palette)
{
    QPalette parentPalette = extra.isAllocated() ? extra->requestedPalette.resolve(palette) : palette;
    parentPalette.resolve(extra.isAllocated() ? extra->requestedPalette.resolve() | palette.resolve() : palette.resolve());

    const QPalette defaultPalette = QQuickTheme::palette(QQuickTheme::Label);
    const QPalette resolvedPalette = parentPalette.resolve(defaultPalette);

    setPalette_helper(resolvedPalette);
}

void QQuickLabelPrivate::updatePalette(const QPalette &palette)
{
    Q_Q(QQuickLabel);
    QPalette oldPalette = resolvedPalette;
    resolvedPalette = palette;

    QQuickControlPrivate::updatePaletteRecur(q, palette);

    if (oldPalette != palette)
        emit q->paletteChanged();
}

void QQuickLabelPrivate::textChanged(const QString &text)
{
#if QT_CONFIG(accessibility)
    Q_Q(QQuickLabel);
    if (QQuickAccessibleAttached *accessibleAttached = QQuickControlPrivate::accessibleAttached(q))
        accessibleAttached->setName(text);
#else
    Q_UNUSED(text)
#endif
}

#if QT_CONFIG(accessibility)
void QQuickLabelPrivate::accessibilityActiveChanged(bool active)
{
    if (!active)
        return;

    Q_Q(QQuickLabel);
    QQuickAccessibleAttached *accessibleAttached = qobject_cast<QQuickAccessibleAttached *>(qmlAttachedPropertiesObject<QQuickAccessibleAttached>(q, true));
    Q_ASSERT(accessibleAttached);
    accessibleAttached->setRole(accessibleRole());
    accessibleAttached->setName(text);
}

QAccessible::Role QQuickLabelPrivate::accessibleRole() const
{
    return QAccessible::StaticText;
}
#endif

static inline QString backgroundName() { return QStringLiteral("background"); }

void QQuickLabelPrivate::cancelBackground()
{
    Q_Q(QQuickLabel);
    quickCancelDeferred(q, backgroundName());
}

void QQuickLabelPrivate::executeBackground(bool complete)
{
    Q_Q(QQuickLabel);
    if (background.wasExecuted())
        return;

    if (!background || complete)
        quickBeginDeferred(q, backgroundName(), background);
    if (complete)
        quickCompleteDeferred(q, backgroundName(), background);
}

void QQuickLabelPrivate::itemGeometryChanged(QQuickItem *item, QQuickGeometryChange change, const QRectF &diff)
{
    Q_UNUSED(diff);
    if (resizingBackground || item != background || !change.sizeChange())
        return;

    QQuickItemPrivate *p = QQuickItemPrivate::get(item);
    extra.value().hasBackgroundWidth = p->widthValid;
    extra.value().hasBackgroundHeight = p->heightValid;
    resizeBackground();
}

void QQuickLabelPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_Q(QQuickLabel);
    if (item == background)
        emit q->implicitBackgroundWidthChanged();
}

void QQuickLabelPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_Q(QQuickLabel);
    if (item == background)
        emit q->implicitBackgroundHeightChanged();
}

void QQuickLabelPrivate::itemDestroyed(QQuickItem *item)
{
    Q_Q(QQuickLabel);
    if (item == background) {
        background = nullptr;
        emit q->implicitBackgroundWidthChanged();
        emit q->implicitBackgroundHeightChanged();
    }
}

QQuickLabel::QQuickLabel(QQuickItem *parent)
    : QQuickText(*(new QQuickLabelPrivate), parent)
{
    Q_D(QQuickLabel);
    QObjectPrivate::connect(this, &QQuickText::textChanged, d, &QQuickLabelPrivate::textChanged);
}

QQuickLabel::~QQuickLabel()
{
    Q_D(QQuickLabel);
    QQuickControlPrivate::removeImplicitSizeListener(d->background, d, QQuickControlPrivate::ImplicitSizeChanges | QQuickItemPrivate::Geometry);
}

QFont QQuickLabel::font() const
{
    return QQuickText::font();
}

void QQuickLabel::setFont(const QFont &font)
{
    Q_D(QQuickLabel);
    if (d->extra.value().requestedFont.resolve() == font.resolve() && d->extra.value().requestedFont == font)
        return;

    d->extra.value().requestedFont = font;
    d->resolveFont();
}

/*!
    \qmlproperty Item QtQuick.Controls::Label::background

    This property holds the background item.

    \note If the background item has no explicit size specified, it automatically
          follows the control's size. In most cases, there is no need to specify
          width or height for a background item.

    \sa {Customizing Label}
*/
QQuickItem *QQuickLabel::background() const
{
    QQuickLabelPrivate *d = const_cast<QQuickLabelPrivate *>(d_func());
    if (!d->background)
        d->executeBackground();
    return d->background;
}

void QQuickLabel::setBackground(QQuickItem *background)
{
    Q_D(QQuickLabel);
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
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty palette QtQuick.Controls::Label::palette

    This property holds the palette currently set for the label.

    \sa Control::palette
*/
QPalette QQuickLabel::palette() const
{
    Q_D(const QQuickLabel);
    QPalette palette = d->resolvedPalette;
    if (!isEnabled())
        palette.setCurrentColorGroup(QPalette::Disabled);
    return palette;
}

void QQuickLabel::setPalette(const QPalette &palette)
{
    Q_D(QQuickLabel);
    if (d->extra.value().requestedPalette.resolve() == palette.resolve() && d->extra.value().requestedPalette == palette)
        return;

    d->extra.value().requestedPalette = palette;
    d->resolvePalette();
}

void QQuickLabel::resetPalette()
{
    setPalette(QPalette());
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Label::implicitBackgroundWidth
    \readonly

    This property holds the implicit background width.

    The value is equal to \c {background ? background.implicitWidth : 0}.

    \sa implicitBackgroundHeight
*/
qreal QQuickLabel::implicitBackgroundWidth() const
{
    Q_D(const QQuickLabel);
    if (!d->background)
        return 0;
    return d->background->implicitWidth();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Label::implicitBackgroundHeight
    \readonly

    This property holds the implicit background height.

    The value is equal to \c {background ? background.implicitHeight : 0}.

    \sa implicitBackgroundWidth
*/
qreal QQuickLabel::implicitBackgroundHeight() const
{
    Q_D(const QQuickLabel);
    if (!d->background)
        return 0;
    return d->background->implicitHeight();
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Label::topInset

    This property holds the top inset for the background.

    \sa {Control Layout}, bottomInset
*/
qreal QQuickLabel::topInset() const
{
    Q_D(const QQuickLabel);
    return d->getTopInset();
}

void QQuickLabel::setTopInset(qreal inset)
{
    Q_D(QQuickLabel);
    d->setTopInset(inset);
}

void QQuickLabel::resetTopInset()
{
    Q_D(QQuickLabel);
    d->setTopInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Label::leftInset

    This property holds the left inset for the background.

    \sa {Control Layout}, rightInset
*/
qreal QQuickLabel::leftInset() const
{
    Q_D(const QQuickLabel);
    return d->getLeftInset();
}

void QQuickLabel::setLeftInset(qreal inset)
{
    Q_D(QQuickLabel);
    d->setLeftInset(inset);
}

void QQuickLabel::resetLeftInset()
{
    Q_D(QQuickLabel);
    d->setLeftInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Label::rightInset

    This property holds the right inset for the background.

    \sa {Control Layout}, leftInset
*/
qreal QQuickLabel::rightInset() const
{
    Q_D(const QQuickLabel);
    return d->getRightInset();
}

void QQuickLabel::setRightInset(qreal inset)
{
    Q_D(QQuickLabel);
    d->setRightInset(inset);
}

void QQuickLabel::resetRightInset()
{
    Q_D(QQuickLabel);
    d->setRightInset(0, true);
}

/*!
    \since QtQuick.Controls 2.5 (Qt 5.12)
    \qmlproperty real QtQuick.Controls::Label::bottomInset

    This property holds the bottom inset for the background.

    \sa {Control Layout}, topInset
*/
qreal QQuickLabel::bottomInset() const
{
    Q_D(const QQuickLabel);
    return d->getBottomInset();
}

void QQuickLabel::setBottomInset(qreal inset)
{
    Q_D(QQuickLabel);
    d->setBottomInset(inset);
}

void QQuickLabel::resetBottomInset()
{
    Q_D(QQuickLabel);
    d->setBottomInset(0, true);
}

void QQuickLabel::classBegin()
{
    Q_D(QQuickLabel);
    QQuickText::classBegin();
    d->resolveFont();
    d->resolvePalette();
}

void QQuickLabel::componentComplete()
{
    Q_D(QQuickLabel);
    d->executeBackground(true);
    QQuickText::componentComplete();
    d->resizeBackground();
#if QT_CONFIG(accessibility)
    if (QAccessible::isActive())
        d->accessibilityActiveChanged(true);
#endif
}

void QQuickLabel::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    Q_D(QQuickLabel);
    QQuickText::itemChange(change, value);
    switch (change) {
    case ItemEnabledHasChanged:
        emit paletteChanged();
        break;
    case ItemSceneChange:
    case ItemParentHasChanged:
        if ((change == ItemParentHasChanged && value.item) || (change == ItemSceneChange && value.window)) {
            d->resolveFont();
            d->resolvePalette();
        }
        break;
    default:
        break;
    }
}

void QQuickLabel::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickLabel);
    QQuickText::geometryChanged(newGeometry, oldGeometry);
    d->resizeBackground();
}

void QQuickLabel::insetChange(const QMarginsF &newInset, const QMarginsF &oldInset)
{
    Q_D(QQuickLabel);
    Q_UNUSED(newInset);
    Q_UNUSED(oldInset);
    d->resizeBackground();
}

QT_END_NAMESPACE
