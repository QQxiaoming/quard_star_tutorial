import QtQml 2.0
import "script.js" as Script

QtObject {
    property int value: Script.getter()
}
