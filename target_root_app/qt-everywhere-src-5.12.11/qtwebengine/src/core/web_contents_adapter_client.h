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

#ifndef WEB_CONTENTS_ADAPTER_CLIENT_H
#define WEB_CONTENTS_ADAPTER_CLIENT_H

#include "qtwebenginecoreglobal_p.h"

#include <QFlags>
#include <QRect>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QUrl>

QT_FORWARD_DECLARE_CLASS(CertificateErrorController)
QT_FORWARD_DECLARE_CLASS(ClientCertSelectController)
QT_FORWARD_DECLARE_CLASS(QKeyEvent)
QT_FORWARD_DECLARE_CLASS(QVariant)
QT_FORWARD_DECLARE_CLASS(QWebEngineQuotaRequest)
QT_FORWARD_DECLARE_CLASS(QWebEngineRegisterProtocolHandlerRequest)

namespace content {
struct DropData;
}

namespace QtWebEngineCore {

class AuthenticationDialogController;
class ProfileAdapter;
class ColorChooserController;
class FilePickerController;
class JavaScriptDialogController;
class RenderWidgetHostViewQt;
class RenderWidgetHostViewQtDelegate;
class RenderWidgetHostViewQtDelegateClient;
class WebContentsAdapter;
class WebContentsDelegateQt;
class WebEngineSettings;

// Must match blink::WebReferrerPolicy
enum class ReferrerPolicy {
    Always,
    Default,
    NoReferrerWhenDowngrade,
    Never,
    Origin,
    OriginWhenCrossOrigin,
    NoReferrerWhenDowngradeOriginWhenCrossOrigin,
    SameOrigin,
    StrictOrigin,
    Last = StrictOrigin,
};

class WebEngineContextMenuSharedData : public QSharedData {

public:
    WebEngineContextMenuSharedData()
        :  hasImageContent(false)
        , isEditable(false)
        , isSpellCheckerEnabled(false)
        , mediaType(0)
        , mediaFlags(0)
        , editFlags(0)
    {
    }
    bool hasImageContent;
    bool isEditable;
    bool isSpellCheckerEnabled;
    uint mediaType;
    uint mediaFlags;
    uint editFlags;
    QPoint pos;
    QUrl linkUrl;
    QUrl unfilteredLinkUrl;
    QUrl mediaUrl;
    QString linkText;
    QString selectedText;
    QString suggestedFileName;
    QString misspelledWord;
    QStringList spellCheckerSuggestions;
    QUrl pageUrl;
    QUrl frameUrl;
    ReferrerPolicy referrerPolicy = ReferrerPolicy::Default;
    // Some likely candidates for future additions as we add support for the related actions:
    //    bool isImageBlocked;
    //    <enum tbd> mediaType;
    //    ...
};

class WebEngineContextMenuData {
public:
    // Must match blink::WebContextMenuData::MediaType:
    enum MediaType {
        // No special node is in context.
        MediaTypeNone = 0x0,
        // An image node is selected.
        MediaTypeImage,
        // A video node is selected.
        MediaTypeVideo,
        // An audio node is selected.
        MediaTypeAudio,
        // A canvas node is selected.
        MediaTypeCanvas,
        // A file node is selected.
        MediaTypeFile,
        // A plugin node is selected.
        MediaTypePlugin,
        MediaTypeLast = MediaTypePlugin
    };
    // Must match blink::WebContextMenuData::MediaFlags:
    enum MediaFlags {
        MediaNone = 0x0,
        MediaInError = 0x1,
        MediaPaused = 0x2,
        MediaMuted = 0x4,
        MediaLoop = 0x8,
        MediaCanSave = 0x10,
        MediaHasAudio = 0x20,
        MediaCanToggleControls = 0x40,
        MediaControls = 0x80,
        MediaCanPrint = 0x100,
        MediaCanRotate = 0x200,
    };

    // Must match blink::WebContextMenuData::EditFlags:
    enum EditFlags {
        CanDoNone = 0x0,
        CanUndo = 0x1,
        CanRedo = 0x2,
        CanCut = 0x4,
        CanCopy = 0x8,
        CanPaste = 0x10,
        CanDelete = 0x20,
        CanSelectAll = 0x40,
        CanTranslate = 0x80,
        CanEditRichly = 0x100,
    };

    WebEngineContextMenuData():d(new WebEngineContextMenuSharedData) {
    }

    void setPosition(const QPoint &pos) {
        d->pos = pos;
    }

    QPoint position() const {
        return d->pos;
    }

    void setLinkUrl(const QUrl &url) {
        d->linkUrl = url;
    }

    QUrl linkUrl() const {
        return d->linkUrl;
    }

    void setUnfilteredLinkUrl(const QUrl &url) {
        d->unfilteredLinkUrl = url;
    }

    QUrl unfilteredLinkUrl() const {
        return d->unfilteredLinkUrl;
    }

    void setLinkText(const QString &text) {
        d->linkText = text;
    }

    QString linkText() const {
        return d->linkText;
    }

