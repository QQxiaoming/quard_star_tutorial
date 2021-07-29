/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qwebenginepage.h"
#include "qwebenginepage_p.h"

#include "authentication_dialog_controller.h"
#include "profile_adapter.h"
#include "certificate_error_controller.h"
#include "color_chooser_controller.h"
#include "favicon_manager.h"
#include "file_picker_controller.h"
#include "javascript_dialog_controller.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "printing/pdfium_document_wrapper_qt.h"
#endif
#include "qwebenginecertificateerror.h"
#include "qwebenginefullscreenrequest.h"
#include "qwebenginehistory.h"
#include "qwebenginehistory_p.h"
#include "qwebengineprofile.h"
#include "qwebengineprofile_p.h"
#include "qwebenginequotarequest.h"
#include "qwebengineregisterprotocolhandlerrequest.h"
#include "qwebenginescriptcollection_p.h"
#include "qwebenginesettings.h"
#include "qwebengineview.h"
#include "qwebengineview_p.h"
#include "render_widget_host_view_qt_delegate_widget.h"
#include "web_contents_adapter.h"
#include "web_engine_settings.h"
#include "qwebenginescript.h"

#include <QAction>
#include <QApplication>
#include <QAuthenticator>
#include <QClipboard>
#if QT_CONFIG(colordialog)
#include <QColorDialog>
#endif
#include <QContextMenuEvent>
#if QT_CONFIG(filedialog)
#include <QFileDialog>
#endif
#include <QKeyEvent>
#include <QIcon>
#if QT_CONFIG(inputdialog)
#include <QInputDialog>
#endif
#include <QLayout>
#include <QLoggingCategory>
#if QT_CONFIG(menu)
#include <QMenu>
#endif
#if QT_CONFIG(messagebox)
#include <QMessageBox>
#endif
#include <QMimeData>
#if QT_CONFIG(webengine_printing_and_pdf)
#include <QPrinter>
#endif
#include <QStandardPaths>
#include <QStyle>
#include <QTimer>
#include <QUrl>

QT_BEGIN_NAMESPACE

using namespace QtWebEngineCore;

static const int MaxTooltipLength = 1024;

#if QT_CONFIG(webengine_printing_and_pdf)
static bool printPdfDataOnPrinter(const QByteArray& data, QPrinter& printer)
{
    if (!data.size()) {
        qWarning("Failure to print on printer %ls: Print result data is empty.",
                 qUtf16Printable(printer.printerName()));
        return false;
    }

    QSize pageSize = printer.pageRect().size();
    PdfiumDocumentWrapperQt pdfiumWrapper(data.constData(), data.size(), pageSize);

    int toPage = printer.toPage();
    int fromPage = printer.fromPage();
    bool ascendingOrder = true;

    if (fromPage == 0 && toPage == 0) {
        fromPage = 1;
        toPage = pdfiumWrapper.pageCount();
    }
    fromPage = qMax(1, fromPage);
    toPage = qMin(pdfiumWrapper.pageCount(), toPage);

    if (printer.pageOrder() == QPrinter::LastPageFirst) {
        qSwap(fromPage, toPage);
        ascendingOrder = false;
    }

    int pageCopies = 1;
    int documentCopies = 1;

    if (!printer.supportsMultipleCopies())
        documentCopies = printer.copyCount();

    if (printer.collateCopies()) {
        pageCopies = documentCopies;
        documentCopies = 1;
    }

    QPainter painter;
    if (!painter.begin(&printer)) {
        qWarning("Failure to print on printer %ls: Could not open printer for painting.",
                  qUtf16Printable(printer.printerName()));
        return false;
    }

    for (int printedDocuments = 0; printedDocuments < documentCopies; printedDocuments++) {
        int currentPageIndex = fromPage;
        while (true) {
            for (int printedPages = 0; printedPages < pageCopies; printedPages++) {
                if (printer.printerState() == QPrinter::Aborted
                        || printer.printerState() == QPrinter::Error)
                    return false;

                QImage currentImage = pdfiumWrapper.pageAsQImage(currentPageIndex - 1);
                if (currentImage.isNull())
                    return false;

                // Painting operations are automatically clipped to the bounds of the drawable part of the page.
                painter.drawImage(QRect(0, 0, pageSize.width(), pageSize.height()), currentImage, currentImage.rect());
                if (printedPages < pageCopies - 1)
                    printer.newPage();
            }

            if (currentPageIndex == toPage)
                break;

            if (ascendingOrder)
                currentPageIndex++;
            else
                currentPageIndex--;

            printer.newPage();
        }
        if (printedDocuments < documentCopies - 1)
            printer.newPage();
    }
    painter.end();

    return true;
}
#endif // QT_CONFIG(webengine_printing_and_pdf)

static QWebEnginePage::WebWindowType toWindowType(WebContentsAdapterClient::WindowOpenDisposition disposition)
{
    switch (disposition) {
    case WebContentsAdapterClient::NewForegroundTabDisposition:
        return QWebEnginePage::WebBrowserTab;
    case WebContentsAdapterClient::NewBackgroundTabDisposition:
        return QWebEnginePage::WebBrowserBackgroundTab;
    case WebContentsAdapterClient::NewPopupDisposition:
        return QWebEnginePage::WebDialog;
    case WebContentsAdapterClient::NewWindowDisposition:
        return QWebEnginePage::WebBrowserWindow;
    default:
        Q_UNREACHABLE();
    }
}

QWebEnginePage::WebAction editorActionForKeyEvent(QKeyEvent* event)
{
    static struct {
        QKeySequence::StandardKey standardKey;
        QWebEnginePage::WebAction action;
    } editorActions[] = {
        { QKeySequence::Cut, QWebEnginePage::Cut },
        { QKeySequence::Copy, QWebEnginePage::Copy },
        { QKeySequence::Paste, QWebEnginePage::Paste },
        { QKeySequence::Undo, QWebEnginePage::Undo },
        { QKeySequence::Redo, QWebEnginePage::Redo },
        { QKeySequence::SelectAll, QWebEnginePage::SelectAll },
        { QKeySequence::UnknownKey, QWebEnginePage::NoWebAction }
    };
    for (int i = 0; editorActions[i].standardKey != QKeySequence::UnknownKey; ++i)
        if (event == editorActions[i].standardKey)
            return editorActions[i].action;

    return QWebEnginePage::NoWebAction;
}

QWebEnginePagePrivate::QWebEnginePagePrivate(QWebEngineProfile *_profile)
    : adapter(QSharedPointer<WebContentsAdapter>::create())
    , history(new QWebEngineHistory(new QWebEngineHistoryPrivate(this)))
    , profile(_profile ? _profile : QWebEngineProfile::defaultProfile())
    , settings(new QWebEngineSettings(profile->settings()))
    , view(0)
    , isLoading(false)
    , scriptCollection(new QWebEngineScriptCollectionPrivate(profileAdapter()->userResourceController(), adapter))
    , m_isBeingAdopted(false)
    , m_backgroundColor(Qt::white)
    , fullscreenMode(false)
    , webChannel(nullptr)
    , webChannelWorldId(QWebEngineScript::MainWorld)
    , defaultAudioMuted(false)
    , defaultZoomFactor(1.0)
#if QT_CONFIG(webengine_printing_and_pdf)
    , currentPrinter(nullptr)
#endif
{
    memset(actions, 0, sizeof(actions));

    qRegisterMetaType<QWebEngineQuotaRequest>();
    qRegisterMetaType<QWebEngineRegisterProtocolHandlerRequest>();

    // See wasShown() and wasHidden().
    wasShownTimer.setSingleShot(true);
    QObject::connect(&wasShownTimer, &QTimer::timeout, [this](){
        ensureInitialized();
        wasShown();
    });

    profile->d_ptr->addWebContentsAdapterClient(this);
}

QWebEnginePagePrivate::~QWebEnginePagePrivate()
{
    delete history;
    delete settings;
    profile->d_ptr->removeWebContentsAdapterClient(this);
}

RenderWidgetHostViewQtDelegate *QWebEnginePagePrivate::CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client)
{
    // Set the QWebEngineView as the parent for a popup delegate, so that the new popup window
    // responds properly to clicks in case the QWebEngineView is inside a modal QDialog. Setting the
    // parent essentially notifies the OS that the popup window is part of the modal session, and
    // should allow interaction.
    // The new delegate will not be deleted by the parent view though, because we unset the parent
    // when the parent is destroyed. The delegate will be destroyed by Chromium when the popup is
    // dismissed.
    return new RenderWidgetHostViewQtDelegateWidget(client, this->view);
}

void QWebEnginePagePrivate::initializationFinished()
{
    if (m_backgroundColor != Qt::white)
        adapter->setBackgroundColor(m_backgroundColor);
#if QT_CONFIG(webengine_webchannel)
    if (webChannel)
        adapter->setWebChannel(webChannel, webChannelWorldId);
#endif
    if (defaultAudioMuted != adapter->isAudioMuted())
        adapter->setAudioMuted(defaultAudioMuted);
    if (!qFuzzyCompare(adapter->currentZoomFactor(), defaultZoomFactor))
        adapter->setZoomFactor(defaultZoomFactor);

    scriptCollection.d->initializationFinished(adapter);

    m_isBeingAdopted = false;
}

void QWebEnginePagePrivate::titleChanged(const QString &title)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->titleChanged(title);
}

void QWebEnginePagePrivate::urlChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    explicitUrl = QUrl();
    Q_EMIT q->urlChanged(url);
}

void QWebEnginePagePrivate::iconChanged(const QUrl &url)
{
    Q_Q(QWebEnginePage);
    if (iconUrl == url)
        return;
    iconUrl = url;
    Q_EMIT q->iconUrlChanged(iconUrl);
    Q_EMIT q->iconChanged(adapter->faviconManager()->getIcon());
}

void QWebEnginePagePrivate::loadProgressChanged(int progress)
{
    Q_Q(QWebEnginePage);
    QTimer::singleShot(0, q, [q, progress] () { Q_EMIT q->loadProgress(progress); });
}

void QWebEnginePagePrivate::didUpdateTargetURL(const QUrl &hoveredUrl)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->linkHovered(hoveredUrl.toString());
}

void QWebEnginePagePrivate::selectionChanged()
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->selectionChanged();
}

void QWebEnginePagePrivate::recentlyAudibleChanged(bool recentlyAudible)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->recentlyAudibleChanged(recentlyAudible);
}

QRectF QWebEnginePagePrivate::viewportRect() const
{
    return view ? view->rect() : QRectF();
}

qreal QWebEnginePagePrivate::dpiScale() const
{
    return 1.0;
}

QColor QWebEnginePagePrivate::backgroundColor() const
{
    return m_backgroundColor;
}

