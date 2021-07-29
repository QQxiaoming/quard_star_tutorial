import QtQuick 2.9
import QtQuick.Window 2.3

Item {
    id: element1
    width: 200
    height: 200
    objectName: "inner"


    Rectangle {
        id: rectangle2
        objectName: "most inner"
        x: 59
        y: 51
        width: 200
        height: 200
        color: "#ffffff"
    }

    Component.onCompleted: {
        rectangle2.color = "green"
    }
}
