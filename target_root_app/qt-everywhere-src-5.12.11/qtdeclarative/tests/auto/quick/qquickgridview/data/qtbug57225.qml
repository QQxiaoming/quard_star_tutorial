import QtQuick 2.0

Rectangle {
    id: root
    width: 200
    height: 200

    property int duration: 100
    property int count: grid.count

    Component {
        id: myDelegate
        Rectangle {
            id: wrapper

            property string nameData: name
            property bool removalStarted: false
            property real minX: 0
            property real minY: 0

            onXChanged: if (removalStarted) grid.recordPosition(x, y)
            onYChanged: if (removalStarted) grid.recordPosition(x, y)

            objectName: "wrapper"
            width: 80
            height: 80
            border.width: 1
            Column {
                Text { text: index }
                Text {
                    text: wrapper.x + ", " + wrapper.y
                }
                Text {
                    id: textName
                    objectName: "textName"
                    text: name
                }
            }
            color: GridView.isCurrentItem ? "lightsteelblue" : "white"

            GridView.onRemove: SequentialAnimation {
                PropertyAction { target: wrapper; property: "removalStarted"; value: true }
                PropertyAction { target: wrapper; property: "GridView.delayRemove"; value: true }
                NumberAnimation { target: wrapper; property: "scale"; to: 0.5; duration: root.duration; easing.type: Easing.InOutQuad }
                PropertyAction { target: wrapper; property: "GridView.delayRemove"; value: false }
                PropertyAction { target: grid; property: "animationDone"; value: true }
            }

        }
    }

    GridView {
        id: grid

        property int displaceTransitionsDone: 0
        property bool animationDone: false
        property point minimumPosition: Qt.point(0, 0)

        signal delegateMoved(real x, real y)

        objectName: "grid"
        focus: true
        anchors.fill: parent
        cacheBuffer: 0
        cellWidth: 80
        cellHeight: 80
        model: testModel
        delegate: myDelegate

        displaced: Transition {
            id: transition
            SequentialAnimation {
                NumberAnimation {
                    properties: "x,y"
                    duration: root.duration
                    easing.type: Easing.OutBounce
                }
                ScriptAction { script: grid.displaceTransitionsDone += 1 }
            }
        }

        function recordPosition(index, x, y) {
            if (x < minimumPosition.x || y < minimumPosition.y) {
                minimumPosition = Qt.point(x, y)
            }
        }
    }

    Rectangle {
        anchors.fill: grid
        color: "lightsteelblue"
        opacity: 0.2
    }
}

