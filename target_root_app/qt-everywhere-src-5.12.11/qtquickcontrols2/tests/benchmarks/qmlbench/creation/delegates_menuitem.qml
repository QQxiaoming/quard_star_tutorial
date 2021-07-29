import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 1000
    delegate: MenuItem {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        text: "MenuItem"
        checkable: index % 2
        checked: index % 2
        down: index % 3
    }
}
