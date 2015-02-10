import QtQuick 2.0
import Unity.Application 0.1

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

    }

    MultiPointTouchArea {
        anchors.fill: parent
        minimumTouchPoints: 3
        maximumTouchPoints: 4
        touchPoints: [
            TouchPoint { id: point }
        ]
        property Item window: null
        property real previousX: 0
        property real previousY: 0

        onPressed: {
            // if at least 2 touch points are within a Window, select that Window
            window = windowContainer.childAt(point.x, point.y);

            // save mouse position
            previousX = point.x
            previousY = point.y
        }

        onUpdated: {
            if (!window) return;

            var offset = point.x - previousX
            window.x = offset

            offset = point.y - previousY
            window.y = offset
        }

        onReleased: {
            window = null
        }

        MultiPointTouchArea {
            id: touchArea
            x: unityLogo.x
            y: unityLogo.y
            width: unityLogo.width
            height: unityLogo.height
            minimumTouchPoints:1
            maximumTouchPoints:1
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

        Item {
            id: windowContainer
            anchors.fill: parent

            Repeater {
                model: ApplicationManager
                delegate: Repeater {
                    model: ApplicationManager.get(index).session.surfaces

                    Component.onCompleted: print('new app!')

                    delegate: Rectangle {
                        id: decoration
                        readonly property var surface: modelData
                        Component.onCompleted: {
                            var decorationHeight = (surface.type === MirSurfaceItem.Normal || surface.type === MirSurfaceItem.Dialog) ? 20 : 0

                            decoration.width = Qt.binding(function(){ return surface.width; })
                            decoration.height = Qt.binding(function(){ return surface.height + decorationHeight; })
                            decoration.x = surface.requestedX
                            decoration.y = surface.requestedY
                            surface.parent = decoration
                            surface.anchors.fill = decoration
                            surface.anchors.topMargin = decorationHeight
                            decoration.visible = Qt.binding(function(){ return surface.state !== MirSurfaceItem.Minimized })
                        }

                        visible: surface.state !== MirSurfaceItem.Minimized
                        color: "red"
                    }
                }

            }
        }
    }

    Rectangle {
        width: 30; height: 30
        color: "green"
        x: point.x
        y: point.y
    }


    Connections {
        target: SurfaceManager
        onSurfaceCreated: {
            print("new surface", surface.name, "type", surface.type, "state", surface.state, "geom", surface.requestedX, surface.requestedY, surface.width, surface.height)
        }
        onSurfaceDestroyed: {
            print("surface destroying", surface.name, surface.width, surface.height)
            surface.release()
        }
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
