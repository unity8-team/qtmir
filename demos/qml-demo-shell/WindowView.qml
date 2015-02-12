import QtQuick 2.2
import Unity.Application 0.1

FocusScope {
    id: windowView
    focus: true

    Item {
        id: rendererContainer
        anchors.fill: parent

        Repeater {
            id: renderer
            model: ApplicationManager
            delegate: Repeater {
                model: (session) ? session.surfaces : null
                readonly property var session: (application) ? application.session : null
                readonly property var application: ApplicationManager.get(index)

                Component.onCompleted: print('new app!')

                delegate: Window {
                    id: decoration
                    windowData: modelData
                    focus: true
                    Component.onCompleted: {
                        //if (windowView.activeFocus) {
                            ApplicationManager.focusApplication(application.appId)
                            forceActiveFocus(Qt.ActiveWindowFocusReason);
                        //}
                    }
                }
            }
        }
    }

    function getWindowForSurface(surface) {
        for (var i=0; i<rendererContainer.children.length; i++) {
            var window = rendererContainer.children[i];
            if (window.windowData === surface) {
                return window;
            }
        }
        return null;
    }

//    MultiPointTouchArea {
//        anchors.fill: parent
//        minimumTouchPoints: 3
//        maximumTouchPoints: 4
//        touchPoints: [
//            TouchPoint { id: point }
//        ]
//        //mouseEnabled: false
//        property Item window: null
//        property real previousX: 0
//        property real previousY: 0

//        onPressed: {
//            // if at least 2 touch points are within a Window, select that Window
//            window = rendererContainer.childAt(point.x, point.y);

//            // save mouse position
//            previousX = point.x
//            previousY = point.y
//        }

//        onUpdated: {
//            if (!window) return;

//            var offset = point.x - previousX
//            window.x = offset

//            offset = point.y - previousY
//            window.y = offset
//        }

//        onReleased: {
//            window = null
//        }
//    }

    Connections {
        target: SurfaceManager
        onSurfaceCreated: {
            print("new surface", surface.name, "type", surface.type, "state", surface.state, "geom", surface.requestedX, surface.requestedY, surface.width, surface.height)
        }
        onSurfaceDestroyed: {
            print("surface destroying", surface, surface.width, surface.height)
            var window = getWindowForSurface(surface);
            if (window) {
                window.state = "closed"
                //surface.release() - will be called by Window after close animation completed
            } else {
                print("Unknown surface closed?!")
            }
        }
    }
}
