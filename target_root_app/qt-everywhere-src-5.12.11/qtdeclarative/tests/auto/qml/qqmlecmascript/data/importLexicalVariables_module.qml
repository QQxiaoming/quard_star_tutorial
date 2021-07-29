import QtQuick 2.0
import "importLexicalVariables.mjs" as TestRunner
import "importLexicalVariables_module.mjs" as LibraryUnderTest

QtObject {
    id: root
    function runTest() {
        return TestRunner.runTest(LibraryUnderTest);
    }
}
