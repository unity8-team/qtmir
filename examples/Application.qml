// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

// Shows the types added by the Ubuntu application plugin and how to use them.

// Here is how to create a window with specific surface role (Greeter here) from QML.
//
// import QtQuick.Window 2.0
//
// Window {
//     width: 400
//     height: 400
//     Component.onCompleted: {
//         window.role = ApplicationManager.Greeter
//         window.visible = true
//     }
// }

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

    Row {
        Repeater {
            model: ApplicationManager.applications
            delegate: ApplicationImage {
                id: applicationImage
                width: 720 / 4; height: 1280 / 4
                source: application
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
