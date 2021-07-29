import QtQuick 2.0

Rectangle {
    width: 240
    height: 320

    MultiPointTouchArea {
        width: parent.width
        height: 160
        minimumTouchPoints: 2
        maximumTouchPoints: 2
        onGestureStarted: gesture.grab()
        touchPoints: [
            TouchPoint { id: point11; objectName: "point11" },
            TouchPoint { id: point12; objectName: "point12" }
        ]
        Rectangle {
            color: "red"
            width: 10; height: 10; radius: 5
            x: point11.x - radius; y: point11.y - radius
            visible: point11.pressed
        }
        Rectangle {
            color: "tomato"
            width: 10; height: 10; radius: 5
            x: point12.x - radius; y: point12.y - radius
            visible: point12.pressed
        }
    }

    MultiPointTouchArea {
        width: parent.width
        height: 160
        y: 160
        minimumTouchPoints: 3
        maximumTouchPoints: 3
        onGestureStarted: gesture.grab()
        touchPoints: [
            TouchPoint { id: point21; objectName: "point21" },
            TouchPoint { id: point22; objectName: "point22" },
            TouchPoint { id: point23; objectName: "point23" }
        ]
        Rectangle {
            color: "lightgreen"
            width: 10; height: 10; radius: 5
            x: point21.x - radius; y: point21.y - radius
            visible: point21.pressed
        }
        Rectangle {
            color: "green"
            width: 10; height: 10; radius: 5
            x: point22.x - radius; y: point22.y - radius
            visible: point22.pressed
        }
        Rectangle {
            color: "darkgreen"
            width: 10; height: 10; radius: 5
            x: point23.x - radius; y: point23.y - radius
            visible: point23.pressed
        }
    }
}
