// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

import QtQuick 2.0
import Ubuntu.Application 0.1

Rectangle {
    id: surface
    width: 720
    height: 1280
    color: "blue"

    MouseArea {
        id: touchArea
        anchors.fill: parent 
        onClicked: {
            ApplicationManager.focusFavoriteApplication(ApplicationManager.Gallery);
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
            text: "Number of applications running: %1".arg(ApplicationManager.applications.count)
        }

        Text {
            font.family: "Ubuntu"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
            text: ApplicationManager.applications.count >= 1 ?
                      "First is \"%1\" %2".arg(ApplicationManager.applications.get(0).name)
                                          .arg(ApplicationManager.applications.get(0))
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
                onClicked: ApplicationManager.applications.move(1, 0)
            }
        }
    }

    Row {
        anchors {
            top: header.bottom
            topMargin: 10
        }
        Repeater {
            model: ApplicationManager.applications
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

        // Start the watcher so that the ApplicationManager applications model can be populated.
        ApplicationManager.startWatcher();
    }
}
