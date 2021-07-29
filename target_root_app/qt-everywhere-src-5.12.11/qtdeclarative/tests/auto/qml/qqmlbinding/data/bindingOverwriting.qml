import QtQuick 2.9

Text {
    visible: text && enabled
    enabled: font.pixelSize === 25
    font: enabled ? Qt.font({ "pixelSize": 25 }) : Qt.font({ "pixelSize": 50 })

    Component.onCompleted: {
        enabled = Qt.binding(function() { return visible; }); // replacement binding, not breaking
        visible = true; // breaks visible binding
        font.bold = true; // breaks font binding
    }
}
