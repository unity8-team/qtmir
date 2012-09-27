// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

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
