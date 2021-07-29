import QtQuick 2.0
import QtQml.Models 2.12

ListView {
    width: 100
    height: 100
    model: DelegateModel {
        id: visualModel
        objectName: "visualModel"
        model: myModel
        delegate: Component {
            Rectangle {
                height: 25
                width: 100
                Text { objectName: "display"; text: display }
            }
        }
    }
}
