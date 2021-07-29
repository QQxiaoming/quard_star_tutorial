import QtQuick 2.0
import QtQml.Models 2.12

DelegateModel {
    id: visualModel

    objectName: "visualModel"

    groups: [
        DelegateModelGroup { id: visibleItems; objectName: "visibleItems"; name: "visible"; includeByDefault: true },
        DelegateModelGroup { id: selectedItems; objectName: "selectedItems"; name: "selected" },
        DelegateModelGroup { id: unnamed; objectName: "unnamed" },
        DelegateModelGroup { id: capitalised; objectName: "capitalised"; name: "Capitalised" }
    ]
}
