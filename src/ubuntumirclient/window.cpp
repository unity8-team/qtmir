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

// Local
#include "window.h"
#include "clipboard.h"
#include "nativeinterface.h"
#include "input.h"
#include "screen.h"
#include "logging.h"

#include <mir_toolkit/mir_client_library.h>

// Qt
#include <qpa/qwindowsysteminterface.h>
#include <QMutexLocker>
#include <QSize>
#include <QtMath>
#include <private/qeglconvenience_p.h>

// Platform API
#include <ubuntu/application/instance.h>

#include <EGL/egl.h>

Q_LOGGING_CATEGORY(ubuntumirclientBufferSwap, "ubuntumirclient.bufferSwap", QtWarningMsg)

const Qt::WindowType LowChromeWindowHint = (Qt::WindowType)0x00800000;

namespace
{

// FIXME: this used to be defined by platform-api, but it's been removed in v3. Change ubuntu-keyboard to use
// a different enum for window roles.
enum UAUiWindowRole {
    U_MAIN_ROLE = 1,
    U_DASH_ROLE,
    U_INDICATOR_ROLE,
    U_NOTIFICATIONS_ROLE,
    U_GREETER_ROLE,
    U_LAUNCHER_ROLE,
    U_ON_SCREEN_KEYBOARD_ROLE,
    U_SHUTDOWN_DIALOG_ROLE,
};

struct MirSpecDeleter
{
    void operator()(MirSurfaceSpec *spec) { mir_surface_spec_release(spec); }
};

using Spec = std::unique_ptr<MirSurfaceSpec, MirSpecDeleter>;

EGLNativeWindowType nativeWindowFor(MirSurface *surf)
{
    auto stream = mir_surface_get_buffer_stream(surf);
    return reinterpret_cast<EGLNativeWindowType>(mir_buffer_stream_get_egl_native_window(stream));
}

const char *qtWindowStateToStr(Qt::WindowState state)
{
    switch (state) {
    case Qt::WindowNoState:
        return "NoState";
    case Qt::WindowFullScreen:
        return "FullScreen";
    case Qt::WindowMaximized:
        return "Maximized";
    case Qt::WindowMinimized:
        return "Minimized";
    case Qt::WindowActive:
        return "Active";
    default:
        return "!?";
    }
}

const char *mirSurfaceStateToStr(MirSurfaceState surfaceState)
{
    switch (surfaceState) {
    case mir_surface_state_unknown: return "unknown";
    case mir_surface_state_restored: return "restored";
    case mir_surface_state_minimized: return "minimized";
    case mir_surface_state_maximized: return "vertmaximized";
    case mir_surface_state_vertmaximized: return "vertmaximized";
    case mir_surface_state_fullscreen: return "fullscreen";
    case mir_surface_state_horizmaximized: return "horizmaximized";
    case mir_surface_state_hidden: return "hidden";
    default: return "!?";
    }
}

const char *mirPixelFormatToStr(MirPixelFormat pixelFormat)
{
    switch (pixelFormat) {
    case mir_pixel_format_invalid:   return "invalid";
    case mir_pixel_format_abgr_8888: return "ABGR8888";
    case mir_pixel_format_xbgr_8888: return "XBGR8888";
    case mir_pixel_format_argb_8888: return "ARGB8888";
    case mir_pixel_format_xrgb_8888: return "XRGB8888";
    case mir_pixel_format_bgr_888:   return "BGR888";
    case mir_pixel_format_rgb_888:   return "RGB888";
    case mir_pixel_format_rgb_565:   return "RGB565";
    case mir_pixel_format_rgba_5551: return "RGBA5551";
    case mir_pixel_format_rgba_4444: return "RGBA4444";
    case mir_pixel_formats:
    default:                         return "???";
    }
}

MirSurfaceState qtWindowStateToMirSurfaceState(Qt::WindowState state)
{
    switch (state) {
    case Qt::WindowNoState:
        return mir_surface_state_restored;
    case Qt::WindowFullScreen:
        return mir_surface_state_fullscreen;
    case Qt::WindowMaximized:
        return mir_surface_state_maximized;
    case Qt::WindowMinimized:
        return mir_surface_state_minimized;
    default:
        qCWarning(ubuntumirclient, "Unexpected Qt::WindowState: %d", state);
        return mir_surface_state_restored;
    }
}

WId makeId()
{
    static int id = 1;
    return id++;
}

UAUiWindowRole roleFor(QWindow *window)
{
    QVariant roleVariant = window->property("role");
    if (!roleVariant.isValid())
        return U_MAIN_ROLE;

    uint role = roleVariant.toUInt();
    if (role < U_MAIN_ROLE || role > U_SHUTDOWN_DIALOG_ROLE)
        return U_MAIN_ROLE;

    return static_cast<UAUiWindowRole>(role);
}

UbuntuWindow *transientParentFor(QWindow *window)
{
    QWindow *parent = window->transientParent();
    return parent ? static_cast<UbuntuWindow *>(parent->handle()) : nullptr;
}

Spec makeSurfaceSpec(QWindow *window, UbuntuInput *input, MirPixelFormat pixelFormat, MirConnection *connection)
{
    const auto geom = window->geometry();
    const int width = geom.width() > 0 ? geom.width() : 1;
    const int height = geom.height() > 0 ? geom.height() : 1;

    if (U_ON_SCREEN_KEYBOARD_ROLE == roleFor(window)) {
        qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating input method surface (width=%d, height=%d", window, width, height);
        return Spec{mir_connection_create_spec_for_input_method(connection, width, height, pixelFormat)};
    }

    const Qt::WindowType type = window->type();
    if (type == Qt::Popup) {
        auto parent = transientParentFor(window);
        if (parent == nullptr) {
            //NOTE: We cannot have a parentless popup -
            //try using the last surface to receive input as that will most likely be
            //the one that caused this popup to be created
            parent = input->lastFocusedWindow();
        }
        if (parent) {
            auto pos = geom.topLeft();
            pos -= parent->geometry().topLeft();
            MirRectangle location{pos.x(), pos.y(), 0, 0};
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating menu surface(width:%d, height:%d)", window, width, height);
            return Spec{mir_connection_create_spec_for_menu(
                    connection, width, height, pixelFormat, parent->mirSurface(),
                    &location, mir_edge_attachment_any)};
        } else {
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - cannot create a menu without a parent!", window);
        }
    } else if (type == Qt::Dialog) {
        auto parent = transientParentFor(window);
        if (parent) {
            // Modal dialog
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating modal dialog (width=%d, height=%d", window, width, height);
            return Spec{mir_connection_create_spec_for_modal_dialog(connection, width, height, pixelFormat, parent->mirSurface())};
        } else {
            // TODO: do Qt parentless dialogs have the same semantics as mir?
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating parentless dialog (width=%d, height=%d)", window, width, height);
            return Spec{mir_connection_create_spec_for_dialog(connection, width, height, pixelFormat)};
        }
    }
    qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating normal surface(type=0x%x, width=%d, height=%d)", window, type, width, height);
    return Spec{mir_connection_create_spec_for_normal_surface(connection, width, height, pixelFormat)};
}

void setSizingConstraints(MirSurfaceSpec *spec, const QSize& minSize, const QSize& maxSize, const QSize& increment)
{
    mir_surface_spec_set_min_width(spec, minSize.width());
    mir_surface_spec_set_min_height(spec, minSize.height());
    if (maxSize.width() >= minSize.width()) {
        mir_surface_spec_set_max_width(spec, maxSize.width());
    }
    if (maxSize.height() >= minSize.height()) {
        mir_surface_spec_set_max_height(spec, maxSize.height());
    }
    if (increment.width() > 0) {
        mir_surface_spec_set_width_increment(spec, increment.width());
    }
    if (increment.height() > 0) {
        mir_surface_spec_set_height_increment(spec, increment.height());
    }
}

MirSurface *createMirSurface(QWindow *window, int mirOutputId, UbuntuInput *input, MirPixelFormat pixelFormat,
                             MirConnection *connection, mir_surface_event_callback inputCallback,
                             void* inputContext)
{
    auto spec = makeSurfaceSpec(window, input, pixelFormat, connection);

    // Install event handler as early as possible
    mir_surface_spec_set_event_handler(spec.get(), inputCallback, inputContext);

    const auto title = window->title().toUtf8();
    mir_surface_spec_set_name(spec.get(), title.constData());

    setSizingConstraints(spec.get(), window->minimumSize(), window->maximumSize(), window->sizeIncrement());

    if (window->windowState() == Qt::WindowFullScreen) {
        mir_surface_spec_set_fullscreen_on_output(spec.get(), mirOutputId);
    }

    if (window->flags() & LowChromeWindowHint) {
        mir_surface_spec_set_shell_chrome(spec.get(), mir_shell_chrome_low);
    }

    auto surface = mir_surface_create_sync(spec.get());
    Q_ASSERT(mir_surface_is_valid(surface));
    return surface;
}

MirPixelFormat disableAlphaBufferIfPossible(MirPixelFormat pixelFormat)
{
    switch(pixelFormat) {
    case mir_pixel_format_abgr_8888:
        return mir_pixel_format_xbgr_8888;
    case mir_pixel_format_argb_8888:
        return mir_pixel_format_xrgb_8888;
    default: // can do nothing, leave it alone
        return pixelFormat;
    }
}

// FIXME - in order to work around https://bugs.launchpad.net/mir/+bug/1346633
// we need to guess the panel height (3GU)
int panelHeight()
{
    const int defaultGridUnit = 8;
    int gridUnit = defaultGridUnit;
    QByteArray gridUnitString = qgetenv("GRID_UNIT_PX");
    if (!gridUnitString.isEmpty()) {
        bool ok;
        gridUnit = gridUnitString.toInt(&ok);
        if (!ok) {
            gridUnit = defaultGridUnit;
        }
    }
    return gridUnit * 3;
}

} //namespace

class UbuntuSurface
{
public:
    UbuntuSurface(UbuntuWindow *platformWindow, EGLDisplay display, UbuntuInput *input, MirConnection *connection)
        : mWindow(platformWindow->window())
        , mPlatformWindow(platformWindow)
        , mInput(input)
        , mConnection(connection)
        , mEglDisplay(display)
        , mNeedsRepaint(false)
        , mParented(mWindow->transientParent() || mWindow->parent())
        , mFormat(mWindow->requestedFormat())
        , mShellChrome(mWindow->flags() & LowChromeWindowHint ? mir_shell_chrome_low : mir_shell_chrome_normal)
    {
        // Have Qt choose most suitable EGLConfig for the requested surface format, and update format to reflect it
        EGLConfig config = q_configFromGLFormat(display, mFormat, true);
        if (config == 0) {
            // Older Intel Atom-based devices only support OpenGL 1.4 compatibility profile but by default
            // QML asks for at least OpenGL 2.0. The XCB GLX backend ignores this request and returns a
            // 1.4 context, but the XCB EGL backend tries to honour it, and fails. The 1.4 context appears to
            // have sufficient capabilities on MESA (i915) to render correctly however. So reduce the default
            // requested OpenGL version to 1.0 to ensure EGL will give us a working context (lp:1549455).
            static const bool isMesa = QString(eglQueryString(display, EGL_VENDOR)).contains(QStringLiteral("Mesa"));
            if (isMesa) {
                qCDebug(ubuntumirclient, "Attempting to choose OpenGL 1.4 context which may suit Mesa");
                mFormat.setMajorVersion(1);
                mFormat.setMinorVersion(4);
                config = q_configFromGLFormat(display, mFormat, true);
            }
        }
        if (config == 0) {
            qCritical() << "Qt failed to choose a suitable EGLConfig to suit the surface format" << mFormat;
        }

        mFormat = q_glFormatFromConfig(display, config, mFormat);

        // Have Mir decide the pixel format most suited to the chosen EGLConfig. This is the only way
        // Mir will know what EGLConfig has been chosen - it cannot deduce it from the buffers.
        auto pixelFormat = mir_connection_get_egl_pixel_format(connection, display, config);
        // But the chosen EGLConfig might have an alpha buffer enabled, even if not requested by the client.
        // If that's the case, try to edit the chosen pixel format in order to disable the alpha buffer.
        // This is an optimisation for the compositor, as it can avoid blending this surface.
        if (mWindow->requestedFormat().alphaBufferSize() < 0) {
            pixelFormat = disableAlphaBufferIfPossible(pixelFormat);
        }

        const auto outputId = static_cast<UbuntuScreen *>(mWindow->screen()->handle())->mirOutputId();

        mMirSurface = createMirSurface(mWindow, outputId, input, pixelFormat, connection, surfaceEventCallback, this);
        mEglSurface = eglCreateWindowSurface(mEglDisplay, config, nativeWindowFor(mMirSurface), nullptr);

        // Window manager can give us a final size different from what we asked for
        // so let's check what we ended up getting
        MirSurfaceParameters parameters;
        mir_surface_get_parameters(mMirSurface, &parameters);

        auto geom = mWindow->geometry();
        geom.setWidth(parameters.width);
        geom.setHeight(parameters.height);
        if (mWindow->windowState() == Qt::WindowFullScreen) {
            geom.setY(0);
        } else {
            geom.setY(panelHeight());
        }

        // Assume that the buffer size matches the surface size at creation time
        mBufferSize = geom.size();
        platformWindow->QPlatformWindow::setGeometry(geom);
        QWindowSystemInterface::handleGeometryChange(mWindow, geom);

        qCDebug(ubuntumirclient) << "Created surface with geometry:" << geom << "title:" << mWindow->title()
                                 << "role:" << roleFor(mWindow)
                                 << "\nRequested format:" << mWindow->requestedFormat()
                                 << "\nActual format:" << mFormat
                                 << "with associated Mir pixel format:" << mirPixelFormatToStr(pixelFormat);
    }

    ~UbuntuSurface()
    {
        if (mEglSurface != EGL_NO_SURFACE)
            eglDestroySurface(mEglDisplay, mEglSurface);
        if (mMirSurface)
            mir_surface_release_sync(mMirSurface);
    }

    UbuntuSurface(const UbuntuSurface &) = delete;
    UbuntuSurface& operator=(const UbuntuSurface &) = delete;

    void resize(const QSize& newSize);
    void updateTitle(const QString& title);
    void setSizingConstraints(const QSize& minSize, const QSize& maxSize, const QSize& increment);

    void onSwapBuffersDone();
    void handleSurfaceResized(int width, int height);
    int needsRepaint() const;

    MirSurfaceState state() const { return mir_surface_get_state(mMirSurface); }
    void setState(MirSurfaceState state);

    MirSurfaceType type() const { return mir_surface_get_type(mMirSurface); }

    void setShellChrome(MirShellChrome shellChrome);

    EGLSurface eglSurface() const { return mEglSurface; }
    MirSurface *mirSurface() const { return mMirSurface; }

    void setSurfaceParent(MirSurface*);
    bool hasParent() const { return mParented; }

    QSurfaceFormat format() const { return mFormat; }

private:
    static void surfaceEventCallback(MirSurface* surface, const MirEvent *event, void* context);
    void postEvent(const MirEvent *event);

    QWindow * const mWindow;
    UbuntuWindow * const mPlatformWindow;
    UbuntuInput * const mInput;
    MirConnection * const mConnection;

    MirSurface* mMirSurface;
    const EGLDisplay mEglDisplay;
    EGLSurface mEglSurface;

    bool mNeedsRepaint;
    bool mParented;
    QSize mBufferSize;
    QSurfaceFormat mFormat;

    QMutex mTargetSizeMutex;
    QSize mTargetSize;
    MirShellChrome mShellChrome;
};

void UbuntuSurface::resize(const QSize& size)
{
    qCDebug(ubuntumirclient,"resize(window=%p, width=%d, height=%d)", mWindow, size.width(), size.height());

    if (mWindow->windowState() == Qt::WindowFullScreen || mWindow->windowState() == Qt::WindowMaximized) {
        qCDebug(ubuntumirclient, "resize(window=%p) - not resizing, window is maximized or fullscreen", mWindow);
        return;
    }

    if (size.isEmpty()) {
        qCDebug(ubuntumirclient, "resize(window=%p) - not resizing, size is empty", mWindow);
        return;
    }

    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_width(spec.get(), size.width());
    mir_surface_spec_set_height(spec.get(), size.height());
    mir_surface_apply_spec(mMirSurface, spec.get());
}

void UbuntuSurface::updateTitle(const QString& newTitle)
{
    const auto title = newTitle.toUtf8();
    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_name(spec.get(), title.constData());
    mir_surface_apply_spec(mMirSurface, spec.get());
}

void UbuntuSurface::setSizingConstraints(const QSize& minSize, const QSize& maxSize, const QSize& increment)
{
    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    ::setSizingConstraints(spec.get(), minSize, maxSize, increment);
    mir_surface_apply_spec(mMirSurface, spec.get());
}

void UbuntuSurface::handleSurfaceResized(int width, int height)
{
    QMutexLocker lock(&mTargetSizeMutex);

    // mir's resize event is mainly a signal that we need to redraw our content. We use the
    // width/height as identifiers to figure out if this is the latest surface resize event
    // that has posted, discarding any old ones. This avoids issuing too many redraw events.
    // see TODO in postEvent as the ideal way we should handle this.
    // The actual buffer size may or may have not changed at this point, so let the rendering
    // thread drive the window geometry updates.
    mNeedsRepaint = mTargetSize.width() == width && mTargetSize.height() == height;
}

int UbuntuSurface::needsRepaint() const
{
    if (mNeedsRepaint) {
        if (mTargetSize != mBufferSize) {
            //If the buffer hasn't changed yet, we need at least two redraws,
            //once to get the new buffer size and propagate the geometry changes
            //and the second to redraw the content at the new size
            return 2;
        } else {
            // The buffer size has already been updated so we only need one redraw
            // to render at the new size
            return 1;
        }
    }
    return 0;
}

void UbuntuSurface::setState(MirSurfaceState state)
{
    mir_wait_for(mir_surface_set_state(mMirSurface, state));
}

void UbuntuSurface::setShellChrome(MirShellChrome chrome)
{
    if (chrome != mShellChrome) {
        auto spec = Spec{mir_connection_create_spec_for_changes(mConnection)};
        mir_surface_spec_set_shell_chrome(spec.get(), chrome);
        mir_surface_apply_spec(mMirSurface, spec.get());

        mShellChrome = chrome;
    }
}

void UbuntuSurface::onSwapBuffersDone()
{
    static int sFrameNumber = 0;
    ++sFrameNumber;

    EGLint eglSurfaceWidth = -1;
    EGLint eglSurfaceHeight = -1;
    eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, &eglSurfaceWidth);
    eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, &eglSurfaceHeight);

