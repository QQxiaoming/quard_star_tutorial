/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaType>
#include <QQmlListProperty>
#include <QtTest/QtTest>
#include <QtWebEngine/QQuickWebEngineProfile>
#include <QtWebEngine/QQuickWebEngineScript>
#include <QtWebEngineCore/QWebEngineQuotaRequest>
#include <QtWebEngineCore/QWebEngineRegisterProtocolHandlerRequest>
#include <private/qquickwebengineview_p.h>
#include <private/qquickwebengineaction_p.h>
#include <private/qquickwebenginecertificateerror_p.h>
#include <private/qquickwebenginedialogrequests_p.h>
#include <private/qquickwebenginedownloaditem_p.h>
#include <private/qquickwebenginehistory_p.h>
#include <private/qquickwebengineloadrequest_p.h>
#include <private/qquickwebenginenavigationrequest_p.h>
#include <private/qquickwebenginenewviewrequest_p.h>
#include <private/qquickwebenginesettings_p.h>
#include <private/qquickwebenginesingleton_p.h>
#include <private/qquickwebenginecontextmenurequest_p.h>

class tst_publicapi : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void publicAPI();
};

static const QList<const QMetaObject *> typesToCheck = QList<const QMetaObject *>()
    << &QQuickWebEngineView::staticMetaObject
    << &QQuickWebEngineAction::staticMetaObject
    << &QQuickWebEngineCertificateError::staticMetaObject
    << &QQuickWebEngineDownloadItem::staticMetaObject
    << &QQuickWebEngineHistory::staticMetaObject
    << &QQuickWebEngineHistoryListModel::staticMetaObject
    << &QQuickWebEngineLoadRequest::staticMetaObject
    << &QQuickWebEngineNavigationRequest::staticMetaObject
    << &QQuickWebEngineNewViewRequest::staticMetaObject
    << &QQuickWebEngineProfile::staticMetaObject
    << &QQuickWebEngineScript::staticMetaObject
    << &QQuickWebEngineSettings::staticMetaObject
    << &QQuickWebEngineFullScreenRequest::staticMetaObject
    << &QQuickWebEngineSingleton::staticMetaObject
    << &QQuickWebEngineAuthenticationDialogRequest::staticMetaObject
    << &QQuickWebEngineJavaScriptDialogRequest::staticMetaObject
    << &QQuickWebEngineColorDialogRequest::staticMetaObject
    << &QQuickWebEngineFileDialogRequest::staticMetaObject
    << &QQuickWebEngineFormValidationMessageRequest::staticMetaObject
    << &QQuickWebEngineContextMenuRequest::staticMetaObject
    << &QWebEngineQuotaRequest::staticMetaObject
    << &QWebEngineRegisterProtocolHandlerRequest::staticMetaObject
    ;

static QList<const char *> knownEnumNames = QList<const char *>();

static const QStringList hardcodedTypes = QStringList()
    << "QJSValue"
    << "QQmlListProperty<QQuickWebEngineScript>"
    << "QQmlWebChannel*"
    // Ignore the testSupport types without making a fuss.
    << "QQuickWebEngineTestSupport*"
    << "QQuickWebEngineErrorPage*"
    << "const QQuickWebEngineContextMenuData*"
    << "QWebEngineCookieStore*"
    ;

