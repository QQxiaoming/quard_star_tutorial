import QtQml 2.0
import ScopedEnumsWithNameClashTest 1.0

QtObject {
    property bool success: false

    Component.onCompleted: {
        success = (ScopedEnum.ScopedEnum.OtherScopedEnum === 3)
    }
}
