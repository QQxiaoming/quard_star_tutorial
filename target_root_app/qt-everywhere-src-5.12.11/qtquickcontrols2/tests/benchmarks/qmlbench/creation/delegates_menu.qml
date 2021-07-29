import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 200
    delegate: Item {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        width: menu.width
        height: menu.height
        Menu {
            id: menu
            visible: true
            MenuItem { text: "MenuItem1" }
            MenuItem { text: "MenuItem2" }
            MenuItem { text: "MenuItem3" }
        }
    }
}
