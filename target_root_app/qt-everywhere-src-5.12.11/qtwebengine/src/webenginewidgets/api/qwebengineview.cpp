/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebengineview.h"
#include "qwebengineview_p.h"

#include "qwebenginepage_p.h"
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"

#if QT_CONFIG(action)
#include <QAction>
#endif
#if QT_CONFIG(menu)
#include <QMenu>
#endif
#include <QContextMenuEvent>
#include <QToolTip>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE

void QWebEngineViewPrivate::pageChanged(QWebEnginePage *oldPage, QWebEnginePage *newPage)
{
    Q_Q(QWebEngineView);

    if (oldPage) {
        oldPage->disconnect(q);
    }

    if (newPage) {
        QObject::connect(newPage, &QWebEnginePage::titleChanged, q, &QWebEngineView::titleChanged);
        QObject::connect(newPage, &QWebEnginePage::urlChanged, q, &QWebEngineView::urlChanged);
        QObject::connect(newPage, &QWebEnginePage::iconUrlChanged, q, &QWebEngineView::iconUrlChanged);
        QObject::connect(newPage, &QWebEnginePage::iconChanged, q, &QWebEngineView::iconChanged);
        QObject::connect(newPage, &QWebEnginePage::loadStarted, q, &QWebEngineView::loadStarted);
        QObject::connect(newPage, &QWebEnginePage::loadProgress, q, &QWebEngineView::loadProgress);
        QObject::connect(newPage, &QWebEnginePage::loadFinished, q, &QWebEngineView::loadFinished);
        QObject::connect(newPage, &QWebEnginePage::selectionChanged, q, &QWebEngineView::selectionChanged);
        QObject::connect(newPage, &QWebEnginePage::renderProcessTerminated, q, &QWebEngineView::renderProcessTerminated);
    }

    auto oldUrl = oldPage ? oldPage->url() : QUrl();
    auto newUrl = newPage ? newPage->url() : QUrl();
    if (oldUrl != newUrl)
        Q_EMIT q->urlChanged(newUrl);

    auto oldTitle = oldPage ? oldPage->title() : QString();
    auto newTitle = newPage ? newPage->title() : QString();
    if (oldTitle != newTitle)
        Q_EMIT q->titleChanged(newTitle);

    auto oldIcon = oldPage ? oldPage->iconUrl() : QUrl();
    auto newIcon = newPage ? newPage->iconUrl() : QUrl();
    if (oldIcon != newIcon) {
        Q_EMIT q->iconUrlChanged(newIcon);
        Q_EMIT q->iconChanged(newPage ? newPage->icon() : QIcon());
    }

    if ((oldPage && oldPage->hasSelection()) || (newPage && newPage->hasSelection()))
        Q_EMIT q->selectionChanged();
}

void QWebEngineViewPrivate::widgetChanged(QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *oldWidget,
                                          QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *newWidget)
{
    Q_Q(QWebEngineView);

    if (oldWidget) {
        q->layout()->removeWidget(oldWidget);
        oldWidget->hide();
    }

    if (newWidget) {
        q->layout()->addWidget(newWidget);
        q->setFocusProxy(newWidget);
        newWidget->show();
    }
}

#ifndef QT_NO_ACCESSIBILITY
static QAccessibleInterface *webAccessibleFactory(const QString &, QObject *object)
{
    if (QWebEngineView *v = qobject_cast<QWebEngineView*>(object))
        return new QWebEngineViewAccessible(v);
    return Q_NULLPTR;
}
#endif // QT_NO_ACCESSIBILITY

QWebEngineViewPrivate::QWebEngineViewPrivate()
    : page(0)
    , m_dragEntered(false)
    , m_ownsPage(false)
{
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::installFactory(&webAccessibleFactory);
#endif // QT_NO_ACCESSIBILITY
}

