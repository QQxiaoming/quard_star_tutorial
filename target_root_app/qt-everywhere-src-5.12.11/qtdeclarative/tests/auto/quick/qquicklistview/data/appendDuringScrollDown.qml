import QtQuick 2.6

ListView {
    width: 320; height: 240
    focus: true
    delegate: Text {
        height: 40; width: parent.width
        text: model.text
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
    highlight: Rectangle { color: "red" }
    model: ListModel {
        ListElement { text: "0" }
        ListElement { text: "1" }
        ListElement { text: "2" }
        ListElement { text: "3" }
        ListElement { text: "4" }
        ListElement { text: "5" }
        ListElement { text: "6" }
        ListElement { text: "7" }
        ListElement { text: "8" }
        ListElement { text: "9" }
    }

    readonly property Item topItem: itemAt(0, contentY)
    onTopItemChanged: model.append({ "text": "new" })
}
