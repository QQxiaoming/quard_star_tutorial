import QtQuick 2.12
import Qt.test 1.0

Item {
    id: root
    objectName: "root Item"
    width: 320
    height: 480

    Rectangle {
        objectName: "eventItem's bounds"
        anchors.fill: eventItem
        color: "lightsteelblue"
    }

    EventItem {
        id: eventItem
        objectName: "eventItem1"
        x: 5
        y: 5
        height: 30
        width: 30

        Component.onCompleted: handlerComponent.createObject(eventItem)

        Component {
            id: handlerComponent

            EventHandler {
                objectName: "eventHandler"
            }
        }
    }
}
