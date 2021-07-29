import QtQuick 2.0
import QtTest 1.0

Item {

    Item {
        id : testItem
        property string name : "testObject"
        property var object : this
        function testMethod() {
            return -1;
        }
    }

    ListModel {
        id : listModel
        dynamicRoles : true
    }

    function exec() {
        listModel.append(testItem);
        listModel.append({ item : testItem });
        return true;
    }
}
