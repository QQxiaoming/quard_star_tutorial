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

import QtQuick 2.0
import QtTest 1.0
import QtWebEngine 1.2

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 300

    TestCase {
        name: "WebEngineViewSettings"

        function test_javascriptEnabled() {
            webEngineView.settings.javascriptEnabled = true;

            webEngineView.url = Qt.resolvedUrl("javascript.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "New Title");
        }

        function test_javascriptDisabled() {
            webEngineView.settings.javascriptEnabled = false;

            webEngineView.url = Qt.resolvedUrl("javascript.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "Original Title");
        }

        function test_localStorageDisabled() {
            webEngineView.settings.javascriptEnabled = true;
            webEngineView.settings.localStorageEnabled = false;

            webEngineView.url = Qt.resolvedUrl("localStorage.html");
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "Original Title");
        }

        function test_localStorageEnabled() {
            webEngineView.settings.localStorageEnabled = true;
            webEngineView.settings.javascriptEnabled = true;

            webEngineView.url = Qt.resolvedUrl("localStorage.html");
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView.reload();
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(webEngineView, "title", "New Title");
        }

        function test_settingsAffectCurrentViewOnly()  {
            var webEngineView2 = Qt.createQmlObject('TestWebEngineView {width: 400; height: 300;}', webEngineView);

            webEngineView.settings.javascriptEnabled = true;
            webEngineView2.settings.javascriptEnabled = true;

            var testUrl = Qt.resolvedUrl("javascript.html");

            webEngineView.url = testUrl;
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView2.url = testUrl;
            verify(webEngineView2.waitForLoadSucceeded());

            tryCompare(webEngineView, "title", "New Title");
            tryCompare(webEngineView2, "title", "New Title");

            webEngineView.settings.javascriptEnabled = false;

            webEngineView.url = testUrl;
            verify(webEngineView.waitForLoadSucceeded());
            webEngineView2.url = testUrl;
            verify(webEngineView2.waitForLoadSucceeded());

            tryCompare(webEngineView, "title", "Original Title");
            tryCompare(webEngineView2, "title", "New Title");

            webEngineView2.destroy();
        }
    }
}