void QWebEnginePagePrivate::loadStarted(const QUrl &provisionalUrl, bool isErrorPage)
{
    Q_UNUSED(provisionalUrl);
    Q_Q(QWebEnginePage);

    if (isErrorPage)
        return;

    isLoading = true;
    QTimer::singleShot(0, q, &QWebEnginePage::loadStarted);
}

void QWebEnginePagePrivate::loadFinished(bool success, const QUrl &url, bool isErrorPage, int errorCode, const QString &errorDescription)
{
    Q_Q(QWebEnginePage);
    Q_UNUSED(url);
    Q_UNUSED(errorCode);
    Q_UNUSED(errorDescription);

    if (isErrorPage) {
        Q_ASSERT(settings->testAttribute(QWebEngineSettings::ErrorPageEnabled));
        QTimer::singleShot(0, q, [q](){
            emit q->loadFinished(false);
        });
        return;
    }

    isLoading = false;
    if (success)
        explicitUrl = QUrl();
    // Delay notifying failure until the error-page is done loading.
    // Error-pages are not loaded on failures due to abort.
    if (success || errorCode == -3 /* ERR_ABORTED*/ || !settings->testAttribute(QWebEngineSettings::ErrorPageEnabled)) {
        QTimer::singleShot(0, q, [q, success](){
            emit q->loadFinished(success);
        });
    }
}

void QWebEnginePagePrivate::didPrintPageToPdf(const QString &filePath, bool success)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->pdfPrintingFinished(filePath, success);
}

void QWebEnginePagePrivate::focusContainer()
{
    if (view) {
        view->activateWindow();
        view->setFocus();
    }
}

void QWebEnginePagePrivate::unhandledKeyEvent(QKeyEvent *event)
{
    if (view && view->parentWidget())
        QGuiApplication::sendEvent(view->parentWidget(), event);
}

void QWebEnginePagePrivate::adoptNewWindow(QSharedPointer<WebContentsAdapter> newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect &initialGeometry, const QUrl &targetUrl)
{
    Q_Q(QWebEnginePage);
    Q_UNUSED(userGesture);
    Q_UNUSED(targetUrl);

    QWebEnginePage *newPage = q->createWindow(toWindowType(disposition));
    if (!newPage)
        return;

    if (newPage->d_func() == this) {
        // If createWindow returns /this/ we must delay the adoption.
        Q_ASSERT(q == newPage);
        // WebContents might be null if we just opened a new page for navigation, in that case
        // avoid referencing newWebContents so that it is deleted and WebContentsDelegateQt::OpenURLFromTab
        // will fall back to navigating current page.
        if (newWebContents->webContents()) {
            QTimer::singleShot(0, q, [this, newPage, newWebContents, initialGeometry] () {
                adoptNewWindowImpl(newPage, newWebContents, initialGeometry);
            });
        }
    } else {
        adoptNewWindowImpl(newPage, newWebContents, initialGeometry);
    }
}

void QWebEnginePagePrivate::adoptNewWindowImpl(QWebEnginePage *newPage,
        const QSharedPointer<WebContentsAdapter> &newWebContents, const QRect &initialGeometry)
{
    // Mark the new page as being in the process of being adopted, so that a second mouse move event
    // sent by newWebContents->initialize() gets filtered in RenderWidgetHostViewQt::forwardEvent.
    // The first mouse move event is being sent by q->createWindow(). This is necessary because
    // Chromium does not get a mouse move acknowledgment message between the two events, and
    // InputRouterImpl::ProcessMouseAck is not executed, thus all subsequent mouse move events
    // get coalesced together, and don't get processed at all.
    // The mouse move events are actually sent as a result of show() being called on
    // RenderWidgetHostViewQtDelegateWidget, both when creating the window and when initialize is
    // called.
    newPage->d_func()->m_isBeingAdopted = true;

    // Overwrite the new page's WebContents with ours.
    newPage->d_func()->adapter = newWebContents;
    newWebContents->setClient(newPage->d_func());

    if (!initialGeometry.isEmpty())
        emit newPage->geometryChangeRequested(initialGeometry);
}

bool QWebEnginePagePrivate::isBeingAdopted()
{
    return m_isBeingAdopted;
}

void QWebEnginePagePrivate::close()
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->windowCloseRequested();
}

void QWebEnginePagePrivate::windowCloseRejected()
{
    // Do nothing for now.
}

void QWebEnginePagePrivate::didRunJavaScript(quint64 requestId, const QVariant& result)
{
    m_callbacks.invoke(requestId, result);
}

void QWebEnginePagePrivate::didFetchDocumentMarkup(quint64 requestId, const QString& result)
{
    m_callbacks.invoke(requestId, result);
}

void QWebEnginePagePrivate::didFetchDocumentInnerText(quint64 requestId, const QString& result)
{
    m_callbacks.invoke(requestId, result);
}

void QWebEnginePagePrivate::didFindText(quint64 requestId, int matchCount)
{
    m_callbacks.invoke(requestId, matchCount > 0);
}

void QWebEnginePagePrivate::didPrintPage(quint64 requestId, const QByteArray &result)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    // If no currentPrinter is set that means that were printing to PDF only.
    if (!currentPrinter) {
        m_callbacks.invoke(requestId, result);
        return;
    }

    bool printerResult = printPdfDataOnPrinter(result, *currentPrinter);

    currentPrinter = nullptr;
    m_callbacks.invoke(requestId, printerResult);
#else
    // we should never enter this branch, but just for safe-keeping...
    Q_UNUSED(result);
    m_callbacks.invoke(requestId, QByteArray());
#endif
}

bool QWebEnginePagePrivate::passOnFocus(bool reverse)
{
    if (view)
        return view->focusNextPrevChild(!reverse);
    return false;
}

void QWebEnginePagePrivate::authenticationRequired(QSharedPointer<AuthenticationDialogController> controller)
{
    Q_Q(QWebEnginePage);
    QAuthenticator networkAuth;
    networkAuth.setRealm(controller->realm());

    if (controller->isProxy())
        Q_EMIT q->proxyAuthenticationRequired(controller->url(), &networkAuth, controller->host());
    else
        Q_EMIT q->authenticationRequired(controller->url(), &networkAuth);

    // Authentication has been cancelled
    if (networkAuth.isNull()) {
        controller->reject();
        return;
    }

    controller->accept(networkAuth.user(), networkAuth.password());
}

void QWebEnginePagePrivate::releaseProfile()
{
    qDebug("Release of profile requested but WebEnginePage still not deleted. Expect troubles !");
    // this is not the way to go, but might avoid the crash if user code does not make any calls to page.
    delete q_ptr->d_ptr.take();
}

void QWebEnginePagePrivate::showColorDialog(QSharedPointer<ColorChooserController> controller)
{
#if QT_CONFIG(colordialog)
    QColorDialog *dialog = new QColorDialog(controller.data()->initialColor(), view);

    QColorDialog::connect(dialog, SIGNAL(colorSelected(QColor)), controller.data(), SLOT(accept(QColor)));
    QColorDialog::connect(dialog, SIGNAL(rejected()), controller.data(), SLOT(reject()));

    // Delete when done
    QColorDialog::connect(dialog, SIGNAL(colorSelected(QColor)), dialog, SLOT(deleteLater()));
    QColorDialog::connect(dialog, SIGNAL(rejected()), dialog, SLOT(deleteLater()));

    dialog->open();
#else
    Q_UNUSED(controller);
#endif
}

void QWebEnginePagePrivate::runMediaAccessPermissionRequest(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags requestFlags)
{
    Q_Q(QWebEnginePage);
    QWebEnginePage::Feature feature;
    if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture) &&
        requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        feature = QWebEnginePage::MediaAudioVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaAudioCapture))
        feature = QWebEnginePage::MediaAudioCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaVideoCapture))
        feature = QWebEnginePage::MediaVideoCapture;
    else if (requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopAudioCapture) &&
             requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture))
        feature = QWebEnginePage::DesktopAudioVideoCapture;
    else // if (requestFlags.testFlag(WebContentsAdapterClient::MediaDesktopVideoCapture))
        feature = QWebEnginePage::DesktopVideoCapture;
    Q_EMIT q->featurePermissionRequested(securityOrigin, feature);
}

void QWebEnginePagePrivate::runGeolocationPermissionRequest(const QUrl &securityOrigin)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->featurePermissionRequested(securityOrigin, QWebEnginePage::Geolocation);
}

void QWebEnginePagePrivate::runMouseLockPermissionRequest(const QUrl &securityOrigin)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->featurePermissionRequested(securityOrigin, QWebEnginePage::MouseLock);
}

void QWebEnginePagePrivate::runQuotaRequest(QWebEngineQuotaRequest request)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->quotaRequested(request);
}

void QWebEnginePagePrivate::runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest request)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->registerProtocolHandlerRequested(request);
}

QObject *QWebEnginePagePrivate::accessibilityParentObject()
{
    return view;
}

void QWebEnginePagePrivate::updateAction(QWebEnginePage::WebAction action) const
{
#ifdef QT_NO_ACTION
    Q_UNUSED(action)
#else
    QAction *a = actions[action];
    if (!a)
        return;

    bool enabled = true;

    switch (action) {
    case QWebEnginePage::Back:
        enabled = adapter->canGoBack();
        break;
    case QWebEnginePage::Forward:
        enabled = adapter->canGoForward();
        break;
    case QWebEnginePage::Stop:
        enabled = isLoading;
        break;
    case QWebEnginePage::Reload:
    case QWebEnginePage::ReloadAndBypassCache:
        enabled = !isLoading;
        break;
    case QWebEnginePage::ViewSource:
        enabled = adapter->canViewSource();
        break;
    case QWebEnginePage::Cut:
    case QWebEnginePage::Copy:
    case QWebEnginePage::Paste:
    case QWebEnginePage::Undo:
    case QWebEnginePage::Redo:
    case QWebEnginePage::SelectAll:
    case QWebEnginePage::PasteAndMatchStyle:
    case QWebEnginePage::Unselect:
        enabled = adapter->hasFocusedFrame();
        break;
    default:
        break;
    }

    a->setEnabled(enabled);
#endif // QT_NO_ACTION
}

void QWebEnginePagePrivate::updateNavigationActions()
{
    updateAction(QWebEnginePage::Back);
    updateAction(QWebEnginePage::Forward);
    updateAction(QWebEnginePage::Stop);
    updateAction(QWebEnginePage::Reload);
    updateAction(QWebEnginePage::ReloadAndBypassCache);
    updateAction(QWebEnginePage::ViewSource);
}

void QWebEnginePagePrivate::updateEditActions()
{
    updateAction(QWebEnginePage::Cut);
    updateAction(QWebEnginePage::Copy);
    updateAction(QWebEnginePage::Paste);
    updateAction(QWebEnginePage::Undo);
    updateAction(QWebEnginePage::Redo);
    updateAction(QWebEnginePage::SelectAll);
    updateAction(QWebEnginePage::PasteAndMatchStyle);
    updateAction(QWebEnginePage::Unselect);
}

