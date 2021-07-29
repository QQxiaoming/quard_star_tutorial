import QtQuick 2.0
import "importJsModule.1.mjs" as JSModule

Item {
    property bool test: JSModule.ok()
}