    const bool validSize = eglSurfaceWidth > 0 && eglSurfaceHeight > 0;

    if (validSize && (mBufferSize.width() != eglSurfaceWidth || mBufferSize.height() != eglSurfaceHeight)) {

        qCDebug(ubuntumirclientBufferSwap, "onSwapBuffersDone(window=%p) [%d] - size changed (%d, %d) => (%d, %d)",
               mWindow, sFrameNumber, mBufferSize.width(), mBufferSize.height(), eglSurfaceWidth, eglSurfaceHeight);

        mBufferSize.rwidth() = eglSurfaceWidth;
        mBufferSize.rheight() = eglSurfaceHeight;

        QRect newGeometry = mPlatformWindow->geometry();
        newGeometry.setSize(mBufferSize);

        mPlatformWindow->QPlatformWindow::setGeometry(newGeometry);
        QWindowSystemInterface::handleGeometryChange(mWindow, newGeometry);
    } else {
        qCDebug(ubuntumirclientBufferSwap, "onSwapBuffersDone(window=%p) [%d] - buffer size (%d,%d)",
               mWindow, sFrameNumber, mBufferSize.width(), mBufferSize.height());
    }
}

void UbuntuSurface::surfaceEventCallback(MirSurface *surface, const MirEvent *event, void* context)
{
    Q_UNUSED(surface);
    Q_ASSERT(context != nullptr);

    auto s = static_cast<UbuntuSurface *>(context);
    s->postEvent(event);
}

