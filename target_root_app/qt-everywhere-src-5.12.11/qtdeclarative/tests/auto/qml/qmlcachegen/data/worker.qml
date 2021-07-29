import QtQml 2.0
import QtQuick 2.0
QtObject {
    property bool success: false
    property WorkerScript worker: WorkerScript {
        source: "worker.js"
        onMessage: {
            success = true
        }
    }
    Component.onCompleted: worker.sendMessage("Hello")
}
