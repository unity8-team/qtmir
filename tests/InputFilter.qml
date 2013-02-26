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
import Ubuntu.Application 0.1

Item {
    width: 700; height: 700

    InputFilterArea {
        id: topLeftFilter
        width: 500
        height: 500
        blockInput: true

        Rectangle {
            anchors.fill: parent
            color: parent.blockInput ? "red" : "green"
            opacity: parent.blockInput ? 1.0 : 0.8
        }
    }

    MouseArea {
        anchors.centerIn: parent
        width: 500
        height: 200
        onClicked: {
            topLeftFilter.blockInput = !topLeftFilter.blockInput;
            bottomRightFilter.blockInput = !bottomRightFilter.blockInput;
        }

        Rectangle {
            anchors.fill: parent
            color: "grey"
        }

        Text {
            anchors.centerIn: parent
            text: "Press to toggle filters"
            font.pixelSize: 45
            color: "lightgrey"
        }
    }

    InputFilterArea {
        id: bottomRightFilter
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 500
        height: 500
        blockInput: true

        Rectangle {
            anchors.fill: parent
            color: parent.blockInput ? "red" : "green"
            opacity: parent.blockInput ? 1.0 : 0.8
        }
    }
}
