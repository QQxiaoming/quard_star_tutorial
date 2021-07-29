import QtQml 2.2

QtObject {
    id: window

    property Component itemComponent: Qt.createComponent("MyItem.qml")
    property MyItem item

    property Timer timer: Timer {
        running: true
        interval: 10
        repeat: true
        onTriggered: {
            item = itemComponent.createObject(null, {});
            gc();
        }
    }
}
