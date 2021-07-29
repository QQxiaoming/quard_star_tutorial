pragma Singleton
import QtQuick 2.10

QtObject {
    enum MyEnum {
        Value0,
        Value1,
        Value2
    }

    property int value: MySingleton.Value2
}