static const QStringList expectedAPI = QStringList()
    << "QQuickWebEngineAction.text --> QString"
    << "QQuickWebEngineAction.iconName --> QString"
    << "QQuickWebEngineAction.enabled --> bool"
    << "QQuickWebEngineAction.triggered() --> void"
    << "QQuickWebEngineAction.enabledChanged() --> void"
    << "QQuickWebEngineAction.trigger() --> void"
    << "QQuickWebEngineAuthenticationDialogRequest.AuthenticationTypeHTTP --> AuthenticationType"
    << "QQuickWebEngineAuthenticationDialogRequest.AuthenticationTypeProxy --> AuthenticationType"
    << "QQuickWebEngineAuthenticationDialogRequest.accepted --> bool"
    << "QQuickWebEngineAuthenticationDialogRequest.dialogAccept(QString,QString) --> void"
    << "QQuickWebEngineAuthenticationDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineAuthenticationDialogRequest.proxyHost --> QString"
    << "QQuickWebEngineAuthenticationDialogRequest.realm --> QString"
    << "QQuickWebEngineAuthenticationDialogRequest.type --> AuthenticationType"
    << "QQuickWebEngineAuthenticationDialogRequest.url --> QUrl"
    << "QQuickWebEngineCertificateError.CertificateAuthorityInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateCommonNameInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateContainsErrors --> Error"
    << "QQuickWebEngineCertificateError.CertificateDateInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateInvalid --> Error"
    << "QQuickWebEngineCertificateError.CertificateNameConstraintViolation --> Error"
    << "QQuickWebEngineCertificateError.CertificateNoRevocationMechanism --> Error"
    << "QQuickWebEngineCertificateError.CertificateNonUniqueName --> Error"
    << "QQuickWebEngineCertificateError.CertificateRevoked --> Error"
    << "QQuickWebEngineCertificateError.CertificateTransparencyRequired --> Error"
    << "QQuickWebEngineCertificateError.CertificateUnableToCheckRevocation --> Error"
    << "QQuickWebEngineCertificateError.CertificateValidityTooLong --> Error"
    << "QQuickWebEngineCertificateError.CertificateWeakKey --> Error"
    << "QQuickWebEngineCertificateError.CertificateWeakSignatureAlgorithm --> Error"
    << "QQuickWebEngineCertificateError.SslPinnedKeyNotInCertificateChain --> Error"
    << "QQuickWebEngineCertificateError.defer() --> void"
    << "QQuickWebEngineCertificateError.description --> QString"
    << "QQuickWebEngineCertificateError.error --> Error"
    << "QQuickWebEngineCertificateError.ignoreCertificateError() --> void"
    << "QQuickWebEngineCertificateError.overridable --> bool"
    << "QQuickWebEngineCertificateError.rejectCertificate() --> void"
    << "QQuickWebEngineCertificateError.url --> QUrl"
    << "QQuickWebEngineColorDialogRequest.accepted --> bool"
    << "QQuickWebEngineColorDialogRequest.color --> QColor"
    << "QQuickWebEngineContextMenuRequest.CanUndo --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanRedo --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanCut --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanCopy --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanPaste --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanDelete --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanSelectAll --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanTranslate --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.CanEditRichly --> EditFlags"
    << "QQuickWebEngineColorDialogRequest.dialogAccept(QColor) --> void"
    << "QQuickWebEngineColorDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineContextMenuRequest.editFlags --> EditFlags"
    << "QQuickWebEngineContextMenuRequest.MediaInError --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaPaused --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaMuted --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaLoop --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaCanSave --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaHasAudio --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaCanToggleControls --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaControls --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaCanPrint --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaCanRotate --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.MediaTypeAudio --> MediaType"
    << "QQuickWebEngineContextMenuRequest.MediaTypeCanvas --> MediaType"
    << "QQuickWebEngineContextMenuRequest.MediaTypeFile --> MediaType"
    << "QQuickWebEngineContextMenuRequest.MediaTypeImage --> MediaType"
    << "QQuickWebEngineContextMenuRequest.MediaTypeNone --> MediaType"
    << "QQuickWebEngineContextMenuRequest.MediaTypePlugin --> MediaType"
    << "QQuickWebEngineContextMenuRequest.MediaTypeVideo --> MediaType"
    << "QQuickWebEngineContextMenuRequest.accepted --> bool"
    << "QQuickWebEngineContextMenuRequest.isContentEditable --> bool"
    << "QQuickWebEngineContextMenuRequest.linkText --> QString"
    << "QQuickWebEngineContextMenuRequest.linkUrl --> QUrl"
    << "QQuickWebEngineContextMenuRequest.mediaFlags --> MediaFlags"
    << "QQuickWebEngineContextMenuRequest.mediaType --> MediaType"
    << "QQuickWebEngineContextMenuRequest.mediaUrl --> QUrl"
    << "QQuickWebEngineContextMenuRequest.misspelledWord --> QString"
    << "QQuickWebEngineContextMenuRequest.selectedText --> QString"
    << "QQuickWebEngineContextMenuRequest.spellCheckerSuggestions --> QStringList"
    << "QQuickWebEngineContextMenuRequest.x --> int"
    << "QQuickWebEngineContextMenuRequest.y --> int"
    << "QQuickWebEngineDownloadItem.Attachment --> DownloadType"
    << "QQuickWebEngineDownloadItem.CompleteHtmlSaveFormat --> SavePageFormat"
    << "QQuickWebEngineDownloadItem.DownloadAttribute --> DownloadType"
    << "QQuickWebEngineDownloadItem.DownloadCancelled --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadCompleted --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadInProgress --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadInterrupted --> DownloadState"
    << "QQuickWebEngineDownloadItem.DownloadRequested --> DownloadState"
    << "QQuickWebEngineDownloadItem.FileAccessDenied --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileBlocked --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileFailed --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileHashMismatch --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileNameTooLong --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileNoSpace --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileSecurityCheckFailed --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileTooLarge --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileTooShort --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileTransientError --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.FileVirusInfected --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.MimeHtmlSaveFormat --> SavePageFormat"
    << "QQuickWebEngineDownloadItem.NetworkDisconnected --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.NetworkFailed --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.NetworkInvalidRequest --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.NetworkServerDown --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.NetworkTimeout --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.NoReason --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.SavePage --> DownloadType"
    << "QQuickWebEngineDownloadItem.ServerBadContent --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.ServerCertProblem --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.ServerFailed --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.ServerForbidden --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.ServerUnauthorized --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.ServerUnreachable --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.SingleHtmlSaveFormat --> SavePageFormat"
    << "QQuickWebEngineDownloadItem.UnknownSaveFormat --> SavePageFormat"
    << "QQuickWebEngineDownloadItem.UserCanceled --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.UserRequested --> DownloadType"
    << "QQuickWebEngineDownloadItem.accept() --> void"
    << "QQuickWebEngineDownloadItem.cancel() --> void"
    << "QQuickWebEngineDownloadItem.id --> uint"
    << "QQuickWebEngineDownloadItem.interruptReason --> DownloadInterruptReason"
    << "QQuickWebEngineDownloadItem.interruptReasonChanged() --> void"
    << "QQuickWebEngineDownloadItem.interruptReasonString --> QString"
    << "QQuickWebEngineDownloadItem.isFinished --> bool"
    << "QQuickWebEngineDownloadItem.isFinishedChanged() --> void"
    << "QQuickWebEngineDownloadItem.isPaused --> bool"
    << "QQuickWebEngineDownloadItem.isPausedChanged() --> void"
    << "QQuickWebEngineDownloadItem.isSavePageDownload --> bool"
    << "QQuickWebEngineDownloadItem.mimeType --> QString"
    << "QQuickWebEngineDownloadItem.mimeTypeChanged() --> void"
    << "QQuickWebEngineDownloadItem.path --> QString"
    << "QQuickWebEngineDownloadItem.pathChanged() --> void"
    << "QQuickWebEngineDownloadItem.pause() --> void"
    << "QQuickWebEngineDownloadItem.receivedBytes --> qlonglong"
    << "QQuickWebEngineDownloadItem.receivedBytesChanged() --> void"
    << "QQuickWebEngineDownloadItem.resume() --> void"
    << "QQuickWebEngineDownloadItem.savePageFormat --> SavePageFormat"
    << "QQuickWebEngineDownloadItem.savePageFormatChanged() --> void"
    << "QQuickWebEngineDownloadItem.state --> DownloadState"
    << "QQuickWebEngineDownloadItem.stateChanged() --> void"
    << "QQuickWebEngineDownloadItem.totalBytes --> qlonglong"
    << "QQuickWebEngineDownloadItem.totalBytesChanged() --> void"
    << "QQuickWebEngineDownloadItem.type --> DownloadType"
    << "QQuickWebEngineDownloadItem.typeChanged() --> void"
    << "QQuickWebEngineDownloadItem.view --> QQuickWebEngineView*"
    << "QQuickWebEngineFileDialogRequest.FileModeOpen --> FileMode"
    << "QQuickWebEngineFileDialogRequest.FileModeOpenMultiple --> FileMode"
    << "QQuickWebEngineFileDialogRequest.FileModeSave --> FileMode"
    << "QQuickWebEngineFileDialogRequest.FileModeUploadFolder --> FileMode"
    << "QQuickWebEngineFileDialogRequest.accepted --> bool"
    << "QQuickWebEngineFileDialogRequest.acceptedMimeTypes --> QStringList"
    << "QQuickWebEngineFileDialogRequest.defaultFileName --> QString"
    << "QQuickWebEngineFileDialogRequest.dialogAccept(QStringList) --> void"
    << "QQuickWebEngineFileDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineFileDialogRequest.mode --> FileMode"
    << "QQuickWebEngineFormValidationMessageRequest.Hide --> RequestType"
    << "QQuickWebEngineFormValidationMessageRequest.Move --> RequestType"
    << "QQuickWebEngineFormValidationMessageRequest.Show --> RequestType"
    << "QQuickWebEngineFormValidationMessageRequest.accepted --> bool"
    << "QQuickWebEngineFormValidationMessageRequest.anchor --> QRect"
    << "QQuickWebEngineFormValidationMessageRequest.subText --> QString"
    << "QQuickWebEngineFormValidationMessageRequest.text --> QString"
    << "QQuickWebEngineFormValidationMessageRequest.type --> RequestType"
    << "QQuickWebEngineFullScreenRequest.accept() --> void"
    << "QQuickWebEngineFullScreenRequest.origin --> QUrl"
    << "QQuickWebEngineFullScreenRequest.reject() --> void"
    << "QQuickWebEngineFullScreenRequest.toggleOn --> bool"
    << "QQuickWebEngineHistory.backItems --> QQuickWebEngineHistoryListModel*"
    << "QQuickWebEngineHistory.forwardItems --> QQuickWebEngineHistoryListModel*"
    << "QQuickWebEngineHistory.items --> QQuickWebEngineHistoryListModel*"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypeAlert --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypeBeforeUnload --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypeConfirm --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.DialogTypePrompt --> DialogType"
    << "QQuickWebEngineJavaScriptDialogRequest.accepted --> bool"
    << "QQuickWebEngineJavaScriptDialogRequest.defaultText --> QString"
    << "QQuickWebEngineJavaScriptDialogRequest.dialogAccept() --> void"
    << "QQuickWebEngineJavaScriptDialogRequest.dialogAccept(QString) --> void"
    << "QQuickWebEngineJavaScriptDialogRequest.dialogReject() --> void"
    << "QQuickWebEngineJavaScriptDialogRequest.message --> QString"
    << "QQuickWebEngineJavaScriptDialogRequest.securityOrigin --> QUrl"
    << "QQuickWebEngineJavaScriptDialogRequest.title --> QString"
    << "QQuickWebEngineJavaScriptDialogRequest.type --> DialogType"
    << "QQuickWebEngineLoadRequest.errorCode --> int"
    << "QQuickWebEngineLoadRequest.errorDomain --> QQuickWebEngineView::ErrorDomain"
    << "QQuickWebEngineLoadRequest.errorString --> QString"
    << "QQuickWebEngineLoadRequest.status --> QQuickWebEngineView::LoadStatus"
    << "QQuickWebEngineLoadRequest.url --> QUrl"
    << "QQuickWebEngineNavigationRequest.action --> QQuickWebEngineView::NavigationRequestAction"
    << "QQuickWebEngineNavigationRequest.actionChanged() --> void"
    << "QQuickWebEngineNavigationRequest.isMainFrame --> bool"
    << "QQuickWebEngineNavigationRequest.navigationType --> QQuickWebEngineView::NavigationType"
    << "QQuickWebEngineNavigationRequest.url --> QUrl"
    << "QQuickWebEngineNewViewRequest.destination --> QQuickWebEngineView::NewViewDestination"
    << "QQuickWebEngineNewViewRequest.openIn(QQuickWebEngineView*) --> void"
    << "QQuickWebEngineNewViewRequest.requestedUrl --> QUrl"
    << "QQuickWebEngineNewViewRequest.userInitiated --> bool"
    << "QQuickWebEngineProfile.AllowPersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.DiskHttpCache --> HttpCacheType"
    << "QQuickWebEngineProfile.ForcePersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.MemoryHttpCache --> HttpCacheType"
    << "QQuickWebEngineProfile.NoCache --> HttpCacheType"
    << "QQuickWebEngineProfile.NoPersistentCookies --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.cachePath --> QString"
    << "QQuickWebEngineProfile.cachePathChanged() --> void"
    << "QQuickWebEngineProfile.clearHttpCache() --> void"
    << "QQuickWebEngineProfile.downloadFinished(QQuickWebEngineDownloadItem*) --> void"
    << "QQuickWebEngineProfile.downloadRequested(QQuickWebEngineDownloadItem*) --> void"
    << "QQuickWebEngineProfile.httpAcceptLanguage --> QString"
    << "QQuickWebEngineProfile.httpAcceptLanguageChanged() --> void"
    << "QQuickWebEngineProfile.httpCacheMaximumSize --> int"
    << "QQuickWebEngineProfile.httpCacheMaximumSizeChanged() --> void"
    << "QQuickWebEngineProfile.httpCacheType --> HttpCacheType"
    << "QQuickWebEngineProfile.httpCacheTypeChanged() --> void"
    << "QQuickWebEngineProfile.httpUserAgent --> QString"
    << "QQuickWebEngineProfile.httpUserAgentChanged() --> void"
    << "QQuickWebEngineProfile.offTheRecord --> bool"
    << "QQuickWebEngineProfile.offTheRecordChanged() --> void"
    << "QQuickWebEngineProfile.persistentCookiesPolicy --> PersistentCookiesPolicy"
    << "QQuickWebEngineProfile.persistentCookiesPolicyChanged() --> void"
    << "QQuickWebEngineProfile.persistentStoragePath --> QString"
    << "QQuickWebEngineProfile.persistentStoragePathChanged() --> void"
    << "QQuickWebEngineProfile.spellCheckEnabled --> bool"
    << "QQuickWebEngineProfile.spellCheckEnabledChanged() --> void"
    << "QQuickWebEngineProfile.spellCheckLanguages --> QStringList"
    << "QQuickWebEngineProfile.spellCheckLanguagesChanged() --> void"
    << "QQuickWebEngineProfile.storageName --> QString"
    << "QQuickWebEngineProfile.storageNameChanged() --> void"
    << "QQuickWebEngineProfile.userScripts --> QQmlListProperty<QQuickWebEngineScript>"
    << "QQuickWebEngineScript.ApplicationWorld --> ScriptWorldId"
    << "QQuickWebEngineScript.Deferred --> InjectionPoint"
    << "QQuickWebEngineScript.DocumentCreation --> InjectionPoint"
    << "QQuickWebEngineScript.DocumentReady --> InjectionPoint"
    << "QQuickWebEngineScript.MainWorld --> ScriptWorldId"
    << "QQuickWebEngineScript.UserWorld --> ScriptWorldId"
    << "QQuickWebEngineScript.injectionPoint --> InjectionPoint"
    << "QQuickWebEngineScript.injectionPointChanged(InjectionPoint) --> void"
    << "QQuickWebEngineScript.name --> QString"
    << "QQuickWebEngineScript.nameChanged(QString) --> void"
    << "QQuickWebEngineScript.runOnSubframes --> bool"
    << "QQuickWebEngineScript.runOnSubframesChanged(bool) --> void"
    << "QQuickWebEngineScript.setInjectionPoint(InjectionPoint) --> void"
    << "QQuickWebEngineScript.setName(QString) --> void"
    << "QQuickWebEngineScript.setRunOnSubframes(bool) --> void"
    << "QQuickWebEngineScript.setSourceCode(QString) --> void"
    << "QQuickWebEngineScript.setSourceUrl(QUrl) --> void"
    << "QQuickWebEngineScript.setWorldId(ScriptWorldId) --> void"
    << "QQuickWebEngineScript.sourceCode --> QString"
    << "QQuickWebEngineScript.sourceCodeChanged(QString) --> void"
    << "QQuickWebEngineScript.sourceUrl --> QUrl"
    << "QQuickWebEngineScript.sourceUrlChanged(QUrl) --> void"
    << "QQuickWebEngineScript.toString() --> QString"
    << "QQuickWebEngineScript.worldId --> ScriptWorldId"
    << "QQuickWebEngineScript.worldIdChanged(ScriptWorldId) --> void"
    << "QQuickWebEngineSettings.AllowAllUnknownUrlSchemes --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.AllowUnknownUrlSchemesFromUserInteraction --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.DisallowUnknownUrlSchemes --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.accelerated2dCanvasEnabled --> bool"
    << "QQuickWebEngineSettings.accelerated2dCanvasEnabledChanged() --> void"
    << "QQuickWebEngineSettings.allowGeolocationOnInsecureOrigins --> bool"
    << "QQuickWebEngineSettings.allowGeolocationOnInsecureOriginsChanged() --> void"
    << "QQuickWebEngineSettings.allowRunningInsecureContent --> bool"
    << "QQuickWebEngineSettings.allowRunningInsecureContentChanged() --> void"
    << "QQuickWebEngineSettings.allowWindowActivationFromJavaScript --> bool"
    << "QQuickWebEngineSettings.allowWindowActivationFromJavaScriptChanged() --> void"
    << "QQuickWebEngineSettings.autoLoadIconsForPage --> bool"
    << "QQuickWebEngineSettings.autoLoadIconsForPageChanged() --> void"
    << "QQuickWebEngineSettings.autoLoadImages --> bool"
    << "QQuickWebEngineSettings.autoLoadImagesChanged() --> void"
    << "QQuickWebEngineSettings.defaultTextEncoding --> QString"
    << "QQuickWebEngineSettings.defaultTextEncodingChanged() --> void"
    << "QQuickWebEngineSettings.dnsPrefetchEnabled --> bool"
    << "QQuickWebEngineSettings.dnsPrefetchEnabledChanged() --> void"
    << "QQuickWebEngineSettings.errorPageEnabled --> bool"
    << "QQuickWebEngineSettings.errorPageEnabledChanged() --> void"
    << "QQuickWebEngineSettings.focusOnNavigationEnabled --> bool"
    << "QQuickWebEngineSettings.focusOnNavigationEnabledChanged() --> void"
    << "QQuickWebEngineSettings.fullScreenSupportEnabled --> bool"
    << "QQuickWebEngineSettings.fullScreenSupportEnabledChanged() --> void"
    << "QQuickWebEngineSettings.hyperlinkAuditingEnabled --> bool"
    << "QQuickWebEngineSettings.hyperlinkAuditingEnabledChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanAccessClipboard --> bool"
    << "QQuickWebEngineSettings.javascriptCanAccessClipboardChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanOpenWindows --> bool"
    << "QQuickWebEngineSettings.javascriptCanOpenWindowsChanged() --> void"
    << "QQuickWebEngineSettings.javascriptCanPaste --> bool"
    << "QQuickWebEngineSettings.javascriptCanPasteChanged() --> void"
    << "QQuickWebEngineSettings.javascriptEnabled --> bool"
    << "QQuickWebEngineSettings.javascriptEnabledChanged() --> void"
    << "QQuickWebEngineSettings.linksIncludedInFocusChain --> bool"
    << "QQuickWebEngineSettings.linksIncludedInFocusChainChanged() --> void"
    << "QQuickWebEngineSettings.localContentCanAccessFileUrls --> bool"
    << "QQuickWebEngineSettings.localContentCanAccessFileUrlsChanged() --> void"
    << "QQuickWebEngineSettings.localContentCanAccessRemoteUrls --> bool"
    << "QQuickWebEngineSettings.localContentCanAccessRemoteUrlsChanged() --> void"
    << "QQuickWebEngineSettings.localStorageEnabled --> bool"
    << "QQuickWebEngineSettings.localStorageEnabledChanged() --> void"
    << "QQuickWebEngineSettings.playbackRequiresUserGesture --> bool"
    << "QQuickWebEngineSettings.playbackRequiresUserGestureChanged() --> void"
    << "QQuickWebEngineSettings.pluginsEnabled --> bool"
    << "QQuickWebEngineSettings.pluginsEnabledChanged() --> void"
    << "QQuickWebEngineSettings.printElementBackgrounds --> bool"
    << "QQuickWebEngineSettings.printElementBackgroundsChanged() --> void"
    << "QQuickWebEngineSettings.screenCaptureEnabled --> bool"
    << "QQuickWebEngineSettings.screenCaptureEnabledChanged() --> void"
    << "QQuickWebEngineSettings.showScrollBars --> bool"
    << "QQuickWebEngineSettings.showScrollBarsChanged() --> void"
    << "QQuickWebEngineSettings.spatialNavigationEnabled --> bool"
    << "QQuickWebEngineSettings.spatialNavigationEnabledChanged() --> void"
    << "QQuickWebEngineSettings.touchIconsEnabled --> bool"
    << "QQuickWebEngineSettings.touchIconsEnabledChanged() --> void"
    << "QQuickWebEngineSettings.unknownUrlSchemePolicy --> UnknownUrlSchemePolicy"
    << "QQuickWebEngineSettings.unknownUrlSchemePolicyChanged() --> void"
    << "QQuickWebEngineSettings.webGLEnabled --> bool"
    << "QQuickWebEngineSettings.webGLEnabledChanged() --> void"
    << "QQuickWebEngineSettings.webRTCPublicInterfacesOnly --> bool"
    << "QQuickWebEngineSettings.webRTCPublicInterfacesOnlyChanged() --> void"
    << "QQuickWebEngineSingleton.defaultProfile --> QQuickWebEngineProfile*"
    << "QQuickWebEngineSingleton.settings --> QQuickWebEngineSettings*"
    << "QQuickWebEngineView.action(WebAction) --> QQuickWebEngineAction*"
    << "QQuickWebEngineView.A0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A3Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4Plus --> PrintedPageSizeId"
    << "QQuickWebEngineView.A4Small --> PrintedPageSizeId"
    << "QQuickWebEngineView.A5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A5Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.A6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.A9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.AbnormalTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.AcceptRequest --> NavigationRequestAction"
    << "QQuickWebEngineView.AlignCenter --> WebAction"
    << "QQuickWebEngineView.AlignJustified --> WebAction"
    << "QQuickWebEngineView.AlignLeft --> WebAction"
    << "QQuickWebEngineView.AlignRight --> WebAction"
    << "QQuickWebEngineView.AnsiA --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiB --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiC --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiD --> PrintedPageSizeId"
    << "QQuickWebEngineView.AnsiE --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchA --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchB --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchC --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchD --> PrintedPageSizeId"
    << "QQuickWebEngineView.ArchE --> PrintedPageSizeId"
    << "QQuickWebEngineView.B0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B5Extra --> PrintedPageSizeId"
    << "QQuickWebEngineView.B6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.B9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Back --> WebAction"
    << "QQuickWebEngineView.BackForwardNavigation --> NavigationType"
    << "QQuickWebEngineView.C5E --> PrintedPageSizeId"
    << "QQuickWebEngineView.CertificateErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Comm10E --> PrintedPageSizeId"
    << "QQuickWebEngineView.ConnectionErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Copy --> WebAction"
    << "QQuickWebEngineView.CopyImageToClipboard --> WebAction"
    << "QQuickWebEngineView.CopyImageUrlToClipboard --> WebAction"
    << "QQuickWebEngineView.CopyLinkToClipboard --> WebAction"
    << "QQuickWebEngineView.CopyMediaUrlToClipboard --> WebAction"
    << "QQuickWebEngineView.CrashedTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.Custom --> PrintedPageSizeId"
    << "QQuickWebEngineView.Cut --> WebAction"
    << "QQuickWebEngineView.DLE --> PrintedPageSizeId"
    << "QQuickWebEngineView.DesktopAudioVideoCapture --> Feature"
    << "QQuickWebEngineView.DesktopVideoCapture --> Feature"
    << "QQuickWebEngineView.DnsErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.DoublePostcard --> PrintedPageSizeId"
    << "QQuickWebEngineView.DownloadImageToDisk --> WebAction"
    << "QQuickWebEngineView.DownloadLinkToDisk --> WebAction"
    << "QQuickWebEngineView.DownloadMediaToDisk --> WebAction"
    << "QQuickWebEngineView.Envelope10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope12 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope14 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Envelope9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeB4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeB5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeB6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC65 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeC7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeChou3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeChou4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeDL --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeInvite --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeItalian --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeKaku2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeKaku3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeMonarch --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePersonal --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopePrc9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.EnvelopeYou4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.ErrorMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.Executive --> PrintedPageSizeId"
    << "QQuickWebEngineView.ExecutiveStandard --> PrintedPageSizeId"
    << "QQuickWebEngineView.ExitFullScreen --> WebAction"
    << "QQuickWebEngineView.FanFoldGerman --> PrintedPageSizeId"
    << "QQuickWebEngineView.FanFoldGermanLegal --> PrintedPageSizeId"
    << "QQuickWebEngineView.FanFoldUS --> PrintedPageSizeId"
    << "QQuickWebEngineView.FindBackward --> FindFlags"
    << "QQuickWebEngineView.FindCaseSensitively --> FindFlags"
    << "QQuickWebEngineView.Folio --> PrintedPageSizeId"
    << "QQuickWebEngineView.FormSubmittedNavigation --> NavigationType"
    << "QQuickWebEngineView.Forward --> WebAction"
    << "QQuickWebEngineView.FtpErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.Geolocation --> Feature"
    << "QQuickWebEngineView.HttpErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.IgnoreRequest --> NavigationRequestAction"
    << "QQuickWebEngineView.Imperial10x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial10x13 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial10x14 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial12x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial15x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial7x9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial8x10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial9x11 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Imperial9x12 --> PrintedPageSizeId"
    << "QQuickWebEngineView.Indent --> WebAction"
    << "QQuickWebEngineView.InfoMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.InsertOrderedList --> WebAction"
    << "QQuickWebEngineView.InsertUnorderedList --> WebAction"
    << "QQuickWebEngineView.InspectElement --> WebAction"
    << "QQuickWebEngineView.InternalErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.JisB0 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB1 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB10 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB2 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB3 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB4 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB5 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB6 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB7 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB8 --> PrintedPageSizeId"
    << "QQuickWebEngineView.JisB9 --> PrintedPageSizeId"
    << "QQuickWebEngineView.KilledTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.Landscape --> PrintedPageOrientation"
    << "QQuickWebEngineView.LastPageSize --> PrintedPageSizeId"
    << "QQuickWebEngineView.Ledger --> PrintedPageSizeId"
    << "QQuickWebEngineView.Legal --> PrintedPageSizeId"
    << "QQuickWebEngineView.LegalExtra --> PrintedPageSizeId"
    << "QQuickWebEngineView.Letter --> PrintedPageSizeId"
    << "QQuickWebEngineView.LetterExtra --> PrintedPageSizeId"
    << "QQuickWebEngineView.LetterPlus --> PrintedPageSizeId"
    << "QQuickWebEngineView.LetterSmall --> PrintedPageSizeId"
    << "QQuickWebEngineView.LinkClickedNavigation --> NavigationType"
    << "QQuickWebEngineView.LoadFailedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadStartedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadStoppedStatus --> LoadStatus"
    << "QQuickWebEngineView.LoadSucceededStatus --> LoadStatus"
    << "QQuickWebEngineView.MediaAudioCapture --> Feature"
    << "QQuickWebEngineView.MediaAudioVideoCapture --> Feature"
    << "QQuickWebEngineView.MediaVideoCapture --> Feature"
    << "QQuickWebEngineView.NPageSize --> PrintedPageSizeId"
    << "QQuickWebEngineView.NPaperSize --> PrintedPageSizeId"
    << "QQuickWebEngineView.NewViewInBackgroundTab --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInDialog --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInTab --> NewViewDestination"
    << "QQuickWebEngineView.NewViewInWindow --> NewViewDestination"
    << "QQuickWebEngineView.NoErrorDomain --> ErrorDomain"
    << "QQuickWebEngineView.NoWebAction --> WebAction"
    << "QQuickWebEngineView.NormalTerminationStatus --> RenderProcessTerminationStatus"
    << "QQuickWebEngineView.Note --> PrintedPageSizeId"
    << "QQuickWebEngineView.OpenLinkInNewTab --> WebAction"
    << "QQuickWebEngineView.OpenLinkInNewWindow --> WebAction"
    << "QQuickWebEngineView.OpenLinkInThisWindow --> WebAction"
    << "QQuickWebEngineView.OtherNavigation --> NavigationType"
    << "QQuickWebEngineView.Outdent --> WebAction"
    << "QQuickWebEngineView.Paste --> WebAction"
    << "QQuickWebEngineView.PasteAndMatchStyle --> WebAction"
    << "QQuickWebEngineView.Portrait --> PrintedPageOrientation"
    << "QQuickWebEngineView.Postcard --> PrintedPageSizeId"
    << "QQuickWebEngineView.Prc16K --> PrintedPageSizeId"
    << "QQuickWebEngineView.Prc32K --> PrintedPageSizeId"
    << "QQuickWebEngineView.Prc32KBig --> PrintedPageSizeId"
    << "QQuickWebEngineView.Quarto --> PrintedPageSizeId"
    << "QQuickWebEngineView.Redo --> WebAction"
    << "QQuickWebEngineView.Reload --> WebAction"
    << "QQuickWebEngineView.ReloadAndBypassCache --> WebAction"
    << "QQuickWebEngineView.ReloadNavigation --> NavigationType"
    << "QQuickWebEngineView.RequestClose --> WebAction"
    << "QQuickWebEngineView.SavePage --> WebAction"
    << "QQuickWebEngineView.SelectAll --> WebAction"
    << "QQuickWebEngineView.Statement --> PrintedPageSizeId"
    << "QQuickWebEngineView.Stop --> WebAction"
    << "QQuickWebEngineView.SuperA --> PrintedPageSizeId"
    << "QQuickWebEngineView.SuperB --> PrintedPageSizeId"
    << "QQuickWebEngineView.Tabloid --> PrintedPageSizeId"
    << "QQuickWebEngineView.TabloidExtra --> PrintedPageSizeId"
    << "QQuickWebEngineView.ToggleBold --> WebAction"
    << "QQuickWebEngineView.ToggleItalic --> WebAction"
    << "QQuickWebEngineView.ToggleMediaControls --> WebAction"
    << "QQuickWebEngineView.ToggleMediaLoop --> WebAction"
    << "QQuickWebEngineView.ToggleMediaMute --> WebAction"
    << "QQuickWebEngineView.ToggleMediaPlayPause --> WebAction"
    << "QQuickWebEngineView.ToggleStrikethrough --> WebAction"
    << "QQuickWebEngineView.ToggleUnderline --> WebAction"
    << "QQuickWebEngineView.TypedNavigation --> NavigationType"
    << "QQuickWebEngineView.Undo --> WebAction"
    << "QQuickWebEngineView.Unselect --> WebAction"
    << "QQuickWebEngineView.ViewSource --> WebAction"
    << "QQuickWebEngineView.WarningMessageLevel --> JavaScriptConsoleMessageLevel"
    << "QQuickWebEngineView.WebActionCount --> WebAction"
    << "QQuickWebEngineView.activeFocusOnPress --> bool"
    << "QQuickWebEngineView.activeFocusOnPressChanged(bool) --> void"
    << "QQuickWebEngineView.audioMuted --> bool"
    << "QQuickWebEngineView.audioMutedChanged(bool) --> void"
    << "QQuickWebEngineView.authenticationDialogRequested(QQuickWebEngineAuthenticationDialogRequest*) --> void"
    << "QQuickWebEngineView.backgroundColor --> QColor"
    << "QQuickWebEngineView.backgroundColorChanged() --> void"
    << "QQuickWebEngineView.canGoBack --> bool"
    << "QQuickWebEngineView.canGoForward --> bool"
    << "QQuickWebEngineView.certificateError(QQuickWebEngineCertificateError*) --> void"
    << "QQuickWebEngineView.colorDialogRequested(QQuickWebEngineColorDialogRequest*) --> void"
    << "QQuickWebEngineView.contentsSize --> QSizeF"
    << "QQuickWebEngineView.contentsSizeChanged(QSizeF) --> void"
    << "QQuickWebEngineView.contextMenuRequested(QQuickWebEngineContextMenuRequest*) --> void"
    << "QQuickWebEngineView.devToolsView --> QQuickWebEngineView*"
    << "QQuickWebEngineView.devToolsViewChanged() --> void"
    << "QQuickWebEngineView.featurePermissionRequested(QUrl,Feature) --> void"
    << "QQuickWebEngineView.fileDialogRequested(QQuickWebEngineFileDialogRequest*) --> void"
    << "QQuickWebEngineView.findText(QString) --> void"
    << "QQuickWebEngineView.findText(QString,FindFlags) --> void"
    << "QQuickWebEngineView.findText(QString,FindFlags,QJSValue) --> void"
    << "QQuickWebEngineView.formValidationMessageRequested(QQuickWebEngineFormValidationMessageRequest*) --> void"
    << "QQuickWebEngineView.fullScreenCancelled() --> void"
    << "QQuickWebEngineView.fullScreenRequested(QQuickWebEngineFullScreenRequest) --> void"
    << "QQuickWebEngineView.geometryChangeRequested(QRect,QRect) --> void"
    << "QQuickWebEngineView.goBack() --> void"
    << "QQuickWebEngineView.goBackOrForward(int) --> void"
    << "QQuickWebEngineView.goForward() --> void"
    << "QQuickWebEngineView.grantFeaturePermission(QUrl,Feature,bool) --> void"
    << "QQuickWebEngineView.icon --> QUrl"
    << "QQuickWebEngineView.iconChanged() --> void"
    << "QQuickWebEngineView.inspectedView --> QQuickWebEngineView*"
    << "QQuickWebEngineView.inspectedViewChanged() --> void"
    << "QQuickWebEngineView.isFullScreen --> bool"
    << "QQuickWebEngineView.isFullScreenChanged() --> void"
    << "QQuickWebEngineView.javaScriptConsoleMessage(JavaScriptConsoleMessageLevel,QString,int,QString) --> void"
    << "QQuickWebEngineView.javaScriptDialogRequested(QQuickWebEngineJavaScriptDialogRequest*) --> void"
    << "QQuickWebEngineView.linkHovered(QUrl) --> void"
    << "QQuickWebEngineView.loadHtml(QString) --> void"
    << "QQuickWebEngineView.loadHtml(QString,QUrl) --> void"
    << "QQuickWebEngineView.loadProgress --> int"
    << "QQuickWebEngineView.loadProgressChanged() --> void"
    << "QQuickWebEngineView.loading --> bool"
    << "QQuickWebEngineView.loadingChanged(QQuickWebEngineLoadRequest*) --> void"
    << "QQuickWebEngineView.navigationHistory --> QQuickWebEngineHistory*"
    << "QQuickWebEngineView.navigationRequested(QQuickWebEngineNavigationRequest*) --> void"
    << "QQuickWebEngineView.newViewRequested(QQuickWebEngineNewViewRequest*) --> void"
    << "QQuickWebEngineView.pdfPrintingFinished(QString,bool) --> void"
    << "QQuickWebEngineView.printRequested() --> void"
    << "QQuickWebEngineView.printToPdf(QJSValue) --> void"
    << "QQuickWebEngineView.printToPdf(QJSValue,PrintedPageSizeId) --> void"
    << "QQuickWebEngineView.printToPdf(QJSValue,PrintedPageSizeId,PrintedPageOrientation) --> void"
    << "QQuickWebEngineView.printToPdf(QString) --> void"
    << "QQuickWebEngineView.printToPdf(QString,PrintedPageSizeId) --> void"
    << "QQuickWebEngineView.printToPdf(QString,PrintedPageSizeId,PrintedPageOrientation) --> void"
    << "QQuickWebEngineView.profile --> QQuickWebEngineProfile*"
    << "QQuickWebEngineView.profileChanged() --> void"
    << "QQuickWebEngineView.quotaRequested(QWebEngineQuotaRequest) --> void"
    << "QQuickWebEngineView.recentlyAudible --> bool"
    << "QQuickWebEngineView.recentlyAudibleChanged(bool) --> void"
    << "QQuickWebEngineView.registerProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest) --> void"
    << "QQuickWebEngineView.reload() --> void"
    << "QQuickWebEngineView.reloadAndBypassCache() --> void"
    << "QQuickWebEngineView.renderProcessTerminated(RenderProcessTerminationStatus,int) --> void"
    << "QQuickWebEngineView.replaceMisspelledWord(QString) --> void"
    << "QQuickWebEngineView.runJavaScript(QString) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,QJSValue) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,uint) --> void"
    << "QQuickWebEngineView.runJavaScript(QString,uint,QJSValue) --> void"
    << "QQuickWebEngineView.scrollPosition --> QPointF"
    << "QQuickWebEngineView.scrollPositionChanged(QPointF) --> void"
    << "QQuickWebEngineView.setActiveFocusOnPress(bool) --> void"
    << "QQuickWebEngineView.settings --> QQuickWebEngineSettings*"
    << "QQuickWebEngineView.stop() --> void"
