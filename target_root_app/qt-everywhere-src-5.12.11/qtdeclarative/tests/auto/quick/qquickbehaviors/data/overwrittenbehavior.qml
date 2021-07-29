import QtQuick 2.4

Item {
    property bool behaviorTriggered
    property bool someProperty

    ItemWithInnerBehavior {
        //the existence of this property triggers the bug
        property bool iDoAbsolutelyNothing

        Component.onCompleted: parent.someProperty = true
        someValue: parent.someProperty
    }
}
