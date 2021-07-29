import QtQuick 2.12
import QtQuick.Layouts 1.12

Item {
    implicitWidth: children[0].implicitWidth * children[0].scale
    implicitHeight: children[0].implicitHeight * children[0].scale

    Layout.alignment: Qt.AlignHCenter
}
