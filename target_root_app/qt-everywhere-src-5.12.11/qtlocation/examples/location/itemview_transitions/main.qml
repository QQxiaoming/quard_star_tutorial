/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

import QtQuick 2.0
import QtQuick.Window 2.0
import QtLocation 5.12
import QtPositioning 5.12
import QtQuick.Shapes 1.1

Window {
    width: 512
    height: 512
    visible: true

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        id: map
        visible: true
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(59.91, 10.75) // Oslo
        zoomLevel: 9.5

        onMapReadyChanged: miv.model = osloListModel

        Text {
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Long press to drop a marker"
            color: "grey"
            font.pixelSize: 24
            opacity: 0.8
            z: parent.z + 10
        }

        MouseArea {
            anchors.fill: parent
            onPressAndHold: {
                var crd = map.toCoordinate(Qt.point(mouseX, mouseY))
                console.log(crd)
                markerModel.clear()
                markerModel.append({ "latitude": crd.latitude, "longitude": crd.longitude})
            }
        }

        //! [MarkerView]
        MapItemView {
            id: mivMarker

            add: Transition {
                NumberAnimation {
                    property: "slideIn"
                    from: 50
                    to: 0
                    duration: 500
                    easing.type: Easing.OutBounce
                    easing.amplitude: 3.0
                }
            }

            remove: Transition {
                NumberAnimation {
                    property: "opacity"
                    to: 0.1
                    duration: 50
                }
            }

            model: ListModel {
                id: markerModel
            }
            delegate: Component {
                MapQuickItem {
                    coordinate: QtPositioning.coordinate(latitude, longitude)
                    anchorPoint: Qt.point(e1.width * 0.5, e1.height + slideIn)
                    property real slideIn : 0
                    sourceItem: Shape {
                        id: e1
                        vendorExtensionsEnabled: false
                        width: 32
                        height: 32
                        visible: true

                        transform: Scale {
                            origin.y: e1.height * 0.5
                            yScale: -1
                        }

                        ShapePath {
                            id: c_sp1
                            strokeWidth: -1
                            fillColor: Qt.rgba(1,0,1,1.0)

                            property real half: e1.width * 0.5
                            property real quarter: e1.width * 0.25
                            property point center: Qt.point(e1.x + e1.width * 0.5 , e1.y + e1.height * 0.5)


                            property point top: Qt.point(center.x, center.y - half )
                            property point bottomLeft: Qt.point(center.x - half, center.y + half )
                            property point bottomRight: Qt.point(center.x + half, center.y + half )

                            startX: center.x;
                            startY: center.y + half

                            PathLine { x: c_sp1.bottomLeft.x; y: c_sp1.bottomLeft.y }
                            PathLine { x: c_sp1.top.x; y: c_sp1.top.y }
                            PathLine { x: c_sp1.bottomRight.x; y: c_sp1.bottomRight.y }
                            PathLine { x: c_sp1.center.x; y: c_sp1.center.y + c_sp1.half }
                        }
                    }
                }
            }
        }
        //! [MarkerView]

        //! [OsloAdministrativeRegions]
        MapItemView {
            id: miv
            model: OsloListModel {
                id: osloListModel
            }
            add: Transition {
                NumberAnimation {
                    property: "animationScale"
                    from: 0.2
                    to: 1
                    duration: 800
                    easing.type: Easing.OutCubic
                }
            }
            delegate: Component {
                MapPolygon {
                    function fromMercator(l, centroid) {
                        var res = []
                        for (var i  = 0; i < l.length; i++) {
                            var vtx = l[i]
                            var offset = Qt.point((vtx.x - centroid.x) * animationScale,
                                                  (vtx.y - centroid.y) * animationScale)
                            var pt = Qt.point(centroid.x + offset.x, centroid.y + offset.y)
                            res.push( QtPositioning.mercatorToCoord(pt) )
                        }
                        return res;
                    }

                    path: fromMercator(osloListModel.geometries[name+"_"+adminLevel]
                                       , osloListModel.centroids[name+"_"+adminLevel] )
                    color: ((adminLevel > 4) ? "lightsteelblue" : 'firebrick')
                    property real animationScale : 1
                    opacity: ((adminLevel < 9) ? 0.1 : 0.8)
                    visible: true
                }
            }
        }
        //! [OsloAdministrativeRegions]
    }
}
