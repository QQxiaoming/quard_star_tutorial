/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

import QtQuick 2.0
import QtWebEngine 1.4
import QtQuick.Window 2.0
import QtTest 1.0
import io.qt.tester 1.0

Window {
    width: 50
    height: 50
    visible: true

    TestHandler {
        id: handler
        onJavaScript: function(script) {
            view.runJavaScript(script , function(result) {
                handler.ready = true
            })
        }
        onLoadPage: function(url) {
            if (view.url === url) {
                handler.ready = true
                return
            }
            view.url = url
        }
    }

    WebEngineView {
        id: view
        anchors.fill: parent
        onLoadingChanged: function(request) {
            if (request.status === WebEngineView.LoadSucceededStatus) {
                handler.ready = true
            } else if (request.status === WebEngineView.LoadFailedStatus) {
                console.log("Page was not successfully loaded from qrc! Status: " + request.status
                    + ", error [code: " + request.errorCode + "]: '" + request.errorString + "'")
            }
        }

        onContextMenuRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onAuthenticationDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onJavaScriptDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onColorDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }

        onFileDialogRequested: function(request) {
            request.accepted = true;
            handler.request = request;
        }
    }
}