/*!
    \fn QWebEngineView::renderProcessTerminated(QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
    \since 5.6

    This signal is emitted when the render process is terminated with a non-zero exit status.
    \a terminationStatus is the termination status of the process and \a exitCode is the status code
    with which the process terminated.
*/

/*!
    \fn void QWebEngineView::iconChanged(const QIcon &icon)
    \since 5.7

    This signal is emitted when the icon ("favicon") associated with the
    view is changed. The new icon is specified by \a icon.

    \sa icon(), iconUrl(), iconUrlChanged()
*/

QWebEngineView::QWebEngineView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new QWebEngineViewPrivate)
{
    Q_D(QWebEngineView);
    d->q_ptr = this;
    setAcceptDrops(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

QWebEngineView::~QWebEngineView()
{
    blockSignals(true);
    QWebEnginePagePrivate::bindPageAndView(nullptr, this);
}

QWebEnginePage* QWebEngineView::page() const
{
    Q_D(const QWebEngineView);
    if (!d->page) {
        QWebEngineView *that = const_cast<QWebEngineView*>(this);
        that->setPage(new QWebEnginePage(that));
        d->m_ownsPage = true;
    }
    return d->page;
}

void QWebEngineView::setPage(QWebEnginePage *newPage)
{
    QWebEnginePagePrivate::bindPageAndView(newPage, this);
}

void QWebEngineView::load(const QUrl& url)
{
    page()->load(url);
}

/*!
    \since 5.9
    Issues the specified \a request and loads the response.

    \sa load(), setUrl(), url(), urlChanged(), QUrl::fromUserInput()
*/
void QWebEngineView::load(const QWebEngineHttpRequest &request)
{
    page()->load(request);
}

void QWebEngineView::setHtml(const QString& html, const QUrl& baseUrl)
{
    page()->setHtml(html, baseUrl);
}

void QWebEngineView::setContent(const QByteArray& data, const QString& mimeType, const QUrl& baseUrl)
{
    page()->setContent(data, mimeType, baseUrl);
}

QWebEngineHistory* QWebEngineView::history() const
{
    return page()->history();
}

QString QWebEngineView::title() const
{
    return page()->title();
}

void QWebEngineView::setUrl(const QUrl &url)
{
    page()->setUrl(url);
}

QUrl QWebEngineView::url() const
{
    return page()->url();
}

QUrl QWebEngineView::iconUrl() const
{
    return page()->iconUrl();
}

/*!
    \property QWebEngineView::icon
    \brief The icon associated with the page currently viewed.
    \since 5.7

    By default, this property contains a null icon.

    \sa iconChanged(), iconUrl(), iconUrlChanged()
*/
QIcon QWebEngineView::icon() const
{
    return page()->icon();
}

bool QWebEngineView::hasSelection() const
{
    return page()->hasSelection();
}

QString QWebEngineView::selectedText() const
{
    return page()->selectedText();
}

#ifndef QT_NO_ACTION
QAction* QWebEngineView::pageAction(QWebEnginePage::WebAction action) const
{
    return page()->action(action);
}
#endif

void QWebEngineView::triggerPageAction(QWebEnginePage::WebAction action, bool checked)
{
    page()->triggerAction(action, checked);
}

void QWebEngineView::findText(const QString &subString, QWebEnginePage::FindFlags options, const QWebEngineCallback<bool> &resultCallback)
{
    page()->findText(subString, options, resultCallback);
}

/*!
 * \reimp
 */
QSize QWebEngineView::sizeHint() const
{
    // TODO: Remove this override for Qt 6
    return QWidget::sizeHint();
}

QWebEngineSettings *QWebEngineView::settings() const
{
    return page()->settings();
}

void QWebEngineView::stop()
{
    page()->triggerAction(QWebEnginePage::Stop);
}

void QWebEngineView::back()
{
    page()->triggerAction(QWebEnginePage::Back);
}

void QWebEngineView::forward()
{
    page()->triggerAction(QWebEnginePage::Forward);
}

void QWebEngineView::reload()
{
    page()->triggerAction(QWebEnginePage::Reload);
}

QWebEngineView *QWebEngineView::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type)
    return 0;
}

