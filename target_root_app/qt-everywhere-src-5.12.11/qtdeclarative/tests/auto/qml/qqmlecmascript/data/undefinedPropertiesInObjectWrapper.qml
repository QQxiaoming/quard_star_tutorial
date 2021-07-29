import QtQuick 2.12

QtObject {
    property list<QtObject> entries: [
        QtObject {
            readonly property color color: "green"
        },
        QtObject {
        }
    ]

    property Row row: Row {
        Repeater {
            model: entries
            Rectangle {
                color: model.color ? model.color : "red"
            }
        }
    }
}
