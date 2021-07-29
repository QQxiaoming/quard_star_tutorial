import QtQuick 2.8

Item {
    GroupedPropertiesRevisionComponent2 {
        textEdit.onEditingFinished: console.log("test")
    }
}
