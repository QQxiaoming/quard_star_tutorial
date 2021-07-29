/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebView module of the Qt Toolkit.
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

#include "qdarwinwebview_p.h"
#include <private/qwebview_p.h>
#include <private/qwebviewloadrequest_p.h>
#include "qtwebviewfunctions.h"

#include <QtCore/private/qglobal_p.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <QtCore/qvariant.h>

#include <CoreFoundation/CoreFoundation.h>
#include <WebKit/WebKit.h>

#ifdef Q_OS_IOS
#import <UIKit/UIKit.h>
#import <UIKit/UIGestureRecognizerSubclass.h>
#endif

#ifdef Q_OS_MACOS
#include <AppKit/AppKit.h>

typedef NSView UIView;
#endif

QT_BEGIN_NAMESPACE

static inline CGRect toCGRect(const QRectF &rect)
{
    return CGRectMake(rect.x(), rect.y(), rect.width(), rect.height());
}

QT_END_NAMESPACE
// -------------------------------------------------------------------------

#ifdef Q_OS_IOS
@implementation QIOSNativeViewSelectedRecognizer

- (id)initWithQWindowControllerItem:(QNativeViewController *)item
{
    self = [super initWithTarget:self action:@selector(nativeViewSelected:)];
    if (self) {
        self.cancelsTouchesInView = NO;
        self.delaysTouchesEnded = NO;
        m_item = item;
    }
    return self;
}

- (BOOL)canPreventGestureRecognizer:(UIGestureRecognizer *)other
{
    Q_UNUSED(other);
    return NO;
}

- (BOOL)canBePreventedByGestureRecognizer:(UIGestureRecognizer *)other
{
    Q_UNUSED(other);
    return NO;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    Q_UNUSED(touches);
    Q_UNUSED(event);
    self.state = UIGestureRecognizerStateRecognized;
}

- (void)nativeViewSelected:(UIGestureRecognizer *)gestureRecognizer
{
    Q_UNUSED(gestureRecognizer);
    m_item->setFocus(true);
}

@end
#endif

// -------------------------------------------------------------------------

@interface QtWKWebViewDelegate : NSObject<WKNavigationDelegate> {
    QDarwinWebViewPrivate *qDarwinWebViewPrivate;
}
- (QtWKWebViewDelegate *)initWithQAbstractWebView:(QDarwinWebViewPrivate *)webViewPrivate;
- (void)pageDone;
- (void)handleError:(NSError *)error;

// protocol:
- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation;
- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation;
- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation
      withError:(NSError *)error;
- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation
      withError:(NSError *)error;

@end

@implementation QtWKWebViewDelegate
- (QtWKWebViewDelegate *)initWithQAbstractWebView:(QDarwinWebViewPrivate *)webViewPrivate
{
    if ((self = [super init])) {
        Q_ASSERT(webViewPrivate);
        qDarwinWebViewPrivate = webViewPrivate;
    }
    return self;
}

- (void)pageDone
{
    Q_EMIT qDarwinWebViewPrivate->loadProgressChanged(qDarwinWebViewPrivate->loadProgress());
    Q_EMIT qDarwinWebViewPrivate->titleChanged(qDarwinWebViewPrivate->title());
}

