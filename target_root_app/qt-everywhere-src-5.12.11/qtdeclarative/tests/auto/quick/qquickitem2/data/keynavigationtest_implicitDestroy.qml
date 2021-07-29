import QtQuick 2.12

Item {
    id: root

    function createImplicitKeyNavigation() {
        var item = Qt.createQmlObject("import QtQuick 2.0; Item { }", root);
        item.KeyNavigation.up = root
        item.destroy();

        forceActiveFocus();
    }
}
