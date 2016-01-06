/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UBUNTU_SCREEN_H
#define UBUNTU_SCREEN_H

#include <qpa/qplatformscreen.h>
#include <QSurfaceFormat>

#include <mircommon/mir_toolkit/common.h> // just for MirFormFactor enum

#include <EGL/egl.h>

#include "cursor.h"

struct MirConnection;
struct MirDisplayOutput;

class UbuntuScreen : public QObject, public QPlatformScreen
{
    Q_OBJECT
public:
    UbuntuScreen(const MirDisplayOutput &output, MirConnection *connection);
    virtual ~UbuntuScreen();

    // QPlatformScreen methods.
    QImage::Format format() const override { return mFormat; }
    int depth() const override { return mDepth; }
    QRect geometry() const override { return mGeometry; }
    QRect availableGeometry() const override { return mGeometry; }
    QSizeF physicalSize() const override { return mPhysicalSize; }
    qreal devicePixelRatio() const override { return mDevicePixelRatio; }
    QDpi logicalDpi() const override;
    Qt::ScreenOrientation nativeOrientation() const override { return mNativeOrientation; }
    Qt::ScreenOrientation orientation() const override { return mNativeOrientation; }
    QPlatformCursor *cursor() const override { return const_cast<UbuntuCursor*>(&mCursor); }

    // New methods.
    QSurfaceFormat surfaceFormat() const { return mSurfaceFormat; }
    EGLDisplay eglDisplay() const { return mEglDisplay; }
    EGLConfig eglConfig() const { return mEglConfig; }
    EGLNativeDisplayType eglNativeDisplay() const { return mEglNativeDisplay; }

    // Additional Screen properties from Mir
    uint32_t outputId() const { return mOutputId; }
    MirFormFactor formFactor() const { return mFormFactor; }
    float scale() const { return mScale; }

    // Internally used methods
    void setMirDisplayOutput(const MirDisplayOutput &output);
    void handleWindowSurfaceResize(int width, int height);
    uint32_t mirOutputId() const { return mOutputId; }

    // QObject methods.
    void customEvent(QEvent* event) override;

private:
    QRect mGeometry, mNativeGeometry;
    QSizeF mPhysicalSize;
    qreal mDevicePixelRatio;
    Qt::ScreenOrientation mNativeOrientation;
    Qt::ScreenOrientation mCurrentOrientation;
    QImage::Format mFormat;
    int mDepth;
    int mDpi;
    qreal mRefreshRate;
    MirFormFactor mFormFactor;
    float mScale;
    uint32_t mOutputId;
    EGLDisplay mEglDisplay;
    EGLConfig mEglConfig;
    EGLNativeDisplayType mEglNativeDisplay;
    QSurfaceFormat mSurfaceFormat;
    UbuntuCursor mCursor; //GERRY try const
};

#endif // UBUNTU_SCREEN_H