void UbuntuSurface::postEvent(const MirEvent *event)
{
    if (mir_event_type_resize == mir_event_get_type(event)) {
        // TODO: The current event queue just accumulates all resize events;
        // It would be nicer if we could update just one event if that event has not been dispatched.
        // As a workaround, we use the width/height as an identifier of this latest event
        // so the event handler (handleSurfaceResized) can discard/ignore old ones.
        const auto resizeEvent = mir_event_get_resize_event(event);
        const auto width =  mir_resize_event_get_width(resizeEvent);
        const auto height =  mir_resize_event_get_height(resizeEvent);
        qCDebug(ubuntumirclient, "resizeEvent(window=%p, width=%d, height=%d)", mWindow, width, height);

        QMutexLocker lock(&mTargetSizeMutex);
        mTargetSize.rwidth() = width;
        mTargetSize.rheight() = height;
    }

    mInput->postEvent(mPlatformWindow, event);
}

void UbuntuSurface::setSurfaceParent(MirSurface* parent)
{
    qCDebug(ubuntumirclient, "setSurfaceParent(window=%p)", mWindow);

    mParented = true;
    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_parent(spec.get(), parent);
    mir_surface_apply_spec(mMirSurface, spec.get());
}

UbuntuWindow::UbuntuWindow(QWindow *w, const QSharedPointer<UbuntuClipboard> &clipboard,
                           UbuntuInput *input, UbuntuNativeInterface *native, EGLDisplay eglDisplay, MirConnection *mirConnection)
    : QObject(nullptr)
    , QPlatformWindow(w)
    , mId(makeId())
    , mClipboard(clipboard)
    , mWindowState(w->windowState())
    , mWindowFlags(w->flags())
    , mWindowVisible(false)
    , mWindowExposed(true)
    , mNativeInterface(native)
    , mSurface(new UbuntuSurface{this, eglDisplay, input, mirConnection})
    , mScale(1.0)
    , mFormFactor(mir_form_factor_unknown)
{
    qCDebug(ubuntumirclient, "UbuntuWindow(window=%p, screen=%p, input=%p, surf=%p) with title '%s', role: '%d'",
            w, w->screen()->handle(), input, mSurface.get(), qPrintable(window()->title()), roleFor(window()));

    updatePanelHeightHack(w->windowState() != Qt::WindowFullScreen);
}

