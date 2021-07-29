import QtQuick 2.0

TranslationChangeBase {
    id: root

    ListModel {
        id: listModel
        ListElement {
            text: qsTr("translate me")
        }
    }

    baseProperty: "do not translate"
    property string text1: qsTr("translate me")
    function weDoTranslations() {
        return qsTr("translate me")
    }
    property string text2: weDoTranslations()
    property string text3
    property string fromListModel: listModel.get(0).text

    states: [
        State {
            name: "default"
            when: 1 == 1
            PropertyChanges {
                target: root
                text3: qsTr("translate me")
            }
        }
    ]
}
