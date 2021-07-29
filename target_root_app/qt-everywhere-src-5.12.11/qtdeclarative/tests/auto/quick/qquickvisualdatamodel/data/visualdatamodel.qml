import QtQuick 2.0
import QtQml.Models 2.12

DelegateModel {
    function setRoot() {
        rootIndex = modelIndex(0);
    }
    function setRootToParent() {
        rootIndex = parentModelIndex();
    }
    model: myModel
    delegate: Item { property bool modelChildren: hasModelChildren }
}
