import QtQml 2.0
QtObject {
    id: root
    property bool testFunc;
    property bool ok: false
    property QtObject subObject: QtObject {
        function testFunc()
        {
            root.ok = true
        }

        Component.onCompleted: testFunc()
    }
}
