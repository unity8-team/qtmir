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
//    ApplicationManager.startProcess("snowshoe")
//
// In order to pass additional arguments, a second optional argument can be used:
//    ApplicationManager.startProcess("snowshoe", [ "http://www.ubuntu.com" ])
//
// An application can be stopped by using:
//     ApplicationManager.stopProcess(application)
//
// In order to focus or start a favorite application, that function can be used:
//     ApplicationManager.focusFavoriteApplication(ApplicationManager.GalleryApplication);
//
// A good way to use Application.qml is to launch it using:
//     qmlscene-ubuntu --fullscreen --session 1 --role 0 Application.qml
// and then to launch other applications using:
//     qmlscene-ubuntu --session 0 --role 1 MovingLogo.qml \
//         --desktop_file_hint=/usr/share/applications/goodhope.desktop --stage_hint=main_stage

import QtQuick 2.0
import Unity.Application 0.1

Rectangle {
    id: surface
    width: 2560
    height: 1600
    color: "#000020"

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
            // ApplicationManager.focusFavoriteApplication(ApplicationManager.BrowserApplication);
        }
    }

    Column {
        id: mainHeader

        anchors {
            left: parent.left
            right: parent.right
        }
        spacing: 10

        Text {
            font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 40; color: "white"
            text: "Main stage:"
        }
    }

    Row {
        id: mainRow

        anchors {
            top: mainHeader.bottom
            topMargin: 10
        }

        Repeater {
            model: ApplicationManager.mainStageApplications
            delegate: ApplicationImage {
                id: applicationImage
                width: 2560 / 5; height: 1600 / 5
                source: application
                Text {
                    font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
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

    Column {
        id: sideHeader
        y: surface.height / 2

        anchors {
            left: parent.left
            right: parent.right
        }
        spacing: 10

        Text {
            font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 40; color: "white"
            text: "Side stage:"
        }
    }

    Row {
        anchors {
            top: sideHeader.bottom
            topMargin: 10
        }

        Repeater {
            model: ApplicationManager.sideStageApplications
            delegate: ApplicationImage {
                id: applicationImage
                width: 2560 / 5; height: 1600 / 5
                source: application
                Text {
                    font.family: "Ubuntu Mono"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
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
