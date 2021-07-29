import QtQml 2.0
import "script.mjs" as Script

QtObject {
    property bool ok: Script.ok()
}
