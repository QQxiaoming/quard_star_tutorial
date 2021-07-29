import QtQuick 2.0
import QtQml.Models 2.12

ListView {
    width: 100
    height: 100

    function contains(array, value) {
        for (var i = 0; i < array.length; ++i)
            if (array[i] == value)
                return true
        return false
    }

    model: visualModel
    DelegateModel {
        id: visualModel

        objectName: "visualModel"

        groups: [
            DelegateModelGroup { id: visibleItems; objectName: "visibleItems"; name: "visible"; includeByDefault: true },
            DelegateModelGroup { id: selectedItems; objectName: "selectedItems"; name: "selected" }
        ]

        model: myModel
        delegate: Item {
            id: delegate

            objectName: "delegate"
            width: 100
            height: 2
            property variant test1: name
            property variant test2: index
            property variant test3: DelegateModel.itemsIndex
            property variant test4: DelegateModel.inItems
            property variant test5: DelegateModel.visibleIndex
            property variant test6: DelegateModel.inVisible
            property variant test7: DelegateModel.selectedIndex
            property variant test8: DelegateModel.inSelected
            property variant test9: DelegateModel.groups

            function hide() { DelegateModel.inVisible = false }
            function select() { DelegateModel.inSelected = true }
        }
    }
}
