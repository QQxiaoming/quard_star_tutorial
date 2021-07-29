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

#include "qquickpopupitem_p_p.h"
#include "qquickapplicationwindow_p.h"
#include "qquickshortcutcontext_p_p.h"
#include "qquickpage_p_p.h"
#include "qquickcontentitem_p.h"
#include "qquickpopup_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtGui/private/qshortcutmap_p.h>
#include <QtGui/private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

class QQuickPopupItemPrivate : public QQuickPagePrivate
{
    Q_DECLARE_PUBLIC(QQuickPopupItem)

public:
    QQuickPopupItemPrivate(QQuickPopup *popup);

    void implicitWidthChanged() override;
    void implicitHeightChanged() override;

    void resolveFont() override;
    void resolvePalette() override;

    QQuickItem *getContentItem() override;

    void cancelContentItem() override;
    void executeContentItem(bool complete = false) override;

    void cancelBackground() override;
    void executeBackground(bool complete = false) override;

    int backId = 0;
    int escapeId = 0;
    QQuickPopup *popup = nullptr;
};

QQuickPopupItemPrivate::QQuickPopupItemPrivate(QQuickPopup *popup)
    : popup(popup)
{
    isTabFence = true;
}

void QQuickPopupItemPrivate::implicitWidthChanged()
{
    QQuickPagePrivate::implicitWidthChanged();
    emit popup->implicitWidthChanged();
}

void QQuickPopupItemPrivate::implicitHeightChanged()
{
    QQuickPagePrivate::implicitHeightChanged();
    emit popup->implicitHeightChanged();
}

void QQuickPopupItemPrivate::resolveFont()
{
    if (QQuickApplicationWindow *window = qobject_cast<QQuickApplicationWindow *>(popup->window()))
        inheritFont(window->font());
    else
        inheritFont(QQuickTheme::font(QQuickTheme::System));
}

void QQuickPopupItemPrivate::resolvePalette()
{
    if (QQuickApplicationWindow *window = qobject_cast<QQuickApplicationWindow *>(popup->window()))
        inheritPalette(window->palette());
    else
        inheritPalette(QQuickTheme::palette(QQuickTheme::System));
}

QQuickItem *QQuickPopupItemPrivate::getContentItem()
{
    Q_Q(QQuickPopupItem);
    if (QQuickItem *item = QQuickPagePrivate::getContentItem())
        return item;

    return new QQuickContentItem(popup, q);
}

static inline QString contentItemName() { return QStringLiteral("contentItem"); }

void QQuickPopupItemPrivate::cancelContentItem()
{
    quickCancelDeferred(popup, contentItemName());
}

void QQuickPopupItemPrivate::executeContentItem(bool complete)
{
    if (contentItem.wasExecuted())
        return;

    if (!contentItem || complete)
        quickBeginDeferred(popup, contentItemName(), contentItem);
    if (complete)
        quickCompleteDeferred(popup, contentItemName(), contentItem);
}

static inline QString backgroundName() { return QStringLiteral("background"); }

void QQuickPopupItemPrivate::cancelBackground()
{
    quickCancelDeferred(popup, backgroundName());
}

void QQuickPopupItemPrivate::executeBackground(bool complete)
{
    if (background.wasExecuted())
        return;

    if (!background || complete)
        quickBeginDeferred(popup, backgroundName(), background);
    if (complete)
        quickCompleteDeferred(popup, backgroundName(), background);
}

QQuickPopupItem::QQuickPopupItem(QQuickPopup *popup)
    : QQuickPage(*(new QQuickPopupItemPrivate(popup)), nullptr)
{
    setParent(popup);
    setFlag(ItemIsFocusScope);
    setAcceptedMouseButtons(Qt::AllButtons);
#if QT_CONFIG(cursor)
    setCursor(Qt::ArrowCursor);
#endif

#if QT_CONFIG(quicktemplates2_hover)
    // TODO: switch to QStyleHints::useHoverEffects in Qt 5.8
    setHoverEnabled(true);
    // setAcceptHoverEvents(QGuiApplication::styleHints()->useHoverEffects());
    // connect(QGuiApplication::styleHints(), &QStyleHints::useHoverEffectsChanged, this, &QQuickItem::setAcceptHoverEvents);
#endif
}

void QQuickPopupItem::grabShortcut()
{
#if QT_CONFIG(shortcut)
    Q_D(QQuickPopupItem);
    QGuiApplicationPrivate *pApp = QGuiApplicationPrivate::instance();
    if (!d->backId)
        d->backId = pApp->shortcutMap.addShortcut(this, Qt::Key_Back, Qt::WindowShortcut, QQuickShortcutContext::matcher);
    if (!d->escapeId)
        d->escapeId = pApp->shortcutMap.addShortcut(this, Qt::Key_Escape, Qt::WindowShortcut, QQuickShortcutContext::matcher);
#endif
}

void QQuickPopupItem::ungrabShortcut()
{
#if QT_CONFIG(shortcut)
    Q_D(QQuickPopupItem);
    QGuiApplicationPrivate *pApp = QGuiApplicationPrivate::instance();
    if (d->backId) {
        pApp->shortcutMap.removeShortcut(d->backId, this);
        d->backId = 0;
    }
    if (d->escapeId) {
        pApp->shortcutMap.removeShortcut(d->escapeId, this);
        d->escapeId = 0;
    }
#endif
}

void QQuickPopupItem::updatePolish()
{
    Q_D(QQuickPopupItem);
    return QQuickPopupPrivate::get(d->popup)->reposition();
}