#ifndef QT_NO_ACTION
void QWebEnginePagePrivate::_q_webActionTriggered(bool checked)
{
    Q_Q(QWebEnginePage);
    QAction *a = qobject_cast<QAction *>(q->sender());
    if (!a)
        return;
    QWebEnginePage::WebAction action = static_cast<QWebEnginePage::WebAction>(a->data().toInt());
    q->triggerAction(action, checked);
}
#endif // QT_NO_ACTION

void QWebEnginePagePrivate::recreateFromSerializedHistory(QDataStream &input)
{
    QSharedPointer<WebContentsAdapter> newWebContents = WebContentsAdapter::createFromSerializedNavigationHistory(input, this);
    if (newWebContents) {
        adapter = std::move(newWebContents);
        adapter->setClient(this);
        adapter->loadDefault();
        if (view && view->isVisible())
            wasShown();
    }
}

void QWebEnginePagePrivate::updateScrollPosition(const QPointF &position)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->scrollPositionChanged(position);
}

void QWebEnginePagePrivate::updateContentsSize(const QSizeF &size)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->contentsSizeChanged(size);
}

void QWebEnginePagePrivate::setFullScreenMode(bool fullscreen)
{
    if (fullscreenMode != fullscreen) {
        fullscreenMode = fullscreen;
        adapter->changedFullScreen();
    }
}

ProfileAdapter* QWebEnginePagePrivate::profileAdapter()
{
    return profile->d_ptr->profileAdapter();
}

WebContentsAdapter *QWebEnginePagePrivate::webContentsAdapter()
{
    ensureInitialized();
    return adapter.data();
}

const QObject *QWebEnginePagePrivate::holdingQObject() const
{
    Q_Q(const QWebEnginePage);
    return q;
}

void QWebEnginePagePrivate::widgetChanged(RenderWidgetHostViewQtDelegate *newWidgetBase)
{
    Q_Q(QWebEnginePage);
    bindPageAndWidget(q, static_cast<RenderWidgetHostViewQtDelegateWidget *>(newWidgetBase));
}

void QWebEnginePagePrivate::ensureInitialized() const
{
    if (!adapter->isInitialized())
        adapter->loadDefault();
}

void QWebEnginePagePrivate::bindPageAndView(QWebEnginePage *page, QWebEngineView *view)
{
    auto oldView = page ? page->d_func()->view : nullptr;
    auto oldPage = view ? view->d_func()->page : nullptr;

    bool ownNewPage = false;
    bool deleteOldPage = false;

    // Change pointers first.

    if (page && oldView != view) {
        if (oldView) {
            ownNewPage = oldView->d_func()->m_ownsPage;
            oldView->d_func()->page = nullptr;
            oldView->d_func()->m_ownsPage = false;
        }
        page->d_func()->view = view;
    }

    if (view && oldPage != page) {
        if (oldPage) {
            if (oldPage->d_func())
                oldPage->d_func()->view = nullptr;
            deleteOldPage = view->d_func()->m_ownsPage;
        }
        view->d_func()->m_ownsPage = ownNewPage;
        view->d_func()->page = page;
    }

    // Then notify.

    auto widget = page ? page->d_func()->widget : nullptr;
    auto oldWidget = (oldPage && oldPage->d_func()) ? oldPage->d_func()->widget : nullptr;

    if (page && oldView != view && oldView) {
        oldView->d_func()->pageChanged(page, nullptr);
        if (widget)
            oldView->d_func()->widgetChanged(widget, nullptr);
    }

    if (view && oldPage != page) {
        if (oldPage && oldPage->d_func())
            view->d_func()->pageChanged(oldPage, page);
        else
            view->d_func()->pageChanged(nullptr, page);
        if (oldWidget != widget)
            view->d_func()->widgetChanged(oldWidget, widget);
    }
    if (deleteOldPage)
        delete oldPage;
}

void QWebEnginePagePrivate::bindPageAndWidget(QWebEnginePage *page, RenderWidgetHostViewQtDelegateWidget *widget)
{
    auto oldPage = widget ? widget->m_page : nullptr;
    auto oldWidget = page ? page->d_func()->widget : nullptr;

    // Change pointers first.

    if (widget && oldPage != page) {
        if (oldPage && oldPage->d_func())
            oldPage->d_func()->widget = nullptr;
        widget->m_page = page;
    }

    if (page && oldWidget != widget) {
        if (oldWidget)
            oldWidget->m_page = nullptr;
        page->d_func()->widget = widget;
    }

    // Then notify.

    if (widget && oldPage != page && oldPage && oldPage->d_func()) {
        if (auto oldView = oldPage->d_func()->view)
            oldView->d_func()->widgetChanged(widget, nullptr);
    }

    if (page && oldWidget != widget) {
        if (auto view = page->d_func()->view)
            view->d_func()->widgetChanged(oldWidget, widget);
    }
}

QWebEnginePage::QWebEnginePage(QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEnginePagePrivate())
{
    Q_D(QWebEnginePage);
    d->q_ptr = this;
    d->adapter->setClient(d);
}

/*!
    \fn void QWebEnginePage::printRequested()
    \since 5.12

    This signal is emitted when the JavaScript \c{window.print()} method is called.
    Typically, the signal handler can simply call printToPdf().

    \sa printToPdf()
*/

/*!
    \enum QWebEnginePage::RenderProcessTerminationStatus
    \since 5.6

    This enum describes the status with which the render process terminated:

    \value  NormalTerminationStatus
            The render process terminated normally.
    \value  AbnormalTerminationStatus
            The render process terminated with with a non-zero exit status.
    \value  CrashedTerminationStatus
            The render process crashed, for example because of a segmentation fault.
    \value  KilledTerminationStatus
            The render process was killed, for example by \c SIGKILL or task manager kill.
*/

/*!
    \fn QWebEnginePage::renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode)
    \since 5.6

    This signal is emitted when the render process is terminated with a non-zero exit status.
    \a terminationStatus is the termination status of the process and \a exitCode is the status code
    with which the process terminated.
*/

/*!
    \fn QWebEnginePage::fullScreenRequested(QWebEngineFullScreenRequest fullScreenRequest)

    This signal is emitted when the web page issues the request to enter fullscreen mode for
    a web-element, usually a video element.

    The request object \a fullScreenRequest can be used to accept or reject the request.

    If the request is accepted the element requesting fullscreen will fill the viewport,
    but it is up to the application to make the view fullscreen or move the page to a view
    that is fullscreen.

    \sa QWebEngineSettings::FullScreenSupportEnabled
*/

/*!
    \fn QWebEnginePage::quotaRequested(QWebEngineQuotaRequest quotaRequest)
    \since 5.11

    This signal is emitted when the web page requests larger persistent storage
    than the application's current allocation in File System API. The default quota
    is 0 bytes.

    The request object \a quotaRequest can be used to accept or reject the request.
*/

/*!
    \fn QWebEnginePage::registerProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request)
    \since 5.11

    This signal is emitted when the web page tries to register a custom protocol
    using the \l registerProtocolHandler API.

    The request object \a request can be used to accept or reject the request:

    \snippet webenginewidgets/simplebrowser/webpage.cpp registerProtocolHandlerRequested
*/

/*!
    \fn void QWebEnginePage::pdfPrintingFinished(const QString &filePath, bool success)
    \since 5.9

    This signal is emitted when printing the web page into a PDF file has
    finished.
    \a filePath will contain the path the file was requested to be created
    at, and \a success will be \c true if the file was successfully created and
    \c false otherwise.

    \sa printToPdf()
*/

/*!
    \property QWebEnginePage::scrollPosition
    \since 5.7

    \brief The scroll position of the page contents.
*/

/*!
    \property QWebEnginePage::contentsSize
    \since 5.7

    \brief The size of the page contents.
*/

/*!
    \fn void QWebEnginePage::audioMutedChanged(bool muted)
    \since 5.7

    This signal is emitted when the page's \a muted state changes.
    \note Not to be confused with a specific HTML5 audio or video element being muted.
*/

/*!
    \fn void QWebEnginePage::recentlyAudibleChanged(bool recentlyAudible);
    \since 5.7

    This signal is emitted when the page's audible state, \a recentlyAudible, changes, because
    the audio is played or stopped.

    \note The signal is also emitted when calling the setAudioMuted() method.
    Also, if the audio is paused, this signal is emitted with an approximate \b{two-second
    delay}, from the moment the audio is paused.
*/

/*!
    \fn void QWebEnginePage::iconUrlChanged(const QUrl &url)

    This signal is emitted when the URL of the icon ("favicon") associated with the
    page is changed. The new URL is specified by \a url.

    \sa iconUrl(), icon(), iconChanged()
*/

/*!
    \fn void QWebEnginePage::iconChanged(const QIcon &icon)
    \since 5.7

    This signal is emitted when the icon ("favicon") associated with the
    page is changed. The new icon is specified by \a icon.

    \sa icon(), iconUrl(), iconUrlChanged()
*/

/*!
    Constructs an empty web engine page in the web engine profile \a profile with the parent
    \a parent.

    If the profile is not the default profile, the caller must ensure that the profile stays alive
    for as long as the page does.

    \since 5.5
*/
QWebEnginePage::QWebEnginePage(QWebEngineProfile *profile, QObject* parent)
    : QObject(parent)
    , d_ptr(new QWebEnginePagePrivate(profile))
{
    Q_D(QWebEnginePage);
    d->q_ptr = this;
    d->adapter->setClient(d);
}

QWebEnginePage::~QWebEnginePage()
{
    if (d_ptr) {
        // d_ptr might be exceptionally null if profile adapter got deleted first
        setDevToolsPage(nullptr);
        d_ptr->adapter->stopFinding();
        QWebEnginePagePrivate::bindPageAndView(this, nullptr);
        QWebEnginePagePrivate::bindPageAndWidget(this, nullptr);
    }
}

QWebEngineHistory *QWebEnginePage::history() const
{
    Q_D(const QWebEnginePage);
    return d->history;
}

QWebEngineSettings *QWebEnginePage::settings() const
{
    Q_D(const QWebEnginePage);
    return d->settings;
}

/*!
 * Returns a pointer to the web channel instance used by this page or a null pointer if none was set.
 * This channel automatically uses the internal web engine transport mechanism over Chromium IPC
 * that is exposed in the JavaScript context of this page as \c qt.webChannelTransport.
 *
 * \since 5.5
 * \sa setWebChannel()
 */
QWebChannel *QWebEnginePage::webChannel() const
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(const QWebEnginePage);
    return d->webChannel;
#endif
    qWarning("WebEngine compiled without webchannel support");
    return nullptr;
}

