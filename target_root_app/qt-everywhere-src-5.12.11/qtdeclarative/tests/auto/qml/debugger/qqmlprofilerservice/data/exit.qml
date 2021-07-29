import QtQml 2.0

QtObject {
    property Timer timer: Timer {
        running: true
        interval: 1
        onTriggered: Qt.quit();
    }
}
