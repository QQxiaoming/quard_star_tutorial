import QtQuick 2.12
import "imports/MyPlugin"

Item {
    width: MySettings.baseWidth
    height: 100

    MyComponent {
        anchors.centerIn: parent
    }
}
