import QtQml 2.0

QtObject {
    Component.onCompleted: {
        console.log("End: " + Qt.application.arguments[1]);
        Qt.quit()
    }
}
