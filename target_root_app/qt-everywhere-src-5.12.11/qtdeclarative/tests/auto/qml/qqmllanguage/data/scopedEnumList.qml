import QtQuick 2.0
import Test 1.0

MyTypeObject {
    property ListModel myModel: ListModel {
        ListElement {
            myData: MyTypeObject.MyScopedEnum
        }
    }
}
