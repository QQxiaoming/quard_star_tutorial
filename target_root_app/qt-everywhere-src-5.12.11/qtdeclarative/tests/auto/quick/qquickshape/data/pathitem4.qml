import QtQuick 2.9
import tst_qquickpathitem 1.0

Item {
    width: 200
    height: 150

    Shape {
        vendorExtensionsEnabled: false
        objectName: "pathItem"
        anchors.fill: parent

        ShapePath {
            strokeColor: "red"
            fillColor: "green"
            startX: 40; startY: 30
            PathQuad { x: 50; y: 80; controlX: 0; controlY: 80 }
            PathLine { x: 150; y: 80 }
            PathQuad { x: 160; y: 30; controlX: 200; controlY: 80 }
        }

        ShapePath {
            strokeWidth: 10
            fillColor: "transparent"
            strokeColor: "blue"
            startX: 40; startY: 30
            PathCubic { x: 50; y: 80; control1X: 0; control1Y: 80; control2X: 100; control2Y: 100 }
        }

        ShapePath {
            fillGradient: LinearGradient {
                y2: 150
                GradientStop { position: 0; color: "yellow" }
                GradientStop { position: 1; color: "green" }
            }

            startX: 10; startY: 100
            PathArc {
                relativeX: 50; y: 100
                radiusX: 25; radiusY: 25
            }
            PathArc {
                relativeX: 50; y: 100
                radiusX: 25; radiusY: 35
            }
            PathArc {
                relativeX: 50; y: 100
                radiusX: 25; radiusY: 60
            }
            PathArc {
                relativeX: 50; y: 100
                radiusX: 50; radiusY: 120
            }
        }
    }
}
