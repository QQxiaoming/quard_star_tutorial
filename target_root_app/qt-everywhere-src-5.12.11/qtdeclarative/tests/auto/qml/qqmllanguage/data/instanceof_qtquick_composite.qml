import QtQuick 2.0
import "instanceOf"

Item {
    id: itemInstance

    Rectangle {
        id: rectangleInstance
    }

    MouseArea {
        id: mouseAreaInstance
    }

    CustomRectangle {
        id: customRectangleInstance
    }
    CustomRectangleWithProp {
        id: customRectangleWithPropInstance
    }
    CustomMouseArea {
        id: customMouseAreaInstance
    }
}


