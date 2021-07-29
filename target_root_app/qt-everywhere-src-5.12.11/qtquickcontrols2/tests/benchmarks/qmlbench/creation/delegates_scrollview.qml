import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 250
    delegate: ScrollView {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        width: 100
        height: 100
        Item {
            implicitWidth: 200
            implicitHeight: 200
        }
        ScrollBar.vertical.active: true
        ScrollBar.vertical.pressed: index % 3 === 1
        ScrollBar.horizontal.active: true
        ScrollBar.horizontal.pressed: index % 3 === 2
    }
}
