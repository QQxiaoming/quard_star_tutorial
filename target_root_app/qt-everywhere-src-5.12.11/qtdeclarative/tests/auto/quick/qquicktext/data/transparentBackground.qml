import QtQuick 2.0

Rectangle {
    width: 200
    height: 200
    color: "white"
    Text {
        objectName: "text"
        textFormat: Text.RichText
        anchors.fill: parent
        color: "black"
        text: "<h1 style=\"background-color:rgba(255,255,255,0.00)\">foo</h1>"
        verticalAlignment: Text.AlignTop
        horizontalAlignment: Text.AlignLeft
    }
}
