import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12

Row {
    width: 200
    height: 200

    IconImage {
        source: "qrc:/data/icons/testtheme/appointment-new.svg"
        sourceSize: Qt.size(22, 22)
    }
    Image {
        source: "qrc:/data/icons/testtheme/appointment-new.svg"
        sourceSize: Qt.size(22, 22)
    }
}
