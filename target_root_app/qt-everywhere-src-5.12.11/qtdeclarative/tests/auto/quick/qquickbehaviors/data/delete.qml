import QtQuick 2.12

Item {
    visible: true
    width: 640
    height: 480

    Component.onCompleted: {
            myLoader.active = false
    }

    Loader {
        id: myLoader

        active: true
        sourceComponent: Item {
            width: 100
            height: 100
            id: myPopup

            NumberAnimation {
                id: anim
            }

            Rectangle {
                color: "black"
                Component.onCompleted: {
                    opacity = 20
                }

                Behavior on opacity {
                    animation: anim
                }
            }
        }
    }
}