    void setSelectedText(const QString &text) {
        d->selectedText = text;
    }

    QString selectedText() const {
        return d->selectedText;
    }

    void setMediaUrl(const QUrl &url) {
        d->mediaUrl = url;
    }

    QUrl mediaUrl() const {
        return d->mediaUrl;
    }

    void setMediaType(MediaType type) {
        d->mediaType = type;
    }

    MediaType mediaType() const {
        return MediaType(d->mediaType);
    }

    void setHasImageContent(bool imageContent) {
        d->hasImageContent = imageContent;
    }

    bool hasImageContent() const {
        return d->hasImageContent;
    }

    void setMediaFlags(MediaFlags flags) {
        d->mediaFlags = flags;
    }

    MediaFlags mediaFlags() const {
        return MediaFlags(d->mediaFlags);
    }

    void setEditFlags(EditFlags flags) {
        d->editFlags = flags;
    }

    EditFlags editFlags() const {
        return EditFlags(d->editFlags);
    }

    void setSuggestedFileName(const QString &filename) {
        d->suggestedFileName = filename;
    }

    QString suggestedFileName() const {
        return d->suggestedFileName;
    }

    void setIsEditable(bool editable) {
        d->isEditable = editable;
    }

    bool isEditable() const {
        return d->isEditable;
    }

    void setIsSpellCheckerEnabled(bool spellCheckerEnabled) {
        d->isSpellCheckerEnabled = spellCheckerEnabled;
    }

    bool isSpellCheckerEnabled() const {
        return d->isSpellCheckerEnabled;
    }

    void setMisspelledWord(const QString &word) {
        d->misspelledWord = word;
    }

    QString misspelledWord() const {
        return d->misspelledWord;
    }

    void setSpellCheckerSuggestions(const QStringList &suggestions) {
        d->spellCheckerSuggestions = suggestions;
    }

    QStringList spellCheckerSuggestions() const {
        return d->spellCheckerSuggestions;
    }

    void setFrameUrl(const QUrl &url) {
        d->frameUrl = url;
    }

    QUrl frameUrl() const {
        return d->frameUrl;
    }

    void setPageUrl(const QUrl &url) {
        d->pageUrl = url;
    }

    QUrl pageUrl() const {
        return d->pageUrl;
    }

    QUrl referrerUrl() const {
        return !d->frameUrl.isEmpty() ? d->frameUrl : d->pageUrl;
    }

    void setReferrerPolicy(ReferrerPolicy referrerPolicy) {
        d->referrerPolicy = referrerPolicy;
    }

    ReferrerPolicy referrerPolicy() const {
        return d->referrerPolicy;
    }

private:
    QSharedDataPointer<WebEngineContextMenuSharedData> d;
};


class QWEBENGINECORE_PRIVATE_EXPORT WebContentsAdapterClient {
public:
    // This must match window_open_disposition_list.h.
    enum WindowOpenDisposition {
        UnknownDisposition = 0,
        CurrentTabDisposition = 1,
        SingletonTabDisposition = 2,
        NewForegroundTabDisposition = 3,
        NewBackgroundTabDisposition = 4,
        NewPopupDisposition = 5,
        NewWindowDisposition = 6,
        SaveToDiskDisposition = 7,
        OffTheRecordDisposition = 8,
        IgnoreActionDisposition = 9,
    };

    // Must match the values in javascript_dialog_type.h.
    enum JavascriptDialogType {
        AlertDialog,
        ConfirmDialog,
        PromptDialog,
        UnloadDialog,
        // Leave room for potential new specs
        InternalAuthorizationDialog = 0x10,
    };

    enum NavigationRequestAction {
        AcceptRequest,
        // Make room in the valid range of the enum for extra actions exposed in Experimental.
        IgnoreRequest = 0xFF
    };

    enum NavigationType {
        LinkNavigation,
        TypedNavigation,
        FormSubmittedNavigation,
        BackForwardNavigation,
        ReloadNavigation,
        OtherNavigation
    };

    enum JavaScriptConsoleMessageLevel {
        Info = 0,
        Warning,
        Error
    };

    enum RenderProcessTerminationStatus {
        NormalTerminationStatus = 0,
        AbnormalTerminationStatus,
        CrashedTerminationStatus,
        KilledTerminationStatus
    };

    enum ClientType {
        QmlClient,
        WidgetsClient
    };

    enum MediaRequestFlag {
        MediaNone = 0,
        MediaAudioCapture = 0x01,
        MediaVideoCapture = 0x02,
        MediaDesktopAudioCapture = 0x04,
        MediaDesktopVideoCapture = 0x08
    };
    Q_DECLARE_FLAGS(MediaRequestFlags, MediaRequestFlag)

