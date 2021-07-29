import QtQuick 2.12

Rectangle {

    width: 240
    height: 320
    color: "#ffffff"

    Component {
        id: myDelegate
        Rectangle {
            id: wrapper
            objectName: "wrapper"
            width: list.orientation == ListView.Vertical ? 240 : 20
            height: list.orientation == ListView.Vertical ? 20 : 240
            border.width: 1
            border.color: "black"
            Text {
                text: index + ":" + (list.orientation == ListView.Vertical ? parent.y : parent.x).toFixed(0)
            }
            color: ListView.isCurrentItem ? "lightsteelblue" : "white"
        }
    }

    ListView {
        id: list
        objectName: "list"
        focus: true
        width: 240
        height: 200
        clip: true
        snapMode: ListView.SnapToItem
        headerPositioning: ListView.OverlayHeader
        model: 30
        delegate: myDelegate
        orientation: ListView.Vertical
        verticalLayoutDirection: ListView.BottomToTop

        header: Rectangle {
            width: list.orientation == Qt.Vertical ? 240 : 30
            height: list.orientation == Qt.Vertical ? 30 : 240
            objectName: "header";
            color: "green"
            z: 11
            Text {
                anchors.centerIn: parent
                text: "header " + (list.orientation == ListView.Vertical ? parent.y : parent.x).toFixed(1)
            }
        }
    }

    Rectangle {
        color: "red"
        opacity: 0.5
        width: txt.implicitWidth + 50
        height: txt.implicitHeight
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        Text {
            id: txt
            anchors.centerIn: parent
            text:  "header position: " + (list.orientation == ListView.Vertical ? list.headerItem.y : list.headerItem.x).toFixed(1)
              + "\ncontent position: " + (list.orientation == ListView.Vertical ? list.contentY : list.contentX).toFixed(1)
        }
    }
}
