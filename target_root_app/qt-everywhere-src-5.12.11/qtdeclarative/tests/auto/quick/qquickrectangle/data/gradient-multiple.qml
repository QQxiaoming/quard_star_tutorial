import QtQuick 2.0

Item {
    property alias firstRectangle: r1
    property alias secondRectangle: r2
    Rectangle {
        id: r1
        gradient: someObject.someGradient
        anchors.fill: parent
    }
    Rectangle {
        id: r2
        gradient: someObject.someGradient
        anchors.fill: parent
    }

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
}

