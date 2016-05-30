#ifndef SURFACEFORMATFILTER_H
#define SURFACEFORMATFILTER_H

#include <QString>
#include <QSurfaceFormat>
#include <EGL/egl.h>

class UbuntuSurfaceFormatFilter {
public:
    static void filter(QSurfaceFormat &format, EGLDisplay display)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        static const bool doNotFilter = qEnvironmentVariableIntValue("QTUBUNTU_NO_FORMAT_FILTER");
#else
        static const bool doNotFilter = qgetenv("QTUBUNTU_NO_FORMAT_FILTER").toInt();
#endif
        if (doNotFilter) {
            return;
        }

        static const bool isMesa = QString(eglQueryString(display, EGL_VENDOR)).toLower().contains(QStringLiteral("mesa"));

        // If client has not explicitly requested any color depth, try default to RGB888. Otherwise
        // Qt on mobile devices tends to choose a lower color format like RGB565.
        if (format.redBufferSize() < 0
                && format.greenBufferSize() < 0
                && format.blueBufferSize() < 0) {
            format.setRedBufferSize(8);
            format.setGreenBufferSize(8);
            format.setBlueBufferSize(8);
        }

        // Older Intel Atom-based devices only support OpenGL 1.4 compatibility profile but by default
        // QML asks for at least OpenGL 2.0. The XCB GLX backend ignores this request and returns a
        // 1.4 context, but the XCB EGL backend tries to honour it, and fails. The 1.4 context appears to
        // have sufficient capabilities on MESA (i915) to render correctly however. So reduce the default
        // requested OpenGL version to 1.0 to ensure EGL will give us a working context (lp:1549455).
        if (isMesa) {
            format.setMajorVersion(1);
            format.setMinorVersion(0);
        }
    }
};

#endif // SURFACEFORMATFILTER_H
