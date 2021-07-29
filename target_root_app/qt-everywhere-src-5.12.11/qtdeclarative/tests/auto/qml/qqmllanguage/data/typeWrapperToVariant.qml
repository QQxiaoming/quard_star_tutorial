import QtQml 2.0

QtObject {
    id: root

    property QtObject target: QtObject {
        Component.onCompleted: {
            root.connections.target = root.target.Component
        }
    }

    property Connections connections: Connections {
        ignoreUnknownSignals: true
    }
}