/*!
 * \overload
 *
 * Sets the web channel instance to be used by this page to \a channel and installs
 * it in the main JavaScript world.
 *
 * With this method the web channel can be accessed by web page content. If the content
 * is not under your control and might be hostile, this could be a security issue and
 * you should consider installing it in a private JavaScript world.
 *
 * \since 5.5
 * \sa QWebEngineScript::MainWorld
 */

void QWebEnginePage::setWebChannel(QWebChannel *channel)
{
    setWebChannel(channel, QWebEngineScript::MainWorld);
}

/*!
 * Sets the web channel instance to be used by this page to \a channel and connects it to
 * web engine's transport using Chromium IPC messages. The transport is exposed in the JavaScript
 * world \a worldId as
 * \c qt.webChannelTransport, which should be used when using the \l{Qt WebChannel JavaScript API}.
 *
 * \note The page does not take ownership of the channel object.
 * \note Only one web channel can be installed per page, setting one even in another JavaScript
 *       world uninstalls any already installed web channel.
 *
 * \since 5.7
 * \sa QWebEngineScript::ScriptWorldId
 */
void QWebEnginePage::setWebChannel(QWebChannel *channel, uint worldId)
{
#if QT_CONFIG(webengine_webchannel)
    Q_D(QWebEnginePage);
    if (d->webChannel != channel || d->webChannelWorldId != worldId) {
        d->webChannel = channel;
        d->webChannelWorldId = worldId;
        d->adapter->setWebChannel(channel, worldId);
    }
#else
    Q_UNUSED(channel)
    Q_UNUSED(worldId)
    qWarning("WebEngine compiled without webchannel support");
#endif
}

/*!
    \property QWebEnginePage::backgroundColor
    \brief The page's background color behind the document's body.
    \since 5.6

    You can set the background color to Qt::transparent or to a translucent
    color to see through the document, or you can set it to match your
    web content in a hybrid application to prevent the white flashes that may appear
    during loading.

    The default value is white.
*/
QColor QWebEnginePage::backgroundColor() const
{
    Q_D(const QWebEnginePage);
    return d->m_backgroundColor;
}

void QWebEnginePage::setBackgroundColor(const QColor &color)
{
    Q_D(QWebEnginePage);
    if (d->m_backgroundColor == color)
        return;
    d->m_backgroundColor = color;
    d->adapter->setBackgroundColor(color);
}

/*!
 * Save the currently loaded web page to disk.
 *
 * The web page is saved to \a filePath in the specified \a{format}.
 *
 * This is a short cut for the following actions:
 * \list
 *   \li Trigger the Save web action.
 *   \li Accept the next download item and set the specified file path and save format.
 * \endlist
 *
 * This function issues an asynchronous download request for the web page and returns immediately.
 *
 * \sa QWebEngineDownloadItem::SavePageFormat
 * \since 5.8
 */
void QWebEnginePage::save(const QString &filePath,
                          QWebEngineDownloadItem::SavePageFormat format) const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    d->adapter->save(filePath, format);
}

/*!
    \property QWebEnginePage::audioMuted
    \brief Whether the current page audio is muted.
    \since 5.7

    The default value is \c false.
    \sa recentlyAudible
*/
bool QWebEnginePage::isAudioMuted() const {
    Q_D(const QWebEnginePage);
    if (d->adapter->isInitialized())
        return d->adapter->isAudioMuted();
    return d->defaultAudioMuted;
}

void QWebEnginePage::setAudioMuted(bool muted) {
    Q_D(QWebEnginePage);
    d->defaultAudioMuted = muted;
    if (d->adapter->isInitialized())
        d->adapter->setAudioMuted(muted);
}

/*!
    \property QWebEnginePage::recentlyAudible
    \brief The current page's \e {audible state}, that is, whether audio was recently played
    or not.
    \since 5.7

    The default value is \c false.
    \sa audioMuted
*/
bool QWebEnginePage::recentlyAudible() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->isInitialized() && d->adapter->recentlyAudible();
}

void QWebEnginePage::setView(QWidget *newViewBase)
{
    QWebEnginePagePrivate::bindPageAndView(this, qobject_cast<QWebEngineView *>(newViewBase));
}

QWidget *QWebEnginePage::view() const
{
    Q_D(const QWebEnginePage);
    return d->view;
}

/*!
    Returns the web engine profile the page belongs to.
    \since 5.5
*/
QWebEngineProfile *QWebEnginePage::profile() const
{
    Q_D(const QWebEnginePage);
    return d->profile;
}

bool QWebEnginePage::hasSelection() const
{
    return !selectedText().isEmpty();
}

QString QWebEnginePage::selectedText() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->selectedText();
}

#ifndef QT_NO_ACTION
QAction *QWebEnginePage::action(WebAction action) const
{
    Q_D(const QWebEnginePage);
    if (action == QWebEnginePage::NoWebAction)
        return 0;
    if (d->actions[action])
        return d->actions[action];

    QString text;
    QIcon icon;
    QStyle *style = d->view ? d->view->style() : qApp->style();

    switch (action) {
    case Back:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Back);
        icon = style->standardIcon(QStyle::SP_ArrowBack);
        break;
    case Forward:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Forward);
        icon = style->standardIcon(QStyle::SP_ArrowForward);
        break;
    case Stop:
        text = tr("Stop");
        icon = style->standardIcon(QStyle::SP_BrowserStop);
        break;
    case Reload:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Reload);
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    case ReloadAndBypassCache:
        text = tr("Reload and Bypass Cache");
        icon = style->standardIcon(QStyle::SP_BrowserReload);
        break;
    case Cut:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Cut);
        break;
    case Copy:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Copy);
        break;
    case Paste:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Paste);
        break;
    case Undo:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Undo);
        break;
    case Redo:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::Redo);
        break;
    case SelectAll:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::SelectAll);
        break;
    case PasteAndMatchStyle:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::PasteAndMatchStyle);
        break;
    case OpenLinkInThisWindow:
        text = tr("Open link in this window");
        break;
    case OpenLinkInNewWindow:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::OpenLinkInNewWindow);
        break;
    case OpenLinkInNewTab:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::OpenLinkInNewTab);
        break;
    case OpenLinkInNewBackgroundTab:
        text = tr("Open link in new background tab");
        break;
    case CopyLinkToClipboard:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyLinkToClipboard);
        break;
    case DownloadLinkToDisk:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::DownloadLinkToDisk);
        break;
    case CopyImageToClipboard:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyImageToClipboard);
        break;
    case CopyImageUrlToClipboard:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyImageUrlToClipboard);
        break;
    case DownloadImageToDisk:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::DownloadImageToDisk);
        break;
    case CopyMediaUrlToClipboard:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::CopyMediaUrlToClipboard);
        break;
    case ToggleMediaControls:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ToggleMediaControls);
        break;
    case ToggleMediaLoop:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ToggleMediaLoop);
        break;
    case ToggleMediaPlayPause:
        text = tr("Toggle Play/Pause");
        break;
    case ToggleMediaMute:
        text = tr("Toggle Mute");
        break;
    case DownloadMediaToDisk:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::DownloadMediaToDisk);
        break;
    case InspectElement:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::InspectElement);
        break;
    case ExitFullScreen:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ExitFullScreen);
        break;
    case RequestClose:
        text = tr("Close Page");
        break;
    case Unselect:
        text = tr("Unselect");
        break;
    case SavePage:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::SavePage);
        break;
    case ViewSource:
        text = RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem::ViewSource);
        break;
    case ToggleBold:
        text = tr("&Bold");
        break;
    case ToggleItalic:
        text = tr("&Italic");
        break;
    case ToggleUnderline:
        text = tr("&Underline");
        break;
    case ToggleStrikethrough:
        text = tr("&Strikethrough");
        break;
    case AlignLeft:
        text = tr("Align &Left");
        break;
    case AlignCenter:
        text = tr("Align &Center");
        break;
    case AlignRight:
        text = tr("Align &Right");
        break;
    case AlignJustified:
        text = tr("Align &Justified");
        break;
    case Indent:
        text = tr("&Indent");
        break;
    case Outdent:
        text = tr("&Outdent");
        break;
    case InsertOrderedList:
        text = tr("Insert &Ordered List");
        break;
    case InsertUnorderedList:
        text = tr("Insert &Unordered List");
        break;
    case NoWebAction:
    case WebActionCount:
        Q_UNREACHABLE();
        break;
    }

    QAction *a = new QAction(const_cast<QWebEnginePage*>(this));
    a->setText(text);
    a->setData(action);
    a->setIcon(icon);

    connect(a, SIGNAL(triggered(bool)), this, SLOT(_q_webActionTriggered(bool)));

    d->actions[action] = a;
    d->updateAction(action);
    return a;
}
#endif // QT_NO_ACTION

