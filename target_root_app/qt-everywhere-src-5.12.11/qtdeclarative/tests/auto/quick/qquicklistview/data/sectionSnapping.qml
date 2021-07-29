import QtQuick 2.0

ListView {
    width: 400
    height: 400
    preferredHighlightBegin: 100
    preferredHighlightEnd: 100
    highlightRangeMode: ListView.StrictlyEnforceRange

    model: ListModel {
        ListElement { section: "1" }
        ListElement { section: "1" }
        ListElement { section: "1" }
        ListElement { section: "2" }
        ListElement { section: "2" }
        ListElement { section: "2" }
    }

    delegate: Rectangle {
        width: parent.width
        height: 50
        color: index % 2 ? "lightsteelblue" : "steelblue"
        Text {
            anchors.centerIn: parent
            color: "white"
            text: model.index
        }
    }

    section.property: "section"
    section.delegate: Rectangle {
        width: parent.width
        height: 50
        color: "green"
        Text {
            anchors.centerIn: parent
            color: "white"
            text: "section"
        }
    }

    highlight: Rectangle {
        y: 100
        z: 100
        width: parent.width
        height: 50
        color: "#80FF0000"
    }
}
