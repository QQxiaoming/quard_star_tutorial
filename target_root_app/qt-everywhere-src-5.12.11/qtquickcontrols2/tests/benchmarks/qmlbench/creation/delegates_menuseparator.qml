import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 2000
    delegate: MenuSeparator {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
    }
}
