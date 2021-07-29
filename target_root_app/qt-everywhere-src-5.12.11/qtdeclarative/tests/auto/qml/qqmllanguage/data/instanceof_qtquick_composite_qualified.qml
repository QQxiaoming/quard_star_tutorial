import QtQuick 2.0 as QuickImport
import "instanceOf" as CustomImport

QuickImport.Item {
    id: itemInstance

    QuickImport.Rectangle {
        id: rectangleInstance
    }

    QuickImport.MouseArea {
        id: mouseAreaInstance
    }

    CustomImport.CustomRectangle {
        id: customRectangleInstance
    }
    CustomImport.CustomRectangleWithProp {
        id: customRectangleWithPropInstance
    }
    CustomImport.CustomMouseArea {
        id: customMouseAreaInstance
    }
}



