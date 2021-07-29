import QtQml 2.0
import Test 1.0

SimplePropertyMap {
    Component.onCompleted: {
        console.log("expected output")
        newProperty = 42
    }
}
