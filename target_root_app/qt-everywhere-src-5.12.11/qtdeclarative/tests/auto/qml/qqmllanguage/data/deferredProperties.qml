import QtQml 2.0
import Test 1.0
MyDeferredProperties {
    groupProperty: QtObject {
        objectName: "outerobj"
        property bool wasCompleted: false
        Component.onCompleted: wasCompleted = true
    }
    QtObject {
        objectName: "outerlist1"
        property bool wasCompleted: false
        Component.onCompleted: wasCompleted = true
    }
    QtObject {
        objectName: "outerlist2"
        property bool wasCompleted: false
        Component.onCompleted: wasCompleted = true
    }
}
