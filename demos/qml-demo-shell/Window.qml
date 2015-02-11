import QtQuick 2.2
import Ubuntu.Components 1.1
import Unity.Application 0.1

FocusScope {
    id: root

    property var windowData

    width: windowWidth + d.resizeEdge
    height: windowHeight + decoration.height + d.resizeEdge
    visible: windowData.state !== MirSurfaceItem.Minimized
    readonly property int windowX: x
    readonly property int windowY: y + decoration.height
    readonly property int windowWidth: windowData.width
    readonly property int windowHeight: windowData.height
    readonly property alias resizable: d.resizable
    readonly property bool movable: d.movable // false implies is anchored to parent

    /* Other info */
    readonly property string type: windowData.type     // regular, floating, dialog, satellite, popup, gloss, tip, freestyle (for info purposes only)
    readonly property string windowState: "normal" // windowData.state   // normal, maximized, minimized, fullscreen
    readonly property bool interactive: true //windowData.interactive // may not be necessary, could go in MirSurface instead
//    readonly property int parentId: windowData.parentId
//    readonly property int windowId: windowData.id
//    readonly property var childrenIds: windowData.childrenIds

    Component.onCompleted: {
        windowData.parent = surfaceContainer
        windowData.height = Qt.binding( function() { return surfaceContainer.height } )
        windowData.width = Qt.binding( function() { return surfaceContainer.width } )

        d.setPosition(windowData)
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

        /* Decoration */
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

        readonly property bool canBeMaximized: {
            return windowData.type === MirSurfaceItem.Normal
        }
        readonly property bool canBeMinimized: {
            return windowData.type === MirSurfaceItem.Normal
        }
        readonly property bool canBeClosed: {
            return windowData.type === MirSurfaceItem.Normal || windowData.type === MirSurfaceItem.Dialog
        }

        function setPosition(surface) {
            var parentWindow = windowView.getWindowForSurface(surface.parentSurface)
            if (!surface.parentSurface || !parentWindow) {
                root.x = surface.requestedX
                root.y = surface.requestedY
                return;
            }

            // position relative to parent - for Dialogs position in center of parent
            if (surface.type === MirSurfaceItem.Dialog && surface.requestedX === 0 && surface.requestedX === 0) {
                root.x = Qt.binding( function() { return parentWindow.windowX + (parentWindow.width - root.width) / 2; } )
                root.y = Qt.binding( function() { return parentWindow.windowY - decoration.height + (parentWindow.height - root.height) / 2; } )
            } else {
                root.x = Qt.binding( function() { return parentWindow.windowX + surface.requestedX; } )
                root.y = Qt.binding( function() { return parentWindow.windowY + surface.requestedY; } )
            }
        }
    }

    focus: interactive

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
        onClose: root.close();
        onMaximize: root.maximize();
        onMinimize: root.minimize();
    }

    Item {
        id: surfaceContainer
        objectName: "window"
        anchors { left: decoration.left; top: decoration.bottom; right: decoration.right; bottom: parent.bottom;
                  bottomMargin: d.resizeEdge}
        Rectangle { anchors.fill: parent; color: "red"}
    }
}
