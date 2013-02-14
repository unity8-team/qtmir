// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3, as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

import QtQuick 2.0

Item {
    id: surface

    // Hard-coded Samsung Galaxy Nexus screen size.
    width: 720
    height: 1280

    // Needed for the key events to be emitted.
    focus: true

    MultiPointTouchArea {
        anchors.fill: parent
        touchPoints: [
            TouchPoint { id: point1 }, TouchPoint { id: point2 }, TouchPoint { id: point3 },
            TouchPoint { id: point4 }, TouchPoint { id: point5 }
        ]
    }

    // Touch elements.
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

    // Key elements.
    Text {
        id: keyText
        font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 75; color: "#DFDFDF"
        anchors.centerIn: surface
        visible: false
        text: ""
    }
    Keys.onPressed: {
        if (event.key == Qt.Key_PowerOff)
            keyText.text = "Power Off";
        else if (event.key == Qt.Key_VolumeUp)
            keyText.text = "Volume Up";
        else if (event.key == Qt.Key_VolumeDown)
            keyText.text = "Volume Down";
        keyText.visible = true;
    }
    Keys.onReleased: {
        keyText.visible = false;
    }
}
