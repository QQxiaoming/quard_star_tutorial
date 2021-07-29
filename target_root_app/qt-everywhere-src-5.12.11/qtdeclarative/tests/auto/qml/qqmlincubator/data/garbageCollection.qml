import QtQuick 2.0

QtObject {
    id: root

    property var incubator

    function getAndClearIncubator() {
        var result = incubator
        incubator = null
        return result
    }

    Component.onCompleted: {
        var c = Qt.createComponent("statusChanged.qml"); // use existing simple type for convenience
        var incubator = c.incubateObject(root);
        incubator.onStatusChanged = function(status) { if (status === 1) root.incubator = incubator }
    }
}