bool QQuickPopupItem::event(QEvent *event)
{
#if QT_CONFIG(shortcut)
    Q_D(QQuickPopupItem);
    if (event->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(event);
        if (se->shortcutId() == d->escapeId || se->shortcutId() == d->backId) {
            QQuickPopupPrivate *p = QQuickPopupPrivate::get(d->popup);
            if (p->interactive) {
                p->closeOrReject();
                return true;
            }
        }
    }
#endif
    return QQuickItem::event(event);
}

bool QQuickPopupItem::childMouseEventFilter(QQuickItem *child, QEvent *event)
{
    Q_D(QQuickPopupItem);
    return d->popup->childMouseEventFilter(child, event);
}

void QQuickPopupItem::focusInEvent(QFocusEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->focusInEvent(event);
}

void QQuickPopupItem::focusOutEvent(QFocusEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->focusOutEvent(event);
}

void QQuickPopupItem::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->keyPressEvent(event);
}

void QQuickPopupItem::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->keyReleaseEvent(event);
}

void QQuickPopupItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->mousePressEvent(event);
}

void QQuickPopupItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->mouseMoveEvent(event);
}

void QQuickPopupItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->mouseReleaseEvent(event);
}

void QQuickPopupItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->mouseDoubleClickEvent(event);
}

void QQuickPopupItem::mouseUngrabEvent()
{
    Q_D(QQuickPopupItem);
    d->popup->mouseUngrabEvent();
}

#if QT_CONFIG(quicktemplates2_multitouch)
void QQuickPopupItem::touchEvent(QTouchEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->touchEvent(event);
}

void QQuickPopupItem::touchUngrabEvent()
{
    Q_D(QQuickPopupItem);
    d->popup->touchUngrabEvent();
}
#endif

#if QT_CONFIG(wheelevent)
void QQuickPopupItem::wheelEvent(QWheelEvent *event)
{
    Q_D(QQuickPopupItem);
    d->popup->wheelEvent(event);
}
#endif

void QQuickPopupItem::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_D(QQuickPopupItem);
    QQuickPage::contentItemChange(newItem, oldItem);
    d->popup->contentItemChange(newItem, oldItem);
}

void QQuickPopupItem::contentSizeChange(const QSizeF &newSize, const QSizeF &oldSize)
{
    Q_D(QQuickPopupItem);
    QQuickPage::contentSizeChange(newSize, oldSize);
    d->popup->contentSizeChange(newSize, oldSize);
}

void QQuickPopupItem::fontChange(const QFont &newFont, const QFont &oldFont)
{
    Q_D(QQuickPopupItem);
    QQuickPage::fontChange(newFont, oldFont);
    d->popup->fontChange(newFont, oldFont);
}

void QQuickPopupItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickPopupItem);
    QQuickPage::geometryChanged(newGeometry, oldGeometry);
    d->popup->geometryChanged(newGeometry, oldGeometry);
}

void QQuickPopupItem::localeChange(const QLocale &newLocale, const QLocale &oldLocale)
{
    Q_D(QQuickPopupItem);
    QQuickPage::localeChange(newLocale, oldLocale);
    d->popup->localeChange(newLocale, oldLocale);
}

void QQuickPopupItem::mirrorChange()
{
    Q_D(QQuickPopupItem);
    emit d->popup->mirroredChanged();
}

void QQuickPopupItem::itemChange(ItemChange change, const ItemChangeData &data)
{
    Q_D(QQuickPopupItem);
    QQuickPage::itemChange(change, data);
    d->popup->itemChange(change, data);
}

void QQuickPopupItem::paddingChange(const QMarginsF &newPadding, const QMarginsF &oldPadding)
{
    Q_D(QQuickPopupItem);
    QQuickPage::paddingChange(newPadding, oldPadding);
    d->popup->paddingChange(newPadding, oldPadding);
}

void QQuickPopupItem::paletteChange(const QPalette &newPalette, const QPalette &oldPalette)
{
    Q_D(QQuickPopupItem);
    QQuickPage::paletteChange(newPalette, oldPalette);
    d->popup->paletteChange(newPalette, oldPalette);
}

void QQuickPopupItem::enabledChange()
{
    Q_D(QQuickPopupItem);
    // Just having QQuickPopup connect our QQuickItem::enabledChanged() signal
    // to its enabledChanged() signal is enough for the enabled property to work,
    // but we must also ensure that its paletteChanged() signal is emitted
    // so that bindings to palette are re-evaluated, because QQuickControl::palette()
    // returns a different palette depending on whether or not the control is enabled.
    // To save a connection, we also emit enabledChanged here.
    emit d->popup->enabledChanged();
    emit d->popup->paletteChanged();
}

QFont QQuickPopupItem::defaultFont() const
{
    Q_D(const QQuickPopupItem);
    return d->popup->defaultFont();
}

QPalette QQuickPopupItem::defaultPalette() const
{
    Q_D(const QQuickPopupItem);
    return d->popup->defaultPalette();
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickPopupItem::accessibleRole() const
{
    Q_D(const QQuickPopupItem);
    return d->popup->accessibleRole();
}

void QQuickPopupItem::accessibilityActiveChanged(bool active)
{
    Q_D(const QQuickPopupItem);
    QQuickPage::accessibilityActiveChanged(active);
    d->popup->accessibilityActiveChanged(active);
}
#endif

QT_END_NAMESPACE
