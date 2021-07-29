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

#ifndef WEB_CONTENTS_ADAPTER_H
#define WEB_CONTENTS_ADAPTER_H

#include "qtwebenginecoreglobal_p.h"
#include "web_contents_adapter_client.h"
#include <memory>
#include <QtGui/qtgui-config.h>
#include <QtWebEngineCore/qwebenginehttprequest.h>

#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QUrl>

namespace content {
class WebContents;
struct WebPreferences;
struct OpenURLParams;
class SiteInstance;
}

QT_BEGIN_NAMESPACE
class QAccessibleInterface;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QMimeData;
class QPageLayout;
class QString;
class QTemporaryDir;
class QWebChannel;
QT_END_NAMESPACE

namespace QtWebEngineCore {

class DevToolsFrontendQt;
class FaviconManager;
class MessagePassingInterface;
class ProfileQt;
class RenderViewObserverHostQt;
class WebChannelIPCTransportHost;
class WebEngineContext;

class QWEBENGINECORE_PRIVATE_EXPORT WebContentsAdapter : public QEnableSharedFromThis<WebContentsAdapter> {
public:
    static QSharedPointer<WebContentsAdapter> createFromSerializedNavigationHistory(QDataStream &input, WebContentsAdapterClient *adapterClient);
    WebContentsAdapter();
    WebContentsAdapter(std::unique_ptr<content::WebContents> webContents);
    ~WebContentsAdapter();

    void setClient(WebContentsAdapterClient *adapterClient);

    bool isInitialized() const;

    // These and only these methods will initialize the WebContentsAdapter. All
    // other methods below will do nothing until one of these has been called.
    void loadDefault();
    void load(const QUrl &url);
    void load(const QWebEngineHttpRequest &request);
    void setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl);

    bool canGoBack() const;
    bool canGoForward() const;
    void stop();
    void reload();
    void reloadAndBypassCache();
    void save(const QString &filePath = QString(), int savePageFormat = -1);
    QUrl activeUrl() const;
    QUrl requestedUrl() const;
    QString pageTitle() const;
    QString selectedText() const;
    QUrl iconUrl() const;

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void pasteAndMatchStyle();
    void selectAll();
    void unselect();

    void navigateToIndex(int);
    void navigateToOffset(int);
    int navigationEntryCount();
    int currentNavigationEntryIndex();
    QUrl getNavigationEntryOriginalUrl(int index);
    QUrl getNavigationEntryUrl(int index);
    QString getNavigationEntryTitle(int index);
    QDateTime getNavigationEntryTimestamp(int index);
    QUrl getNavigationEntryIconUrl(int index);
    void clearNavigationHistory();
    void serializeNavigationHistory(QDataStream &output);
    void setZoomFactor(qreal);
    qreal currentZoomFactor() const;
    void runJavaScript(const QString &javaScript, quint32 worldId);
    quint64 runJavaScriptCallbackResult(const QString &javaScript, quint32 worldId);
    quint64 fetchDocumentMarkup();
    quint64 fetchDocumentInnerText();
    quint64 findText(const QString &subString, bool caseSensitively, bool findBackward);
    void stopFinding();
    void updateWebPreferences(const content::WebPreferences &webPreferences);
    void download(const QUrl &url, const QString &suggestedFileName,
                  const QUrl &referrerUrl = QUrl(),
                  ReferrerPolicy referrerPolicy = ReferrerPolicy::Default);
    bool isAudioMuted() const;
    void setAudioMuted(bool mute);
    bool recentlyAudible();

    // Must match blink::WebMediaPlayerAction::Type.
    enum MediaPlayerAction {
        MediaPlayerNoAction,
        MediaPlayerPlay,
        MediaPlayerMute,
        MediaPlayerLoop,
        MediaPlayerControls,
        MediaPlayerTypeLast = MediaPlayerControls
    };
    void copyImageAt(const QPoint &location);
    void executeMediaPlayerActionAt(const QPoint &location, MediaPlayerAction action, bool enable);

    void inspectElementAt(const QPoint &location);
    bool hasInspector() const;
    void exitFullScreen();
    void requestClose();
    void changedFullScreen();
    void openDevToolsFrontend(QSharedPointer<WebContentsAdapter> devtoolsFrontend);
    void closeDevToolsFrontend();
    void devToolsFrontendDestroyed(DevToolsFrontendQt *frontend);

    void wasShown();
    void wasHidden();
    void grantMediaAccessPermission(const QUrl &securityOrigin, WebContentsAdapterClient::MediaRequestFlags flags);
    void runGeolocationRequestCallback(const QUrl &securityOrigin, bool allowed);
    void grantMouseLockPermission(bool granted);

    void dpiScaleChanged();
    void setBackgroundColor(const QColor &color);
    QAccessibleInterface *browserAccessible();
    ProfileQt* profile();
    ProfileAdapter* profileAdapter();
#if QT_CONFIG(webengine_webchannel)
    QWebChannel *webChannel() const;
    void setWebChannel(QWebChannel *, uint worldId);
#endif
    FaviconManager *faviconManager();

    QPointF lastScrollOffset() const;
    QSizeF lastContentsSize() const;

#if QT_CONFIG(draganddrop)
    void startDragging(QObject *dragSource, const content::DropData &dropData,
                       Qt::DropActions allowedActions, const QPixmap &pixmap, const QPoint &offset);
    void enterDrag(QDragEnterEvent *e, const QPointF &screenPos);
    Qt::DropAction updateDragPosition(QDragMoveEvent *e, const QPointF &screenPos);
    void updateDragAction(int action);
    void endDragging(QDropEvent *e, const QPointF &screenPos);
    void leaveDrag();
#endif // QT_CONFIG(draganddrop)
    void printToPDF(const QPageLayout&, const QString&);
    quint64 printToPDFCallbackResult(const QPageLayout &,
                                     bool colorMode = true,
                                     bool useCustomMargins = true);

    void replaceMisspelling(const QString &word);
    void viewSource();
    bool canViewSource();
    void focusIfNecessary();
    bool isFindTextInProgress() const;
    bool hasFocusedFrame() const;

    // meant to be used within WebEngineCore only
    void initialize(content::SiteInstance *site);
    content::WebContents *webContents() const;

private:
    Q_DISABLE_COPY(WebContentsAdapter)
    void waitForUpdateDragActionCalled();
    bool handleDropDataFileContents(const content::DropData &dropData, QMimeData *mimeData);

    ProfileAdapter *m_profileAdapter;
    std::unique_ptr<content::WebContents> m_webContents;
    std::unique_ptr<WebContentsDelegateQt> m_webContentsDelegate;
    std::unique_ptr<RenderViewObserverHostQt> m_renderViewObserverHost;
#if QT_CONFIG(webengine_webchannel)
    std::unique_ptr<WebChannelIPCTransportHost> m_webChannelTransport;
    QWebChannel *m_webChannel;
    unsigned int m_webChannelWorld;
#endif
    WebContentsAdapterClient *m_adapterClient;
    quint64 m_nextRequestId;
    int m_lastFindRequestId;
    std::unique_ptr<content::DropData> m_currentDropData;
    uint m_currentDropAction;
    bool m_updateDragActionCalled;
    QPointF m_lastDragClientPos;
    QPointF m_lastDragScreenPos;
    std::unique_ptr<QTemporaryDir> m_dndTmpDir;
    DevToolsFrontendQt *m_devToolsFrontend;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_ADAPTER_H
