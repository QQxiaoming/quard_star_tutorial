import QtQml 2.2

QtObject {
    property Component some: QtObject {
        property int rrr: 2
        property Component onemore: QtObject {
            property int brrrr: -1
        }
    }
}
