import QtQuick 2.0
import "importLexicalVariables.mjs" as TestRunner
import "importLexicalVariables_script.js" as LibraryUnderTest

QtObject {
    id: root
    function runTest() {
        return TestRunner.runTest(LibraryUnderTest);
    }
}
