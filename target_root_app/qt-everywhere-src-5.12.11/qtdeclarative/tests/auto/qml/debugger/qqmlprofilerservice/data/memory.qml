import QtQml 2.0

Timer {
    interval: 1
    running: true

    function recurse(i) {
        var x = { t: [1, 2, 3, 4] }
        console.log(x.t[i]);
        if (i < 3)
            recurse(i + 1);
        else
            Qt.quit();
    }

    onTriggered: recurse(0)
}
