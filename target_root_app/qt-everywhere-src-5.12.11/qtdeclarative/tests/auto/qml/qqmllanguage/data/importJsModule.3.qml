import QtQuick 2.0
import "importJsModule.3.mjs" as JSModule

Item {
    property bool test: JSModule.ok()
}
