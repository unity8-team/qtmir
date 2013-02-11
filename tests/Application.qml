// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// That example demonstrates the types added by the Ubuntu application plugin and how to use them.
//
// Here is how to create a window with specific surface role (Greeter here) from QML.
//
//    import QtQuick.Window 2.0
//
//    Window {
//        width: 400
//        height: 400
//        Component.onCompleted: {
//            window.role = ApplicationManager.Greeter
//            window.visible = true
//        }
//    }
//
// Here is how to start a new process:
//    ApplicationManager.startProcess("/usr/share/applications/snowshoe.desktop")
//
// In order to pass additional arguments, a second optional argument can be used:
//    ApplicationManager.startProcess("/usr/share/applications/snowshoe.desktop",
//                                    [ "http://www.ubuntu.com" ])
//
// An application can be stopped by using:
//     ApplicationManager.stopProcess(application)
//
// In order to focus or start a favorite application, that function can be used:
//     ApplicationManager.focusFavoriteApplication(ApplicationManager.GalleryApplication);

import QtQuick 2.0
import Ubuntu.Application 0.1

Rectangle {
    id: surface
    width: 720
    height: 1280
    color: "blue"

    Connections {
        target: ApplicationManager
        onFocusRequested: {
            var favoriteApplicationStr = [
                'CameraApplication', 'GalleryApplication', 'BrowserApplication', 'ShareApplication'
            ];
            print("focus request:", favoriteApplicationStr[favoriteApplication])
        }
    }

    MouseArea {
        id: touchArea
        anchors.fill: parent
        onClicked: {
            ApplicationManager.focusFavoriteApplication(ApplicationManager.BrowserApplication);
        }
    }

    Column {
        id: header

        anchors {
            left: parent.left
            right: parent.right
        }
        spacing: 10

        Text {
            font.family: "Ubuntu"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
            text: "Apps running in main stage: %1".arg(ApplicationManager.mainStageApplications.count)
        }

        Text {
            font.family: "Ubuntu"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
            text: ApplicationManager.mainStageApplications.count >= 1 ?
                      "First is \"%1\" %2".arg(ApplicationManager.mainStageApplications.get(0).name)
                                          .arg(ApplicationManager.mainStageApplications.get(0))
                    : "Start an application with --desktop_file_hint=..."
        }

        Item {
            id: moveButton
            anchors {
                left: parent.left
                right: parent.right
            }
            height: 100

            Rectangle {
                anchors.fill: parent
                color: "#e9ecd9"
            }
            Text {
                anchors.centerIn: parent
                font.family: "Ubuntu"; font.weight: Font.Bold; font.pixelSize: 30; color: "darkgrey"
                text: "Move first application to second place"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: ApplicationManager.mainStageApplications.move(1, 0)
            }
        }
    }

    Row {
        anchors {
            top: header.bottom
            topMargin: 10
        }
        Repeater {
            model: ApplicationManager.mainStageApplications
            delegate: ApplicationImage {
                id: applicationImage
                width: 720 / 4; height: 1280 / 4
                source: application
                Text {
                    font.family: "Ubuntu"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
                    text: application.name
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: applicationImage.scheduleUpdate()
                }

                Timer {
                    running: true
                    onTriggered: applicationImage.scheduleUpdate()
                }
            }
        }
    }

    Component.onCompleted: {
        // Display form factor and stage hints.
        var formFactorHintStr = [
            'Desktop', 'Phone', 'Tablet'
        ];
        var stageHintStr = [
            'Main', 'Integration', 'Share', 'ContentPicking', 'Side', 'Configuration'
        ];
        print('Form factor hint:', formFactorHintStr[ApplicationManager.formFactorHint]);
        print('Stage hint:', stageHintStr[ApplicationManager.stageHint]);
    }
}
