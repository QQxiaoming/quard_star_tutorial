import QtQml 2.0

import constants 1.0

QtObject {
    function createClosure() {
        return function() { return Sing.song; }
    }
    function createComponentFactory() {
        return function(parentObj) {
            return Qt.createQmlObject('import QtQml 2.0; QtObject { property string test: "ok"; }', parentObj);
        }
    }
}
