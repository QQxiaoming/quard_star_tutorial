import Qt3D.Core 2.10
import Qt3D.Render 2.10
import Qt3D.Input 2.0
import Qt3D.Extras 2.10

Entity {
    id: root

    property Effect effect: skinnedPbrEffect
    property url source: ""
    property alias createJointsEnabled: skeleton.createJointsEnabled
    property alias transform: transform
    property color baseColor: "red"
    property alias rootJoint: skeleton.rootJoint
    property alias skeleton: skeleton

    components: [
        Transform {
            id: transform
            rotationX: -90
        },
        Mesh {
            source: root.source
        },
        Armature {
            skeleton: SkeletonLoader {
                id: skeleton
                source: root.source
                onStatusChanged: console.log("skeleton loader status: " + status)
                onJointCountChanged: console.log("skeleton has " + jointCount + " joints")
            }
        },
        Material {
            effect: root.effect

            parameters: [
                Parameter { name: "baseColor"; value: root.baseColor }
            ]
        }
    ]
}
