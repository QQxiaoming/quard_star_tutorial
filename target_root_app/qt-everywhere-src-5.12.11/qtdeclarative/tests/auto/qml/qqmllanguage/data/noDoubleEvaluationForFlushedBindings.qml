import QtQml 2.0

QtObject {
    property int firstVar: secondVar + 1
    property int secondVar: {
        stats.increaseEvaluationCounter()
        return 1
    }
}
