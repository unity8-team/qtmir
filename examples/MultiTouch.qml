// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

import QtQuick 2.0

Item {
    id: surface

    // Hard-coded Samsung Galaxy Nexus screen size.
    width: 720
    height: 1280

    MultiPointTouchArea {
        anchors.fill: parent
        touchPoints: [
            TouchPoint { id: point1 }, TouchPoint { id: point2 }, TouchPoint { id: point3 },
            TouchPoint { id: point4 }, TouchPoint { id: point5 }
        ]
    }

    Text {
        font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 100; color: "#9F9F00"
        x: point5.x + 50; y: point5.y - 200
        visible: point5.pressed
        text: "5"
    }
    Text {
        font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 100; color: "#AF00AF"
        x: point4.x + 50; y: point4.y - 200
        visible: point4.pressed
        text: "4"
    }
    Text {
        font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 100; color: "#0000EF"
        x: point3.x + 50; y: point3.y - 200
        visible: point3.pressed
        text: "3"
    }
    Text {
        font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 100; color: "#00FF00"
        x: point2.x + 50; y: point2.y - 200
        visible: point2.pressed
        text: "2"
    }
    Text {
        font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 100; color: "#FF0000"
        x: point1.x + 50; y: point1.y - 200
        visible: point1.pressed
        text: "1"
    }
}
