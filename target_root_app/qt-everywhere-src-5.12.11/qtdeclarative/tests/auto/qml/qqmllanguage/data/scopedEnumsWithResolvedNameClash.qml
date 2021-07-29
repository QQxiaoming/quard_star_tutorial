import QtQml 2.0
import ScopedEnumsWithResolvedNameClashTest 1.0

QtObject {
    property bool success: false

    Component.onCompleted: {
        success = (ScopedEnum.ScopedEnum.OtherScopedEnum === 3)
            && (ScopedEnum.OtherScopedEnum.ScopedVal2 === 1)
    }
}