- (void)handleError:(NSError *)error
{
    [self pageDone];
    NSString *errorString = [error localizedDescription];
    NSURL *failingURL = error.userInfo[@"NSErrorFailingURLKey"];
    const QUrl url = [failingURL isKindOfClass:[NSURL class]]
                        ? QUrl::fromNSURL(failingURL) : qDarwinWebViewPrivate->url();
    Q_EMIT qDarwinWebViewPrivate->loadingChanged(
                QWebViewLoadRequestPrivate(url, QWebView::LoadFailedStatus,
                                           QString::fromNSString(errorString)));
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation
{
    Q_UNUSED(webView);
    // WKNavigationDelegate gives us per-frame notifications while the QWebView API
    // should provide per-page notifications. Therefore we keep track of the last frame
    // to be started, if that finishes or fails then we indicate that it has loaded.
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        qDarwinWebViewPrivate->wkNavigation = navigation;
    else
        return;

    Q_EMIT qDarwinWebViewPrivate->loadingChanged(
                QWebViewLoadRequestPrivate(qDarwinWebViewPrivate->url(),
                                           QWebView::LoadStartedStatus,
                                           QString()));
    Q_EMIT qDarwinWebViewPrivate->loadProgressChanged(qDarwinWebViewPrivate->loadProgress());
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    Q_UNUSED(webView);
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        return;

    [self pageDone];
    Q_EMIT qDarwinWebViewPrivate->loadingChanged(
                QWebViewLoadRequestPrivate(qDarwinWebViewPrivate->url(),
                                           QWebView::LoadSucceededStatus,
                                           QString()));
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation
      withError:(NSError *)error
{
    Q_UNUSED(webView);
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        return;
    [self handleError:error];
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation
      withError:(NSError *)error
{
    Q_UNUSED(webView);
    if (qDarwinWebViewPrivate->wkNavigation != navigation)
        return;
    [self handleError:error];
}

- (void)webView:(WKWebView *)webView
decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction
                decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
                __attribute__((availability(ios_app_extension,unavailable)))
{
    Q_UNUSED(webView);
    NSURL *url = navigationAction.request.URL;
    const BOOL handled = (^{
        // For links with target="_blank", open externally
        if (!navigationAction.targetFrame)
            return NO;

#if QT_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(101300, 110000)
        if (__builtin_available(macOS 10.13, iOS 11.0, *)) {
            return [WKWebView handlesURLScheme:url.scheme];
        } else
#endif
        {
            // +[WKWebView handlesURLScheme:] is a stub that calls
            // WebCore::SchemeRegistry::isBuiltinScheme();
            // replicate that as closely as possible
            return [@[
                @"about", @"applewebdata", @"blob", @"data",
                @"file", @"http", @"https", @"javascript",
#ifdef Q_OS_MACOS
                @"safari-extension",
#endif
                @"webkit-fake-url", @"wss", @"x-apple-content-filter",
#ifdef Q_OS_MACOS
                @"x-apple-ql-id"
#endif
                ] containsObject:url.scheme];
        }
    })();
    if (!handled) {
#ifdef Q_OS_MACOS
        [[NSWorkspace sharedWorkspace] openURL:url];
#elif defined(Q_OS_IOS)
        // Check if it can be opened first, if it is a file scheme then it can't
        // be opened, therefore if it is a _blank target in that case we need to open
        // inside the current webview
        if ([[UIApplication sharedApplication] canOpenURL:url])
            [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        else if (!navigationAction.targetFrame)
            [webView loadRequest:navigationAction.request];
#endif
    }
    decisionHandler(handled ? WKNavigationActionPolicyAllow : WKNavigationActionPolicyCancel);
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change
                       context:(void *)context {
    Q_UNUSED(object);
    Q_UNUSED(change);
    Q_UNUSED(context);
    if ([keyPath isEqualToString:@"estimatedProgress"]) {
        Q_EMIT qDarwinWebViewPrivate->loadProgressChanged(qDarwinWebViewPrivate->loadProgress());
    }
}

@end

QT_BEGIN_NAMESPACE

QDarwinWebViewPrivate::QDarwinWebViewPrivate(QObject *p)
    : QAbstractWebView(p)
    , wkWebView(nil)
#ifdef Q_OS_IOS
    , m_recognizer(0)
#endif
{
    CGRect frame = CGRectMake(0.0, 0.0, 400, 400);
    wkWebView = [[WKWebView alloc] initWithFrame:frame];
    wkWebView.navigationDelegate = [[QtWKWebViewDelegate alloc] initWithQAbstractWebView:this];
    [wkWebView addObserver:wkWebView.navigationDelegate forKeyPath:@"estimatedProgress"
                   options:NSKeyValueObservingOptions(NSKeyValueObservingOptionNew)
                   context:nil];

#ifdef Q_OS_IOS
    m_recognizer = [[QIOSNativeViewSelectedRecognizer alloc] initWithQWindowControllerItem:this];
    [wkWebView addGestureRecognizer:m_recognizer];
#endif
}

QDarwinWebViewPrivate::~QDarwinWebViewPrivate()
{
    [wkWebView stopLoading];
    [wkWebView removeObserver:wkWebView.navigationDelegate forKeyPath:@"estimatedProgress"
                      context:nil];
    [wkWebView.navigationDelegate release];
    wkWebView.navigationDelegate = nil;
    [wkWebView release];
#ifdef Q_OS_IOS
    [m_recognizer release];
#endif
}

QUrl QDarwinWebViewPrivate::url() const
{
    return QUrl::fromNSURL(wkWebView.URL);
}

void QDarwinWebViewPrivate::setUrl(const QUrl &url)
{
    if (url.isValid()) {
        if (url.isLocalFile()) {
            // We need to pass local files via loadFileURL and the read access should cover
            // the directory that the file is in, to facilitate loading referenced images etc
            [wkWebView loadFileURL:url.toNSURL()
           allowingReadAccessToURL:QUrl(url.toString(QUrl::RemoveFilename)).toNSURL()];
        } else {
            [wkWebView loadRequest:[NSURLRequest requestWithURL:url.toNSURL()]];
        }
    }
}

void QDarwinWebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    [wkWebView loadHTMLString:html.toNSString() baseURL:baseUrl.toNSURL()];
}

bool QDarwinWebViewPrivate::canGoBack() const
{
    return wkWebView.canGoBack;
}

bool QDarwinWebViewPrivate::canGoForward() const
{
    return wkWebView.canGoForward;
}

QString QDarwinWebViewPrivate::title() const
{
    return QString::fromNSString(wkWebView.title);
}

int QDarwinWebViewPrivate::loadProgress() const
{
    return int(wkWebView.estimatedProgress * 100);
}

bool QDarwinWebViewPrivate::isLoading() const
{
    return wkWebView.loading;
}

void QDarwinWebViewPrivate::setParentView(QObject *view)
{
    m_parentView = view;

    if (!wkWebView)
        return;

    QWindow *w = qobject_cast<QWindow *>(view);
    if (w) {
        UIView *parentView = reinterpret_cast<UIView *>(w->winId());
        [parentView addSubview:wkWebView];
    } else {
        [wkWebView removeFromSuperview];
    }
}

QObject *QDarwinWebViewPrivate::parentView() const
{
    return m_parentView;
}

void QDarwinWebViewPrivate::setGeometry(const QRect &geometry)
{
    if (!wkWebView)
        return;

    [wkWebView setFrame:toCGRect(geometry)];
}

void QDarwinWebViewPrivate::setVisibility(QWindow::Visibility visibility)
{
    Q_UNUSED(visibility);
}

void QDarwinWebViewPrivate::setVisible(bool visible)
{
    [wkWebView setHidden:!visible];
}

void QDarwinWebViewPrivate::setFocus(bool focus)
{
    Q_EMIT requestFocus(focus);
}

void QDarwinWebViewPrivate::goBack()
{
    [wkWebView goBack];
}

void QDarwinWebViewPrivate::goForward()
{
    [wkWebView goForward];
}

void QDarwinWebViewPrivate::stop()
{
    [wkWebView stopLoading];
}

void QDarwinWebViewPrivate::reload()
{
    [wkWebView reload];
}

QVariant fromNSNumber(const NSNumber *number)
{
    if (!number)
        return QVariant();
    if (strcmp([number objCType], @encode(BOOL)) == 0) {
        return QVariant::fromValue(!![number boolValue]);
    } else if (strcmp([number objCType], @encode(signed char)) == 0) {
        return QVariant::fromValue([number charValue]);
    } else if (strcmp([number objCType], @encode(unsigned char)) == 0) {
        return QVariant::fromValue([number unsignedCharValue]);
    } else if (strcmp([number objCType], @encode(signed short)) == 0) {
        return QVariant::fromValue([number shortValue]);
    } else if (strcmp([number objCType], @encode(unsigned short)) == 0) {
        return QVariant::fromValue([number unsignedShortValue]);
    } else if (strcmp([number objCType], @encode(signed int)) == 0) {
        return QVariant::fromValue([number intValue]);
    } else if (strcmp([number objCType], @encode(unsigned int)) == 0) {
        return QVariant::fromValue([number unsignedIntValue]);
    } else if (strcmp([number objCType], @encode(signed long long)) == 0) {
        return QVariant::fromValue([number longLongValue]);
    } else if (strcmp([number objCType], @encode(unsigned long long)) == 0) {
        return QVariant::fromValue([number unsignedLongLongValue]);
    } else if (strcmp([number objCType], @encode(float)) == 0) {
        return QVariant::fromValue([number floatValue]);
    } else if (strcmp([number objCType], @encode(double)) == 0) {
        return QVariant::fromValue([number doubleValue]);
    }
    return QVariant();
}

QVariant fromJSValue(id result)
{
    if ([result isKindOfClass:[NSString class]])
        return QString::fromNSString(static_cast<NSString *>(result));
    if ([result isKindOfClass:[NSNumber class]])
        return fromNSNumber(static_cast<NSNumber *>(result));
    if ([result isKindOfClass:[NSDate class]])
        return QDateTime::fromNSDate(static_cast<NSDate *>(result));

    // JSValue also supports arrays and dictionaries, but we don't handle that yet
    return QVariant();
}

void QDarwinWebViewPrivate::runJavaScriptPrivate(const QString &script, int callbackId)
{
    [wkWebView evaluateJavaScript:script.toNSString() completionHandler:^(id result, NSError *) {
        if (callbackId != -1)
            Q_EMIT javaScriptResult(callbackId, fromJSValue(result));
    }];
}

QT_END_NAMESPACE
