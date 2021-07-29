import QtQuick 2.3
import QtQuick.Window 2.2

Item {
    property bool finished: loader.status === Loader.Ready && loader.progress === 1

    ListModel {
        id: listModel
        ListElement { i:0 }
        ListElement { i:1 }
        ListElement { i:2 }
        ListElement { i:3 }
    }

    Timer {
        running: true
        interval: 1
        repeat: count < 5
        property int count : 0

        onTriggered: {
            listModel.move(listModel.count - 1, listModel.count - 2, 1)
            ++count
        }
    }

    Loader {
        id: loader
        asynchronous: true
        sourceComponent: Row {
            spacing: 4
            Repeater {
                model: listModel
                delegate: Column {
                    spacing: 4
                    Repeater {
                        model: 4
                        delegate: Rectangle {
                            width: 30
                            height: 30
                            color: "blue"
                            Component.onCompleted: {
                                ctrl.wait()
                            }
                        }
                    }
                }
            }
        }
    }
}
