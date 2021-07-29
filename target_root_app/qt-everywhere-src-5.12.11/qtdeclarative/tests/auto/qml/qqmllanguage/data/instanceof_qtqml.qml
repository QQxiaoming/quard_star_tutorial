import QtQml 2.0

QtObject {
    id: qtobjectInstance

    property Timer aTimer: Timer {
        id: timerInstance
    }

    property Connections aConnections: Connections {
        id: connectionsInstance
    }
}