UbuntuWindow::~UbuntuWindow()
{
    qCDebug(ubuntumirclient, "~UbuntuWindow(window=%p)", this);
}

void UbuntuWindow::handleSurfaceResized(int width, int height)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "handleSurfaceResize(window=%p, size=(%dx%d)px", window(), width, height);

    mSurface->handleSurfaceResized(width, height);

    // This resize event could have occurred just after the last buffer swap for this window.
    // This means the client may still be holding a buffer with the older size. The first redraw call
    // will then render at the old size. After swapping the client now will get a new buffer with the
    // updated size but it still needs re-rendering so another redraw may be needed.
    // A mir API to drop the currently held buffer would help here, so that we wouldn't have to redraw twice
    auto const numRepaints = mSurface->needsRepaint();
    lock.unlock();
    qCDebug(ubuntumirclient, "handleSurfaceResize(window=%p) redraw %d times", window(), numRepaints);
    for (int i = 0; i < numRepaints; i++) {
        qCDebug(ubuntumirclient, "handleSurfaceResize(window=%p) repainting size=(%dx%d)dp", window(), geometry().size().width(), geometry().size().height());
        QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));
    }
}

void UbuntuWindow::handleSurfaceExposeChange(bool exposed)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "handleSurfaceExposeChange(window=%p, exposed=%s)", window(), exposed ? "true" : "false");

    if (mWindowExposed == exposed) return;
    mWindowExposed = exposed;

    lock.unlock();
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));
}

