import QtQuick 2.0

Rectangle {
    property bool hasMouse: mouseArea.containsMouse
    height: 200
    width: 200
    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent
    }
}
