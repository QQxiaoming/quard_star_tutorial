import QtQuick 2.0
import QtRemoteObjects 5.12
import usertypes 1.0

Item {
    property int result: replica.clock.hour

    property Node sharedNode: Node {
        registryUrl: "local:testSubObject"
    }

    property TypeWithSubObjectReplica replica: TypeWithSubObjectReplica {
        node: sharedNode
        onStateChanged: if (state == TypeWithSubObjectReplica.Valid) clock.pushHour(7)
    }
}

