import QtQuick 2.9
import QtQuick.Shapes 1.0

Item {
    width: 320
    height: 480

    Column {
        Item {
            width: 200
            height: 100

            Repeater {
                model: 2
                Shape {
                    anchors.fill: parent
                    vendorExtensionsEnabled: false

                    ShapePath {
                        fillColor: "transparent"
                        strokeColor: model.index === 0 ? "red" : "blue"
                        strokeStyle: ShapePath.DashLine
                        strokeWidth: 4

                        startX: 4; startY: 4
                        PathArc {
                            id: arc
                            x: 96; y: 96
                            radiusX: 100; radiusY: 100
                            direction: model.index === 0 ? PathArc.Clockwise : PathArc.Counterclockwise
                        }
                    }
                }
            }
        }

        Item {
            width: 200
            height: 100

            Repeater {
                model: 2
                Shape {
                    anchors.fill: parent
                    vendorExtensionsEnabled: false

                    ShapePath {
                        fillColor: "transparent"
                        strokeColor: model.index === 0 ? "red" : "blue"
                        strokeStyle: ShapePath.DashLine
                        strokeWidth: 4

                        startX: 50; startY: 100
                        PathArc {
                            x: 100; y: 150
                            radiusX: 50; radiusY: 50
                            useLargeArc: model.index === 1
                        }
                    }
                }
            }
        }

        Item {
            width: 200
            height: 100

            Repeater {
                model: 2
                Shape {
                    anchors.fill: parent
                    vendorExtensionsEnabled: false

                    ShapePath {
                        fillColor: "transparent"
                        strokeColor: model.index === 0 ? "red" : "blue"
                        strokeStyle: ShapePath.DashLine
                        strokeWidth: 4

                        startX: 50; startY: 150
                        PathArc {
                            x: 150; y: 150
                            radiusX: 50; radiusY: 20
                            xAxisRotation: model.index === 0 ? 0 : 45
                        }
                    }
                }
            }

            Repeater {
                model: 2
                Shape {
                    anchors.fill: parent
                    vendorExtensionsEnabled: false

                    ShapePath {
                        fillColor: "transparent"
                        strokeColor: model.index === 0 ? "red" : "blue"

                        startX: 50; startY: 150
                        PathArc {
                            x: 150; y: 150
                            radiusX: 50; radiusY: 20
                            xAxisRotation: model.index === 0 ? 0 : 45
                            direction: PathArc.Counterclockwise
                        }
                    }
                }
            }
        }
    }
}
