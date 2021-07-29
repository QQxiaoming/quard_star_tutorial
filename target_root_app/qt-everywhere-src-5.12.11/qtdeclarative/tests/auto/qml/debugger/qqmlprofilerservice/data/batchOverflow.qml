import QtQml 2.2

QtObject {
    function iterate(dictionaryTable, j) {
        var word = "a" + j.toString()
        dictionaryTable[word] = null;
    }

    Component.onCompleted: {
        var dictionaryTable = {};
        for (var j = 0; j < 256; ++j)
            iterate(dictionaryTable, j);
    }

    property Timer timer: Timer {
        interval: 1
        running: true;
        onTriggered: Qt.quit();
    }
}
