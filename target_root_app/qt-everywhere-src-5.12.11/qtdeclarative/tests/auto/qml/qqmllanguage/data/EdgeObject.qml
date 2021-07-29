import QtQuick 2.0

Item {
    property int edgeWidth: bg.width

    Rectangle {
        id: bg
    }

    states: [
        State {
            when: 1 === 1
            PropertyChanges {
                target: bg
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }
    ]
}
