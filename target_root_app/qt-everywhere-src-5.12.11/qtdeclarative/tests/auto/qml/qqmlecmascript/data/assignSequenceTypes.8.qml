import QtQml 2.0
QtObject {
    function tryWritingReadOnlySequence() {
        try {
            Qt.application.arguments.push("hello")
        } catch (e) {

            try {
                Qt.application.arguments.sort()
            } catch (e) {
                return true
            }
        }
        return false
    }
}
