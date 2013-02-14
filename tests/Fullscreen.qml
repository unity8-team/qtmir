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

// Non graphical test watching fullscreen states of main and side stages.

import QtQuick 2.0
import Ubuntu.Application 0.1

Item {
    Timer {
        id: time
        interval: 1000
        repeat: true
        onTriggered: {
            if (ApplicationManager.mainStageFocusedApplication) {
                print(ApplicationManager.sideStageFocusedApplication.name,
                      "in main stage is fullscreen: ",
                      ApplicationManager.sideStageFocusedApplication.fullscreen)
            }
            if (ApplicationManager.sideStageFocusedApplication) {
                print(ApplicationManager.sideStageFocusedApplication.name,
                      "in side stage is fullscreen: ",
                      ApplicationManager.sideStageFocusedApplication.fullscreen)
            }
        }
        Component.onCompleted: time.start()
    }
    Component.onCompleted: {
        time.start()
    }
}
