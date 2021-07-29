import QtQuick 2.0

Item {
    width: 100
    height: 30

    Text {
        width: parent ? parent.width : 0
        height: parent ? parent.height : 0
        elide: Text.ElideRight
        text: "wot"
    }
}
