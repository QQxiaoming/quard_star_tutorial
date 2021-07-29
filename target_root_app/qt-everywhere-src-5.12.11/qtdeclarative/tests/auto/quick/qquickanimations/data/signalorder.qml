import QtQuick 2.12


Item {
    id: wrapper
    width: 400; height: 400

    Rectangle {
        id: greenRect
        width: 50; height: 50
        color: "red"

        ColorAnimation on color {
            id: colorAnimation
            objectName: "ColorAnimation"
            to: "green"
            duration: 10
            running: false
        }

        ParallelAnimation {
            id: parallelAnimation
            objectName: "ParallelAnimation"
            running: false
        }
    }
}
