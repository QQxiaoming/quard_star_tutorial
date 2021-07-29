import QtQuick 2.0
import QtMultimedia 5.0

Item {
    width: 200
    height: 200
    VideoOutput {
        objectName: "videoOutput"
        x: 25; y: 50
        width: 150
        height: 100
    }
}
