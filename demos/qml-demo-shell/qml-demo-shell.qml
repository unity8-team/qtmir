import QtQuick 2.2

Rectangle {
    id: root

    focus: true
    Keys.onVolumeUpPressed: {
        console.log("\"Volume Up\" pressed");
    }
    Keys.onVolumeDownPressed: {
        console.log("\"Volume Down\" pressed");
    }

    gradient: Gradient {
        GradientStop { position: 0.0; color: "lightsteelblue" }
        GradientStop { position: 1.0; color: "pink" }
    }

    Image {
        id: unityLogo
        source: "UnityLogo.png"
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
        width: 600
        height: 600

        RotationAnimation {
            id: logoAnimation
            target: unityLogo
            from: 0
            to: 359
            duration: 3000
            easing.type: Easing.Linear
            loops: Animation.Infinite
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                if (logoAnimation.paused) {
                    logoAnimation.resume();
                } else if (logoAnimation.running) {
                    logoAnimation.pause();
                } else {
                    logoAnimation.start();
                }
            }
        }
    }

    WindowView {
        anchors.fill: parent
    }



//    NumberAnimation {
//        id: openAnimation
//        property: "x";
//        from: root.width; to: 10;
//        duration: 1200; easing.type: Easing.InOutQuad
//    }

//    SequentialAnimation {
//        id: closeAnimation
//        property variant surface: null
//        NumberAnimation {
//            target: (closeAnimation.surface && closeAnimation.surface.parent) ? closeAnimation.surface.parent.parent : null
//            property: "scale";
//            to: 0;
//            duration: 500; easing.type: Easing.InQuad
//        }
//        ScriptAction {
//            script: {
//                closeAnimation.surface.parent.destroy(); //parent.destroy();
//                closeAnimation.surface.release();
//                print("surface destroyed")
//            }
//        }
//    }
}