void QWebEnginePage::triggerAction(WebAction action, bool)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    const QtWebEngineCore::WebEngineContextMenuData *menuData = d->contextData.d;
    switch (action) {
    case Back:
        d->adapter->navigateToOffset(-1);
        break;
    case Forward:
        d->adapter->navigateToOffset(1);
        break;
    case Stop:
        d->adapter->stop();
        break;
    case Reload:
        d->adapter->reload();
        break;
    case ReloadAndBypassCache:
        d->adapter->reloadAndBypassCache();
        break;
    case Cut:
        d->adapter->cut();
        break;
    case Copy:
        d->adapter->copy();
        break;
    case Paste:
        d->adapter->paste();
        break;
    case Undo:
        d->adapter->undo();
        break;
    case Redo:
        d->adapter->redo();
        break;
    case SelectAll:
        d->adapter->selectAll();
        break;
    case PasteAndMatchStyle:
        d->adapter->pasteAndMatchStyle();
        break;
    case Unselect:
        d->adapter->unselect();
        break;
    case OpenLinkInThisWindow:
        if (menuData && menuData->linkUrl().isValid())
            setUrl(menuData->linkUrl());
        break;
    case OpenLinkInNewWindow:
        if (menuData && menuData->linkUrl().isValid()) {
            QWebEnginePage *newPage = createWindow(WebBrowserWindow);
            if (newPage)
                newPage->setUrl(menuData->linkUrl());
        }
        break;
    case OpenLinkInNewTab:
        if (menuData && menuData->linkUrl().isValid()) {
            QWebEnginePage *newPage = createWindow(WebBrowserTab);
            if (newPage)
                newPage->setUrl(menuData->linkUrl());
        }
        break;
    case OpenLinkInNewBackgroundTab:
        if (menuData && menuData->linkUrl().isValid()) {
            QWebEnginePage *newPage = createWindow(WebBrowserBackgroundTab);
            if (newPage)
                newPage->setUrl(menuData->linkUrl());
        }
        break;
    case CopyLinkToClipboard:
        if (menuData && !menuData->unfilteredLinkUrl().isEmpty()) {
            QString urlString = menuData->unfilteredLinkUrl().toString(QUrl::FullyEncoded);
            QString title = menuData->linkText().toHtmlEscaped();
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<a href=\"") + urlString + QStringLiteral("\">") + title + QStringLiteral("</a>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << menuData->unfilteredLinkUrl());
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case DownloadLinkToDisk:
        if (menuData && menuData->linkUrl().isValid())
            d->adapter->download(menuData->linkUrl(), menuData->suggestedFileName(),
                                 menuData->referrerUrl(), menuData->referrerPolicy());

        break;
    case CopyImageToClipboard:
        if (menuData && menuData->hasImageContent() &&
                (menuData->mediaType() == WebEngineContextMenuData::MediaTypeImage ||
                 menuData->mediaType() == WebEngineContextMenuData::MediaTypeCanvas))
        {
            d->adapter->copyImageAt(menuData->position());
        }
        break;
    case CopyImageUrlToClipboard:
        if (menuData && menuData->mediaUrl().isValid() && menuData->mediaType() == WebEngineContextMenuData::MediaTypeImage) {
            QString urlString = menuData->mediaUrl().toString(QUrl::FullyEncoded);
            QString title = menuData->linkText();
            if (!title.isEmpty())
                title = QStringLiteral(" alt=\"%1\"").arg(title.toHtmlEscaped());
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            QString html = QStringLiteral("<img src=\"") + urlString + QStringLiteral("\"") + title + QStringLiteral("></img>");
            data->setHtml(html);
            data->setUrls(QList<QUrl>() << menuData->mediaUrl());
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case DownloadImageToDisk:
    case DownloadMediaToDisk:
        if (menuData && menuData->mediaUrl().isValid())
            d->adapter->download(menuData->mediaUrl(), menuData->suggestedFileName(),
                                 menuData->referrerUrl(), menuData->referrerPolicy());
        break;
    case CopyMediaUrlToClipboard:
        if (menuData && menuData->mediaUrl().isValid() &&
                (menuData->mediaType() == WebEngineContextMenuData::MediaTypeAudio ||
                 menuData->mediaType() == WebEngineContextMenuData::MediaTypeVideo))
        {
            QString urlString = menuData->mediaUrl().toString(QUrl::FullyEncoded);
            QMimeData *data = new QMimeData();
            data->setText(urlString);
            if (menuData->mediaType() == WebEngineContextMenuData::MediaTypeAudio)
                data->setHtml(QStringLiteral("<audio src=\"") + urlString + QStringLiteral("\"></audio>"));
            else
                data->setHtml(QStringLiteral("<video src=\"") + urlString + QStringLiteral("\"></video>"));
            data->setUrls(QList<QUrl>() << menuData->mediaUrl());
            qApp->clipboard()->setMimeData(data);
        }
        break;
    case ToggleMediaControls:
        if (menuData && menuData->mediaUrl().isValid() && menuData->mediaFlags() & WebEngineContextMenuData::MediaCanToggleControls) {
            bool enable = !(menuData->mediaFlags() & WebEngineContextMenuData::MediaControls);
            d->adapter->executeMediaPlayerActionAt(menuData->position(), WebContentsAdapter::MediaPlayerControls, enable);
        }
        break;
    case ToggleMediaLoop:
        if (menuData && menuData->mediaUrl().isValid() &&
                (menuData->mediaType() == WebEngineContextMenuData::MediaTypeAudio ||
                 menuData->mediaType() == WebEngineContextMenuData::MediaTypeVideo))
        {
            bool enable = !(menuData->mediaFlags() & WebEngineContextMenuData::MediaLoop);
            d->adapter->executeMediaPlayerActionAt(menuData->position(), WebContentsAdapter::MediaPlayerLoop, enable);
        }
        break;
    case ToggleMediaPlayPause:
        if (menuData && menuData->mediaUrl().isValid() &&
                (menuData->mediaType() == WebEngineContextMenuData::MediaTypeAudio ||
                 menuData->mediaType() == WebEngineContextMenuData::MediaTypeVideo))
        {
            bool enable = (menuData->mediaFlags() & WebEngineContextMenuData::MediaPaused);
            d->adapter->executeMediaPlayerActionAt(menuData->position(), WebContentsAdapter::MediaPlayerPlay, enable);
        }
        break;
    case ToggleMediaMute:
        if (menuData && menuData->mediaUrl().isValid() && menuData->mediaFlags() & WebEngineContextMenuData::MediaHasAudio) {
            // Make sure to negate the value, so that toggling actually works.
            bool enable = !(menuData->mediaFlags() & WebEngineContextMenuData::MediaMuted);
            d->adapter->executeMediaPlayerActionAt(menuData->position(), WebContentsAdapter::MediaPlayerMute, enable);
        }
        break;
    case InspectElement:
        if (menuData)
            d->adapter->inspectElementAt(menuData->position());
        break;
    case ExitFullScreen:
        // See under ViewSource, anything that can trigger a delete of the current view is dangerous to call directly here.
        QTimer::singleShot(0, this, [d](){ d->adapter->exitFullScreen(); });
        break;
    case RequestClose:
        d->adapter->requestClose();
        break;
    case SavePage:
        d->adapter->save();
        break;
    case ViewSource:
        // This is a workaround to make the ViewSource action working in a context menu.
        // The WebContentsAdapter::viewSource() method deletes a
        // RenderWidgetHostViewQtDelegateWidget instance which passes the control to the event
        // loop. If the QMenu::aboutToHide() signal is connected to the QObject::deleteLater()
        // slot the QMenu is deleted by the event handler while the ViewSource action is still not
        // completed. This may lead to a crash. To avoid this the WebContentsAdapter::viewSource()
        // method is called indirectly via the QTimer::singleShot() function which schedules the
        // the viewSource() call after the QMenu's destruction.
        QTimer::singleShot(0, this, [d](){ d->adapter->viewSource(); });
        break;
    case ToggleBold:
        runJavaScript(QStringLiteral("document.execCommand('bold');"), QWebEngineScript::ApplicationWorld);
        break;
    case ToggleItalic:
        runJavaScript(QStringLiteral("document.execCommand('italic');"), QWebEngineScript::ApplicationWorld);
        break;
    case ToggleUnderline:
        runJavaScript(QStringLiteral("document.execCommand('underline');"), QWebEngineScript::ApplicationWorld);
        break;
    case ToggleStrikethrough:
        runJavaScript(QStringLiteral("document.execCommand('strikethrough');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignLeft:
        runJavaScript(QStringLiteral("document.execCommand('justifyLeft');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignCenter:
        runJavaScript(QStringLiteral("document.execCommand('justifyCenter');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignRight:
        runJavaScript(QStringLiteral("document.execCommand('justifyRight');"), QWebEngineScript::ApplicationWorld);
        break;
    case AlignJustified:
        runJavaScript(QStringLiteral("document.execCommand('justifyFull');"), QWebEngineScript::ApplicationWorld);
        break;
    case Indent:
        runJavaScript(QStringLiteral("document.execCommand('indent');"), QWebEngineScript::ApplicationWorld);
        break;
    case Outdent:
        runJavaScript(QStringLiteral("document.execCommand('outdent');"), QWebEngineScript::ApplicationWorld);
        break;
    case InsertOrderedList:
        runJavaScript(QStringLiteral("document.execCommand('insertOrderedList');"), QWebEngineScript::ApplicationWorld);
        break;
    case InsertUnorderedList:
        runJavaScript(QStringLiteral("document.execCommand('insertUnorderedList');"), QWebEngineScript::ApplicationWorld);
        break;
    case NoWebAction:
        break;
    case WebActionCount:
        Q_UNREACHABLE();
        break;
    }
}

/*!
 * \since 5.8
 * Replace the current misspelled word with \a replacement.
 *
 * The current misspelled word can be found in QWebEngineContextMenuData::misspelledWord(),
 * and suggested replacements in QWebEngineContextMenuData::spellCheckerSuggestions().
 *
 * \sa contextMenuData(),
 */

void QWebEnginePage::replaceMisspelledWord(const QString &replacement)
{
    Q_D(QWebEnginePage);
    d->adapter->replaceMisspelling(replacement);
}

void QWebEnginePage::findText(const QString &subString, FindFlags options, const QWebEngineCallback<bool> &resultCallback)
{
    Q_D(QWebEnginePage);
    if (!d->adapter->isInitialized()) {
        d->m_callbacks.invokeEmpty(resultCallback);
        return;
    }
    if (subString.isEmpty()) {
        d->adapter->stopFinding();
        d->m_callbacks.invokeEmpty(resultCallback);
    } else {
        quint64 requestId = d->adapter->findText(subString, options & FindCaseSensitively, options & FindBackward);
        d->m_callbacks.registerCallback(requestId, resultCallback);
    }
}

/*!
 * \reimp
 */
bool QWebEnginePage::event(QEvent *e)
{
    return QObject::event(e);
}

void QWebEnginePagePrivate::wasShown()
{
    if (!adapter->isInitialized()) {
        // On the one hand, it is too early to initialize here. The application
        // may call show() before load(), or it may call show() from
        // createWindow(), and then we would create an unnecessary blank
        // WebContents here. On the other hand, if the application calls show()
        // then it expects something to be shown, so we have to initialize.
        // Therefore we have to delay the initialization via the event loop.
        wasShownTimer.start();
        return;
    }
    adapter->wasShown();
}

void QWebEnginePagePrivate::wasHidden()
{
    if (!adapter->isInitialized()) {
        // Cancel timer from wasShown() above.
        wasShownTimer.stop();
        return;
    }
    adapter->wasHidden();
}

void QWebEnginePagePrivate::contextMenuRequested(const WebEngineContextMenuData &data)
{
#if QT_CONFIG(action)
    if (!view)
        return;

    contextData.reset();
    switch (view->contextMenuPolicy()) {
    case Qt::DefaultContextMenu:
    {
        contextData = data;
        QContextMenuEvent event(QContextMenuEvent::Mouse, data.position(), view->mapToGlobal(data.position()));
        view->contextMenuEvent(&event);
        return;
    }
    case Qt::CustomContextMenu:
        contextData = data;
        Q_EMIT view->customContextMenuRequested(data.position());
        return;
    case Qt::ActionsContextMenu:
        if (view->actions().count()) {
            QContextMenuEvent event(QContextMenuEvent::Mouse, data.position(), view->mapToGlobal(data.position()));
            QMenu::exec(view->actions(), event.globalPos(), 0, view);
        }
        return;
    case Qt::PreventContextMenu:
    case Qt::NoContextMenu:
        return;
    }

    Q_UNREACHABLE();
#else
    Q_UNUSED(data);
#endif // QT_CONFIG(action)
}

void QWebEnginePagePrivate::navigationRequested(int navigationType, const QUrl &url, int &navigationRequestAction, bool isMainFrame)
{
    Q_Q(QWebEnginePage);
    bool accepted = q->acceptNavigationRequest(url, static_cast<QWebEnginePage::NavigationType>(navigationType), isMainFrame);
    if (accepted && adapter->isFindTextInProgress())
        adapter->stopFinding();
    navigationRequestAction = accepted ? WebContentsAdapterClient::AcceptRequest : WebContentsAdapterClient::IgnoreRequest;
}

void QWebEnginePagePrivate::requestFullScreenMode(const QUrl &origin, bool fullscreen)
{
    Q_Q(QWebEnginePage);
    QWebEngineFullScreenRequest request(q, origin, fullscreen);
    Q_EMIT q->fullScreenRequested(request);
}

bool QWebEnginePagePrivate::isFullScreenMode() const
{
    return fullscreenMode;
}

void QWebEnginePagePrivate::javascriptDialog(QSharedPointer<JavaScriptDialogController> controller)
{
    Q_Q(QWebEnginePage);
    bool accepted = false;
    QString promptResult;
    switch (controller->type()) {
    case AlertDialog:
        q->javaScriptAlert(controller->securityOrigin(), controller->message());
        accepted = true;
        break;
    case ConfirmDialog:
        accepted = q->javaScriptConfirm(controller->securityOrigin(), controller->message());
        break;
    case PromptDialog:
        accepted = q->javaScriptPrompt(controller->securityOrigin(), controller->message(), controller->defaultPrompt(), &promptResult);
        if (accepted)
            controller->textProvided(promptResult);
        break;
    case UnloadDialog:
        accepted = q->javaScriptConfirm(controller->securityOrigin(), QCoreApplication::translate("QWebEnginePage", "Are you sure you want to leave this page? Changes that you made may not be saved."));
        break;
    case InternalAuthorizationDialog:
#if QT_CONFIG(messagebox)
        accepted = (QMessageBox::question(view, controller->title(), controller->message(), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
#endif // QT_CONFIG(messagebox)
        break;
    }
    if (accepted)
        controller->accept();
    else
        controller->reject();
}

void QWebEnginePagePrivate::allowCertificateError(const QSharedPointer<CertificateErrorController> &controller)
{
    Q_Q(QWebEnginePage);
    bool accepted = false;

    QWebEngineCertificateError error(controller->error(), controller->url(), controller->overridable(), controller->errorString());
    accepted = q->certificateError(error);
    controller->accept(error.isOverridable() && accepted);
}

void QWebEnginePagePrivate::selectClientCert(const QSharedPointer<ClientCertSelectController> &controller)
{
#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    Q_Q(QWebEnginePage);
    QWebEngineClientCertificateSelection certSelection(controller);

    Q_EMIT q->selectClientCertificate(certSelection);
#else
    Q_UNUSED(controller);
#endif
}

#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
/*!
    \fn void QWebEnginePage::selectClientCertificate(QWebEngineClientCertificateSelection clientCertificateSelection)
    \since 5.12

    This signal is emitted when a web site requests an SSL client certificate, and one or more were
    found in system's client certificate store.

    Handling the signal is asynchronous, and loading will be waiting until a certificate is selected,
    or the last copy of \a clientCertificateSelection is destroyed.

    If the signal is not handled, \a clientCertificateSelection is automatically destroyed, and loading
    will continue without a client certificate.

    \sa QWebEngineClientCertificateSelection
*/
#endif

void QWebEnginePagePrivate::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_Q(QWebEnginePage);
    q->javaScriptConsoleMessage(static_cast<QWebEnginePage::JavaScriptConsoleMessageLevel>(level), message, lineNumber, sourceID);
}

void QWebEnginePagePrivate::renderProcessTerminated(RenderProcessTerminationStatus terminationStatus,
                                                int exitCode)
{
    Q_Q(QWebEnginePage);
    Q_EMIT q->renderProcessTerminated(static_cast<QWebEnginePage::RenderProcessTerminationStatus>(
                                      terminationStatus), exitCode);
}

void QWebEnginePagePrivate::requestGeometryChange(const QRect &geometry, const QRect &frameGeometry)
{
    Q_UNUSED(geometry);
    Q_Q(QWebEnginePage);
    Q_EMIT q->geometryChangeRequested(frameGeometry);
}

void QWebEnginePagePrivate::startDragging(const content::DropData &dropData,
                                          Qt::DropActions allowedActions, const QPixmap &pixmap,
                                          const QPoint &offset)
{
#if !QT_CONFIG(draganddrop)
    Q_UNUSED(dropData);
    Q_UNUSED(allowedActions);
    Q_UNUSED(pixmap);
    Q_UNUSED(offset);
#else
    adapter->startDragging(view, dropData, allowedActions, pixmap, offset);
#endif // QT_CONFIG(draganddrop)
}

bool QWebEnginePagePrivate::supportsDragging() const
{
    return true;
}

bool QWebEnginePagePrivate::isEnabled() const
{
    const Q_Q(QWebEnginePage);
    const QWidget *view = q->view();
    if (view)
        return view->isEnabled();
    return true;
}

void QWebEnginePagePrivate::setToolTip(const QString &toolTipText)
{
    if (!view)
        return;

    // Hide tooltip if shown.
    if (toolTipText.isEmpty()) {
        if (!view->toolTip().isEmpty())
            view->setToolTip(QString());

        return;
    }

    // Update tooltip if text was changed.
    QString wrappedTip = QLatin1String("<p style=\"white-space:pre-wrap\">")
            % toolTipText.toHtmlEscaped().left(MaxTooltipLength)
            % QLatin1String("</p>");
    if (view->toolTip() != wrappedTip)
        view->setToolTip(wrappedTip);
}

void QWebEnginePagePrivate::printRequested()
{
    Q_Q(QWebEnginePage);
    QTimer::singleShot(0, q, [q](){
        Q_EMIT q->printRequested();
    });
}

#if QT_CONFIG(menu)
QMenu *QWebEnginePage::createStandardContextMenu()
{
    Q_D(QWebEnginePage);
    if (!d->contextData.d)
        return nullptr;
    d->ensureInitialized();

    QMenu *menu = new QMenu(d->view);
    const WebEngineContextMenuData &contextMenuData = *d->contextData.d;

    QContextMenuBuilder contextMenuBuilder(contextMenuData, this, menu);

    contextMenuBuilder.initMenu();

    menu->setAttribute(Qt::WA_DeleteOnClose, true);

    return menu;
}
#endif // QT_CONFIG(menu)

void QWebEnginePage::setFeaturePermission(const QUrl &securityOrigin, QWebEnginePage::Feature feature, QWebEnginePage::PermissionPolicy policy)
{
    Q_D(QWebEnginePage);
    if (policy == PermissionUnknown)
        return;

    const WebContentsAdapterClient::MediaRequestFlags audioVideoCaptureFlags(
        WebContentsAdapterClient::MediaVideoCapture |
        WebContentsAdapterClient::MediaAudioCapture);
    const WebContentsAdapterClient::MediaRequestFlags desktopAudioVideoCaptureFlags(
        WebContentsAdapterClient::MediaDesktopVideoCapture |
        WebContentsAdapterClient::MediaDesktopAudioCapture);

    if (policy == PermissionGrantedByUser) {
        switch (feature) {
        case MediaAudioVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, audioVideoCaptureFlags);
            break;
        case MediaAudioCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaAudioCapture);
            break;
        case MediaVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaVideoCapture);
            break;
        case DesktopAudioVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, desktopAudioVideoCaptureFlags);
            break;
        case DesktopVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaDesktopVideoCapture);
            break;
        case Geolocation:
            d->adapter->runGeolocationRequestCallback(securityOrigin, true);
            break;
        case MouseLock:
            d->adapter->grantMouseLockPermission(true);
            break;
        case Notifications:
            break;
        }
    } else { // if (policy == PermissionDeniedByUser)
        switch (feature) {
        case MediaAudioVideoCapture:
        case MediaAudioCapture:
        case MediaVideoCapture:
        case DesktopAudioVideoCapture:
        case DesktopVideoCapture:
            d->adapter->grantMediaAccessPermission(securityOrigin, WebContentsAdapterClient::MediaNone);
            break;
        case Geolocation:
            d->adapter->runGeolocationRequestCallback(securityOrigin, false);
            break;
        case MouseLock:
            d->adapter->grantMouseLockPermission(false);
            break;
        case Notifications:
            break;
        }
    }
}

static inline QWebEnginePage::FileSelectionMode toPublic(FilePickerController::FileChooserMode mode)
{
    // Should the underlying values change, we'll need a switch here.
    return static_cast<QWebEnginePage::FileSelectionMode>(mode);
}

void QWebEnginePagePrivate::runFileChooser(QSharedPointer<FilePickerController> controller)
{
    Q_Q(QWebEnginePage);

    QStringList selectedFileNames = q->chooseFiles(toPublic(controller->mode()), (QStringList() << controller->defaultFileName()), controller->acceptedMimeTypes());

    if (!selectedFileNames.empty())
        controller->accepted(selectedFileNames);
    else
        controller->rejected();
}

WebEngineSettings *QWebEnginePagePrivate::webEngineSettings() const
{
    return settings->d_func();
}

/*!
    \since 5.10
    Downloads the resource from the location given by \a url to a local file.

    If \a filename is given, it is used as the suggested file name.
    If it is relative, the file is saved in the standard download location with
    the given name.
    If it is a null or empty QString, the default file name is used.

    This will emit QWebEngineProfile::downloadRequested() after the download
    has started.
*/

void QWebEnginePage::download(const QUrl& url, const QString& filename)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    d->adapter->download(url, filename);
}

void QWebEnginePage::load(const QUrl& url)
{
    Q_D(QWebEnginePage);
    d->adapter->load(url);
}

/*!
    \since 5.9
    Issues the specified \a request and loads the response.

    \sa load(), setUrl(), url(), urlChanged(), QUrl::fromUserInput()
*/
void QWebEnginePage::load(const QWebEngineHttpRequest& request)
{
    Q_D(QWebEnginePage);
    d->adapter->load(request);
}

void QWebEnginePage::toHtml(const QWebEngineCallback<const QString &> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    quint64 requestId = d->adapter->fetchDocumentMarkup();
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

void QWebEnginePage::toPlainText(const QWebEngineCallback<const QString &> &resultCallback) const
{
    Q_D(const QWebEnginePage);
    d->ensureInitialized();
    quint64 requestId = d->adapter->fetchDocumentInnerText();
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

void QWebEnginePage::setHtml(const QString &html, const QUrl &baseUrl)
{
    setContent(html.toUtf8(), QStringLiteral("text/html;charset=UTF-8"), baseUrl);
}

void QWebEnginePage::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
    Q_D(QWebEnginePage);
    d->adapter->setContent(data, mimeType, baseUrl);
}

QString QWebEnginePage::title() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->pageTitle();
}

void QWebEnginePage::setUrl(const QUrl &url)
{
    Q_D(QWebEnginePage);
    d->explicitUrl = url;
    load(url);
}

QUrl QWebEnginePage::url() const
{
    Q_D(const QWebEnginePage);
    return d->explicitUrl.isValid() ? d->explicitUrl : d->adapter->activeUrl();
}

QUrl QWebEnginePage::requestedUrl() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->requestedUrl();
}

/*!
    \property QWebEnginePage::iconUrl
    \brief The URL of the icon associated with the page currently viewed.

    By default, this property contains an empty URL.

    \sa iconUrlChanged(), icon(), iconChanged()
*/
QUrl QWebEnginePage::iconUrl() const
{
    Q_D(const QWebEnginePage);
    return d->iconUrl;
}

/*!
    \property QWebEnginePage::icon
    \brief The icon associated with the page currently viewed.
    \since 5.7

    By default, this property contains a null icon. If the web page specifies more than one icon,
    the \c{icon} property encapsulates the available candidate icons in a single,
    scalable \c{QIcon}.

    \sa iconChanged(), iconUrl(), iconUrlChanged()
*/
QIcon QWebEnginePage::icon() const
{
    Q_D(const QWebEnginePage);

    if (d->iconUrl.isEmpty() || !d->adapter->isInitialized())
        return QIcon();

    return d->adapter->faviconManager()->getIcon();
}

qreal QWebEnginePage::zoomFactor() const
{
    Q_D(const QWebEnginePage);
    if (d->adapter->isInitialized())
        return d->adapter->currentZoomFactor();
    return d->defaultZoomFactor;
}

void QWebEnginePage::setZoomFactor(qreal factor)
{
    Q_D(QWebEnginePage);
    d->defaultZoomFactor = factor;
    if (d->adapter->isInitialized())
        d->adapter->setZoomFactor(factor);
}

void QWebEnginePage::runJavaScript(const QString &scriptSource)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    d->adapter->runJavaScript(scriptSource, QWebEngineScript::MainWorld);
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, const QWebEngineCallback<const QVariant &> &resultCallback)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    quint64 requestId = d->adapter->runJavaScriptCallbackResult(scriptSource, QWebEngineScript::MainWorld);
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

