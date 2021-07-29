import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 2000
    delegate: Page {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        width: 100
        height: 100
        header: Item { }
        footer: Item { }
        Item { }
    }
}
