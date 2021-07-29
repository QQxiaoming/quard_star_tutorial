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

import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.8

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    Component {
        id: signalSpy
        SignalSpy { }
    }

    TestCase {
        id: actionTests
        name: "WebEngineAction"
        when: windowShown

        function test_actions_data() {
            return [
                   { webAction: WebEngineView.Back, text: "Back", iconName: "go-previous", enabled: false },
                   { webAction: WebEngineView.Forward, text: "Forward", iconName: "go-next", enabled: false },
                   { webAction: WebEngineView.Stop, text: "Stop", iconName: "", enabled: false },
                   { webAction: WebEngineView.Reload, text: "Reload", iconName: "view-refresh", enabled: true },
                   { webAction: WebEngineView.Cut, text: "Cut", iconName: "Cut", enabled: true },
                   { webAction: WebEngineView.Copy, text: "Copy", iconName: "", enabled: true },
                   { webAction: WebEngineView.Paste, text: "Paste", iconName: "", enabled: true },
                   { webAction: WebEngineView.Undo, text: "Undo", iconName: "", enabled: true },
                   { webAction: WebEngineView.Redo, text: "Redo", iconName: "", enabled: true },
                   { webAction: WebEngineView.SelectAll, text: "Select all", iconName: "", enabled: true },
                   { webAction: WebEngineView.ReloadAndBypassCache, text: "Reload and Bypass Cache", iconName: "", enabled: true },
                   { webAction: WebEngineView.PasteAndMatchStyle, text: "Paste and match style", iconName: "", enabled: true },
                   { webAction: WebEngineView.OpenLinkInThisWindow, text: "Open link in this window", iconName: "", enabled: true },
                   { webAction: WebEngineView.OpenLinkInNewWindow, text: "Open link in new window", iconName: "", enabled: true },
                   { webAction: WebEngineView.OpenLinkInNewTab, text: "Open link in new tab", iconName: "", enabled: true },
                   { webAction: WebEngineView.CopyLinkToClipboard, text: "Copy link address", iconName: "", enabled: true },
                   { webAction: WebEngineView.DownloadLinkToDisk, text: "Save link", iconName: "", enabled: true },
                   { webAction: WebEngineView.CopyImageToClipboard, text: "Copy image", iconName: "", enabled: true },
                   { webAction: WebEngineView.CopyImageUrlToClipboard, text: "Copy image address", iconName: "", enabled: true },
                   { webAction: WebEngineView.DownloadImageToDisk, text: "Save image", iconName: "", enabled: true },
                   { webAction: WebEngineView.CopyMediaUrlToClipboard, text: "Copy media address", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaControls, text: "Show controls", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaLoop, text: "Loop", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaPlayPause, text: "Toggle Play/Pause", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleMediaMute, text: "Toggle Mute", iconName: "", enabled: true },
                   { webAction: WebEngineView.DownloadMediaToDisk, text: "Save media", iconName: "", enabled: true },
                   { webAction: WebEngineView.InspectElement, text: "Inspect", iconName: "", enabled: true },
                   { webAction: WebEngineView.ExitFullScreen, text: "Exit full screen", iconName: "", enabled: true },
                   { webAction: WebEngineView.RequestClose, text: "Close Page", iconName: "", enabled: true },
                   { webAction: WebEngineView.Unselect, text: "Unselect", iconName: "", enabled: true },
                   { webAction: WebEngineView.SavePage, text: "Save page", iconName: "", enabled: true },
                   { webAction: WebEngineView.ViewSource, text: "View page source", iconName: "view-source", enabled: true },
                   { webAction: WebEngineView.ToggleBold, text: "&Bold", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleItalic, text: "&Italic", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleUnderline, text: "&Underline", iconName: "", enabled: true },
                   { webAction: WebEngineView.ToggleStrikethrough, text: "&Strikethrough", iconName: "", enabled: true },
                   { webAction: WebEngineView.AlignLeft, text: "Align &Left", iconName: "", enabled: true },
                   { webAction: WebEngineView.AlignCenter, text: "Align &Center", iconName: "", enabled: true },
                   { webAction: WebEngineView.AlignRight, text: "Align &Right", iconName: "", enabled: true },
                   { webAction: WebEngineView.AlignJustified, text: "Align &Justified", iconName: "", enabled: true },
                   { webAction: WebEngineView.Indent, text: "&Indent", iconName: "", enabled: true },
                   { webAction: WebEngineView.Outdent, text: "&Outdent", iconName: "", enabled: true },
                   { webAction: WebEngineView.InsertOrderedList, text: "Insert &Ordered List", iconName: "", enabled: true },
                   { webAction: WebEngineView.InsertUnorderedList, text: "Insert &Unordered List", iconName: "", enabled: true }
            ];
        }

        function test_actions(row) {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            verify(webEngineView.waitForLoadSucceeded());

            var action = webEngineView.action(row.webAction);
            verify(action);

            compare(action.text, row.text);
            compare(action.iconText, row.iconText);
            compare(action.enabled, row.enabled);
        }

        function test_trigger() {
            webEngineView.url = Qt.resolvedUrl("test1.html");
            verify(webEngineView.waitForLoadSucceeded());

            var copyAction = webEngineView.action(WebEngineView.Copy);
            verify(copyAction);

            var stopAction = webEngineView.action(WebEngineView.Stop);
            verify(stopAction);

            var triggerSpy = createTemporaryObject(signalSpy, actionTests, {target: copyAction, signalName: "triggered"});
            var stopTriggerSpy = createTemporaryObject(signalSpy, actionTests, {target: stopAction, signalName: "triggered"});

            verify(copyAction.enabled);
            copyAction.trigger();
            compare(triggerSpy.count, 1);

            verify(!stopAction.enabled);
            stopAction.trigger();
            compare(stopTriggerSpy.count, 0);
        }

        function test_editActionsWithExplicitFocus() {
            var webView = Qt.createQmlObject("TestWebEngineView { visible: false; }", webEngineView);
            webView.settings.focusOnNavigationEnabled = false;

            // The view is hidden and no focus on the page. Edit actions should be disabled.
            var selectAllAction = webView.action(WebEngineView.SelectAll);
            verify(selectAllAction);
            verify(!selectAllAction.enabled);

            var triggerSpy = createTemporaryObject(signalSpy, webEngineView, {target: selectAllAction, signalName: "triggered"});
            var enabledSpy = createTemporaryObject(signalSpy, webEngineView, {target: selectAllAction, signalName: "enabledChanged"});

            webView.loadHtml("<html><body><div>foo bar</div></body></html>");
            verify(webView.waitForLoadSucceeded());

            // Still no focus because focus on navigation is disabled. Edit actions don't do anything (should not crash).
            verify(!selectAllAction.enabled);
            compare(enabledSpy.count, 0);
            selectAllAction.trigger();
            compare(triggerSpy.count, 0);
            compare(getTextSelection(), "");

            // Focus content by focusing window from JavaScript. Edit actions should be enabled and functional.
            webView.runJavaScript("window.focus();");
            tryVerify(function() { return enabledSpy.count === 1 });
            verify(selectAllAction.enabled);
            selectAllAction.trigger();
            compare(triggerSpy.count, 1);
            tryVerify(function() { return webView.getTextSelection() === "foo bar" });
        }

        function test_editActionsWithInitialFocus() {
            var webView = Qt.createQmlObject("TestWebEngineView { visible: false; }", webEngineView);
            webView.settings.focusOnNavigationEnabled = true;

            // The view is hidden and no focus on the page. Edit actions should be disabled.
            var selectAllAction = webView.action(WebEngineView.SelectAll);
            verify(selectAllAction);
            verify(!selectAllAction.enabled);

            var triggerSpy = createTemporaryObject(signalSpy, webEngineView, {target: selectAllAction, signalName: "triggered"});
            var enabledSpy = createTemporaryObject(signalSpy, webEngineView, {target: selectAllAction, signalName: "enabledChanged"});

            webView.loadHtml("<html><body><div>foo bar</div></body></html>");
            verify(webView.waitForLoadSucceeded());

            // Content gets initial focus.
            tryVerify(function() { return enabledSpy.count === 1 });
            verify(selectAllAction.enabled);
            selectAllAction.trigger();
            compare(triggerSpy.count, 1);
            tryVerify(function() { return webView.getTextSelection() === "foo bar" });
        }
    }
}
