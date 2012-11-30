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
        width: 500
        height: 500
        anchors.centerIn: parent
        color: "yellow"
        opacity: 0.8
    }

    MouseArea {
        id: touchArea
        anchors.fill: rect
        onClicked: {
            applicationManager.focusFavoriteApplication(ApplicationManager.Gallery);
        }
    }

    Component.onCompleted: {
        var formFactorHintStr = [
            'Desktop', 'Phone', 'Tablet'
        ];
        var stageHintStr = [
            'Main', 'Integration', 'Share', 'ContentPicking', 'Side', 'Configuration'
        ];
        print('Form factor hint:', formFactorHintStr[applicationManager.formFactorHint]);
        print('Stage hint:', stageHintStr[applicationManager.stageHint]);
    }
}
