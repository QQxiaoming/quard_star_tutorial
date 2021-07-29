import QtQuick 2.9
import QtQuick.Window 2.3

Item {
    visible: true
    width: 640
    height: 480
    objectName: "componentRoot"

    Rectangle {
        objectName: "topLevelComplete"
        id: rectangle
        x: 378
        y: 91
        width: 100
        height: 100
        color: "#ffffff"
    }

    Item {
        id: element
        x: 14
        y: 39
        width: 120
        height: 120

        Rectangle {
            id: rectangle1
            objectName: "implemented"
            x: 43
            y: 52
            width: 110
            height: 110
            color: "#ffffff"

            Component.onCompleted: {
                rectangle1.color = "blue"
            }
        }
    }

    Component02 {
        id: element1
        x: 88
        y: 251
    }

    Component.onCompleted: {
        rectangle.color = "red"
    }
}