void QWebEnginePage::runJavaScript(const QString &scriptSource, quint32 worldId)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    d->adapter->runJavaScript(scriptSource, worldId);
}

void QWebEnginePage::runJavaScript(const QString& scriptSource, quint32 worldId, const QWebEngineCallback<const QVariant &> &resultCallback)
{
    Q_D(QWebEnginePage);
    d->ensureInitialized();
    quint64 requestId = d->adapter->runJavaScriptCallbackResult(scriptSource, worldId);
    d->m_callbacks.registerCallback(requestId, resultCallback);
}

/*!
    Returns the collection of scripts that are injected into the page.

    In addition, a page might also execute scripts
    added through QWebEngineProfile::scripts().

    \sa QWebEngineScriptCollection, QWebEngineScript, {Script Injection}
*/

QWebEngineScriptCollection &QWebEnginePage::scripts()
{
    Q_D(QWebEnginePage);
    return d->scriptCollection;
}

QWebEnginePage *QWebEnginePage::createWindow(WebWindowType type)
{
    Q_D(QWebEnginePage);
    if (d->view) {
        QWebEngineView *newView = d->view->createWindow(type);
        if (newView)
            return newView->page();
    }
    return 0;
}

/*!
    \since 5.11
    Returns the page this page is inspecting, if any.

    Returns \c nullptr if this page is not a developer tools page.

    \sa setInspectedPage(), devToolsPage()
*/

