import QtQuick 2.0

Item {
    width: 320
    height: 480

    Text {
        anchors.centerIn: parent
        font.family: "Arial"
        font.pixelSize: 16
        textFormat: Text.RichText
        text: "<table><tr><td/><td valign=\"top\"><img src=\"data/logo.png\"></td></tr></table>"
    }
}