#if QT_CONFIG(webengine_testsupport)
    << "QQuickWebEngineView.testSupport --> QQuickWebEngineTestSupport*"
    << "QQuickWebEngineView.testSupportChanged() --> void"
#endif
    << "QQuickWebEngineView.title --> QString"
    << "QQuickWebEngineView.titleChanged() --> void"
    << "QQuickWebEngineView.triggerWebAction(WebAction) --> void"
    << "QQuickWebEngineView.url --> QUrl"
    << "QQuickWebEngineView.urlChanged() --> void"
    << "QQuickWebEngineView.userScripts --> QQmlListProperty<QQuickWebEngineScript>"
    << "QQuickWebEngineView.webChannel --> QQmlWebChannel*"
    << "QQuickWebEngineView.webChannelChanged() --> void"
    << "QQuickWebEngineView.webChannelWorld --> uint"
    << "QQuickWebEngineView.webChannelWorldChanged(uint) --> void"
    << "QQuickWebEngineView.windowCloseRequested() --> void"
    << "QQuickWebEngineView.zoomFactor --> double"
    << "QQuickWebEngineView.zoomFactorChanged(double) --> void"
    << "QWebEngineQuotaRequest.accept() --> void"
    << "QWebEngineQuotaRequest.origin --> QUrl"
    << "QWebEngineQuotaRequest.reject() --> void"
    << "QWebEngineQuotaRequest.requestedSize --> qlonglong"
    << "QWebEngineRegisterProtocolHandlerRequest.accept() --> void"
    << "QWebEngineRegisterProtocolHandlerRequest.origin --> QUrl"
    << "QWebEngineRegisterProtocolHandlerRequest.reject() --> void"
    << "QWebEngineRegisterProtocolHandlerRequest.scheme --> QString"
    ;

