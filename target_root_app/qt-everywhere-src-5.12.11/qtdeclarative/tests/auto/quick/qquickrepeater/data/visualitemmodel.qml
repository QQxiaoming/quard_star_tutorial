import QtQuick 2.0
import QtQml.Models 2.12

Rectangle {
    width: 360
    height: 360

    ObjectModel {
        id: visItemModel
        Rectangle {
            width: 20
            height: 20
            color: "red"
        }
    }

    Column {
        anchors.fill: parent
        Repeater {
            model: visItemModel
        }
    }
}

