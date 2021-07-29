import QtQuick 2.9
import QtQuick.Shapes 1.0

Item {
    width: 320
    height: 480

    Column {
        Repeater {
            model: 4
            Item {
                width: 200
                height: 100

                Shape {
                    anchors.fill: parent
                    vendorExtensionsEnabled: false

                    ShapePath {
                        strokeWidth: (model.index + 2) * 2
                        strokeColor: "black"
                        fillColor: "lightBlue"

                        startX: 50; startY: 100
                        PathQuad {
                            x: 150; y: 100
                            controlX: model.index * 10; controlY: model.index * 5
                        }
                    }
                }
            }
        }
    }
}