QWebEnginePage *QWebEnginePage::inspectedPage() const
{
    Q_D(const QWebEnginePage);
    return d->inspectedPage;
}

/*!
    \since 5.11
    Navigates this page to an internal URL that is the developer
    tools of \a page.

    This is the same as calling setDevToolsPage() on \a page
    with \c this as argument.

    \sa inspectedPage(), setDevToolsPage()
*/

void QWebEnginePage::setInspectedPage(QWebEnginePage *page)
{
    Q_D(QWebEnginePage);
    if (d->inspectedPage == page)
        return;
    QWebEnginePage *oldPage = d->inspectedPage;
    d->inspectedPage = nullptr;
    if (oldPage)
        oldPage->setDevToolsPage(nullptr);
    d->inspectedPage = page;
    if (page)
        page->setDevToolsPage(this);
}

/*!
    \since 5.11
    Returns the page that is hosting the developer tools
    of this page, if any.

    Returns \c nullptr if no developer tools page is set.

    \sa setDevToolsPage(), inspectedPage()
*/

QWebEnginePage *QWebEnginePage::devToolsPage() const
{
    Q_D(const QWebEnginePage);
    return d->devToolsPage;
}

/*!
    \since 5.11
    Binds \a devToolsPage to be the developer tools of this page.
    Triggers \a devToolsPage to navigate to an internal URL
    with the developer tools.

    This is the same as calling setInspectedPage() on \a devToolsPage
    with \c this as argument.

    \sa devToolsPage(), setInspectedPage()
*/

void QWebEnginePage::setDevToolsPage(QWebEnginePage *devToolsPage)
{
    Q_D(QWebEnginePage);
    if (d->devToolsPage == devToolsPage)
        return;
    d->ensureInitialized();
    QWebEnginePage *oldDevTools = d->devToolsPage;
    d->devToolsPage = nullptr;
    if (oldDevTools)
        oldDevTools->setInspectedPage(nullptr);
    d->devToolsPage = devToolsPage;
    if (devToolsPage)
        devToolsPage->setInspectedPage(this);
    if (d->adapter) {
        if (devToolsPage)
            d->adapter->openDevToolsFrontend(devToolsPage->d_ptr->adapter);
        else
            d->adapter->closeDevToolsFrontend();
    }
}

ASSERT_ENUMS_MATCH(FilePickerController::Open, QWebEnginePage::FileSelectOpen)
ASSERT_ENUMS_MATCH(FilePickerController::OpenMultiple, QWebEnginePage::FileSelectOpenMultiple)

QStringList QWebEnginePage::chooseFiles(FileSelectionMode mode, const QStringList &oldFiles, const QStringList &acceptedMimeTypes)
{
#if QT_CONFIG(filedialog)
    // FIXME: Should we expose this in QWebPage's API ? Right now it is very open and can contain a mix and match of file extensions (which QFileDialog
    // can work with) and mimetypes ranging from text/plain or images/* to application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
    Q_UNUSED(acceptedMimeTypes);
    QStringList ret;
    QString str;
    switch (static_cast<FilePickerController::FileChooserMode>(mode)) {
    case FilePickerController::OpenMultiple:
        ret = QFileDialog::getOpenFileNames(view(), QString());
        break;
    // Chromium extension, not exposed as part of the public API for now.
    case FilePickerController::UploadFolder:
        str = QFileDialog::getExistingDirectory(view(), tr("Select folder to upload"));
        if (!str.isNull())
            ret << str;
        break;
    case FilePickerController::Save:
        str = QFileDialog::getSaveFileName(view(), QString(), (QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + oldFiles.first()));
        if (!str.isNull())
            ret << str;
        break;
    case FilePickerController::Open:
        str = QFileDialog::getOpenFileName(view(), QString(), oldFiles.first());
        if (!str.isNull())
            ret << str;
        break;
    }
    return ret;
#else
    Q_UNUSED(mode);
    Q_UNUSED(oldFiles);
    Q_UNUSED(acceptedMimeTypes);

    return QStringList();
#endif // QT_CONFIG(filedialog)
}

void QWebEnginePage::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
#if QT_CONFIG(messagebox)
    QMessageBox::information(view(), QStringLiteral("Javascript Alert - %1").arg(url().toString()), msg);
#else
    Q_UNUSED(msg);
#endif // QT_CONFIG(messagebox)
}

bool QWebEnginePage::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
{
    Q_UNUSED(securityOrigin);
#if QT_CONFIG(messagebox)
    return (QMessageBox::information(view(), QStringLiteral("Javascript Confirm - %1").arg(url().toString()), msg, QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok);
#else
    Q_UNUSED(msg);
    return false;
#endif // QT_CONFIG(messagebox)
}

bool QWebEnginePage::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString *result)
{
    Q_UNUSED(securityOrigin);
#if QT_CONFIG(inputdialog)
    bool ret = false;
    if (result)
        *result = QInputDialog::getText(view(), QStringLiteral("Javascript Prompt - %1").arg(url().toString()), msg, QLineEdit::Normal, defaultValue, &ret);
    return ret;
#else
    Q_UNUSED(msg);
    Q_UNUSED(defaultValue);
    Q_UNUSED(result);
    return false;
#endif // QT_CONFIG(inputdialog)
}

void QWebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    static QLoggingCategory loggingCategory("js", QtWarningMsg);
    static QByteArray file = sourceID.toUtf8();
    QMessageLogger logger(file.constData(), lineNumber, nullptr, loggingCategory.categoryName());

    switch (level) {
    case JavaScriptConsoleMessageLevel::InfoMessageLevel:
        if (loggingCategory.isInfoEnabled())
            logger.info().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::WarningMessageLevel:
        if (loggingCategory.isWarningEnabled())
            logger.warning().noquote() << message;
        break;
    case JavaScriptConsoleMessageLevel::ErrorMessageLevel:
        if (loggingCategory.isCriticalEnabled())
            logger.critical().noquote() << message;
        break;
    }
}

bool QWebEnginePage::certificateError(const QWebEngineCertificateError &)
{
    return false;
}

bool QWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(url);
    Q_UNUSED(type);
    Q_UNUSED(isMainFrame);
    return true;
}

QPointF QWebEnginePage::scrollPosition() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->lastScrollOffset();
}

QSizeF QWebEnginePage::contentsSize() const
{
    Q_D(const QWebEnginePage);
    return d->adapter->lastContentsSize();
}

/*!
    Renders the current content of the page into a PDF document and saves it
    in the location specified in \a filePath.
    The page size and orientation of the produced PDF document are taken from
    the values specified in \a pageLayout.

    This method issues an asynchronous request for printing the web page into
    a PDF and returns immediately.
    To be informed about the result of the request, connect to the signal
    pdfPrintingFinished().

    If a file already exists at the provided file path, it will be overwritten.
    \since 5.7
    \sa pdfPrintingFinished()
*/
void QWebEnginePage::printToPdf(const QString &filePath, const QPageLayout &pageLayout)
{
#if QT_CONFIG(webengine_printing_and_pdf)
    Q_D(const QWebEnginePage);
    if (d->currentPrinter) {
        qWarning("Cannot print to PDF while at the same time printing on printer %ls", qUtf16Printable(d->currentPrinter->printerName()));
        return;
    }
    d->ensureInitialized();
    d->adapter->printToPDF(pageLayout, filePath);
#else
    Q_UNUSED(filePath);
    Q_UNUSED(pageLayout);
#endif
}


