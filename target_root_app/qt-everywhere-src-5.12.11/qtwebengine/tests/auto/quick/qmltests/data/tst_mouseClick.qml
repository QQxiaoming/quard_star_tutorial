/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
import QtWebEngine 1.4

import QtWebEngine.testsupport 1.0

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 200

    testSupport: WebEngineTestSupport {
        function mouseMultiClick(item, x, y, clickCount) {
            if (!item)
                qtest_fail("No item given to mouseMultiClick", 1);

            if (x === undefined)
                x = item.width / 2;
            if (y === undefined)
                y = item.height / 2;
            if (!testEvent.mouseMultiClick(item, x, y, clickCount))
                qtest_fail("window not shown", 2);
        }

        function mouseDoubleClick(item, x, y) {
            mouseMultiClick(item, x, y, 2);
        }

        function mouseTripleClick(item, x, y) {
            mouseMultiClick(item, x, y, 3);
        }

        function mouseQuadraClick(item, x, y) {
            mouseMultiClick(item, x, y, 4);
        }
    }


    TestCase {
        name: "WebEngineViewMouseClick"
        when: windowShown

        function test_singleClick() {
            webEngineView.settings.focusOnNavigationEnabled = false;
            webEngineView.loadHtml("<html><body>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());
            verify(!getActiveElementId());

            var center = getElementCenter("input");
            mouseClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            compare(getTextSelection(), "");
        }

        function test_doubleClick() {
            webEngineView.settings.focusOnNavigationEnabled = true;
            webEngineView.loadHtml("<html><body onload='document.getElementById(\"input\").focus()'>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            var center = getElementCenter("input");
            webEngineView.testSupport.mouseDoubleClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            tryVerify(function() { return getTextSelection() == "Company" });

            mouseClick(webEngineView, center.x, center.y);
            tryVerify(function() { return getTextSelection() == "" });
        }

        function test_tripleClick() {
            webEngineView.settings.focusOnNavigationEnabled = true;
            webEngineView.loadHtml("<html><body onload='document.getElementById(\"input\").focus()'>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            var center = getElementCenter("input");
            webEngineView.testSupport.mouseTripleClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            tryVerify(function() { return getTextSelection() == "The Qt Company" });

            mouseClick(webEngineView, center.x, center.y);
            tryVerify(function() { return getTextSelection() == "" });
        }

        function test_quadraClick() {
            webEngineView.settings.focusOnNavigationEnabled = true;
            webEngineView.loadHtml("<html><body onload='document.getElementById(\"input\").focus()'>" +
                                   "<form><input id='input' width='150' type='text' value='The Qt Company' /></form>" +
                                   "</body></html>");
            verify(webEngineView.waitForLoadSucceeded());

            var center = getElementCenter("input");
            webEngineView.testSupport.mouseQuadraClick(webEngineView, center.x, center.y);
            verifyElementHasFocus("input");
            tryVerify(function() { return getTextSelection() == "" });
        }
    }
}
