// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

import QtQuick 2.0
import Ubuntu.Application 0.1

Item {
    id: surface
    width: 720
    height: 1280

    Rectangle {
        id: rect
        anchors.fill: parent
        color: "black"
        opacity: 0.7
    }

    MouseArea {
        id: touchArea
        anchors.fill: rect
        onClicked: {
            ApplicationManager.focusFavoriteApplication(ApplicationManager.Gallery);
        }
    }

    Column {
        Repeater {
            model: ApplicationManager.applications
            delegate: Text {
                font.family: "Ubuntu"; font.weight: Font.Bold; font.pixelSize: 30; color: "white"
                text: " - " + application.name + " (" + application.comment + ")"
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
