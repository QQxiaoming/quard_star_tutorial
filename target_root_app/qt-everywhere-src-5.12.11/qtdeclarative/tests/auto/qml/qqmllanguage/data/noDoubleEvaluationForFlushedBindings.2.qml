import QtQml 2.0

QtObject {
    property int secondVar: {
        stats.increaseEvaluationCounter()
        return 1
    }
    property int firstVar: secondVar + 1
}
