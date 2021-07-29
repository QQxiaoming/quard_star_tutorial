/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#include "qwebview_p.h"
#include "qwebviewplugin_p.h"
#include "qwebviewloadrequest_p.h"
#include "qwebviewfactory_p.h"


QT_BEGIN_NAMESPACE

QWebView::QWebView(QObject *p)
    : QObject(p)
    , d(QWebViewFactory::createWebView())
    , m_progress(0)
{
    d->setParent(this);
    qRegisterMetaType<QWebViewLoadRequestPrivate>();

    connect(d, &QAbstractWebView::titleChanged, this, &QWebView::onTitleChanged);
    connect(d, &QAbstractWebView::urlChanged, this, &QWebView::onUrlChanged);
    connect(d, &QAbstractWebView::loadingChanged, this, &QWebView::onLoadingChanged);
    connect(d, &QAbstractWebView::loadProgressChanged, this, &QWebView::onLoadProgressChanged);
    connect(d, &QAbstractWebView::requestFocus, this, &QWebView::requestFocus);
    connect(d, &QAbstractWebView::javaScriptResult,
            this, &QWebView::javaScriptResult);
}

QWebView::~QWebView()
{
}

QUrl QWebView::url() const
{
    return m_url;
}

void QWebView::setUrl(const QUrl &url)
{
    d->setUrl(url);
}

bool QWebView::canGoBack() const
{
    return d->canGoBack();
}

void QWebView::goBack()
{
    d->goBack();
}

bool QWebView::canGoForward() const
{
    return d->canGoForward();
}

void QWebView::goForward()
{
    d->goForward();
}

void QWebView::reload()
{
    d->reload();
}

void QWebView::stop()
{
    d->stop();
}

QString QWebView::title() const
{
    return m_title;
}

int QWebView::loadProgress() const
{
    return m_progress;
}

bool QWebView::isLoading() const
{
    return d->isLoading();
}

void QWebView::setParentView(QObject *view)
{
    d->setParentView(view);
}

QObject *QWebView::parentView() const
{
    return d->parentView();
}

void QWebView::setGeometry(const QRect &geometry)
{
    d->setGeometry(geometry);
}

void QWebView::setVisibility(QWindow::Visibility visibility)
{
    d->setVisibility(visibility);
}

void QWebView::setVisible(bool visible)
{
    d->setVisible(visible);
}

void QWebView::setFocus(bool focus)
{
    d->setFocus(focus);
}

void QWebView::loadHtml(const QString &html, const QUrl &baseUrl)
{
    d->loadHtml(html, baseUrl);
}

void QWebView::runJavaScriptPrivate(const QString &script,
                                    int callbackId)
{
    d->runJavaScriptPrivate(script, callbackId);
}

void QWebView::onTitleChanged(const QString &title)
{
    if (m_title == title)
        return;

    m_title = title;
    Q_EMIT titleChanged();
}

void QWebView::onUrlChanged(const QUrl &url)
{
    if (m_url == url)
        return;

    m_url = url;
    Q_EMIT urlChanged();
}

void QWebView::onLoadProgressChanged(int progress)
{
    if (m_progress == progress)
        return;

    m_progress = progress;
    Q_EMIT loadProgressChanged();
}

void QWebView::onLoadingChanged(const QWebViewLoadRequestPrivate &loadRequest)
{
    if (loadRequest.m_status == QWebView::LoadFailedStatus)
        m_progress = 0;

    onUrlChanged(loadRequest.m_url);
    Q_EMIT loadingChanged(loadRequest);

}

void QWebView::init()
{
    d->init();
}

QT_END_NAMESPACE
