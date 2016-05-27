#ifndef SURFACEFORMATCHOOSER_H
#define SURFACEFORMATCHOOSER_H

#include <QSurfaceFormat>

class UbuntuSurfaceFormatChooser {
public:
    static void update(QSurfaceFormat &format)
    {
        // If client has not explicitly requested a color depth, try default to ARGB8888. Otherwise
        // Qt on mobile devices tends to choose a lower color format like RGB565 or without alpha.
        if (format.redBufferSize() < 0
                && format.greenBufferSize() < 0
                && format.blueBufferSize() < 0
                && format.alphaBufferSize() < 0) {
            format.setRedBufferSize(8);
            format.setGreenBufferSize(8);
            format.setBlueBufferSize(8);
            format.setAlphaBufferSize(8);
        }

        // Older Intel Atom-based devices only support OpenGL 1.4 compatibility profile but by default
        // QML asks for at least OpenGL 2.0. The XCB GLX backend ignores this request and returns a
        // 1.4 context, but the XCB EGL backend tries to honour it, and fails. The 1.4 context appears to
        // have sufficient capabilities on MESA (i915) to render correctly however. So reduce the default
        // requested OpenGL version to 1.0 to ensure EGL will give us a working context (lp:1549455).
        if (format.majorVersion() == 2 && format.minorVersion() == 0) {
            format.setMajorVersion(1);
            format.setMinorVersion(0);
        }
    }
};

#endif // SURFACEFORMATCHOOSER_H
