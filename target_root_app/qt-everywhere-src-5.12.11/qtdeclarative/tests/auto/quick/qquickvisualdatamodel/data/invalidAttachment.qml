import QtQuick 2.0
import QtQml.Models 2.12

Item {
    property DelegateModel invalidVdm: DelegateModel.model
    Repeater {
        model: 1
        delegate: Item {
            id: outer
            objectName: "delegate"

            property DelegateModel validVdm: outer.DelegateModel.model
            property DelegateModel invalidVdm: inner.DelegateModel.model

            Item {
                id: inner
            }
        }
    }
}
