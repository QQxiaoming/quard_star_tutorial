import QtQuick 2.0
import QtQml.Models 2.2
import QtQuick.Window 2.0

Window {
    width: 300
    height: 300
    visible: true
    DelegateModel {
        id: mdl

        model: 1
        delegate: Package {
            Item {
                id: first
                Package.name: "first"
                objectName: "firstItem"
            }
            Item{
                id: second
                Package.name: "second"
                objectName: "secondItem"
            }
        }
    }

    Repeater {
        id: repeater1
        model: mdl.parts.first
    }
    Repeater {
        id: repeater2
        model: mdl.parts.second
    }
}
