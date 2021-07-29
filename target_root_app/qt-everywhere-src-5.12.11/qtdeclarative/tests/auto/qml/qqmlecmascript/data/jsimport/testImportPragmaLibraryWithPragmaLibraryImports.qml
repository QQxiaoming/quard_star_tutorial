import QtQuick 2.0
import "importPragmaLibraryWithPragmaLibraryImports.js" as LibraryImport

QtObject {
    id: root
    property int testValue: LibraryImport.importIncrementedValue(); // 16
}
