import QtQuick 2.0
import QtQuick.Window 2.0

Window {
    visible: true

    property alias list: repeater.model

    list: ["A", "B", "C"]

    Repeater {
        id: repeater
    }
}

