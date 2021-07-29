import QtQuick 2.6

ListView {
    id: table
    height: 200
    width: 100

    headerPositioning: ListView.OverlayHeader
    header: Rectangle {
        width: table.width
        height: 20
        color: "red"
        z: 100
    }

    footerPositioning: ListView.OverlayFooter
    footer: Rectangle {
        width: table.width
        height: 20
        color: "blue"
        z: 200
    }

    model: 30
    delegate: Rectangle {
        height: 20
        width: table.width
        color: "lightgray"
        Text {
            text: "Item " + index
            anchors.centerIn: parent
        }
    }
}
