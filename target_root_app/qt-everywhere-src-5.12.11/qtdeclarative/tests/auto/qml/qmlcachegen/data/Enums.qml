import QtQml 2.0
QtObject {
    enum Test {
        First = 100,
        Second = 200
    }
    property int value: 0
    Component.onCompleted: value = Enums.Second
}
