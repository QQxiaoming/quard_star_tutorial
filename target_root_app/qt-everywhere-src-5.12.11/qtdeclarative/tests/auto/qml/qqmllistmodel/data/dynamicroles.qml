import QtQuick 2.0

Item {
    id: root

    ListModel {
        id: listModel
        objectName: "listModel"
        dynamicRoles: true

        // have to add elements dynamically when dynamicRoles = true
        function appendNewElement() {
            listModel.append({"name": "test", "obj": null})
        }

        function setElementAgain() {
            var element = listModel.get(0)
            listModel.set(0, element)
        }
    }
}
