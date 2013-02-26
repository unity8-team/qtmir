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

    property real seed

    function lerp(x, a, b) {
        return ((1.0 - x) * a) + (x * b);
    }

    Item {
        id: scene
        anchors.fill: parent

        Image {
            id: logo
            source: "logo.png"

            SequentialAnimation on y {
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0.0; to: surface.height - logo.height
                    duration: Math.floor(lerp(Math.random(surface.seed), 2250.0, 2750.0))
                    easing.type: Easing.InOutSine;
                }
                NumberAnimation {
                    to: 0.0; from: surface.height - logo.height
                    duration: Math.floor(lerp(Math.random(surface.seed), 2250.0, 2750.0))
                    easing.type: Easing.InOutSine;
                }
            }

            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0.0; to: surface.width - logo.width
                    duration: Math.floor(lerp(Math.random(surface.seed), 1750.0, 2250.0))
                    easing.type: Easing.InOutSine;
                }
                NumberAnimation {
                    to: 0.0; from: surface.width - logo.width
                    duration: Math.floor(lerp(Math.random(surface.seed), 1750.0, 2250.0))
                    easing.type: Easing.InOutSine;
                }
            }
        }
    }

    Component.onCompleted: {
        var d = new Date();
        surface.seed = d.getSeconds();
    }
}
