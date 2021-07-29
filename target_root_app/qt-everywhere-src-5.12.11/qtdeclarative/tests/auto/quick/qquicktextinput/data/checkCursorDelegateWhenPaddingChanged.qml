import QtQuick 2.12

Rectangle {
    width: 200
    height: 200
    TextInput {
        objectName: "textInput"
        leftPadding: 10
        focus: true
        cursorDelegate: Rectangle {
            objectName: "cursorDelegate"
            width: 5
            color: "red"
        }
    }
}
