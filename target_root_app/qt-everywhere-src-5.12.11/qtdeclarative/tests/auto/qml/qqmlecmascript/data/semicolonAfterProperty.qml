import QtQml 2.0

QtObject {
    property var field: { "key": "value"};
    property list<QtObject> mylist: [
        QtObject    {id: a},
        QtObject {id: b}
    ];
    property var object: QtObject {};
}
