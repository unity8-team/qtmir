import QtQuick 2.4
import Unity.Application 0.1

MousePointer {
    id: mousePointer
    Image {
        x: -mousePointer.hotspotX
        y: -mousePointer.hotspotY
        source: "image://cursor/" + mousePointer.cursorName
    }
}
