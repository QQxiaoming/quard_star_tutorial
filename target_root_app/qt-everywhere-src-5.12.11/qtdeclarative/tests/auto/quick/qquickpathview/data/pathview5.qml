import QtQuick 2.0

PathView {
    property int countclick: 0
    id: pathview
    y: 0
    width: 348
    height: 480

    interactive: false

    cacheItemCount: 10
    currentIndex: 2
    pathItemCount: 4
    highlightMoveDuration: 1000
    highlightRangeMode : PathView.StrictlyEnforceRange
    preferredHighlightBegin: 0.5
    preferredHighlightEnd: 0.5
    snapMode : PathView.SnapOneItem

    path: Path {
        id: leftPath
        startX: pathview.width / 2 - 800
        startY: pathview.height / 2 - 800

        PathArc {
            x: pathview.width / 2 - 800
            y: pathview.height / 2 + 800
            radiusX: 800
            radiusY: 800
            direction: PathArc.Clockwise
        }
    }

    model: ListModel {
        id: model
        ListElement { objectName:"aqua"; name: "aqua" ;mycolor:"aqua"}
        ListElement { objectName:"blue"; name: "blue" ;mycolor:"blue"}
        ListElement { objectName:"blueviolet"; name: "blueviolet" ;mycolor:"blueviolet"}
        ListElement { objectName:"brown"; name: "brown" ;mycolor:"brown"}
        ListElement { objectName:"chartreuse"; name: "chartreuse" ;mycolor:"chartreuse"}
    }

    delegate: Item {
        id: revolveritem
        objectName: model.objectName

        width: pathview.width
        height: pathview.height

        Rectangle
        {
            id:myRectangle
            color: mycolor
            width: pathview.width -20
            height: pathview.height -20

            Text {
                anchors.centerIn: parent
                text: "index:"+index
                color: "white"
            }
        }
    }
}
