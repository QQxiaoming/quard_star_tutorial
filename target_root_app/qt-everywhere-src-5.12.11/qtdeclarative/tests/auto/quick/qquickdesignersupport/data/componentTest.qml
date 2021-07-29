import QtQuick 2.9
import QtQuick.Window 2.3

Item {
    visible: true
    width: 640
    height: 480

    Component01 {
        id: rectangle
        x: 205
        y: 70
        width: 251
        height: 242
    }

    Item {
        id: element
        x: 14
        y: 39
        width: 285
        height: 304

        Rectangle {
            id: rectangle1
            x: 43
            y: 52
            width: 200
            height: 200
            color: "#ffffff"
        }
    }
}
