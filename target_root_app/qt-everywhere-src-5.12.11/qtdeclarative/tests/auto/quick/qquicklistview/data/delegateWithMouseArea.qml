import QtQuick 2.12

ListView {
    id: root
    objectName: "view"
    width: 600
    height: 600
    model: 3
    snapMode: ListView.SnapOneItem
    boundsBehavior: Flickable.StopAtBounds
    highlightRangeMode: ListView.StrictlyEnforceRange
    preferredHighlightBegin: 0
    preferredHighlightEnd: 0
    highlightMoveDuration: 100
    delegate: Rectangle {
        id: delegateRect
        width: 500
        height: 500
        color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1)
        Text {
            text: index
            font.pixelSize: 128
            anchors.centerIn: parent
        }
        MouseArea {
            anchors.fill: parent
        }
    }
}
