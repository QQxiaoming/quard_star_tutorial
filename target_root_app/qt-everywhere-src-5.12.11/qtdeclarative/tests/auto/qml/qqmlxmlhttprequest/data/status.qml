import QtQuick 2.0

QtObject {
    property string url
    property int expectedStatus

    property bool unsentException: false;
    property bool openedException: false;
    property bool sentException: false;

    property bool headersReceived: false
    property bool loading: false
    property bool done: false
    property bool onloadCalled: false
    property bool onerrorCalled: false
    property bool onloadendCalled: false

    property bool resetException: false

    property bool dataOK: false

    Component.onCompleted: {
        var x = new XMLHttpRequest;

        try {
            var a = x.status;
        } catch (e) {
            if (e.code == DOMException.INVALID_STATE_ERR)
                unsentException = true;
        }

        x.open("GET", url);
        x.setRequestHeader("Accept-Language", "en-US");

        try {
            var a = x.status;
        } catch (e) {
            if (e.code == DOMException.INVALID_STATE_ERR)
                openedException = true;
        }

        // Test to the end
        x.onreadystatechange = function() {
            if (x.readyState == XMLHttpRequest.HEADERS_RECEIVED) {
                if (x.status == expectedStatus)
                    headersReceived = true;
            } else if (x.readyState == XMLHttpRequest.LOADING) {
                if (x.status == expectedStatus)
                    loading = true;
            } else if (x.readyState == XMLHttpRequest.DONE) {
                if (x.status == expectedStatus)
                    done = true;

                dataOK = (x.responseText == "QML Rocks!\n");

                x.open("GET", url);
                x.setRequestHeader("Accept-Language", "en-US");

                try {
                    var a = x.status;
                } catch (e) {
                    if (e.code == DOMException.INVALID_STATE_ERR)
                        resetException = true;
                }

            }
        }
        x.onload = function() {
            // test also that it was called after onreadystatechanged(DONE)
            onloadCalled = (done === true) && (onerrorCalled === false);
        }
        x.onloadend = function() {
            onloadendCalled = (done === true) && (onloadCalled === true || onerrorCalled === true);
        }
        x.onerror = function() {
            onerrorCalled = (done === true) && (onloadCalled === false);
        }

        x.send()

        try {
            var a = x.status;
        } catch (e) {
            if (e.code == DOMException.INVALID_STATE_ERR)
                sentException = true;
        }
    }
}