static bool isCheckedEnum(const QByteArray &typeName)
{
    QList<QByteArray> tokens = typeName.split(':');
    if (tokens.size() == 3) {
        QByteArray &enumClass = tokens[0];
        QByteArray &enumName = tokens[2];
        for (const QMetaObject *mo : typesToCheck) {
            if (mo->className() != enumClass)
                continue;
            for (int i = mo->enumeratorOffset(); i < mo->enumeratorCount(); ++i)
                if (mo->enumerator(i).name() == enumName)
                    return true;
        }
    } else if (tokens.size() == 1) {
        QByteArray &enumName = tokens[0];
        for (const char *knownEnumName : qAsConst(knownEnumNames)) {
            if (enumName == knownEnumName)
                return true;
        }
    }
    return false;
}

static bool isCheckedClass(const QByteArray &typeName)
{
    for (const QMetaObject *mo : typesToCheck) {
        QByteArray moTypeName(mo->className());
        if (moTypeName == typeName || moTypeName + "*" == typeName)
            return true;
    }
    return false;
}

static void checkKnownType(const QByteArray &typeName)
{
    if ((!hardcodedTypes.contains(typeName) && !QMetaType::type(typeName)) || QMetaType::type(typeName) >= QMetaType::User) {
        bool knownEnum = isCheckedEnum(typeName);
        bool knownClass = isCheckedClass(typeName);
        QVERIFY2(knownEnum || knownClass, qPrintable(QString("The API uses an unknown type [%1], you might have to add it to the typesToCheck list.").arg(typeName.constData())));
    }
}

