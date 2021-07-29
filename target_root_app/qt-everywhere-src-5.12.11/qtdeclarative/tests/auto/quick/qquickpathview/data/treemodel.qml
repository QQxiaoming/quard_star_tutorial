import QtQuick 2.0
import QtQml.Models 2.12

PathView {
    width: 320
    height: 240
    function setRoot(index) {
        vdm.rootIndex = vdm.modelIndex(index);
    }
    model: DelegateModel {
        id: vdm
        model: myModel
        delegate: Text { objectName: "wrapper"; text: display }
    }

    path: Path {
        startX: 0; startY: 120
        PathLine { x: 320; y: 120 }
    }
}
