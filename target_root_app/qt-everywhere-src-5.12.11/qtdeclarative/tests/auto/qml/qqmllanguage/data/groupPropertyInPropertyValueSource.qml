import QtQuick 2.0

Rectangle {
    ColorAnimation on color {
        id: animation
        from: "red"
        to: "darkgray"
        duration: 250
        easing.type: Easing.InOutQuad
    }
}
