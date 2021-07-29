import QtQuick 2.0
import QtQuick.Window 2.1

Item {
    width: 400
    height: 400
    objectName: "root Item"

    Loader {
        sourceComponent: Rectangle {
            objectName: "yellow rectangle"
            x: 50; y: 50; width: 300; height: 300
            color: "yellow"
            Window {
                objectName: "red transient Window"
                width: 100
                height: 100
                visible: true // makes it harder, because it wants to become visible before root has a window
                color: "red"
                title: "red"
                flags: Qt.Dialog
                onVisibilityChanged: console.log("visibility " + visibility)
                onVisibleChanged: console.log("visible " + visible)
            }
        }
    }
}
