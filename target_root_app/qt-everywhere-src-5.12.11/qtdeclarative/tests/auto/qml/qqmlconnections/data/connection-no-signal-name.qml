import QtQuick 2.4

Item {
    id: blaBlaBla
    function hint() {
    }

    Connections {
        //target: blaBlaBla
        //onHint: hint();
        on: true
    }
}


