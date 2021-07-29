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
        width: popup.width
        height: popup.height
        Popup {
            id: popup
            visible: true
            modal: (index % 100) === 0
            width: parent.width
            height: parent.height
            Item {
                implicitWidth: 100
                implicitHeight: 100
            }
        }
    }
}
