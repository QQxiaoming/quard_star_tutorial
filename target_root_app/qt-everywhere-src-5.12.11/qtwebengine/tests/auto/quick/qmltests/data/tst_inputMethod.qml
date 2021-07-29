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
    height: 400

    testSupport: WebEngineTestSupport { }

    TestCase {
        name: "WebEngineViewInputMethod"
        when: windowShown

        function init() {
            testSupport.testInputContext.create();
        }

        function cleanup() {
            testSupport.testInputContext.release();
        }

        function test_softwareInputPanel() {
            verify(!Qt.inputMethod.visible);
            webEngineView.loadHtml(
                        "<html><body>" +
                        "   <form><input id='textInput' type='text' /></form>" +
                        "</body></html");
            verify(webEngineView.waitForLoadSucceeded());

            verify(!getActiveElementId());
            verify(!Qt.inputMethod.visible);

            // Show input panel
            webEngineView.runJavaScript("document.getElementById('textInput').focus()");
            webEngineView.verifyElementHasFocus("textInput");
            tryVerify(function() { return Qt.inputMethod.visible; });

            // Hide input panel
            webEngineView.runJavaScript("document.getElementById('textInput').blur()");
            verify(!getActiveElementId());
            tryVerify(function() { return !Qt.inputMethod.visible; });
        }
    }
}

