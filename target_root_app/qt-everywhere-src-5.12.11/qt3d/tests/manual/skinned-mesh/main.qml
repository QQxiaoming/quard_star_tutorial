/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import Qt3D.Core 2.10
import Qt3D.Render 2.10
import Qt3D.Input 2.0
import Qt3D.Animation 2.10
import Qt3D.Extras 2.10
import QtQuick 2.9

DefaultSceneEntity {
    id: scene

    SkinnedPbrEffect {
        id: skinnedPbrEffect
    }

    Timer {
        interval: 2000
        running: true
        repeat: false
        onTriggered: {
            animator1.running = true
            animator2.running = true
            animator3.running = true
            animator4.running = true
            animator5.running = true
        }
    }

    SkinnedEntity {
        id: riggedFigure1
        effect: skinnedPbrEffect
        source: "qrc:/assets/gltf/2.0/RiggedFigure/RiggedFigure.gltf"
        baseColor: "red"

        components: [
//            ClipAnimator {
//                loops: 3
//                clip: AnimationClipLoader { source: "qrc:/jump.json" }
//                channelMapper: ChannelMapper {
//                    mappings: [
//                        SkeletonChannelMapping { target: riggedFigure.skeleton }
//                    ]
//                }
//            }
            BlendedClipAnimator {
                id: animator1
                loops: 5
                blendTree: ClipBlendValue {
                    clip: AnimationClipLoader { source: "qrc:/jump.json" }
                }
                channelMapper: ChannelMapper {
                    mappings: [
                        SkeletonMapping { skeleton: riggedFigure1.skeleton }
                    ]
                }

                onRunningChanged: console.log("running = " + running)
            }
        ]
    }

    SkinnedEntity {
        id: riggedFigure2
        effect: skinnedPbrEffect
        source: "qrc:/assets/gltf/2.0/RiggedFigure/RiggedFigure.gltf"
        baseColor: "purple"
        transform.translation: Qt.vector3d(0.0, 0.0, -1.0);

        components: [
            BlendedClipAnimator {
                id: animator2
                loops: 5
                blendTree: ClipBlendValue {
                    clip: AnimationClipLoader { source: "qrc:/jump.json" }
                }
                channelMapper: ChannelMapper {
                    mappings: [
                        SkeletonMapping { skeleton: riggedFigure2.skeleton }
                    ]
                }

                onRunningChanged: console.log("running = " + running)
            }
        ]
    }

    SkinnedEntity {
        id: riggedFigure3
        effect: skinnedPbrEffect
        source: "qrc:/assets/gltf/2.0/RiggedFigure/RiggedFigure.gltf"
        baseColor: "blue"
        transform.translation: Qt.vector3d(0.0, 0.0, -2.0);

        components: [
            BlendedClipAnimator {
                id: animator3
                loops: 5
                blendTree: ClipBlendValue {
                    clip: AnimationClipLoader { source: "qrc:/jump.json" }
                }
                channelMapper: ChannelMapper {
                    mappings: [
                        SkeletonMapping { skeleton: riggedFigure3.skeleton }
                    ]
                }

                onRunningChanged: console.log("running = " + running)
            }
        ]
    }

    SkinnedEntity {
        id: riggedFigure4
        effect: skinnedPbrEffect
        source: "qrc:/assets/gltf/2.0/RiggedFigure/RiggedFigure.gltf"
        baseColor: "green"
        transform.translation: Qt.vector3d(0.0, 0.0, -3.0);

        components: [
            BlendedClipAnimator {
                id: animator4
                loops: 5
                blendTree: ClipBlendValue {
                    clip: AnimationClipLoader { source: "qrc:/jump.json" }
                }
                channelMapper: ChannelMapper {
                    mappings: [
                        SkeletonMapping { skeleton: riggedFigure4.skeleton }
                    ]
                }

                onRunningChanged: console.log("running = " + running)
            }
        ]
    }

    SkinnedEntity {
        id: riggedFigure5
        effect: skinnedPbrEffect
        source: "qrc:/assets/gltf/2.0/RiggedFigure/RiggedFigure.gltf"
        baseColor: "orange"
        transform.translation: Qt.vector3d(0.0, 0.0, -4.0);

        components: [
            BlendedClipAnimator {
                id: animator5
                loops: 5
                blendTree: ClipBlendValue {
                    clip: AnimationClipLoader { source: "qrc:/jump.json" }
                }
                channelMapper: ChannelMapper {
                    mappings: [
                        SkeletonMapping { skeleton: riggedFigure5.skeleton }
                    ]
                }

                onRunningChanged: console.log("running = " + running)
            }
        ]
    }

    SkinnedEntity {
        id: riggedSimple
        effect: skinnedPbrEffect
        source: "qrc:/assets/gltf/2.0/RiggedSimple/RiggedSimple.gltf"
        baseColor: "blue"
        transform.scale: 0.05
        transform.translation: Qt.vector3d(0.5, 0.25, 0.0)
        createJointsEnabled: true

        onRootJointChanged: {
            var animation = animationComp.createObject(rootJoint)
            var targetJoint = rootJoint.childJoints[0]
            animation.target = targetJoint
            animation.running = true
        }

        Component {
            id: animationComp
            SequentialAnimation {
                id: sequentialAnimation
                property variant target: null
                property real dz: 30.0
                loops: Animation.Infinite

                NumberAnimation {
                    target: sequentialAnimation.target
                    property: "rotationZ"
                    from: -dz
                    to: dz
                    duration: 600
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    target: sequentialAnimation.target
                    property: "rotationZ"
                    from: dz
                    to: -dz
                    duration: 600
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}
