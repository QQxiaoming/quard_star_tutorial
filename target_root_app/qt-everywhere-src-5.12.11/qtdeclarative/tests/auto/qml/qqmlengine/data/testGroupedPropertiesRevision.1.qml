import QtQuick 2.8

Item {
    GroupedPropertiesRevisionComponent1 {
        textEdit.onEditingFinished: console.log("test")
    }
}
