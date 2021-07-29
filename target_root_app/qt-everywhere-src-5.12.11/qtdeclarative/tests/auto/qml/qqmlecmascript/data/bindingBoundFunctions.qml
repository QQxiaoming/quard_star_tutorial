import QtQuick 2.6

QtObject {
    property bool success: false
    property var num: 100
    property var simple: 0
    property var complex: 0


    Component.onCompleted: {
        function s(x) {
            return x
        }
        function c(x) {
            return x + num
        }

        var bound = s.bind(undefined, 100)
        simple = Qt.binding(bound)
        if (simple != 100)
            return;
        var bound = c.bind(undefined, 100)
        complex = Qt.binding(bound);

        if (complex != 200)
            return;
        num = 0;
        if (complex != 100)
            return;

        print("success!!!");
        success = true;
    }
}
