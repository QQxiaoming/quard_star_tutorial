import QtQuick 2.0
Item {
    property rect placement: Qt.rect(0, 0, 100, 100)

    function someFunction() { return 42; }

    property rect partialPlacement
    partialPlacement.x: someFunction()
}
