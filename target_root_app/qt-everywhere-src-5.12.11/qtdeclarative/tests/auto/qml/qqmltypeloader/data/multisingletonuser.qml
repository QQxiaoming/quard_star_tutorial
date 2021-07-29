import QtQml 2.0
import multisingletonmodule 1.0
import cppsingletonmodule 1.0

QtObject {
    property bool ok: Singleton2.ok && CppRegisteredSingleton2.ok
}
