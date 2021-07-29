import QtQuick 2.11
import QtQuick.Window 2.11
import Test 1.0

Window {
    id: window
    visible: true
    width: 200
    height: 200

    property EventSender sender: EventSender { }

    Item {
        width: 200
        height: 200

        MouseArea {
            anchors.fill: parent
        }

        Item {
            width: 200
            height: 200

            Rectangle {
                width: 200
                height: 100
                color: "red"

                MouseArea {
                    anchors.fill: parent
                    onPressed: sender.sendMouseClick(window, 50, 50)
                }
            }

            Rectangle {
                y: 100
                width: 200
                height: 100
                color: "yellow"

                MouseArea {
                    anchors.fill: parent
                    onPressed: sender.sendMouseClick(window, 50, 50)
                }
            }
        }
    }
}
