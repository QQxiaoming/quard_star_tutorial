import QtQml 2.0
import Test 1.0
DeferredProperties {
    groupProperty: QtObject {
        objectName: "innerobj"
        property bool wasCompleted: false
        Component.onCompleted: wasCompleted = true
    }
    QtObject {
        objectName: "innerlist1"
        property bool wasCompleted: false
        Component.onCompleted: wasCompleted = true
    }
    QtObject {
        objectName: "innerlist2"
        property bool wasCompleted: false
        Component.onCompleted: wasCompleted = true
    }
}
