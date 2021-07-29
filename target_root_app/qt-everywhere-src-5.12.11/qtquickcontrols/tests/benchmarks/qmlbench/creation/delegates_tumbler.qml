import QtQuick 2.0
import QmlBench 1.0
import QtQuick.Extras 1.4

CreationBenchmark {
    id: root
    count: 20
    staticCount: 250
    delegate: Tumbler {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        TumblerColumn {
            model: 5
        }
    }
}
