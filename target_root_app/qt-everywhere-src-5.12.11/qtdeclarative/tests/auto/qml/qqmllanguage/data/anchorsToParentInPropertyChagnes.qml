import QtQuick 2.9

Item {
    width: 200
    property int edgeWidth: edge.edgeWidth
    EdgeObject {
        id: edge
        anchors.fill: parent
    }
}
