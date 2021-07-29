import QtQml 2.0
QtObject {
    property Component factory: Qt.createComponent(Qt.resolvedUrl("InitialPropertyTest.qml"), Component.PreferSynchronous)
    property var incubator
    function startIncubation()
    {
        incubator = factory.incubateObject(null, { ok: true, nonExistent: 42 }, Qt.Asynchronous)
    }
}
