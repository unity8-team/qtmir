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

    MouseArea {
        anchors.fill: parent

        hoverEnabled: true

        property bool resizeWidth: false
        property bool resizeHeight: false
        property bool moving: false
        property Window window: null
        property int startX: 0
        property int startWidth: 0
        property int startY: 0
        property int startHeight: 0

        onPressed: {
            mouse.accepted = false // only accepted if desired action detected

            window = rendererContainer.childAt(mouse.x, mouse.y)
            if (!window)
                return

            // item can be either the window surface, shadow or decoration. Need to figure out which
            var mapped = rendererContainer.mapToItem(window, mouse.x, mouse.y)
            var windowComponent = window.childAt(mapped.x, mapped.y)
            if (!windowComponent)
                return

            var name = windowComponent.objectName
            if (name === "window") { // clicked on the window contents
                if (!window.interactive) {
                    mouse.accepted = true;
                    return
                }

                //windowList.requestFocusWindow(window.windowId)
                window.forceActiveFocus(Qt.MouseFocusReason)
            } else if (name === "decoration") {
                window.forceActiveFocus(Qt.MouseFocusReason)
                if (!window.movable) {
                    return
                }

                moving = true;
                startX = mouse.x - window.x
                startY = mouse.y - window.y
                mouse.accepted = true
            } else if (name === "resizeHandle") {
                if (!window.resizable) {
                    return
                }
                // right edge drag?
                if (mouse.x > window.windowWidth) {
                    resizeWidth = true;
                    startX = mouseX;
                    startWidth = window.windowWidth;
                    mouse.accepted = true;
                }
                // bottom edge drag?
                if (mouseY > window.windowHeight) {
                    resizeHeight = true;
                    startY = mouseY;
                    startHeight = window.windowHeight;
                    mouse.accepted = true;
                }
            }
        }

        onPositionChanged: {
            mouse.accepted = false

            if (!window)
                return;

            if (moving) {
                //windowList.moveWindow(window.windowId, mouse.x - startX, mouse.y - startY)
                window.x = mouse.x - startX
                window.y = mouse.y - startY
                mouse.accepted = true
            } else if (resizeWidth || resizeHeight) {
                var newWidth, newHeight
                if (resizeWidth) {
                    newWidth = startWidth + mouse.x - startX
                } else {
                    newWidth = window.windowWidth
                }

                if (resizeHeight) {
                    newHeight = startHeight + mouse.y - startY
                } else {
                    newHeight = window.windowHeight
                }

                //windowList.resizeWindow(window.windowId, newWidth, newHeight)
                window.width = newWidth
                window.height = newHeight
                mouse.accepted = true
            }
        }

        onReleased: {
            if (moving || resizeWidth || resizeHeight) {
                mouse.accepted = false
            }
            window = null;
            moving = false;
            resizeWidth = false;
            resizeHeight = false;
        }
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

    // Debug output
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
}
