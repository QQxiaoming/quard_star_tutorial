import QtQuick 2.0

Item {
    id: root

    Component.onCompleted: {
        var createdObject = objectCreator.create()
        createdObject.del()
        // Shouldn't crash.
        var test = "index" in createdObject
    }
}
