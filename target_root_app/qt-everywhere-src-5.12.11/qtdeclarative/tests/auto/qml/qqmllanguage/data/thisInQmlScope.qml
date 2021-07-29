import QtQml 2.2
QtObject {
    property int x: 42
    property int y: 0
    function g(){
        y = this.x;
    }
    property var f: g

    Component.onCompleted: f()

    property int a: 42
    property int b: 0
    function g_subobj(){
        b = this.a;
    }
    property var f_subobj: g_subobj

    property QtObject subObject: QtObject {
        property int a: 100
        Component.onCompleted: f_subobj()
    }
}
