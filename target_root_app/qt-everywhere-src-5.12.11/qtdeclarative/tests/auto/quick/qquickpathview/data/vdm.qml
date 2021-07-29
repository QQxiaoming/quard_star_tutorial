import QtQuick 2.0
import QtQml.Models 2.12

PathView {
    id: pathView
    width: 240; height: 320

    pathItemCount: 4
    preferredHighlightBegin : 0.5
    preferredHighlightEnd : 0.5

    path: Path {
        startX: 120; startY: 20;
        PathLine { x: 120; y: 300 }
    }

    ListModel {
        id: mo
        ListElement { value: "one" }
        ListElement { value: "two" }
        ListElement { value: "three" }
    }

    model: DelegateModel {
        delegate: Text { text: model.value }
        model : mo
    }
}

