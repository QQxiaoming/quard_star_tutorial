import QtQml 2.0
import "ContextLeak.js" as ContextLeak
QtObject {
    property int value: ContextLeak.value
}
