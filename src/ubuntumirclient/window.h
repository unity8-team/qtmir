/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#ifndef UBUNTU_WINDOW_H
#define UBUNTU_WINDOW_H

#include <qpa/qplatformwindow.h>
#include <QSharedPointer>
#include <QMutex>

#include <mir_toolkit/common.h> // needed only for MirFormFactor enum

#include <memory>

#include <EGL/egl.h>

class UbuntuClipboard;
class UbuntuNativeInterface;
class UbuntuInput;
class UbuntuScreen;
class UbuntuSurface;
struct MirSurface;
class MirConnection;

class UbuntuWindow : public QObject, public QPlatformWindow
{
    Q_OBJECT
public:
    UbuntuWindow(QWindow *w, const QSharedPointer<UbuntuClipboard> &clipboard,
                 UbuntuInput *input, UbuntuNativeInterface* native, EGLDisplay eglDisplay,
                 MirConnection *mirConnection);
    virtual ~UbuntuWindow();

    // QPlatformWindow methods.
    WId winId() const override;
    void setGeometry(const QRect&) override;
    void setWindowState(Qt::WindowState state) override;
    void setWindowFlags(Qt::WindowFlags flags) override;
    void setVisible(bool visible) override;
    void setWindowTitle(const QString &title) override;
    void propagateSizeHints() override;
    bool isExposed() const override;

    QSurfaceFormat format() const override;

    // Additional Window properties exposed by NativeInterface
    MirFormFactor formFactor() const { return mFormFactor; }
    float scale() const { return mScale; }

    // New methods.
    void *eglSurface() const;
    MirSurface *mirSurface() const;
    void handleSurfaceResized(int width, int height);
    void handleSurfaceExposeChange(bool exposed);
    void handleSurfaceFocused();
    void handleSurfaceVisibilityChanged(bool visible);
    void handleSurfaceStateChanged(Qt::WindowState state);
    void onSwapBuffersDone();
    void handleScreenPropertiesChange(MirFormFactor formFactor, float scale);

private:
    void updatePanelHeightHack(bool enable);
    void updateSurfaceState();
    mutable QMutex mMutex;
    const WId mId;
    const QSharedPointer<UbuntuClipboard> mClipboard;
    Qt::WindowState mWindowState;
    Qt::WindowFlags mWindowFlags;
    bool mWindowVisible;
    bool mWindowExposed;
    UbuntuNativeInterface *mNativeInterface;
    std::unique_ptr<UbuntuSurface> mSurface;
    float mScale;
    MirFormFactor mFormFactor;
};

#endif // UBUNTU_WINDOW_H
