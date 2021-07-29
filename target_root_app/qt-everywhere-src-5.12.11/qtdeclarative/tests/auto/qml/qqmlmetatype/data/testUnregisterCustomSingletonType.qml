import QtQuick 2.7
import mytypes 1.0

Item {
    id: root
    property string text: StaticProvider.singletonGetString()
}

