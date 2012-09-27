// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

import QtQuick 2.0

Item {
    id: surface

    // Hard-coded Samsung Galaxy Nexus screen size.
    width: 720
    height: 1280

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
                    duration: Math.floor(lerp(Math.random(), 2250, 2750))
                    easing.type: Easing.InOutSine;
                }
                NumberAnimation {
                    to: 0.0; from: surface.height - logo.height
                    duration: Math.floor(lerp(Math.random(), 2250, 2750))
                    easing.type: Easing.InOutSine;
                }
            }

            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0.0; to: surface.width - logo.width
                    duration: Math.floor(lerp(Math.random(), 1750, 2750))
                    easing.type: Easing.InOutSine;
                }
                NumberAnimation {
                    to: 0.0; from: surface.width - logo.width
                    duration: Math.floor(lerp(Math.random(), 1250, 1750))
                    easing.type: Easing.InOutSine;
                }
            }
        }
    }
}
