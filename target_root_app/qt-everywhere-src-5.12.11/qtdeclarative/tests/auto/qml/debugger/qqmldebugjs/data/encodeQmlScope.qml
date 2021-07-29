import QtQuick 2.0

Item {
    property int a: 0
    property int b: 0
    onAChanged: console.log("inline")
    onBChanged: {
        console.log("extra braces");
    }

    Timer {
        interval: 10
        running: true
        onTriggered: {
            parent.a += 10;
            parent.b -= 10;
        }
    }
}
