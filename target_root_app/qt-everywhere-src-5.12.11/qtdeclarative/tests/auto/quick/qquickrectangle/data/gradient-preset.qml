import QtQuick 2.0

Item {
    Rectangle {
        objectName: "enum"
        gradient: Gradient.NightFade
    }
    Rectangle {
        objectName: "string"
        gradient: "NightFade"
    }
    Rectangle {
        objectName: "invalid"
        gradient: -1
    }
}
