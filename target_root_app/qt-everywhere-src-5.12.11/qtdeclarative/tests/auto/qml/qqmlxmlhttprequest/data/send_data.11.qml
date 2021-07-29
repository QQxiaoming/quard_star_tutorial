import QtQuick 2.0

QtObject {
    property string url

    property bool dataOK: false

    Component.onCompleted: {
        var x = new XMLHttpRequest;
        x.open("POST", url);
        x.setRequestHeader("Accept-Language","en-US");

        // Test to the end
        x.onreadystatechange = function() {
            if (x.readyState == XMLHttpRequest.DONE) {
                dataOK = (x.responseText == "QML Rocks!\n");
            }
        }

        var data = new Uint8Array([1, 2, 3, 0, 3, 2, 1])
        x.send(data.buffer);
    }
}
