import QtQuick 2.9
import QtQuick.Shapes 1.0

Item {
    width: 320
    height: 480

    Shape {
        vendorExtensionsEnabled: false

        anchors.fill: parent

        ShapePath {
            strokeWidth: 1
            strokeColor: "red"
            fillColor: "transparent"
            PathLine { x: 50; y: 50 }
        }
        ShapePath {
            strokeWidth: 2
            strokeColor: "blue"
            fillColor: "transparent"
            startX: 20
            PathLine { x: 70; y: 50 }
        }
        ShapePath {
            strokeWidth: 3
            strokeColor: "green"
            fillColor: "transparent"
            startX: 40
            PathLine { x: 90; y: 50 }
        }
        ShapePath {
            strokeWidth: 4
            strokeColor: "yellow"
            fillColor: "transparent"
            startX: 60
            PathLine { x: 110; y: 50 }
        }
        ShapePath {
            strokeWidth: 5
            strokeColor: "black"
            fillColor: "transparent"
            strokeStyle: ShapePath.DashLine
            startX: 80
            PathLine { x: 130; y: 50 }
        }

        ShapePath {
            strokeWidth: 20
            strokeColor: "gray"
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: 120; startY: 20
            PathLine { x: 200; y: 100 }
        }

        ShapePath {
            strokeColor: "black"
            strokeWidth: 16
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.BevelJoin
            startX: 20
            startY: 100
            PathLine { x: 120; y: 200 }
            PathLine { x: 50; y: 200 }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 16
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.MiterJoin
            startX: 150
            startY: 100
            PathLine { x: 250; y: 200 }
            PathLine { x: 180; y: 200 }
        }
        ShapePath {
            strokeColor: "black"
            strokeWidth: 16
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            startX: 270
            startY: 100
            PathLine { x: 310; y: 200 }
            PathLine { x: 280; y: 200 }
        }
    }
}
