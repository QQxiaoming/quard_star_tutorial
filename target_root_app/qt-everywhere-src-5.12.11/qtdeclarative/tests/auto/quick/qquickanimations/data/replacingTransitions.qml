import QtQuick 2.9

Rectangle {
    id: theRoot
    property alias model: theModel
    property alias addTimer: addToModel
    property alias addTransition: addTrans
    property alias displaceTransition: displaceTrans

    width: 400
    height: 400

    ListModel {
        id: theModel
    }
    Timer {
        id: addToModel
        interval: 1000
        running: false
        repeat: true
        onTriggered: {
            theModel.insert(0, {"name": "item " + theModel.count})
            if (theModel.count > 2)
                stop()
        }
    }
    Component {
        id: listDelegate
        Text {
            text: name
        }
    }
    ListView {
        id: listView

        property int animationDuration: 10000

        anchors.fill: parent
        model: theModel
        delegate: listDelegate
        add: Transition {
            id: addTrans
            NumberAnimation { properties: "x"; from: 400; duration: listView.animationDuration }
            NumberAnimation { properties: "y"; from: 400; duration: listView.animationDuration }
        }
        addDisplaced: Transition {
            id: displaceTrans
            NumberAnimation { properties: "x,y"; duration: listView.animationDuration }
        }
    }
}
