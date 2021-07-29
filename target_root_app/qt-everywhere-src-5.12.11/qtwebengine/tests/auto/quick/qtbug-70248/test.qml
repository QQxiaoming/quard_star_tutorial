import QtQuick 2.9
import QtQuick.Window 2.2
import QtWebEngine 1.3

Window {
    visible: true
    width: 640
    height: 480

    property var url: view && view.url

    WebEngineView {
        id: view
        anchors.fill: parent
    }
}
