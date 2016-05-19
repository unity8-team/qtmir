#ifndef SURFACEFORMATCHOOSER_H
#define SURFACEFORMATCHOOSER_H

#include <QSurfaceFormat>

// If client has not explicitly requested a color depth, try default to ARGB8888.
// Otherwise Qt on mobile devices tends to choose a lower color format like RGB565 or without alpha.
class UbuntuSurfaceFormatChooser {
public:
    static void update(QSurfaceFormat &format)
    {
        if (format.redBufferSize() < 0
                && format.greenBufferSize() < 0
                && format.blueBufferSize() < 0
                && format.alphaBufferSize() < 0) {
            format.setRedBufferSize(8);
            format.setGreenBufferSize(8);
            format.setBlueBufferSize(8);
            format.setAlphaBufferSize(8);
        }
    }
};

#endif // SURFACEFORMATCHOOSER_H
