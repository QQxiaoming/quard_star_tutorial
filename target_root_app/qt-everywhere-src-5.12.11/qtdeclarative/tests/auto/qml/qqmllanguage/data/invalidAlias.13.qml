import QtQml 2.0

QtObject {
    property alias dataValue: dataVal

    invalidAliasComponent {
        id: dataVal
        strValue: "value2"
    }
}
