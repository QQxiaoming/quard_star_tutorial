import QtQuick 2.12
import QmlBench 1.0
import QtQuick.Controls 2.12

CreationBenchmark {
    id: root
    count: 20
    staticCount: 500
    delegate: TextArea {
        x: QmlBench.getRandom() * root.width - width
        y: QmlBench.getRandom() * root.height - height
        text: "Text\nArea"
    }
}
