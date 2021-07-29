import QtQuick 2.10
import "singleton/circular"

QtObject {
    property int value: MySingleton.value
}
