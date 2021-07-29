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
            fillGradient: RadialGradient {
                centerX: 100; centerY: 100; centerRadius: 100
                focalX: 100; focalY: 100; focalRadius: 10
                GradientStop { position: 0; color: "#ffffff" }
                GradientStop { position: 0.11; color: "#f9ffa0" }
                GradientStop { position: 0.13; color: "#f9ff99" }
                GradientStop { position: 0.14; color: "#f3ff86" }
                GradientStop { position: 0.49; color: "#93b353" }
                GradientStop { position: 0.87; color: "#264619" }
                GradientStop { position: 0.96; color: "#0c1306" }
                GradientStop { position: 1; color: "#000000" }
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
