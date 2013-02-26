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

    property real displace
    SequentialAnimation on displace {
        loops: Animation.Infinite
        NumberAnimation { from: 0.0; to: 1.0; duration: 30000 }
    }

    Item {
        id: scene
        anchors.fill: parent
        Image {
            id: logo
            x: (scene.width - sourceSize.width) / 2
            y: (scene.height - sourceSize.height) / 2
            width: sourceSize.width
            height: sourceSize.height
            source: "logo.png"
            sourceSize: Qt.size(400, 400)
        }
    }

    ShaderEffect {
        property variant tex: ShaderEffectSource {
            sourceItem: scene; hideSource: true; live: false; smooth: true
        }
        property variant noise: ShaderEffectSource {
            sourceItem: Image { source: "noise.png"; smooth: true }
            wrapMode: ShaderEffectSource.Repeat
        }
        property variant size: Qt.size(scene.width, scene.height)
        property variant displace: surface.displace

        anchors.fill: parent
        blending: false

        vertexShader: "
            uniform mediump float displace;
            uniform mediump vec2 size;
            uniform mediump mat4 qt_Matrix;
            attribute mediump vec4 qt_Vertex;
            attribute mediump vec2 qt_MultiTexCoord0;
            varying mediump vec2 texCoord;
            varying mediump vec2 noiseCoord;
            void main() {
                texCoord = qt_MultiTexCoord0.xy;
                noiseCoord = vec2(qt_MultiTexCoord0.x * 0.25,
                                  qt_MultiTexCoord0.y * 0.25 + displace);
                gl_Position = qt_Matrix * qt_Vertex;
            }"

        fragmentShader: "
            uniform sampler2D noise;
            uniform sampler2D tex;
            varying mediump vec2 texCoord;
            varying mediump vec2 noiseCoord;
            void main() {
                mediump vec2 noiseVec = texture2D(noise, noiseCoord).xy;
                gl_FragColor = texture2D(tex, texCoord + ((noiseVec - 0.5) * 0.05));
            }"
    }
}
