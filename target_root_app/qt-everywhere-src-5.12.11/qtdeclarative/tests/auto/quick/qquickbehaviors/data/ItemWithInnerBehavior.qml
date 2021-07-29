import QtQuick 2.4

Item {
    id: root

    property bool someValue
    Behavior on someValue {
        ScriptAction { script: { parent.behaviorTriggered = true }}
    }
}
