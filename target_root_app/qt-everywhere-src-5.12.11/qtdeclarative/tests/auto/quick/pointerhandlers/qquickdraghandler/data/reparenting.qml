import QtQuick 2.12

Grid {
    id: root
    objectName: "root"
    property bool reparentOnDrag: true
    width: 200; height: 200
    columns: 3
    spacing: 10
    Repeater {
        model: 9
        anchors.fill: parent
        Item {
            id: gridPlaceholder
            objectName: "gridPlaceholder" + index
            width: 60
            height: 60
            Rectangle {
                id: icon
                border.color: "black"
                color: "beige"
                radius: 3
                width: 60
                height: 60
                onParentChanged :console.log("parent " + parent)
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                }
                DragHandler {
                    id: dragArea
                }
                Text {
                    anchors.centerIn: parent
                    text: index + "@" + Math.round(icon.x) + "," + Math.round(icon.y)
                    font.pointSize: 8
                }
                states: [
                    State {
                        when: dragArea.dragging
                        AnchorChanges {
                            target: icon
                            anchors.horizontalCenter: undefined
                            anchors.verticalCenter: undefined
                        }
                        ParentChange {
                            target: root.reparentOnDrag ? icon : null
                            parent: root
                        }
                        PropertyChanges {
                            target: icon
                            color: "yellow"
                        }
                    }
                ]
            }
        }
    }
}
