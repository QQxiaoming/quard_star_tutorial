pragma Singleton
import QtQuick 2.0
import cppsingletonmodule 1.0

Item {
    property bool ok: CppRegisteredSingleton1.ok
}
