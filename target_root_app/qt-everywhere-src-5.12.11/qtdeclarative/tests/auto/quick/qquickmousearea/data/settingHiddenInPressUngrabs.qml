import QtQuick 2.11
import QtQuick.Window 2.11

Window {
    id: window
    visible: true
    width: 200
    height: 200

    MouseArea {
        objectName: "cat"
        anchors.fill: parent
        onPressed: visible = false
        width: parent.width / 2
        height: parent.height
        Rectangle {
            width: 10
            height: 10
            color: "blue"
        }
    }
    MouseArea {
        objectName: "mouse"
        x:  parent.width / 2
        width: parent.width / 2
        height: parent.height
        onPressed: {
            enabled = false
        }
        Rectangle {
            width: 10
            height: 10
            color: "blue"
        }
    }
}
