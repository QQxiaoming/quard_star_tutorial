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

#include "qtwebviewfunctions.h"

#include "qwebviewfactory_p.h"
#include "qwebviewplugin_p.h"

QT_BEGIN_NAMESPACE

/*!
    \namespace QtWebView
    \inmodule QtWebView
    \brief The QtWebView namespace provides functions that makes it easier to set-up and use the WebView.
    \inheaderfile QtWebView
*/

/*!
    \fn void QtWebView::initialize()
    \keyword qtwebview-initialize

    This function initializes resources or sets options that are required by the different back-ends.

    \note The \c initialize() function needs to be called immediately after the QGuiApplication
    instance is created.
 */

void QtWebView::initialize()
{
    if (QWebViewFactory::requiresExtraInitializationSteps()) {
        // There might be plugins available, but their dependencies might not be met,
        // so make sure we have a valid plugin before using it.
        // Note: A warning will be printed later if we're unable to load the plugin.
        QWebViewPlugin *plugin = QWebViewFactory::getPlugin();
        if (plugin)
            plugin->prepare();
    }
}

QT_END_NAMESPACE
