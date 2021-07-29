import QtQuick 2.0

ListView {
    width: 100
    height: 300
    snapMode: ListView.SnapToItem
    spacing: 100
    model: 10
    delegate: Rectangle {
        height: 100
        width: 100
        color: "blue"
        Text {
            anchors.centerIn: parent
            text: index
        }
    }
}
