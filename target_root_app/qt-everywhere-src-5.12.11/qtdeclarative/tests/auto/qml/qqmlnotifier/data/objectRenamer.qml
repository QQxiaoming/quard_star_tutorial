import QtQml 2.2

QtObject {
    property Timer timer: Timer {
        running: true
        interval: 0
        onTriggered: parent.objectName = "havoc"
    }
}
