import QtQuick 2.0

Rectangle {

    function changeGradient() {
        firstStop.color = "red"
        secondStop.color = "blue"
    }

    QtObject {
        id: someObject
        property Gradient someGradient: Gradient {
            GradientStop { id: firstStop; position: 0.0; color: "gray" }
            GradientStop { id: secondStop; position: 1.0; color: "white" }
        }
    }

    gradient: someObject.someGradient
}

