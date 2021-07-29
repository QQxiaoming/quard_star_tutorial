/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import QtPositioning 5.5
import QtLocation 5.10
import QtLocation.Test 5.6

Item {
    width:100
    height:100
    // General-purpose elements for the test:
    Plugin { id: testPlugin; name: "qmlgeo.test.plugin"; allowExperimental: true }
    Plugin { id: testPlugin2; name: "gmlgeo.test.plugin"; allowExperimental: true }
    Plugin {
        id: testPluginLazyParameter;
        name: "qmlgeo.test.plugin"
        allowExperimental: true
        property string extraTypeName : undefined
        PluginParameter { name: "supported"; value: true}
        PluginParameter { name: "finishRequestImmediately"; value: true}
        PluginParameter { name: "validateWellKnownValues"; value: true}
        PluginParameter { name: "extraMapTypeName"; value: testPluginLazyParameter.extraTypeName}

        Component.onCompleted: {
            extraTypeName = "SomeString"
        }
    }
    Plugin {
        id: testPluginNoVisibleArea;
        name: "qmlgeo.test.plugin";
        allowExperimental: true
        PluginParameter { name: "supportVisibleArea"; value: false}
    }
    Plugin { id: itemsOverlay; name: "itemsoverlay"; }

    property variant coordinate1: QtPositioning.coordinate(10, 11)
    property variant coordinate2: QtPositioning.coordinate(12, 13)
    property variant coordinate3: QtPositioning.coordinate(50, 50, 0)
    property variant coordinate4: QtPositioning.coordinate(80, 80, 0)
    property variant coordinate5: QtPositioning.coordinate(20, 180)
    property variant coordinateCenterVisibleRegion: QtPositioning.coordinate(27, 77)
    property variant coordinateVisible1: QtPositioning.coordinate(28, 77)
    property variant coordinateVisible2: QtPositioning.coordinate(33, 79.1)
    property variant coordinateVisible3: QtPositioning.coordinate(27, 80.5)
    property variant invalidCoordinate: QtPositioning.coordinate()
    property variant altitudelessCoordinate: QtPositioning.coordinate(50, 50)
    property bool allMapsReady: mapZoomOnCompleted.mapReady
                                && mapZoomDefault.mapReady
                                && mapZoomUserInit.mapReady
                                && map.mapReady
                                && mapPar.mapReady
                                && coordinateMap.mapReady
                                && mapTiltBearing.mapReady
                                && mapTiltBearingHere.mapReady
                                && mapTestProjection.mapReady

    Map { id: mapZoomOnCompleted; width: 200; height: 200;
        zoomLevel: 3; center: coordinate1; plugin: testPlugin;
        Component.onCompleted: {
            zoomLevel = 7
        }
    }
    SignalSpy {id: mapZoomSpy; target: mapZoomOnCompleted; signalName: 'zoomLevelChanged'}

    Map { id: mapZoomDefault; width: 200; height: 200;
        center: coordinate1; plugin: testPlugin; }

    Map { id: mapZoomUserInit; width: 210; height: 210;
        zoomLevel: 4; center: coordinate1; plugin: testPlugin;
        Component.onCompleted: {
            console.log("mapZoomUserInit completed")
        }
    }

    Map { id: mapVisibleRegion; width: 800; height: 600;
        center: coordinateCenterVisibleRegion; plugin: testPlugin; zoomLevel: 1.0 }

    Map {id: map; plugin: testPlugin; center: coordinate1; width: 100; height: 100}
    SignalSpy {id: mapCenterSpy; target: map; signalName: 'centerChanged'}

    Map {id: mapPar; plugin: testPlugin; center: coordinate1; width: 512; height: 512}

    Map {id: coordinateMap; plugin: testPlugin; center: coordinate3;
        width: 1000; height: 1000; zoomLevel: 15 }

    Map {id: mapTiltBearing; plugin: testPlugin; center: coordinate1;
        width: 1000; height: 1000; zoomLevel: 4; bearing: 45.0; tilt: 25.0 }

    Map {id: mapTiltBearingHere; plugin: testPlugin; center: coordinate1;
        width: 1000; height: 1000; zoomLevel: 4; bearing: 45.0; tilt: 25.0 }

    Map {
        id: mapWithLazyPlugin
        plugin: testPluginLazyParameter
    }

    Map {
        id: mapTestProjection
        plugin: testPlugin
        width: 200
        height: 200
    }

    MapParameter {
        id: testParameter
        type: "cameraCenter_test"
        property var center: QtPositioning.coordinate(-33.0, -47.0)
    }

    Map {
        id: mapVisibleArea
        width: 256; height: 256;
    }
    Map {
        id: mapVisibleAreaUnsupported
        width: 256; height: 256;
    }
    SignalSpy { id: visibleAreaSpy; target: mapVisibleArea; signalName: 'visibleAreaChanged'}
    SignalSpy { id: visibleAreaUnsupportedSpy; target: mapVisibleAreaUnsupported; signalName: 'visibleAreaChanged'}

    TestCase {
        when: windowShown && allMapsReady
        name: "MapProperties"

        function fuzzy_compare(val, ref) {
            var tolerance = 0.01;
            if ((val > ref - tolerance) && (val < ref + tolerance))
                return true;
            console.log('map fuzzy cmp returns false for value, ref: ' + val + ', ' + ref)
            return false;
        }

        function init() {
            mapCenterSpy.clear();
        }

        function test_lazy_parameter() {
            compare(mapWithLazyPlugin.supportedMapTypes.length, 5)
            compare(mapWithLazyPlugin.supportedMapTypes[4].name, "SomeString")
        }

        function test_map_center() {
            // coordinate is set at map element declaration
            compare(map.center.latitude, 10)
            compare(map.center.longitude, 11)

            // change center and its values
            mapCenterSpy.clear();
            compare(mapCenterSpy.count, 0)
            map.center = coordinate2
            compare(mapCenterSpy.count, 1)
            map.center = coordinate2
            compare(mapCenterSpy.count, 1)

            // change center to dateline
            mapCenterSpy.clear()
            compare(mapCenterSpy.count, 0)
            map.center = coordinate5
            compare(mapCenterSpy.count, 1)
            compare(map.center, coordinate5)

            map.center = coordinate2

            verify(isNaN(map.center.altitude));
            compare(map.center.longitude, 13)
            compare(map.center.latitude, 12)
        }

        function test_map_visible_region()
        {
            mapVisibleRegion.zoomLevel = 1.0
            wait(50)
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible1))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible2))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible3))

            mapVisibleRegion.zoomLevel = 1.88
            verify(LocationTestHelper.waitForPolished(mapVisibleRegion))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible1))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible2))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible3))

            mapVisibleRegion.zoomLevel = 2.12
            verify(LocationTestHelper.waitForPolished(mapVisibleRegion))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible1))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible2))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible3))

            mapVisibleRegion.zoomLevel = 2.5
            verify(LocationTestHelper.waitForPolished(mapVisibleRegion))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible1))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible2))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible3))

            mapVisibleRegion.zoomLevel = 2.7
            verify(LocationTestHelper.waitForPolished(mapVisibleRegion))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible1))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible2))
            verify(mapVisibleRegion.visibleRegion.contains(coordinateVisible3))
        }

        function test_map_parameters()
        {
            // coordinate is set at map element declaration
            var center = mapPar.toCoordinate(Qt.point((mapPar.width - 1) / 2.0, (mapPar.height - 1) / 2.0))
            fuzzyCompare(center.latitude, 10, 0.1)
            fuzzyCompare(center.longitude, 11, 0.1)

            compare(mapPar.mapParameters.length, 0)

            mapPar.addMapParameter(testParameter)

            compare(mapPar.mapParameters.length, 1)

            // Using toCoordinate, below, to verify the actual value of the center, and not what is in the map.center property
            center = mapPar.toCoordinate(Qt.point((mapPar.width - 1) / 2.0, (mapPar.height - 1) / 2.0))
            fuzzyCompare(center.latitude, -33, 0.1)
            fuzzyCompare(center.longitude, -47, 0.1)

            mapPar.addMapParameter(testParameter)
            compare(mapPar.mapParameters.length, 1)

            mapPar.removeMapParameter(testParameter)
            compare(mapPar.mapParameters.length, 0)

            center = mapPar.toCoordinate(Qt.point((mapPar.width - 1) / 2.0, (mapPar.height - 1) / 2.0))
            fuzzyCompare(center.latitude, -33, 0.1)
            fuzzyCompare(center.longitude, -47, 0.1)

            testParameter.center = mapPar.center  // map.center has been affected as the Declarative Map has received the QGeoMap::cameraDataChanged signal
            mapPar.addMapParameter(testParameter)
            compare(mapPar.mapParameters.length, 1)

            center = mapPar.toCoordinate(Qt.point((mapPar.width - 1) / 2.0, (mapPar.height - 1) / 2.0))
            fuzzyCompare(center.latitude, -33, 0.1)
            fuzzyCompare(center.longitude, -47, 0.1)

            testParameter.center = QtPositioning.coordinate(-30.0, -40.0)

            center = mapPar.toCoordinate(Qt.point((mapPar.width - 1) / 2.0, (mapPar.height - 1) / 2.0))
            fuzzyCompare(center.latitude, -30, 0.1)
            fuzzyCompare(center.longitude, -40, 0.1)
            fuzzyCompare(mapPar.center.latitude, -30, 0.1)
            fuzzyCompare(mapPar.center.longitude, -40, 0.1)

            mapPar.removeMapParameter(testParameter)
            compare(mapPar.mapParameters.length, 0)
        }

        function test_map_clamp()
        {
            //valid
            map.center = QtPositioning.coordinate(10.0, 20.5, 30.8)
            map.zoomLevel = 2.0

            compare(map.center.latitude, 10)
            compare(map.center.longitude, 20.5)
            compare(map.center.altitude, 30.8)

            //negative values
            map.center = QtPositioning.coordinate(-50, -20, 100)
            map.zoomLevel = 1.0

            compare(map.center.latitude, -50)
            compare(map.center.longitude, -20)
            compare(map.center.altitude, 100)

            //clamped center negative
            map.center = QtPositioning.coordinate(-89, -45, 0)
            map.zoomLevel = 1.0

            fuzzyCompare(map.center.latitude, -80.8728, 0.001)
            compare(map.center.longitude, -45)
            compare(map.center.altitude, 0)

            //clamped center positive
            map.center = QtPositioning.coordinate(86, 38, 0)
            map.zoomLevel = 1.0

            fuzzyCompare(map.center.latitude, 80.8728, 0.001)
            compare(map.center.longitude, 38)
            compare(map.center.altitude, 0)
        }

        function test_zoom_limits()
        {
            map.center.latitude = 30
            map.center.longitude = 60
            map.zoomLevel = 4

            //initial plugin values
            compare(map.minimumZoomLevel, 0)
            compare(map.maximumZoomLevel, 20)
            compare(map.activeMapType.cameraCapabilities.minimumZoomLevel, 0)
            compare(map.activeMapType.cameraCapabilities.maximumZoomLevel, 20)

            //Higher min level than curr zoom, should change curr zoom
            map.minimumZoomLevel = 5
            map.maximumZoomLevel = 18
            compare(map.zoomLevel, 5)
            compare(map.minimumZoomLevel, 5)
            compare(map.maximumZoomLevel, 18)

            //Trying to set higher than max, max should be set.
            map.maximumZoomLevel = 21
            compare(map.minimumZoomLevel, 5)
            compare(map.maximumZoomLevel, 20)

            //Negative values should be ignored
            map.minimumZoomLevel = -1
            map.maximumZoomLevel = -2
            compare(map.minimumZoomLevel, 5)
            compare(map.maximumZoomLevel, 20)

            //Max limit lower than curr zoom, should change curr zoom
            map.zoomLevel = 18
            map.maximumZoomLevel = 16
            compare(map.zoomLevel, 16)
            compare(map.activeMapType.cameraCapabilities.minimumZoomLevel, 0)
            compare(map.activeMapType.cameraCapabilities.maximumZoomLevel, 20)

            //reseting default
            map.minimumZoomLevel = 0
            map.maximumZoomLevel = 20
            compare(map.minimumZoomLevel, 0)
            compare(map.maximumZoomLevel, 20)
        }

        function test_tilt_limits()
        {
            map.tilt = 0

            //initial plugin values
            compare(map.minimumTilt, 0)
            compare(map.maximumTilt, 60)
            compare(map.activeMapType.cameraCapabilities.minimumTilt, 0)
            compare(map.activeMapType.cameraCapabilities.maximumTilt, 60)

            //Higher min level than curr tilt, should change curr tilt
            map.minimumTilt = 5
            map.maximumTilt = 18
            compare(map.tilt, 5)
            compare(map.minimumTilt, 5)
            compare(map.maximumTilt, 18)
            // Capabilities remain the same
            compare(map.activeMapType.cameraCapabilities.minimumTilt, 0)
            compare(map.activeMapType.cameraCapabilities.maximumTilt, 60)

            //Trying to set higher than max, max should be set.
            map.maximumTilt = 61
            compare(map.minimumTilt, 5)
            compare(map.maximumTilt, 60)

            //Negative values should be ignored
            map.minimumTilt = -1
            map.maximumTilt = -2
            compare(map.minimumTilt, 5)
            compare(map.maximumTilt, 60)

            //Max limit lower than curr zoom, should change curr zoom
            map.tilt = 18
            map.maximumTilt = 16
            compare(map.tilt, 16)

            //resetting default
            map.minimumTilt = 0
            map.maximumTilt = 60
            map.tilt = 0
            compare(map.minimumTilt, 0)
            compare(map.maximumTilt, 60)
            compare(map.tilt, 0)
        }

        function test_fov_limits()
        {
            map.fieldOfView = 45

            //initial plugin values
            compare(map.minimumFieldOfView, 45)
            compare(map.maximumFieldOfView, 45)
            compare(map.activeMapType.cameraCapabilities.minimumFieldOfView, 45)
            compare(map.activeMapType.cameraCapabilities.maximumFieldOfView, 45)

            map.minimumFieldOfView = 5
            map.maximumFieldOfView = 18
            map.fieldOfView = 4
            compare(map.fieldOfView, 45)
            compare(map.minimumFieldOfView, 45)
            compare(map.maximumFieldOfView, 45)

            map.activeMapType = map.supportedMapTypes[3]
            // camera caps are [1-179], user previously asked for [5-18]
            compare(map.minimumFieldOfView, 5)
            compare(map.maximumFieldOfView, 18)
            compare(map.activeMapType.cameraCapabilities.minimumFieldOfView, 1)
            compare(map.activeMapType.cameraCapabilities.maximumFieldOfView, 179)

            map.fieldOfView = 4
            compare(map.fieldOfView, 5)

            //Higher min level than curr fieldOfView, should change curr fieldOfView
            map.minimumFieldOfView = 6
            compare(map.fieldOfView, 6)
            compare(map.minimumFieldOfView, 6)
            compare(map.maximumFieldOfView, 18)

            //Trying to set higher than max, max should be set.
            map.maximumFieldOfView = 179.5
            compare(map.minimumFieldOfView, 6)
            compare(map.maximumFieldOfView, 179)

            //Negative values should be ignored
            map.minimumFieldOfView = -1
            map.maximumFieldOfView = -2
            compare(map.minimumFieldOfView, 6)
            compare(map.maximumFieldOfView, 179)

            //Max limit lower than curr zoom, should change curr zoom
            map.fieldOfView = 18
            compare(map.fieldOfView, 18)
            map.maximumFieldOfView = 16
            compare(map.maximumFieldOfView, 16)
            compare(map.fieldOfView, 16)

            //resetting default
            map.minimumFieldOfView = 1
            map.maximumFieldOfView = 179
            compare(map.minimumFieldOfView, 1)
            compare(map.maximumFieldOfView, 179)

            map.activeMapType = map.supportedMapTypes[0]
            compare(map.minimumFieldOfView, 45)
            compare(map.maximumFieldOfView, 45)
            compare(map.fieldOfView, 45)
            compare(map.activeMapType.cameraCapabilities.minimumFieldOfView, 45)
            compare(map.activeMapType.cameraCapabilities.maximumFieldOfView, 45)
        }

        function test_zoom()
        {
            wait(1000)
            compare(mapZoomOnCompleted.zoomLevel, 7)
            compare(mapZoomDefault.zoomLevel, 8)
            compare(mapZoomUserInit.zoomLevel, 4)

            mapZoomSpy.clear()
            mapZoomOnCompleted.zoomLevel = 6
            tryCompare(mapZoomSpy, "count", 1)
        }

        function test_pan()
        {
            map.center.latitude = 30
            map.center.longitude = 60
            map.zoomLevel = 4
            mapCenterSpy.clear();

            // up left
            tryCompare(mapCenterSpy, "count", 0)
            map.pan(-20,-20)
            tryCompare(mapCenterSpy, "count", 1)
            verify(map.center.latitude > 30)
            verify(map.center.longitude < 60)
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // up
            map.pan(0,-20)
            tryCompare(mapCenterSpy, "count", 1)
            verify(map.center.latitude > 30)
            compare(map.center.longitude, 60)
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // up right
            tryCompare(mapCenterSpy, "count", 0)
            map.pan(20,-20)
            tryCompare(mapCenterSpy, "count", 1)
            verify(map.center.latitude > 30)
            verify(map.center.longitude > 60)
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // left
            map.pan(-20,0)
            tryCompare(mapCenterSpy, "count", 1)
            verify (fuzzy_compare(map.center.latitude, 30))
            verify(map.center.longitude < 60)
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // center
            map.pan(0,0)
            tryCompare(mapCenterSpy, "count", 0)
            compare(map.center.latitude, 30)
            compare(map.center.longitude, 60)
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // right
            map.pan(20,0)
            tryCompare(mapCenterSpy, "count", 1)
            verify (fuzzy_compare(map.center.latitude, 30))
            verify(map.center.longitude > 60)
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // down left
            map.pan(-20,20)
            tryCompare(mapCenterSpy, "count", 1)
            verify (map.center.latitude < 30 )
            verify (map.center.longitude < 60 )
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // down
            map.pan(0,20)
            tryCompare(mapCenterSpy, "count", 1)
            verify (map.center.latitude < 30 )
            verify (fuzzy_compare(map.center.longitude, 60))
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
            // down right
            map.pan(20,20)
            tryCompare(mapCenterSpy, "count", 1)
            verify (map.center.latitude < 30 )
            verify (map.center.longitude > 60 )
            map.center.latitude = 30
            map.center.longitude = 60
            mapCenterSpy.clear()
        }

        function test_map_tilt_bearing()
        {
            compare(map.bearing, 0.0)
            compare(map.tilt, 0.0)
            compare(mapTiltBearing.bearing, 45.0)
            compare(mapTiltBearing.tilt, 25.0)
            compare(mapTiltBearingHere.bearing, 45.0)
            compare(mapTiltBearingHere.tilt, 25.0)

            mapTiltBearing.bearing = 0.0
            mapTiltBearing.tilt = 0.0
            compare(mapTiltBearing.bearing, 0.0)
            compare(mapTiltBearing.tilt, 0.0)

            mapTiltBearing.bearing = 480.0
            mapTiltBearing.tilt = 140.0
            compare(mapTiltBearing.bearing, 120.0)
            compare(mapTiltBearing.tilt, 60.0)

            mapTiltBearing.tilt = -140.0
            compare(mapTiltBearing.tilt, 0.0)

            mapTiltBearingHere.bearing = 45.0
            mapTiltBearingHere.tilt = 25.0
            compare(mapTiltBearingHere.bearing, 45.0)
            compare(mapTiltBearingHere.tilt, 25.0)
            mapTiltBearingHere.bearing = 0.0
            mapTiltBearingHere.tilt = 0.0
            compare(mapTiltBearingHere.bearing, 0.0)
            compare(mapTiltBearingHere.tilt, 0.0)

            mapTiltBearing.bearing = 45.0
            mapTiltBearing.tilt = 25.0
            mapTiltBearing.zoomLevel = 8.0
            compare(mapTiltBearing.bearing, 45.0)
            compare(mapTiltBearing.tilt, 25.0)
        }

        function test_map_setbearing()
        {
            var zeroCoord = QtPositioning.coordinate(0,0)
            mapTiltBearing.bearing = 0.0
            mapTiltBearing.tilt = 0.0
            mapTiltBearing.zoomLevel = 3
            mapTiltBearing.center = zeroCoord
            compare(mapTiltBearing.bearing, 0.0)
            compare(mapTiltBearing.tilt, 0.0)
            compare(mapTiltBearing.zoomLevel, 3)
            compare(mapTiltBearing.center, zeroCoord)

            var fulcrum = QtPositioning.coordinate(20,-20)
            var fulcrumPos = mapTiltBearing.fromCoordinate(fulcrum)
            var bearing = 90.0
            mapTiltBearing.setBearing(bearing, fulcrum)
            var fulcrumPosAfter = mapTiltBearing.fromCoordinate(fulcrum)
            compare(mapTiltBearing.bearing, bearing)
            compare(fulcrumPos, fulcrumPosAfter)

            // resetting
            mapTiltBearing.center = coordinate1
            mapTiltBearing.zoomLevel = 4
            mapTiltBearing.bearing = 45.0
            mapTiltBearing.tilt = 25.0
        }

        function test_map_align_coordinate_to_point()
        {
            var zeroCoord = QtPositioning.coordinate(0,0)
            mapTiltBearing.bearing = 0.0
            mapTiltBearing.tilt = 0.0
            mapTiltBearing.zoomLevel = 3
            mapTiltBearing.center = zeroCoord
            compare(mapTiltBearing.bearing, 0.0)
            compare(mapTiltBearing.tilt, 0.0)
            compare(mapTiltBearing.zoomLevel, 3)
            compare(mapTiltBearing.center, zeroCoord)

            var coord = QtPositioning.coordinate(20,-20)
            var point = Qt.point(400, 400)
            mapTiltBearing.alignCoordinateToPoint(coord, point)
            var coordAfter = mapTiltBearing.toCoordinate(point)
            compare(coord.latitude, coordAfter.latitude)
            compare(coord.longitude, coordAfter.longitude)

            // resetting
            mapTiltBearing.center = coordinate1
            mapTiltBearing.zoomLevel = 4
            mapTiltBearing.bearing = 45.0
            mapTiltBearing.tilt = 25.0
        }

        function test_coordinate_conversion()
        {
            wait(1000)
            mapCenterSpy.clear();
            compare(coordinateMap.center.latitude, 50)
            compare(coordinateMap.center.longitude, 50)
            // valid to screen position
            var point = coordinateMap.fromCoordinate(coordinateMap.center)
            verify (point.x > 495 && point.x < 505)
            verify (point.y > 495 && point.y < 505)
            // valid coordinate without altitude
            point = coordinateMap.fromCoordinate(altitudelessCoordinate)
            verify (point.x > 495 && point.x < 505)
            verify (point.y > 495 && point.y < 505)
            // out of map area in view
            //var oldZoomLevel = coordinateMap.zoomLevel
            //coordinateMap.zoomLevel = 8
            point = coordinateMap.fromCoordinate(coordinate4)
            verify(isNaN(point.x))
            verify(isNaN(point.y))
            //coordinateMap.zoomLevel = oldZoomLevel
            // invalid coordinates
            point = coordinateMap.fromCoordinate(invalidCoordinate)
            verify(isNaN(point.x))
            verify(isNaN(point.y))
            point = coordinateMap.fromCoordinate(null)
            verify(isNaN(point.x))
            verify(isNaN(point.y))
            // valid point to coordinate
            var coord = coordinateMap.toCoordinate(Qt.point(500,500))
            verify(coord.latitude > 49 && coord.latitude < 51)
            verify(coord.longitude > 49 && coord.longitude < 51)
            // beyond
            coord = coordinateMap.toCoordinate(Qt.point(2000, 2000))
            verify(isNaN(coord.latitude))
            verify(isNaN(coord.longitde))
            // invalid
            coord = coordinateMap.toCoordinate(Qt.point(-5, -6))
            verify(isNaN(coord.latitude))
            verify(isNaN(coord.longitde))

            // test with tilting
            coord = QtPositioning.coordinate(45.6, 17.67)
            var pos = mapTestProjection.fromCoordinate(coord, false)
            compare(Math.floor(pos.x), 3339)
            compare(Math.floor(pos.y), 1727)
            mapTestProjection.tilt = 6
            pos = mapTestProjection.fromCoordinate(coord, false)
            compare(Math.floor(pos.x), 11066)
            compare(Math.floor(pos.y), 5577)
            mapTestProjection.tilt = 12
            pos = mapTestProjection.fromCoordinate(coord, false)
            verify(isNaN(pos.latitude))
            verify(isNaN(pos.longitde))
        }

        function test_visible_area()
        {
            wait(1000)
            visibleAreaSpy.clear();
            visibleAreaUnsupportedSpy.clear();
            var defaultRect = Qt.rect(0,0,0,0)

            verify(mapVisibleAreaUnsupported.visibleArea, defaultRect)
            mapVisibleAreaUnsupported.visibleArea = Qt.rect(0,0,256,256)
            compare(visibleAreaUnsupportedSpy.count, 1)
            verify(mapVisibleAreaUnsupported.visibleArea, Qt.rect(0,0,256,256))
            mapVisibleAreaUnsupported.plugin = testPluginNoVisibleArea
            tryCompare(visibleAreaUnsupportedSpy, "count", 2)
            verify(mapVisibleAreaUnsupported.visibleArea, defaultRect)

            verify(mapVisibleArea.visibleArea, defaultRect)
            mapVisibleArea.visibleArea = Qt.rect(0,0,256,256)
            compare(visibleAreaSpy.count, 1)
            verify(mapVisibleArea.visibleArea, Qt.rect(0,0,256,256))
            mapVisibleArea.plugin = testPlugin
            tryCompare(visibleAreaSpy, "count", 1)
            verify(mapVisibleAreaUnsupported.visibleArea, Qt.rect(0,0,256,256))
        }
    }
}
