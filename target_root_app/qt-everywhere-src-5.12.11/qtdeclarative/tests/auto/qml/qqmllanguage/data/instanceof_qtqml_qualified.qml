import QtQml 2.0 as QmlImport

QmlImport.QtObject {
    id: qtobjectInstance

    property QmlImport.Timer aTimer: QmlImport.Timer {
        id: timerInstance
    }

    property QmlImport.Connections aConnections: QmlImport.Connections {
        id: connectionsInstance
    }
}
