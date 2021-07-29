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
import QtWebEngine 1.3
import QtWebEngine.testsupport 1.0
import QtQuick.Window 2.0

TestWebEngineView {
    id: webEngineView
    width: 200
    height: 400

    testSupport: WebEngineTestSupport {
        property var errorPageLoadStatus: null

        function waitForErrorPageLoadSucceeded() {
            var success = _waitFor(function() { return testSupport.errorPageLoadStatus == WebEngineView.LoadSucceededStatus })
            testSupport.errorPageLoadStatus = null
            return success
        }

        errorPage.onLoadingChanged: {
            errorPageLoadStatus = loadRequest.status
        }
    }

    function removeFaviconProviderPrefix(url) {
        return url.toString().substring(16)
    }

    function getFaviconPixel(faviconImage) {
        var grabImage = Qt.createQmlObject("
                import QtQuick 2.5\n
                Image { }", test)
        var faviconCanvas = Qt.createQmlObject("
                import QtQuick 2.5\n
                Canvas { }", test)

        test.tryVerify(function() { return faviconImage.status == Image.Ready });
        faviconImage.grabToImage(function(result) {
                grabImage.source = result.url
            });
        test.tryVerify(function() { return grabImage.status == Image.Ready });

        faviconCanvas.width = faviconImage.width;
        faviconCanvas.height = faviconImage.height;
        var ctx = faviconCanvas.getContext("2d");
        ctx.drawImage(grabImage, 0, 0, grabImage.width, grabImage.height);
        var imageData = ctx.getImageData(Math.round(faviconCanvas.width/2),
                                         Math.round(faviconCanvas.height/2),
                                         faviconCanvas.width,
                                         faviconCanvas.height);

        grabImage.destroy();
        faviconCanvas.destroy();

        return imageData.data;
    }

    SignalSpy {
        id: iconChangedSpy
        target: webEngineView
        signalName: "iconChanged"
    }

    Image {
        id: favicon
        source: webEngineView.icon
    }

    TestCase {
        id: test
        name: "WebEngineFavicon"
        when: windowShown


        function init() {
            // It is worth to restore the initial state with loading a blank page before all test functions.
            webEngineView.url = 'about:blank'
            verify(webEngineView.waitForLoadSucceeded())
            iconChangedSpy.clear()
        }

        function test_faviconLoad() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            compare(favicon.width, 48)
            compare(favicon.height, 48)
        }

        function test_faviconLoadEncodedUrl() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon2.html?favicon=load should work with#whitespace!")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            compare(favicon.width, 16)
            compare(favicon.height, 16)
        }

        function test_noFavicon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("test1.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_aboutBlank() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("about:blank")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_unavailableFavicon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-unavailable.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_errorPageEnabled() {
            WebEngine.settings.errorPageEnabled = true

            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("http://url.invalid")
            webEngineView.url = url
            verify(webEngineView.waitForLoadFailed(20000))
            verify(webEngineView.testSupport.waitForErrorPageLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_errorPageDisabled() {
            WebEngine.settings.errorPageEnabled = false

            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("http://url.invalid")
            webEngineView.url = url
            verify(webEngineView.waitForLoadFailed(20000))

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
        }

        function test_bestFavicon() {
            compare(iconChangedSpy.count, 0)
            var url, iconUrl

            url = Qt.resolvedUrl("favicon-misc.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            // Touch icon is ignored
            compare(iconUrl, Qt.resolvedUrl("icons/qt32.ico"))
            compare(favicon.width, 32)
            compare(favicon.height, 32)

            iconChangedSpy.clear()

            url = Qt.resolvedUrl("favicon-shortcut.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            verify(iconChangedSpy.count >= 1)
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)

            // If the icon URL is empty we have to wait for
            // the second iconChanged signal that propagates the expected URL
            if (iconUrl == Qt.resolvedUrl("")) {
                tryCompare(iconChangedSpy, "count", 2)
                iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            }

            compare(iconUrl, Qt.resolvedUrl("icons/qt144.png"))
            compare(favicon.width, 144)
            compare(favicon.height, 144)
        }

        function test_touchIcon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-touch.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            compare(iconChangedSpy.count, 0)

            var iconUrl = webEngineView.icon
            compare(iconUrl, Qt.resolvedUrl(""))
            compare(favicon.width, 0)
            compare(favicon.height, 0)

            WebEngine.settings.touchIconsEnabled = true

            url = Qt.resolvedUrl("favicon-touch.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            iconUrl = removeFaviconProviderPrefix(webEngineView.icon)
            compare(iconUrl, Qt.resolvedUrl("icons/qt144.png"))
            compare(iconChangedSpy.count, 1)
            compare(favicon.width, 144)
            compare(favicon.height, 144)
        }

        function test_multiIcon() {
            compare(iconChangedSpy.count, 0)

            var url = Qt.resolvedUrl("favicon-multi.html")
            webEngineView.url = url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)
            compare(favicon.width, 64)
            compare(favicon.height, 64)
        }

        function test_faviconProvider_data() {
            return [
                   { tag: "multi 8x8", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 8, value: 16 },
                   { tag: "multi 16x16", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 16, value: 16 },
                   { tag: "multi 17x17", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 17, value: 32 },
                   { tag: "multi 31x31", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 31, value: 32 },
                   { tag: "multi 32x32", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 32, value: 32 },
                   { tag: "multi 33x33", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 33, value: 64 },
                   { tag: "multi 64x64", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 64, value: 64 },
                   { tag: "multi 128x128", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 128, value: 128 },
                   { tag: "multi 255x255", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 255, value: 255 },
                   { tag: "multi 256x256", url: Qt.resolvedUrl("favicon-multi-gray.html"), size: 256, value: 255 },
                   { tag: "candidate 8x8", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 8, value: 16 },
                   { tag: "candidate 16x16", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 16, value: 16 },
                   { tag: "candidate 17x17", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 17, value: 32 },
                   { tag: "candidate 31x31", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 31, value: 32 },
                   { tag: "candidate 32x32", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 32, value: 32 },
                   { tag: "candidate 33x33", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 33, value: 64 },
                   { tag: "candidate 64x64", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 64, value: 64 },
                   { tag: "candidate 128x128", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 128, value: 128 },
                   { tag: "candidate 255x255", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 255, value: 255 },
                   { tag: "candidate 256x256", url: Qt.resolvedUrl("favicon-candidates-gray.html"), size: 256, value: 255 },
            ];
        }

        function test_faviconProvider(row) {
            var faviconImage = Qt.createQmlObject("
                    import QtQuick 2.5\n
                    Image { sourceSize: Qt.size(width, height) }", test)

            compare(iconChangedSpy.count, 0)

            webEngineView.url = row.url
            verify(webEngineView.waitForLoadSucceeded())

            iconChangedSpy.wait()
            compare(iconChangedSpy.count, 1)

            faviconImage.width = row.size / Screen.devicePixelRatio
            faviconImage.height = row.size  / Screen.devicePixelRatio
            faviconImage.source = webEngineView.icon

            var pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], row.value)

            faviconImage.destroy()
        }

        function test_dynamicFavicon() {
            var faviconImage = Qt.createQmlObject("
                    import QtQuick 2.5\n
                    Image { width: 16; height: 16; sourceSize: Qt.size(width, height); }", test)
            faviconImage.source = Qt.binding(function() { return webEngineView.icon; });

            var colors = [
                {"url": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg==", "r": 255, "g": 0, "b": 0},
                {"url": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M/wHwAEBgIApD5fRAAAAABJRU5ErkJggg==", "r": 0, "g": 255, "b": 0},
                {"url": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPj/HwADBwIAMCbHYQAAAABJRU5ErkJggg==", "r": 0, "g": 0, "b": 255},
            ];
            var pixel;

            compare(iconChangedSpy.count, 0);
            webEngineView.loadHtml(
                        "<html>" +
                        "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII='/>" +
                        "</html>"
            );
            verify(webEngineView.waitForLoadSucceeded());
            tryCompare(iconChangedSpy, "count", 1);

            pixel = getFaviconPixel(faviconImage);
            compare(pixel[0], 0);
            compare(pixel[1], 0);
            compare(pixel[2], 0);

            for (var i = 0; i < colors.length; ++i) {
                iconChangedSpy.clear();
                runJavaScript("document.getElementsByTagName('link')[0].href = 'data:image/png;base64," + colors[i]["url"] + "';");
                tryCompare(faviconImage, "source", "image://favicon/data:image/png;base64," + colors[i]["url"]);
                compare(iconChangedSpy.count, 1);

                pixel = getFaviconPixel(faviconImage);
                compare(pixel[0], colors[i]["r"]);
                compare(pixel[1], colors[i]["g"]);
                compare(pixel[2], colors[i]["b"]);
            }

            faviconImage.destroy()
        }

        function test_touchIconWithSameURL()
        {
            WebEngine.settings.touchIconsEnabled = false;

            var icon = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=";

            webEngineView.loadHtml(
                        "<html>" +
                        "<link rel='icon' type='image/png' href='" + icon + "'/>" +
                        "<link rel='apple-touch-icon' type='image/png' href='" + icon + "'/>" +
                        "</html>"
            );
            verify(webEngineView.waitForLoadSucceeded());

            // The default favicon has to be loaded even if its URL is also set as a touch icon while touch icons are disabled.
            tryCompare(iconChangedSpy, "count", 1);
            compare(webEngineView.icon.toString().replace(/^image:\/\/favicon\//, ''), icon);

            iconChangedSpy.clear();

            webEngineView.loadHtml(
                        "<html>" +
                        "<link rel='apple-touch-icon' type='image/png' href='" + icon + "'/>" +
                        "</html>"
            );
            verify(webEngineView.waitForLoadSucceeded());

            // This page only has a touch icon. With disabled touch icons we don't expect any icon to be shown even if the same icon
            // was loaded previously.
            tryCompare(iconChangedSpy, "count", 1);
            verify(!webEngineView.icon.toString().replace(/^image:\/\/favicon\//, ''));
        }
    }
}
