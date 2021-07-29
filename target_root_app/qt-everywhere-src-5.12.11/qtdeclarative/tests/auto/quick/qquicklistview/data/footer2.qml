import QtQuick 2.0

Rectangle {
    width: 240
    height: 320

    Timer {
        running: true
        repeat: false
        interval: 100
        onTriggered: {
            list.model -= 3;
        }
    }

    ListView {
        id: list
        objectName: "list"
        anchors.fill: parent
        model: 3
        delegate: Rectangle {
            color: "red"
            width: 240
            height: 10
        }
        footer: Rectangle {
            color: "blue"
            width: 240
            height: 10
        }
    }
}

