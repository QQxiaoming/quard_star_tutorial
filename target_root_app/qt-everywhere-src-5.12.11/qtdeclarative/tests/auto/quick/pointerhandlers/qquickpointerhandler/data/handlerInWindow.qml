import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.test 1.0

Window {
    id: root
    objectName: "root Window"
    width: 320
    height: 480

    EventHandler {
        objectName: "eventHandler"
    }
}