void UbuntuWindow::handleSurfaceFocused()
{
    qCDebug(ubuntumirclient, "handleSurfaceFocused(window=%p)", window());

    // System clipboard contents might have changed while this window was unfocused and without
    // this process getting notified about it because it might have been suspended (due to
    // application lifecycle policies), thus unable to listen to any changes notified through
    // D-Bus.
    // Therefore let's ensure we are up to date with the system clipboard now that we are getting
    // focused again.
    mClipboard->requestDBusClipboardContents();
}

void UbuntuWindow::handleSurfaceVisibilityChanged(bool visible)
{
    qCDebug(ubuntumirclient, "handleSurfaceFocused(window=%p)", window());

    if (mWindowVisible == visible) return;
    mWindowVisible = visible;

    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));
}

void UbuntuWindow::handleSurfaceStateChanged(Qt::WindowState state)
{
    qCDebug(ubuntumirclient, "handleSurfaceStateChanged(window=%p, %s)", window(), qtWindowStateToStr(state));

    if (mWindowState == state) return;
    mWindowState = state;

    QWindowSystemInterface::handleWindowStateChanged(window(), state);
}

void UbuntuWindow::setWindowState(Qt::WindowState state)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setWindowState(window=%p, %s)", this, qtWindowStateToStr(state));

    if (mWindowState == state) return;
    mWindowState = state;

    lock.unlock();
    updateSurfaceState();
}

