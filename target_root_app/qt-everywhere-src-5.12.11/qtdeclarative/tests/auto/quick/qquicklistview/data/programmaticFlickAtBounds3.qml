import QtQuick 2.9

ListView {
    id: view
    width: 200; height: 400

    property real minOvershoot
    onVerticalOvershootChanged: if (verticalOvershoot < minOvershoot) minOvershoot = verticalOvershoot

    highlightRangeMode: ListView.StrictlyEnforceRange
    preferredHighlightBegin: 0
    preferredHighlightEnd: 0

    model: 10
    delegate: Rectangle {
        width: 200; height: 50
        color: index % 2 ? "red" : "green"
    }
}
