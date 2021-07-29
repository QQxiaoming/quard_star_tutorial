import QtQuick 2.12
import MyPlugin 1.0

Item {
    width: MySettings.baseWidth
    height: 100

    MyComponent {
        anchors.centerIn: parent
    }
}
