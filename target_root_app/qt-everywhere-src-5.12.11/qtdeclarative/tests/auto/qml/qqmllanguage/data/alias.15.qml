import QtQuick 2.6

Item {
    id: root

    property alias symbol: symbol
    symbol.y: 1

    Item {
        id: symbol
    }
}