/*!
    Renders the current content of the page into a PDF document and returns a byte array containing the PDF data
    as parameter to \a resultCallback.
    The page size and orientation of the produced PDF document are taken from the values specified in \a pageLayout.

    The \a resultCallback must take a const reference to a QByteArray as parameter. If printing was successful, this byte array
    will contain the PDF data, otherwise, the byte array will be empty.

    \warning We guarantee that the callback (\a resultCallback) is always called, but it might be done
    during page destruction. When QWebEnginePage is deleted, the callback is triggered with an invalid
    value and it is not safe to use the corresponding QWebEnginePage or QWebEngineView instance inside it.

    \since 5.7
*/
void QWebEnginePage::printToPdf(const QWebEngineCallback<const QByteArray&> &resultCallback, const QPageLayout &pageLayout)
{
    Q_D(QWebEnginePage);
#if QT_CONFIG(webengine_printing_and_pdf)
    if (d->currentPrinter) {
        qWarning("Cannot print to PDF while at the same time printing on printer %ls", qUtf16Printable(d->currentPrinter->printerName()));
        d->m_callbacks.invokeEmpty(resultCallback);
        return;
    }
    d->ensureInitialized();
    quint64 requestId = d->adapter->printToPDFCallbackResult(pageLayout);
    d->m_callbacks.registerCallback(requestId, resultCallback);
#else
    Q_UNUSED(pageLayout);
    d->m_callbacks.invokeEmpty(resultCallback);
#endif
}

/*!
    Renders the current content of the page into a temporary PDF document, then prints it using \a printer.

    The settings for creating and printing the PDF document will be retrieved from the \a printer
    object.
    It is the users responsibility to ensure the \a printer remains valid until \a resultCallback
    has been called.

    \note The rendering of the current content into a temporary PDF document is asynchronous and does
    not block the main thread. However, the subsequent rendering of PDF into \a printer runs on the
    main thread and will therefore block the event loop. Moreover, printing runs on the browser
    process, which is by default not sandboxed.

    The \a resultCallback must take a boolean as parameter. If printing was successful, this
    boolean will have the value \c true, otherwise, its value will be \c false.

    \warning We guarantee that the callback (\a resultCallback) is always called, but it might be done
    during page destruction. When QWebEnginePage is deleted, the callback is triggered with an invalid
    value and it is not safe to use the corresponding QWebEnginePage or QWebEngineView instance inside it.

    \since 5.8
*/
void QWebEnginePage::print(QPrinter *printer, const QWebEngineCallback<bool> &resultCallback)
{
    Q_D(QWebEnginePage);
#if QT_CONFIG(webengine_printing_and_pdf)
    if (d->currentPrinter) {
        qWarning("Cannot print page on printer %ls: Already printing on %ls.", qUtf16Printable(printer->printerName()), qUtf16Printable(d->currentPrinter->printerName()));
        d->m_callbacks.invokeDirectly(resultCallback, false);
        return;
    }
    d->currentPrinter = printer;
    d->ensureInitialized();
    quint64 requestId = d->adapter->printToPDFCallbackResult(printer->pageLayout(),
                                                             printer->colorMode() == QPrinter::Color,
                                                             false);
    d->m_callbacks.registerCallback(requestId, resultCallback);
#else
    Q_UNUSED(printer);
    d->m_callbacks.invokeDirectly(resultCallback, false);
#endif
}

/*!
    \since 5.7

    Returns additional data about the current context menu. It is only guaranteed to be valid during the call to the QWebEngineView::contextMenuEvent()
    handler of the associated QWebEngineView.

    \sa createStandardContextMenu()
*/
const QWebEngineContextMenuData &QWebEnginePage::contextMenuData() const
{
    Q_D(const QWebEnginePage);
    return d->contextData;
}

#if QT_CONFIG(action)
QContextMenuBuilder::QContextMenuBuilder(const QtWebEngineCore::WebEngineContextMenuData &data,
                                         QWebEnginePage *page,
                                         QMenu *menu)
    : QtWebEngineCore::RenderViewContextMenuQt(data)
    , m_page(page)
    , m_menu(menu)
{
}

bool QContextMenuBuilder::hasInspector()
{
    return m_page->d_ptr->adapter->hasInspector();
}

bool QContextMenuBuilder::isFullScreenMode()
{
    return m_page->d_ptr->isFullScreenMode();
}

void QContextMenuBuilder::addMenuItem(ContextMenuItem menuItem)
{
    QPointer<QWebEnginePage> thisRef(m_page);
    QAction *action = 0;

    switch (menuItem) {
    case ContextMenuItem::Back:
        action = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), QWebEnginePage::tr("&Back"), m_menu);
        QObject::connect(action, &QAction::triggered, thisRef->d_ptr->view, &QWebEngineView::back);
        break;
    case ContextMenuItem::Forward:
        action = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), QWebEnginePage::tr("&Forward"), m_menu);
        QObject::connect(action, &QAction::triggered, thisRef->d_ptr->view, &QWebEngineView::forward);
        break;
    case ContextMenuItem::Reload:
        action = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")), QWebEnginePage::tr("&Reload"), m_menu);
        QObject::connect(action, &QAction::triggered, thisRef->d_ptr->view, &QWebEngineView::reload);
        break;
    case ContextMenuItem::Cut:
        action = thisRef->action(QWebEnginePage::Cut);
        break;
    case ContextMenuItem::Copy:
        action = thisRef->action(QWebEnginePage::Copy);
        break;
    case ContextMenuItem::Paste:
        action = thisRef->action(QWebEnginePage::Paste);
        break;
    case ContextMenuItem::Undo:
        action = thisRef->action(QWebEnginePage::Undo);
        break;
    case ContextMenuItem::Redo:
        action = thisRef->action(QWebEnginePage::Redo);
        break;
    case ContextMenuItem::SelectAll:
        action = thisRef->action(QWebEnginePage::SelectAll);
        break;
    case ContextMenuItem::PasteAndMatchStyle:
        action = thisRef->action(QWebEnginePage::PasteAndMatchStyle);
        break;
    case ContextMenuItem::OpenLinkInNewWindow:
        action = thisRef->action(QWebEnginePage::OpenLinkInNewWindow);
        break;
    case ContextMenuItem::OpenLinkInNewTab:
        action = thisRef->action(QWebEnginePage::OpenLinkInNewTab);
        break;
    case ContextMenuItem::CopyLinkToClipboard:
        action = thisRef->action(QWebEnginePage::CopyLinkToClipboard);
        break;
    case ContextMenuItem::DownloadLinkToDisk:
        action = thisRef->action(QWebEnginePage::DownloadLinkToDisk);
        break;
    case ContextMenuItem::CopyImageToClipboard:
        action = thisRef->action(QWebEnginePage::CopyImageToClipboard);
        break;
    case ContextMenuItem::CopyImageUrlToClipboard:
        action = thisRef->action(QWebEnginePage::CopyImageUrlToClipboard);
        break;
    case ContextMenuItem::DownloadImageToDisk:
        action = thisRef->action(QWebEnginePage::DownloadImageToDisk);
        break;
    case ContextMenuItem::CopyMediaUrlToClipboard:
        action = thisRef->action(QWebEnginePage::CopyMediaUrlToClipboard);
        break;
    case ContextMenuItem::ToggleMediaControls:
        action = thisRef->action(QWebEnginePage::ToggleMediaControls);
        break;
    case ContextMenuItem::ToggleMediaLoop:
        action = thisRef->action(QWebEnginePage::ToggleMediaLoop);
        break;
    case ContextMenuItem::DownloadMediaToDisk:
        action = thisRef->action(QWebEnginePage::DownloadMediaToDisk);
        break;
    case ContextMenuItem::InspectElement:
        action = thisRef->action(QWebEnginePage::InspectElement);
        break;
    case ContextMenuItem::ExitFullScreen:
        action = thisRef->action(QWebEnginePage::ExitFullScreen);
        break;
    case ContextMenuItem::SavePage:
        action = thisRef->action(QWebEnginePage::SavePage);
        break;
    case ContextMenuItem::ViewSource:
        action = thisRef->action(QWebEnginePage::ViewSource);
        break;
    case ContextMenuItem::SpellingSuggestions:
        for (int i=0; i < m_contextData.spellCheckerSuggestions().count() && i < 4; i++) {
            action = new QAction(m_menu);
            QString replacement = m_contextData.spellCheckerSuggestions().at(i);
            QObject::connect(action, &QAction::triggered, [thisRef, replacement] { if (thisRef) thisRef->replaceMisspelledWord(replacement); });
            action->setText(replacement);
            m_menu->addAction(action);
        }
        return;
    case ContextMenuItem::Separator:
        if (!m_menu->isEmpty())
            m_menu->addSeparator();
        return;
    }
    action->setEnabled(isMenuItemEnabled(menuItem));
    m_menu->addAction(action);
}

bool QContextMenuBuilder::isMenuItemEnabled(ContextMenuItem menuItem)
{
    switch (menuItem) {
    case ContextMenuItem::Back:
        return m_page->d_ptr->adapter->canGoBack();
    case ContextMenuItem::Forward:
        return m_page->d_ptr->adapter->canGoForward();
    case ContextMenuItem::Reload:
        return true;
    case ContextMenuItem::Cut:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanCut;
    case ContextMenuItem::Copy:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanCopy;
    case ContextMenuItem::Paste:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanPaste;
    case ContextMenuItem::Undo:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanUndo;
    case ContextMenuItem::Redo:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanRedo;
    case ContextMenuItem::SelectAll:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanSelectAll;
    case ContextMenuItem::PasteAndMatchStyle:
        return m_contextData.editFlags() & QtWebEngineCore::WebEngineContextMenuData::CanPaste;
    case ContextMenuItem::OpenLinkInNewWindow:
    case ContextMenuItem::OpenLinkInNewTab:
    case ContextMenuItem::CopyLinkToClipboard:
    case ContextMenuItem::DownloadLinkToDisk:
    case ContextMenuItem::CopyImageToClipboard:
    case ContextMenuItem::CopyImageUrlToClipboard:
    case ContextMenuItem::DownloadImageToDisk:
    case ContextMenuItem::CopyMediaUrlToClipboard:
    case ContextMenuItem::ToggleMediaControls:
    case ContextMenuItem::ToggleMediaLoop:
    case ContextMenuItem::DownloadMediaToDisk:
    case ContextMenuItem::InspectElement:
    case ContextMenuItem::ExitFullScreen:
    case ContextMenuItem::SavePage:
        return true;
    case ContextMenuItem::ViewSource:
        return m_page->d_ptr->adapter->canViewSource();
    case ContextMenuItem::SpellingSuggestions:
    case ContextMenuItem::Separator:
        return true;
    }
    Q_UNREACHABLE();
}
#endif // QT_CONFIG(action)

QT_END_NAMESPACE

#include "moc_qwebenginepage.cpp"
