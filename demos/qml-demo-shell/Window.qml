import QtQuick 2.2
import Ubuntu.Components 1.1
import Unity.Application 0.1

FocusScope {
    id: root

    property var windowData

    width: windowWidth + d.resizeEdge
    height: windowHeight + decoration.height + d.resizeEdge
    visible: (windowData.state !== MirSurfaceItem.Minimized)
    readonly property int windowX: 0
    readonly property int windowY: decoration.height
    readonly property int windowWidth: windowData.implicitWidth
    readonly property int windowHeight: windowData.implicitHeight
    readonly property alias resizable: d.resizable
    readonly property bool movable: d.movable // false implies is anchored to parent

    /* Other info */
    readonly property string type: windowData.type     // regular, floating, dialog, satellite, popup, gloss, tip, freestyle (for info purposes only)
    readonly property bool interactive: d.interactive
//    readonly property int parentId: windowData.parentId
//    readonly property int windowId: windowData.id
//    readonly property var childrenIds: windowData.childrenIds

    Component.onCompleted: {
        windowData.parent = surfaceContainer
        windowData.focus = true

        d.setPosition(windowData)
        state = "open"; // animates surface opening
    }

    QtObject {
        id: d
        property int resizeEdge: (resizable) ? units.gu(2) : 0

        readonly property string title: windowData.name

        readonly property bool resizable: {
            switch (windowData.type) {
            case MirSurfaceItem.Normal:
            case MirSurfaceItem.Freestyle:
            case MirSurfaceItem.Utility:
                return true;
            case MirSurfaceItem.Dialog:
            case MirSurfaceItem.Popover:
            case MirSurfaceItem.Overlay:
            case MirSurfaceItem.InputMethod:
                return false;
            }
        }

        readonly property bool movable: {
            switch (windowData.type) {
            case MirSurfaceItem.Normal:
            case MirSurfaceItem.Freestyle:
            case MirSurfaceItem.Utility:
                return true;
            case MirSurfaceItem.Dialog:
            case MirSurfaceItem.Popover:
            case MirSurfaceItem.Overlay:
            case MirSurfaceItem.InputMethod:
                return false;
            }
        }

        readonly property string decorationType: { // normal, small, none, client-specified
            switch (windowData.type) {
            case MirSurfaceItem.Normal:
            case MirSurfaceItem.Dialog:
                return "normal";
            case MirSurfaceItem.Utility:
                return "small";
            case MirSurfaceItem.Popover:
            case MirSurfaceItem.Freestyle:
            case MirSurfaceItem.Overlay:
            case MirSurfaceItem.InputMethod:
                return "none"
            }
        }

        readonly property string interactive: {
            if (!windowData.enabled
                    || !windowData.live
                    || windowData.type === MirSurfaceItem.Overlay) {
                return false
            } else {
                return true
            }
        }

        readonly property bool canBeMaximized: {
            return windowData.type === MirSurfaceItem.Normal
        }
        readonly property bool canBeMinimized: {
            return windowData.type === MirSurfaceItem.Normal
        }
        readonly property bool canBeClosed: {
            return windowData.type === MirSurfaceItem.Normal || windowData.type === MirSurfaceItem.Dialog
        }

        function setPosition(surface) { print("POSITIONING", surface, "with parent", surface.parentSurface, "and children", surface.childSurfaces)
            // Root surface (i.e. no parent)
            if (!surface.parentSurface) {
                root.x = surface.requestedX
                root.y = surface.requestedY
                positionChildren()
                return
            }

            var parentWindow = windowView.getWindowForSurface(surface.parentSurface)

            // Has a root surface, but its Window not created yet (for clients with a child which made visible before parent)
            if (!parentWindow) {
                // maybe set invisible immediately, mark visible only when parent appears??
                return;
            }

            // set the parent/child relationship in QML
            // root.parent = parentWindow; - FIXME - does not do as I expect: change child's coordinates to be relative to parent
            // relevant error: "QQuickItem::stackAfter: Cannot stack after 0x2048600, which must be a sibling"

            // position relative to parent - for Dialogs position in center of parent
            if (surface.type === MirSurfaceItem.Dialog && surface.requestedX === 0 && surface.requestedX === 0) {
                root.x = Qt.binding( function() { return  parentWindow.x + (parentWindow.width - root.width) / 2; } )
                root.y = Qt.binding( function() { return  parentWindow.y + parentWindow.windowY + (parentWindow.height - root.height) / 2; } )
            } else {
                root.x = Qt.binding( function() { return parentWindow.x + surface.requestedX + parentWindow.windowX; } )
                root.y = Qt.binding( function() { return parentWindow.y + surface.requestedY + parentWindow.windowY; } )
            }
            positionChildren()
        }

        function positionChildren() {
            // If I have children before I'm fully created, reposition them relative to me
            // FIXME: how do I iterate over the childSurfaces model? QML insists it is a QAbstractListModel and I cannot
            // call count or get methods. Instead I use Repeater to get at the individual list items
            for (var i = 0; i < childSurfacesRepeater.count; i++) {
                var child = childSurfacesRepeater.itemAt(i).child;
                var childWindow = windowView.getWindowForSurface(child)
                childWindow.setPosition(child)
            }
        }
    }

    function setPosition(surface) {
        d.setPosition(surface)
    }

    Item {
        visible: false
        Repeater {
            id: childSurfacesRepeater
            model: windowData.childSurfaces
            delegate: Item {
                readonly property var child: modelData
            }
        }
    }

    focus: interactive

    WindowMoveResizeArea {
        target: root
        minWidth: 120
        minHeight: 80
        resizeHandleWidth: units.gu(0.5)
    }

    BorderImage {
        objectName: "resizeHandle"
        anchors {
            fill: root
            margins: -units.gu(2)
        }
        source: "dropshadow2gu.sci"
        opacity: .3
        Behavior on opacity { UbuntuNumberAnimation {} }
        visible: true //root.windowState === "normal"
    }

    WindowDecoration {
        id: decoration
        objectName: "decoration"

        anchors { left: parent.left; top: parent.top; right: parent.right; rightMargin: d.resizeEdge; }
        height: {
            switch (d.decorationType) {
            case "normal":
                return units.gu(4)
            case "small":
                return units.gu(2)
            case "none":
                return 0
            }
        }
        title: d.title
        active: root.activeFocus
        canBeMaximized: d.canBeMaximized
        canBeMinimized: d.canBeMinimized
        canBeClosed: d.canBeClosed
        onClose: windowData.requestClose();
        onMaximize: root.maximize();
        onMinimize: root.minimize();
    }

    Item {
        id: surfaceContainer
        objectName: "window"
        anchors { left: decoration.left; top: decoration.bottom; right: decoration.right; bottom: parent.bottom;
                  bottomMargin: d.resizeEdge}
//        Rectangle { anchors.fill: parent; color: "red"}
        onWidthChanged: windowData.requestResize(width, height)
        onHeightChanged: windowData.requestResize(width, height)
    }

    state: "closed"

    states: [
        State {
            name: "open"
            PropertyChanges { target: root; opacity: 1; scale: 1 }
        },
        State {
            name: "closed"
            PropertyChanges { target: root; opacity: 0; scale: 0.9 }
        }
    ]

    transitions: [
        Transition {
            from: "closed"
            to: "open"
            PropertyAnimation { target: root; properties: "opacity, scale"; duration: 200 }
        },
        Transition {
            from: "open"
            to: "closed"
            SequentialAnimation {
                PropertyAnimation { target: root; properties: "opacity, scale"; duration: 200 }
                ScriptAction { script: windowData.release(); }
            }
        }
    ]
}
