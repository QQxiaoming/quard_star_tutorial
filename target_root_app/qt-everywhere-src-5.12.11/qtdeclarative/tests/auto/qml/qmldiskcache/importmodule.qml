import QtQml 2.0
import "module.mjs" as Module
QtObject {
    property bool ok: Module.ok()
}
