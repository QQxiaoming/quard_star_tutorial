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

#ifndef QWEBENGINEPAGE_P_H
#define QWEBENGINEPAGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwebenginepage.h"

#include "qwebenginecallback_p.h"
#include "qwebenginecontextmenudata.h"
#include "qwebenginescriptcollection.h"
#include "render_view_context_menu_qt.h"
#include "web_contents_adapter_client.h"

#include <QtCore/qcompilerdetection.h>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

namespace QtWebEngineCore {
class RenderWidgetHostViewQtDelegate;
class RenderWidgetHostViewQtDelegateWidget;
class WebContentsAdapter;
}

QT_BEGIN_NAMESPACE
class QWebEngineHistory;
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineSettings;
class QWebEngineView;

QWebEnginePage::WebAction editorActionForKeyEvent(QKeyEvent* event);

class QWebEnginePagePrivate : public QtWebEngineCore::WebContentsAdapterClient
{
public:
    Q_DECLARE_PUBLIC(QWebEnginePage)
    QWebEnginePage *q_ptr;

    QWebEnginePagePrivate(QWebEngineProfile *profile = 0);
    ~QWebEnginePagePrivate();

    QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override;
    QtWebEngineCore::RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(QtWebEngineCore::RenderWidgetHostViewQtDelegateClient *client) override { return CreateRenderWidgetHostViewQtDelegate(client); }
    void initializationFinished() override;
    void titleChanged(const QString&) override;
    void urlChanged(const QUrl&) override;
    void iconChanged(const QUrl&) override;
    void loadProgressChanged(int progress) override;
    void didUpdateTargetURL(const QUrl&) override;
    void selectionChanged() override;
    void recentlyAudibleChanged(bool recentlyAudible) override;
    QRectF viewportRect() const override;
    qreal dpiScale() const override;
    QColor backgroundColor() const override;
    void loadStarted(const QUrl &provisionalUrl, bool isErrorPage = false) override;
    void loadCommitted() override { }
    void loadVisuallyCommitted() override { }
    void loadFinished(bool success, const QUrl &url, bool isErrorPage = false, int errorCode = 0, const QString &errorDescription = QString()) override;
    void focusContainer() override;
    void unhandledKeyEvent(QKeyEvent *event) override;
    void adoptNewWindow(QSharedPointer<QtWebEngineCore::WebContentsAdapter> newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect &initialGeometry, const QUrl &targetUrl) override;
    void adoptNewWindowImpl(QWebEnginePage *newPage,
            const QSharedPointer<QtWebEngineCore::WebContentsAdapter> &newWebContents,
            const QRect &initialGeometry);
    bool isBeingAdopted() override;
    void close() override;
    void windowCloseRejected() override;
    void contextMenuRequested(const QtWebEngineCore::WebEngineContextMenuData &data) override;
    void navigationRequested(int navigationType, const QUrl &url, int &navigationRequestAction, bool isMainFrame) override;
    void requestFullScreenMode(const QUrl &origin, bool fullscreen) override;
    bool isFullScreenMode() const override;
    void javascriptDialog(QSharedPointer<QtWebEngineCore::JavaScriptDialogController>) override;
    void runFileChooser(QSharedPointer<QtWebEngineCore::FilePickerController>) override;
    void showColorDialog(QSharedPointer<QtWebEngineCore::ColorChooserController>) override;
    void didRunJavaScript(quint64 requestId, const QVariant& result) override;
    void didFetchDocumentMarkup(quint64 requestId, const QString& result) override;
    void didFetchDocumentInnerText(quint64 requestId, const QString& result) override;
    void didFindText(quint64 requestId, int matchCount) override;
    void didPrintPage(quint64 requestId, const QByteArray &result) override;
    void didPrintPageToPdf(const QString &filePath, bool success) override;
    bool passOnFocus(bool reverse) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) override;
    void authenticationRequired(QSharedPointer<QtWebEngineCore::AuthenticationDialogController>) override;
    void releaseProfile() override;
    void runMediaAccessPermissionRequest(const QUrl &securityOrigin, MediaRequestFlags requestFlags) override;
    void runGeolocationPermissionRequest(const QUrl &securityOrigin) override;
    void runMouseLockPermissionRequest(const QUrl &securityOrigin) override;
    void runQuotaRequest(QWebEngineQuotaRequest) override;
    void runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest) override;
    QObject *accessibilityParentObject() override;
    QtWebEngineCore::WebEngineSettings *webEngineSettings() const override;
    void allowCertificateError(const QSharedPointer<CertificateErrorController> &controller) override;
    void selectClientCert(const QSharedPointer<ClientCertSelectController> &controller) override;
    void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode) override;
    void requestGeometryChange(const QRect &geometry, const QRect &frameGeometry) override;
    void updateScrollPosition(const QPointF &position) override;
    void updateContentsSize(const QSizeF &size) override;
    void updateNavigationActions() override;
    void updateEditActions() override;
    void startDragging(const content::DropData &dropData, Qt::DropActions allowedActions,
                       const QPixmap &pixmap, const QPoint &offset) override;
    bool supportsDragging() const override;
    bool isEnabled() const override;
    void setToolTip(const QString &toolTipText) override;
    void printRequested() override;
    const QObject *holdingQObject() const override;
    ClientType clientType() override { return QtWebEngineCore::WebContentsAdapterClient::WidgetsClient; }
    void widgetChanged(QtWebEngineCore::RenderWidgetHostViewQtDelegate *newWidget) override;

    QtWebEngineCore::ProfileAdapter *profileAdapter() override;
    QtWebEngineCore::WebContentsAdapter *webContentsAdapter() override;

    void updateAction(QWebEnginePage::WebAction) const;
    void _q_webActionTriggered(bool checked);

    void wasShown();
    void wasHidden();

    QtWebEngineCore::WebContentsAdapter *webContents() { return adapter.data(); }
    void recreateFromSerializedHistory(QDataStream &input);

    void setFullScreenMode(bool);
    void ensureInitialized() const;

    static void bindPageAndView(QWebEnginePage *page, QWebEngineView *view);
    static void bindPageAndWidget(QWebEnginePage *page,
                                  QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *widget);

    QSharedPointer<QtWebEngineCore::WebContentsAdapter> adapter;
    QWebEngineHistory *history;
    QWebEngineProfile *profile;
    QWebEngineSettings *settings;
    QWebEngineView *view;
    QUrl explicitUrl;
    QWebEngineContextMenuData contextData;
    bool isLoading;
    QWebEngineScriptCollection scriptCollection;
    bool m_isBeingAdopted;
    QColor m_backgroundColor;
    bool fullscreenMode;
    QWebChannel *webChannel;
    unsigned int webChannelWorldId;
    QUrl iconUrl;
    bool m_navigationActionTriggered;
    QPointer<QWebEnginePage> inspectedPage;
    QPointer<QWebEnginePage> devToolsPage;
    bool defaultAudioMuted;
    qreal defaultZoomFactor;
    QTimer wasShownTimer;
    QtWebEngineCore::RenderWidgetHostViewQtDelegateWidget *widget = nullptr;

    mutable QtWebEngineCore::CallbackDirectory m_callbacks;
    mutable QAction *actions[QWebEnginePage::WebActionCount];
#if QT_CONFIG(webengine_printing_and_pdf)
    QPrinter *currentPrinter;
#endif
};

class QContextMenuBuilder : public QtWebEngineCore::RenderViewContextMenuQt
{
public:
    QContextMenuBuilder(const QtWebEngineCore::WebEngineContextMenuData &data, QWebEnginePage *page, QMenu *menu);

private:
    virtual bool hasInspector() override;
    virtual bool isFullScreenMode() override;

    virtual void addMenuItem(ContextMenuItem entry) override;
    virtual bool isMenuItemEnabled(ContextMenuItem entry) override;

    QWebEnginePage *m_page;
    QMenu *m_menu;
};

QT_END_NAMESPACE

#endif // QWEBENGINEPAGE_P_H