qreal QWebEngineView::zoomFactor() const
{
    return page()->zoomFactor();
}

void QWebEngineView::setZoomFactor(qreal factor)
{
    page()->setZoomFactor(factor);
}

/*!
 * \reimp
 */
bool QWebEngineView::event(QEvent *ev)
{
    if (ev->type() == QEvent::ContextMenu) {
        if (contextMenuPolicy() == Qt::NoContextMenu) {
            // We forward the contextMenu event to the parent widget
            ev->ignore();
            return false;
        }

        // We swallow spontaneous contextMenu events and synthethize those back later on when we get the
        // HandleContextMenu callback from chromium
        ev->accept();
        return true;
    }

    // Override QWidget's default ToolTip handler since it doesn't hide tooltip on empty text.
    if (ev->type() == QEvent::ToolTip) {
        if (!toolTip().isEmpty())
            QToolTip::showText(static_cast<QHelpEvent *>(ev)->globalPos(), toolTip(), this, QRect(), toolTipDuration());
        else
            QToolTip::hideText();

        ev->accept();
        return true;
    }

    return QWidget::event(ev);
}

/*!
 * \reimp
 */
#if QT_CONFIG(contextmenu)
void QWebEngineView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    menu->popup(event->globalPos());
}
#endif // QT_CONFIG(contextmenu)

/*!
 * \reimp
 */
void QWebEngineView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    page()->d_ptr->wasShown();
}

/*!
 * \reimp
 */
void QWebEngineView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    page()->d_ptr->wasHidden();
}

#if QT_CONFIG(draganddrop)
/*!
    \reimp
*/
void QWebEngineView::dragEnterEvent(QDragEnterEvent *e)
{
    Q_D(QWebEngineView);
    e->accept();
    if (d->m_dragEntered)
        d->page->d_ptr->adapter->leaveDrag();
    d->page->d_ptr->adapter->enterDrag(e, mapToGlobal(e->pos()));
    d->m_dragEntered = true;
}

/*!
    \reimp
*/
void QWebEngineView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_D(QWebEngineView);
    if (!d->m_dragEntered)
        return;
    e->accept();
    d->page->d_ptr->adapter->leaveDrag();
    d->m_dragEntered = false;
}

/*!
    \reimp
*/
void QWebEngineView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QWebEngineView);
    if (!d->m_dragEntered)
        return;
    QtWebEngineCore::WebContentsAdapter *adapter = d->page->d_ptr->adapter.data();
    Qt::DropAction dropAction = adapter->updateDragPosition(e, mapToGlobal(e->pos()));
    if (Qt::IgnoreAction == dropAction) {
        e->ignore();
    } else {
        e->setDropAction(dropAction);
        e->accept();
    }
}

/*!
    \reimp
*/
void QWebEngineView::dropEvent(QDropEvent *e)
{
    Q_D(QWebEngineView);
    if (!d->m_dragEntered)
        return;
    e->accept();
    d->page->d_ptr->adapter->endDragging(e, mapToGlobal(e->pos()));
    d->m_dragEntered = false;
}
#endif // QT_CONFIG(draganddrop)

#ifndef QT_NO_ACCESSIBILITY
int QWebEngineViewAccessible::childCount() const
{
    if (view() && child(0))
        return 1;
    return 0;
}

QAccessibleInterface *QWebEngineViewAccessible::child(int index) const
{
    if (index == 0 && view() && view()->page())
        return view()->page()->d_func()->adapter->browserAccessible();
    return Q_NULLPTR;
}

int QWebEngineViewAccessible::indexOfChild(const QAccessibleInterface *c) const
{
    if (c == child(0))
        return 0;
    return -1;
}
#endif // QT_NO_ACCESSIBILITY

QT_END_NAMESPACE

#include "moc_qwebengineview.cpp"
