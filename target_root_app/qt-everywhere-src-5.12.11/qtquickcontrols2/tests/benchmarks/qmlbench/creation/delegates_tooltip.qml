import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 500
    delegate: Item {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        width: tooltip.width
        height: tooltip.height
        ToolTip {
            id: tooltip
            visible: true
            text: "ToolTip"
        }
    }
}
