import QtQml 2.0

Timer {
    property string stuff: qsTr("foo")

    running: true
    interval: 1
    onTriggered: Qt.quit();
}
