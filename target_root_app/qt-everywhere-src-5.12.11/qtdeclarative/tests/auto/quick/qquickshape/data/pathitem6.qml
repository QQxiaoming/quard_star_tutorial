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
            strokeWidth: 4
            strokeColor: "red"
            fillGradient: ConicalGradient {
                centerX: 100; centerY: 100; angle: 45
                GradientStop { position: 0; color: "#00000000" }
                GradientStop { position: 0.10; color: "#ffe0cc73" }
                GradientStop { position: 0.17; color: "#ffc6a006" }
                GradientStop { position: 0.46; color: "#ff600659" }
                GradientStop { position: 0.72; color: "#ff0680ac" }
                GradientStop { position: 0.92; color: "#ffb9d9e6" }
                GradientStop { position: 1.00; color: "#00000000" }
            }
            fillColor: "blue" // ignored with the gradient set
            strokeStyle: ShapePath.DashLine
            dashPattern: [ 1, 4 ]
            startX: 20; startY: 20
            PathLine { x: 180; y: 130 }
            PathLine { x: 20; y: 130 }
            PathLine { x: 20; y: 20 }
        }
    }
}
