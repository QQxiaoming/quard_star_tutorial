import QtQuick 2.6

Item {
    visible: true
    Item {
        visible: false
        Item {
            objectName: "hiddenChild"
            activeFocusOnTab: true
            focus: true
        }
    }
}