void UbuntuWindow::setWindowFlags(Qt::WindowFlags flags)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setWindowFlags(window=%p, 0x%x)", this, (int)flags);

    if (mWindowFlags == flags) return;
    mWindowFlags = flags;

    mSurface->setShellChrome(mWindowFlags & LowChromeWindowHint ? mir_shell_chrome_low : mir_shell_chrome_normal);
}

/*
    FIXME: Mir does not let clients know the position of their windows in the virtual
    desktop space. So we have this ugly hack that assumes a phone situation where the
    window is always on the top-left corner, right below the indicators panel if not
    in fullscreen.
 */
void UbuntuWindow::updatePanelHeightHack(bool enable)
{
    QMutexLocker lock(&mMutex);

    QRect newGeometry = geometry();
    if (enable) {
        newGeometry.setY(panelHeight());
    } else {
        newGeometry.setY(0);
    }

    if (newGeometry != geometry()) {
        lock.unlock();
        QPlatformWindow::setGeometry(newGeometry);
        QWindowSystemInterface::handleGeometryChange(window(), newGeometry);
    }
}

void UbuntuWindow::setGeometry(const QRect &rect)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setGeometry (window=%p, position=(%d, %d)dp, size=(%dx%d)dp)",
            window(), rect.x(), rect.y(), rect.width(), rect.height());

    //NOTE: mir surfaces cannot be moved by the client so ignore the topLeft coordinates
    const auto newSize = rect.size();

    mSurface->resize(newSize);
    // Note: don't call handleGeometryChange here, wait to see what Mir replies with.
}

