import QtQuick 2.0

ListView {
    width: 400
    height: 400
    focus: true

    model: 10
    delegate: Rectangle {
        width: parent.width
        height: 50
        color: index % 2 ? "blue" : "green"
    }

    snapMode: ListView.SnapOneItem
    Keys.onUpPressed: flick(0,500)
    Keys.onDownPressed: flick(0,-500)
}
