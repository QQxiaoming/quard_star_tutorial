import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12

Row {
    width: 200
    height: 200

    IconImage {
        name: "appointment-new"
        sourceSize: Qt.size(22, 22)
    }
    Image {
        source: "qrc:/icons/testtheme/22x22/actions/appointment-new.png"
    }
}
