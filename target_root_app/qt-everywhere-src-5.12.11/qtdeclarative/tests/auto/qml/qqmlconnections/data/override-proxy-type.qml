import QtQml 2.12
import test.proxy 1.0

Proxy {
    property int testEnum: 0;
    id: proxy
    property Connections connections: Connections {
        target: proxy
        onSomeSignal: testEnum = Proxy.EnumValue;
    }

    Component.onCompleted: someSignal()
}
