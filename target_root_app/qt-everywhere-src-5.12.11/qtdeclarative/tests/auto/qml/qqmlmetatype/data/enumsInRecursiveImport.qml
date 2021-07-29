import QtQml 2.0

import Components 1.0

QtObject {
    property App app: App {
        appState: 0
    }

    property string color: app.color
}
