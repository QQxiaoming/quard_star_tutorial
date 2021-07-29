import QtQuick 2.12

Rectangle {
    color: "#333"
    width: 480; height: 480

    Rectangle {
        color: "#112"
        width: 100
        height: 100
        x: 50; y: 50

        DragHandler {
            id: dragHandler
            margin: 20
        }

        Rectangle {
            id: rect
            anchors.fill: parent
            anchors.margins: -dragHandler.margin
            color: "transparent"
            border.color: "cyan"
            border.width: 2
            radius: 10
            antialiasing: true

            Text {
                color: "cyan"
                text: "drag this margin area"
                font.pixelSize: 10
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
