import QtQml 2.2

QtObject {
    property var result;

    property Connections connections: Connections {
        target: emitter
        onEmitGadget: function(gadget) {
            result = gadget.someProperty;
        }
    }
}
