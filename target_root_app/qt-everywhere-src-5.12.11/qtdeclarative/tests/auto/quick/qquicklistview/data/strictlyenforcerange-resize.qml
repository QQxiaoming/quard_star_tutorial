import QtQuick 2.0

ListView {
    width: 400
    height: 400
    focus: true

    highlightRangeMode: ListView.StrictlyEnforceRange
    highlightMoveVelocity: 200
    preferredHighlightBegin: 150
    preferredHighlightEnd: 150

    property bool completed
    Component.onCompleted: completed = true

    model: 10
    delegate: Item {
        width: parent.width
        height: ListView.isCurrentItem ? 100 : 50

        Text {
            anchors.centerIn: parent
            text: index
        }

        Behavior on height {
            enabled: completed
            SmoothedAnimation { velocity: 200 }
        }
    }
}
