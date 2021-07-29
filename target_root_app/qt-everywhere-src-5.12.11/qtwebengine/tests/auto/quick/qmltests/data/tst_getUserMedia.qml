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

import QtQuick 2.2
import QtTest 1.0
import QtWebEngine 1.6

TestWebEngineView {
    id: webEngineView
    width: 400
    height: 400

    settings.screenCaptureEnabled: true

    TestCase {
        name: "GetUserMedia"
        when: windowShown

        function init_data() {
            return [
                {
                    tag: "device audio",
                    constraints: { audio: true },
                    feature: WebEngineView.MediaAudioCapture,
                },
                {
                    tag: "device video",
                    constraints: { video: true },
                    feature: WebEngineView.MediaVideoCapture,
                },
                {
                    tag: "device audio+video",
                    constraints: { audio: true, video: true },
                    feature: WebEngineView.MediaAudioVideoCapture,
                },
                {
                    tag: "desktop video",
                    constraints: {
                        video: {
                            mandatory: {
                                chromeMediaSource: "desktop"
                            }
                        }
                    },
                    feature: WebEngineView.DesktopVideoCapture,
                },
                {
                    tag: "desktop audio+video",
                    constraints: {
                        audio: {
                            mandatory: {
                                chromeMediaSource: "desktop"
                            }
                        },
                        video: {
                            mandatory: {
                                chromeMediaSource: "desktop"
                            }
                        }
                    },
                    feature: WebEngineView.DesktopAudioVideoCapture,
                }
            ]
        }

        function test_getUserMedia(row) {
            loadSync(Qt.resolvedUrl("test1.html"))

            // 1. Rejecting request on QML side should reject promise on JS side.
            jsGetUserMedia(row.constraints)
            tryVerify(function(){ return gotFeatureRequest(row.feature) })
            rejectPendingRequest()
            tryVerify(function(){ return !jsPromiseFulfilled() && jsPromiseRejected() })

            // 2. Accepting request on QML side should either fulfill or reject the
            // Promise on JS side. Due to the potential lack of physical media devices
            // deeper in the content layer we cannot guarantee that the promise will
            // always be fulfilled, however in this case an error should be returned to
            // JS instead of leaving the Promise in limbo.
            jsGetUserMedia(row.constraints)
            tryVerify(function(){ return gotFeatureRequest(row.feature) })
            acceptPendingRequest()
            tryVerify(function(){ return jsPromiseFulfilled() || jsPromiseRejected() });

            // 3. Media feature permissions are not remembered.
            jsGetUserMedia(row.constraints);
            tryVerify(function(){ return gotFeatureRequest(row.feature) })
            acceptPendingRequest()
            tryVerify(function(){ return jsPromiseFulfilled() || jsPromiseRejected() });
        }
    }

    ////
    // synchronous loading

    signal loadFinished

    SignalSpy {
        id: spyOnLoadFinished
        target: webEngineView
        signalName: "loadFinished"
    }

    onLoadingChanged: {
        if (loadRequest.status == WebEngineLoadRequest.LoadSucceededStatus) {
            loadFinished()
        }
    }

    function loadSync(url) {
        webEngineView.url = url
        spyOnLoadFinished.wait()
    }

    ////
    // synchronous permission requests

    property variant requestedFeature
    property variant requestedSecurityOrigin

    onFeaturePermissionRequested: {
        requestedFeature = feature
        requestedSecurityOrigin = securityOrigin
    }

    function gotFeatureRequest(expectedFeature) {
        return requestedFeature == expectedFeature
    }

    function acceptPendingRequest() {
        webEngineView.grantFeaturePermission(requestedSecurityOrigin, requestedFeature, true)
        requestedFeature = undefined
        requestedSecurityOrigin = undefined
    }

    function rejectPendingRequest() {
        webEngineView.grantFeaturePermission(requestedSecurityOrigin, requestedFeature, false)
        requestedFeature = undefined
        requestedSecurityOrigin = undefined
    }

    ////
    // synchronous JavaScript evaluation

    signal runJavaScriptFinished(variant result)

    SignalSpy {
        id: spyOnRunJavaScriptFinished
        target: webEngineView
        signalName: "runJavaScriptFinished"
    }

    function runJavaScriptSync(code) {
        spyOnRunJavaScriptFinished.clear()
        runJavaScript(code, runJavaScriptFinished)
        spyOnRunJavaScriptFinished.wait()
        return spyOnRunJavaScriptFinished.signalArguments[0][0]
    }

    ////
    // JavaScript snippets

    function jsGetUserMedia(constraints) {
        runJavaScript(
            "var promiseFulfilled = false;" +
            "var promiseRejected = false;" +
            "navigator.mediaDevices.getUserMedia(" + JSON.stringify(constraints) + ")" +
            ".then(stream => { promiseFulfilled = true})" +
            ".catch(err => { promiseRejected = true})")
    }

    function jsPromiseFulfilled() {
        return runJavaScriptSync("promiseFulfilled")
    }

    function jsPromiseRejected() {
        return runJavaScriptSync("promiseRejected")
    }
}