static void gatherAPI(const QString &prefix, const QMetaEnum &metaEnum, QStringList *output)
{
    for (int i = 0; i < metaEnum.keyCount(); ++i)
        *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(metaEnum.key(i)).arg(metaEnum.name());
}

static void gatherAPI(const QString &prefix, const QMetaProperty &property, QStringList *output)
{
    *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(property.name()).arg(property.typeName());
    checkKnownType(property.typeName());
}

static void gatherAPI(const QString &prefix, const QMetaMethod &method, QStringList *output)
{
    if (method.access() != QMetaMethod::Private) {
        const char *methodTypeName = !!strlen(method.typeName()) ? method.typeName() : "void";
        *output << QString::fromLatin1("%1%2 --> %3").arg(prefix).arg(QString::fromLatin1(method.methodSignature())).arg(QString::fromLatin1(methodTypeName));

        checkKnownType(methodTypeName);
        const QList<QByteArray> paramTypes = method.parameterTypes();
        for (const QByteArray &paramType : paramTypes)
            checkKnownType(paramType);
    }
}

static void gatherAPI(const QString &prefix, const QMetaObject *meta, QStringList *output)
{
    // *Offset points us only at the leaf class members, we don't have inheritance in our API yet anyway.
    for (int i = meta->enumeratorOffset(); i < meta->enumeratorCount(); ++i) {
        knownEnumNames << meta->enumerator(i).name();
        gatherAPI(prefix, meta->enumerator(i), output);
    }
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i)
        gatherAPI(prefix, meta->property(i), output);
    for (int i = meta->methodOffset(); i < meta->methodCount(); ++i)
        gatherAPI(prefix, meta->method(i), output);
}

void tst_publicapi::publicAPI()
{
    QStringList actualAPI;
    for (const QMetaObject *meta : typesToCheck)
        gatherAPI(QString::fromLatin1(meta->className()) + ".", meta, &actualAPI);

    // Uncomment to print the actual API.
    // QStringList sortedAPI(actualAPI);
    // std::sort(sortedAPI.begin(), sortedAPI.end());
    // for (const QString &actual : qAsConst(sortedAPI))
    //     printf("    << \"%s\"\n", qPrintable(actual));

    bool apiMatch = true;
    // Make sure that nothing slips in the public API unintentionally.
    for (const QString &actual : qAsConst(actualAPI)) {
        if (!expectedAPI.contains(actual)) {
            QWARN(qPrintable("Expected list is not up-to-date: " + actual));
            apiMatch = false;
        }
    }
    // Make sure that the expected list is up-to-date with intentionally added APIs.
    for (const QString &expected : expectedAPI) {
        if (!actualAPI.contains(expected)) {
            apiMatch = false;
            QWARN(qPrintable("Not implemented: " + expected));
        }
    }

    QVERIFY2(apiMatch, "Unexpected, missing or misspelled API!");
}

QTEST_MAIN(tst_publicapi)

#include "tst_publicapi.moc"

