import QtQuick 2.2
import Unity.Application 0.1

FocusScope {
    id: windowView
    focus: true

    MultiPointTouchArea {
        id: rendererContainer
        anchors.fill: parent
        minimumTouchPoints: 3
        maximumTouchPoints: 4

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

        property Item window: null
        property point previousPoint

        onPressed: {
            // center point of touchpoints
            var target = average(touchPoints)

            // if at least 2 touch points are within a Window, select that Window for moving
            window = rendererContainer.childAt(target.x, target.y);
            if (!window || !window.movable) return;

            // save mouse position
            previousPoint = target
        }

        onUpdated: {
            if (!window) return;

            var target = average(touchPoints)
            var movedBy = target - previousPoint
            window.x += target.x - previousPoint.x
            window.y += target.y - previousPoint.y
            previousPoint = target
        }

        onReleased: {
            window = null
        }

        function average(touchPoints) {
            var point = Qt.point(0, 0)
            for (var i=0; i<touchPoints.length; i++) {
                point.x += touchPoints[i].x
                point.y += touchPoints[i].y
            }
            point.x /= touchPoints.length
            point.y /= touchPoints.length
            return point
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