void UbuntuWindow::setVisible(bool visible)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setVisible (window=%p, visible=%s)", window(), visible ? "true" : "false");

    if (mWindowVisible == visible) return;
    mWindowVisible = visible;

    if (visible) {
        if (!mSurface->hasParent() && window()->type() == Qt::Dialog) {
            // The dialog may have been parented after creation time
            // so morph it into a modal dialog
            auto parent = transientParentFor(window());
            if (parent) {
                mSurface->setSurfaceParent(parent->mirSurface());
            }
        }
    }

    lock.unlock();
    updateSurfaceState();
    QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));
}

void UbuntuWindow::setWindowTitle(const QString& title)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setWindowTitle(window=%p) title=%s)", window(), title.toUtf8().constData());
    mSurface->updateTitle(title);
}

void UbuntuWindow::propagateSizeHints()
{
    QMutexLocker lock(&mMutex);
    const auto win = window();
    qCDebug(ubuntumirclient, "propagateSizeHints(window=%p) min(%d,%d), max(%d,%d) increment(%d, %d)",
            win, win->minimumSize().width(), win->minimumSize().height(),
            win->maximumSize().width(), win->maximumSize().height(),
            win->sizeIncrement().width(), win->sizeIncrement().height());
    mSurface->setSizingConstraints(win->minimumSize(), win->maximumSize(), win->sizeIncrement());
}

bool UbuntuWindow::isExposed() const
{
    return mWindowVisible && mWindowExposed;
}

QSurfaceFormat UbuntuWindow::format() const
{
    return mSurface->format();
}

void* UbuntuWindow::eglSurface() const
{
    return mSurface->eglSurface();
}

MirSurface *UbuntuWindow::mirSurface() const
{
    return mSurface->mirSurface();
}

WId UbuntuWindow::winId() const
{
    return mId;
}

void UbuntuWindow::onSwapBuffersDone()
{
    QMutexLocker lock(&mMutex);
    mSurface->onSwapBuffersDone();
}

void UbuntuWindow::handleScreenPropertiesChange(MirFormFactor formFactor, float scale)
{
    // Update the scale & form factor native-interface properties for the windows affected
    // as there is no convenient way to emit signals for those custom properties on a QScreen
    if (formFactor != mFormFactor) {
        mFormFactor = formFactor;
        Q_EMIT mNativeInterface->windowPropertyChanged(this, QStringLiteral("formFactor"));
    }

    if (!qFuzzyCompare(scale, mScale)) {
        mScale = scale;
        // update the panelHeight hack since it depends on GU
        updatePanelHeightHack(mSurface->state() != mir_surface_state_fullscreen);

        Q_EMIT mNativeInterface->windowPropertyChanged(this, QStringLiteral("scale"));
    }
}

void UbuntuWindow::updateSurfaceState()
{
    QMutexLocker lock(&mMutex);
    MirSurfaceState newState = mWindowVisible ? qtWindowStateToMirSurfaceState(mWindowState) :
                                                mir_surface_state_hidden;
    qCDebug(ubuntumirclient, "updateSurfaceState (window=%p, surfaceState=%s)", window(), mirSurfaceStateToStr(newState));
    if (newState != mSurface->state()) {
        mSurface->setState(newState);

        lock.unlock();
        updatePanelHeightHack(newState != mir_surface_state_fullscreen);
    }
}
