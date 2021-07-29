import QtQuick 2.0
import Test 1.0

MyTypeObject {
    id: obj
    scopedEnum: MyTypeObject.MyScopedEnum.ScopedVal1
    intProperty: MyTypeObject.MyScopedEnum.ScopedVal2
    property int listValue: myModel.get(0).myData
    property int noScope: MyTypeObject.ScopedVal1

    function assignNewValue() {
        scopedEnum = MyTypeObject.MyScopedEnum.ScopedVal2
        noScope = MyTypeObject.ScopedVal2
    }

    property ListModel myModel: ListModel {
        ListElement {
            myData: MyTypeObject.MyScopedEnum.ScopedVal3
        }
    }
}