    virtual ~WebContentsAdapterClient() { }

    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegate(RenderWidgetHostViewQtDelegateClient *client) = 0;
    virtual RenderWidgetHostViewQtDelegate* CreateRenderWidgetHostViewQtDelegateForPopup(RenderWidgetHostViewQtDelegateClient *client) = 0;
    virtual void initializationFinished() = 0;
    virtual void titleChanged(const QString&) = 0;
    virtual void urlChanged(const QUrl&) = 0;
    virtual void iconChanged(const QUrl&) = 0;
    virtual void loadProgressChanged(int progress) = 0;
    virtual void didUpdateTargetURL(const QUrl&) = 0;
    virtual void selectionChanged() = 0;
    virtual void recentlyAudibleChanged(bool recentlyAudible) = 0;
    virtual QRectF viewportRect() const = 0;
    virtual qreal dpiScale() const = 0;
    virtual QColor backgroundColor() const = 0;
    virtual void loadStarted(const QUrl &provisionalUrl, bool isErrorPage = false) = 0;
    virtual void loadCommitted() = 0;
    virtual void loadVisuallyCommitted() = 0;
    virtual void loadFinished(bool success, const QUrl &url, bool isErrorPage = false, int errorCode = 0, const QString &errorDescription = QString()) = 0;
    virtual void focusContainer() = 0;
    virtual void unhandledKeyEvent(QKeyEvent *event) = 0;
    virtual void adoptNewWindow(QSharedPointer<WebContentsAdapter> newWebContents, WindowOpenDisposition disposition, bool userGesture, const QRect & initialGeometry, const QUrl &targetUrl) = 0;
    virtual bool isBeingAdopted() = 0;
    virtual void close() = 0;
    virtual void windowCloseRejected() = 0;
    virtual void contextMenuRequested(const WebEngineContextMenuData &) = 0;
    virtual void navigationRequested(int navigationType, const QUrl &url, int &navigationRequestAction, bool isMainFrame) = 0;
    virtual void requestFullScreenMode(const QUrl &origin, bool fullscreen) = 0;
    virtual bool isFullScreenMode() const = 0;
    virtual void javascriptDialog(QSharedPointer<JavaScriptDialogController>) = 0;
    virtual void runFileChooser(QSharedPointer<FilePickerController>) = 0;
    virtual void showColorDialog(QSharedPointer<ColorChooserController>) = 0;
    virtual void didRunJavaScript(quint64 requestId, const QVariant& result) = 0;
    virtual void didFetchDocumentMarkup(quint64 requestId, const QString& result) = 0;
    virtual void didFetchDocumentInnerText(quint64 requestId, const QString& result) = 0;
    virtual void didFindText(quint64 requestId, int matchCount) = 0;
    virtual void didPrintPage(quint64 requestId, const QByteArray &result) = 0;
    virtual void didPrintPageToPdf(const QString &filePath, bool success) = 0;
    virtual bool passOnFocus(bool reverse) = 0;
    // returns the last QObject (QWidget/QQuickItem) based object in the accessibility
    // hierarchy before going into the BrowserAccessibility tree
    virtual QObject *accessibilityParentObject() = 0;
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) = 0;
    virtual void authenticationRequired(QSharedPointer<AuthenticationDialogController>) = 0;
    virtual void runGeolocationPermissionRequest(const QUrl &securityOrigin) = 0;
    virtual void runMediaAccessPermissionRequest(const QUrl &securityOrigin, MediaRequestFlags requestFlags) = 0;
    virtual void runMouseLockPermissionRequest(const QUrl &securityOrigin) = 0;
    virtual void runQuotaRequest(QWebEngineQuotaRequest) = 0;
    virtual void runRegisterProtocolHandlerRequest(QWebEngineRegisterProtocolHandlerRequest) = 0;
    virtual WebEngineSettings *webEngineSettings() const = 0;
    RenderProcessTerminationStatus renderProcessExitStatus(int);
    virtual void renderProcessTerminated(RenderProcessTerminationStatus terminationStatus, int exitCode) = 0;
    virtual void requestGeometryChange(const QRect &geometry, const QRect &frameGeometry) = 0;
    virtual void allowCertificateError(const QSharedPointer<CertificateErrorController> &errorController) = 0;
    virtual void selectClientCert(const QSharedPointer<ClientCertSelectController> &selectController) = 0;
    virtual void updateScrollPosition(const QPointF &position) = 0;
    virtual void updateContentsSize(const QSizeF &size) = 0;
    virtual void updateNavigationActions() = 0;
    virtual void updateEditActions() = 0;
    virtual void startDragging(const content::DropData &dropData, Qt::DropActions allowedActions,
                               const QPixmap &pixmap, const QPoint &offset) = 0;
    virtual bool supportsDragging() const = 0;
    virtual bool isEnabled() const = 0;
    virtual const QObject *holdingQObject() const = 0;
    virtual void setToolTip(const QString& toolTipText) = 0;
    virtual ClientType clientType() = 0;
    virtual void printRequested() = 0;
    virtual void widgetChanged(RenderWidgetHostViewQtDelegate *newWidget) = 0;

    virtual ProfileAdapter *profileAdapter() = 0;
    virtual WebContentsAdapter* webContentsAdapter() = 0;
    virtual void releaseProfile() = 0;

};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_ADAPTER_CLIENT_H
