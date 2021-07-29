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

import QtQuick 2.5
import QtPositioning 5.3
import QtLocation 5.6

Rectangle{

    function clamp(num, min, max)
    {
      return num < min ? min : num > max ? max : num;
    }

    function minimumScaleFactor()
    {
        var hscalefactor = (400.0 / Math.max(Math.min(map.width, 1000), 400)) * 0.5
        var vscalefactor = (400.0 / Math.max(Math.min(map.height, 1000), 400)) * 0.5
        return Math.min(hscalefactor,vscalefactor)
    }

    function avgScaleFactor()
    {
        var hscalefactor = (400.0 / Math.max(Math.min(map.width, 1000), 400)) * 0.5
        var vscalefactor = (400.0 / Math.max(Math.min(map.height, 1000), 400)) * 0.5
        return (hscalefactor+vscalefactor) * 0.5
    }

    id: miniMapRect
    width: Math.floor(map.width * avgScaleFactor()) + 2
    height: Math.floor(map.height * avgScaleFactor()) + 2
    anchors.right: (parent) ? parent.right : undefined
    anchors.rightMargin: 10
    anchors.top: (parent) ? parent.top : undefined
    anchors.topMargin: 10
    color: "#242424"
    Map {
        id: miniMap
        anchors.top: parent.top
        anchors.topMargin: 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        width: Math.floor(map.width * avgScaleFactor())
        height: Math.floor(map.height * avgScaleFactor())
        zoomLevel: clamp(map.zoomLevel - 4.5, 2.0, 5.0) //(map.zoomLevel > minimumZoomLevel + 3) ? minimumZoomLevel + 3 : 1.5
        center: map.center
        plugin: map.plugin
        gesture.enabled: false
        copyrightsVisible: false
        property double mapZoomLevel : map.zoomLevel

        // cannot use property bindings on map.visibleRegion in MapRectangle because it's non-NOTIFYable
        onCenterChanged: miniMapRectangle.updateCoordinates()
        onMapZoomLevelChanged: miniMapRectangle.updateCoordinates()
        onWidthChanged: miniMapRectangle.updateCoordinates()
        onHeightChanged: miniMapRectangle.updateCoordinates()

        MapRectangle {
            id: miniMapRectangle
            color: "#44ff0000"
            border.width: 1
            border.color: "red"

            function getMapVisibleRegion()
            {
                return QtPositioning.shapeToRectangle(map.visibleRegion)
            }

            function updateCoordinates()
            {
                topLeft.latitude =  getMapVisibleRegion().topLeft.latitude
                topLeft.longitude=  getMapVisibleRegion().topLeft.longitude
                bottomRight.latitude =  getMapVisibleRegion().bottomRight.latitude
                bottomRight.longitude=  getMapVisibleRegion().bottomRight.longitude
                console.log("TopLeft: " + topLeft)
                console.log("BotRigh: " + bottomRight)
            }
        }
    }
}
