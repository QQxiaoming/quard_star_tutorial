import QtQml 2.2

import "$(INTERCEPT)" as Intercepted

QtObject {
    property QtObject view: Intercepted.View {}
}
