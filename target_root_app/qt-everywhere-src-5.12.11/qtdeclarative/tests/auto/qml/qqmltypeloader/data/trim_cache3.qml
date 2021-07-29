import QtQuick 2.0

Item {
    width: 400
    height: 400

    property alias source: loader.source

    Loader {
        id: loader
        source: "ComponentWithIncubator.qml"
    }
}

