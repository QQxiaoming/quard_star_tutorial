import QtQuick 2.9
import QtQuick.Shapes 1.0

Item {
    width: 320
    height: 480

    Column {
        Repeater {
            model: 3
            Shape {
                vendorExtensionsEnabled: false
                width: 200
                height: 150
                ShapePath {
                    strokeColor: "transparent"

                    fillGradient: LinearGradient {
                        id: grad
                        y1: 50; y2: 80
                        spread: model.index === 0 ? ShapeGradient.PadSpread : (model.index === 1 ? ShapeGradient.RepeatSpread : ShapeGradient.ReflectSpread)
                        GradientStop { position: 0; color: "black" }
                        GradientStop { position: 1; color: "red" }
                    }

                    startX: 10; startY: 10
                    PathLine { relativeX: 180; relativeY: 0 }
                    PathLine { relativeX: 0; relativeY: 100 }
                    PathLine { relativeX: -180; relativeY: 0 }
                    PathLine { relativeX: 0; relativeY: -100 }
                }
            }
        }
    }
}
