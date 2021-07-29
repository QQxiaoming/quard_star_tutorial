import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12

Row {
    width: 200
    height: 200

    IconImage {
        source: "qrc:/icons/testtheme/22x22/actions/color-test-original.png"
        sourceSize: Qt.size(22, 22)
        color: "red"
    }
    Image {
        source: "qrc:/icons/testtheme/22x22/actions/color-test-tinted.png"
        fillMode: Image.Pad
    }
}
