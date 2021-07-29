import QtQuick 2.0
import QtQuick.Window 2.0

Rectangle {
    width: 400
    height: 300

    property bool topPressed: top.pressed
    property bool bottomPressed: bottom.pressed

    MouseArea {
        id: top
        objectName: "top"
        width: parent.width
        height: parent.height / 2 - 2
        Rectangle {
            anchors.fill: parent
            color: parent.pressed ? "MediumSeaGreen" : "beige"
        }
    }

    MouseArea {
        id: bottom
        objectName: "bottom"
        y: parent.height / 2
        width: parent.width
        height: parent.height / 2
        Rectangle {
            anchors.fill: parent
            color: parent.pressed ? "MediumSeaGreen" : "beige"
        }
    }
}
