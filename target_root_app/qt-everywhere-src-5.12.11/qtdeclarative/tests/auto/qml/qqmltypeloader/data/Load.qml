import QtQuick 2.0

Item {
    property int xy: loader.xy
    Loader {
        id: loader
        asynchronous: true
        source: 'Base.qml'
        property int xy: item.xy
    }
}
