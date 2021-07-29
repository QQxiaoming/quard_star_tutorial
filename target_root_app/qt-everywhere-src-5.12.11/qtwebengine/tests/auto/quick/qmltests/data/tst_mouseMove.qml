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

Rectangle {
    id: root
    width: 200
    height: 200

    Column {
        anchors.fill: parent
        Rectangle {
            id: placeHolder
            width: parent.width
            height: 100
            color: "red"
        }

        TestWebEngineView {
            id: webEngineView
            width: parent.width
            height: 100

            function getInnerText(element) {
                var innerText;
                runJavaScript("document.getElementById('" + element + "').innerText", function(result) {
                    innerText = result;
                });
                testCase.tryVerify(function() { return innerText != undefined; });
                return innerText;
            }
        }
    }

    TestCase {
        id: testCase
        name: "WebEngineViewMouseMove"
        when: windowShown

        function test_mouseLeave() {
            mouseMove(root, 0, 0);
            webEngineView.loadHtml(
                        "<html>" +
                        "<head><script>" +
                        "function init() {" +
                        " var div = document.getElementById('testDiv');" +
                        " div.onmouseenter = function(e) { div.innerText = 'Mouse IN' };" +
                        " div.onmouseleave = function(e) { div.innerText = 'Mouse OUT' };" +
                        "}" +
                        "</script></head>" +
                        "<body onload='init()' style='margin: 0px; padding: 0px'>" +
                        " <div id='testDiv' style='width: 100%; height: 100%; background-color: green' />" +
                        "</body>" +
                        "</html>");
            verify(webEngineView.waitForLoadSucceeded());
            // Make sure the testDiv text is empty.
            webEngineView.runJavaScript("document.getElementById('testDiv').innerText = ''");
            tryVerify(function() { return !webEngineView.getInnerText("testDiv") });

            for (var i = 90; i < 110; ++i)
                mouseMove(root, 50, i);
            tryVerify(function() { return webEngineView.getInnerText("testDiv") == "Mouse IN" });

            for (var i = 110; i > 90; --i)
                mouseMove(root, 50, i);
            tryVerify(function() { return webEngineView.getInnerText("testDiv") == "Mouse OUT" });
        }
    }
}

