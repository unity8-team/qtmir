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

// Local
#include "window.h"
#include "clipboard.h"
#include "input.h"
#include "screen.h"
#include "logging.h"

#include <mir_toolkit/mir_client_library.h>

// Qt
#include <qpa/qwindowsysteminterface.h>
#include <QMutexLocker>
#include <QSize>

// Platform API
#include <ubuntu/application/instance.h>

#include <EGL/egl.h>

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

EGLNativeWindowType nativeWindowFor(MirSurface *surf)
{
    auto stream = mir_surface_get_buffer_stream(surf);
    return reinterpret_cast<EGLNativeWindowType>(mir_buffer_stream_get_egl_native_window(stream));
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
        LOG("Unexpected Qt::WindowState: %d", state);
        return mir_surface_state_restored;
    }
}

#if !defined(QT_NO_DEBUG)
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
    default:
        return "!?";
    }
}
#endif

WId makeId()
{
    static int id = 1;
    return id++;
}

MirPixelFormat defaultPixelFormatFor(MirConnection *connection)
{
    MirPixelFormat format;
    unsigned int nformats;
    mir_connection_get_available_surface_formats(connection, &format, 1, &nformats);
    return format;
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

std::unique_ptr<MirSurfaceSpec, MirSpecDeleter> makeSurfaceSpec(QWindow *window, UbuntuInput *input, MirConnection *connection)
{
   using up = std::unique_ptr<MirSurfaceSpec, MirSpecDeleter>;

   const auto geom = window->geometry();
   const int width = geom.width();
   const int height = geom.height();
   const auto pixelFormat = defaultPixelFormatFor(connection);

   if (U_ON_SCREEN_KEYBOARD_ROLE == roleFor(window)) {
       DLOG("[ubuntumirclient QPA] makeSurfaceSpec(window=%p) - creating input method surface", window);
       return up{mir_connection_create_spec_for_input_method(connection, width, height, pixelFormat)};
   }

   const Qt::WindowType type = window->type();
   if (type == Qt::Popup) {
       auto parent = transientParentFor(window);
       if (parent == nullptr) {
           //NOTE: We cannot have a parentless popup -
           //try using the last surface to receive input as that will most likely be
           //the one that made caused this popup to be created
           parent = input->lastWindow();
       }
       if (parent) {
           auto pos = geom.topLeft();
           pos -= parent->geometry().topLeft();
           MirRectangle location{pos.x(), pos.y(), 0, 0};
           DLOG("[ubuntumirclient QPA] makeSurfaceSpec(window=%p) - creating menu surface", window);
           return up{mir_connection_create_spec_for_menu(
                       connection, width, height, pixelFormat, parent->mirSurface(),
                       &location, mir_edge_attachment_any)};
       } else {
           DLOG("[ubuntumirclient QPA] makeSurfaceSpec(window=%p) - cannot create a menu without a parent!", window);
       }
   } else if (type == Qt::Dialog) {
       auto parent = transientParentFor(window);
       if (parent) {
           // Modal dialog
           DLOG("[ubuntumirclient QPA] makeSurfaceSpec(window=%p) - creating modal dialog", window);
           return up{mir_connection_create_spec_for_modal_dialog(connection, width, height, pixelFormat, parent->mirSurface())};
       } else {
           // TODO: do Qt parentless dialogs have the same semantics as mir?
           DLOG("[ubuntumirclient QPA] makeSurfaceSpec(window=%p) - creating parentless dialog", window);
           return up{mir_connection_create_spec_for_dialog(connection, width, height, pixelFormat)};
       }
   }
   DLOG("[ubuntumirclient QPA] makeSurfaceSpec(window=%p) - creating normal surface(type=0x%x)", window, type);
   return up{mir_connection_create_spec_for_normal_surface(connection, width, height, pixelFormat)};
}

MirSurface *createMirSurface(QWindow *window, UbuntuScreen *screen, UbuntuInput *input, MirConnection *connection)
{
    Q_UNUSED(screen);
    auto spec = makeSurfaceSpec(window, input, connection);
    const auto title = window->title().toUtf8();
    mir_surface_spec_set_name(spec.get(), title.constData());

    if (window->windowState() == Qt::WindowFullScreen) {
        //FIXME: What does it mean to be fullscreen when there are multiple screens?
        auto displayConfig = mir_connection_create_display_config(connection);
        if (displayConfig->num_outputs > 0) {
            auto outputId = displayConfig->outputs[0].output_id;
            mir_surface_spec_set_fullscreen_on_output(spec.get(), outputId);
        }
    }

    auto surface = mir_surface_create_sync(spec.get());
    Q_ASSERT(mir_surface_is_valid(surface));
    return surface;
}

} //namespace

class UbuntuSurface
{
public:
    UbuntuSurface(UbuntuWindow *platformWindow, UbuntuScreen *screen, UbuntuInput *input, MirConnection *connection)
        : mWindow(platformWindow->window())
        , mPlatformWindow(platformWindow)
        , mScreen(screen)
        , mInput(input)
        , mConnection(connection)
        , mMirSurface(createMirSurface(mWindow, screen, input, connection))
        , mEglDisplay(screen->eglDisplay())
        , mEglSurface(eglCreateWindowSurface(mEglDisplay, screen->eglConfig(), nativeWindowFor(mMirSurface), nullptr))
        , mVisible(false)
        , mWindowState(mWindow->windowState())
        , mResizeCatchUpAttempts(0)

    {
        mir_surface_set_event_handler(mMirSurface, surfaceEventCallback, this);

        // Window manager can give us a final size different from what we asked for
        // so let's check what we ended up getting
        MirSurfaceParameters parameters;
        mir_surface_get_parameters(mMirSurface, &parameters);

        auto geom = mWindow->geometry();
        geom.setWidth(parameters.width);
        geom.setHeight(parameters.height);

        // Assume that the buffer size matches the surface size at creation time
        mBufferSize = geom.size();
        QWindowSystemInterface::handleGeometryChange(mWindow, geom);

#if !defined(QT_NO_DEBUG)
        const auto title = mWindow->title().toUtf8();
#endif
        DLOG("[ubuntumirclient QPA] created surface at (%d, %d) with size (%d, %d), title '%s', role: '%d'\n",
             geom.x(), geom.y(), geom.width(), geom.height(), title.constData(), roleFor(mWindow));
    }

    ~UbuntuSurface()
    {
        if (mEglSurface != EGL_NO_SURFACE)
            eglDestroySurface(mEglDisplay, mEglSurface);
        if (mMirSurface)
            mir_surface_release_sync(mMirSurface);
    }

    void resize(const QSize& newSize);
    void setState(Qt::WindowState newState);
    void setVisible(bool state);

    bool hasBufferSize(int width, int height) const;
    void expectBufferSizeChange();
    bool isWaitingForBufferSizeChange();
    void updateBufferSize(int width, int height);
    const QSize& bufferSize() const { return mBufferSize; }

    EGLSurface eglSurface() const { return mEglSurface; }
    MirSurface *mirSurface() const { return mMirSurface; }

private:
    static void surfaceEventCallback(MirSurface* surface, const MirEvent *event, void* context);
    void postEvent(const MirEvent *event);

    QWindow * const mWindow;
    UbuntuWindow * const mPlatformWindow;
    UbuntuScreen * const mScreen;
    UbuntuInput * const mInput;
    MirConnection * const mConnection;

    MirSurface * const mMirSurface;
    const EGLDisplay mEglDisplay;
    const EGLSurface mEglSurface;

    bool mVisible;
    Qt::WindowState mWindowState;
    QSize mBufferSize;
    int mResizeCatchUpAttempts;
};

void UbuntuSurface::resize(const QSize& size)
{
    DLOG("[ubuntumirclient QPA] resize(window=%p, width=%d, height=%d)", mWindow, size.width(), size.height());

    if (mWindowState == Qt::WindowFullScreen || mWindowState == Qt::WindowMaximized) {
        DLOG("[ubuntumirclient QPA] resize(window=%p) - not resizing, window is maximized or fullscreen", mWindow);
        return;
    }

    using up = std::unique_ptr<MirSurfaceSpec, MirSpecDeleter>;
    auto spec = up{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_width(spec.get(), size.width());
    mir_surface_spec_set_height(spec.get(), size.height());
    mir_surface_apply_spec(mirSurface(), spec.get());

    if (mVisible) {
        QWindowSystemInterface::handleExposeEvent(mWindow, QRect(QPoint(0, 0), size));
    }
}

void UbuntuSurface::setState(Qt::WindowState newState)
{
    mir_wait_for(mir_surface_set_state(mMirSurface, qtWindowStateToMirSurfaceState(newState)));
    mWindowState = newState;
}

void UbuntuSurface::setVisible(bool visible)
{
    if (mVisible == visible)
        return;

    mVisible = visible;

    // TODO: Use the new mir_surface_state_hidden state instead of mir_surface_state_minimized.
    //       Will have to change qtmir and unity8 for that.
    const auto newState = visible ? qtWindowStateToMirSurfaceState(mWindowState) : mir_surface_state_minimized;
    mir_wait_for(mir_surface_set_state(mMirSurface, newState));
}

bool UbuntuSurface::hasBufferSize(int width, int height) const
{
    return mBufferSize.width() == width && mBufferSize.height() == height;
}

void UbuntuSurface::expectBufferSizeChange()
{
    // If the next buffer doesn't have a different size, try some more
    // FIXME: This is working around a mir bug! We really shound't have to
    // swap more than once to get a buffer with the new size!
    mResizeCatchUpAttempts = 2;
}

bool UbuntuSurface::isWaitingForBufferSizeChange()
{
    const bool stillWaiting = mResizeCatchUpAttempts > 0;
    if (stillWaiting) {
        --mResizeCatchUpAttempts;
    }
    return stillWaiting;
}
void UbuntuSurface::updateBufferSize(int width, int height)
{
    mResizeCatchUpAttempts = 0;
    mBufferSize.setWidth(width);
    mBufferSize.setHeight(height);
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
    mInput->postEvent(mPlatformWindow, event);
}

UbuntuWindow::UbuntuWindow(QWindow *w, QSharedPointer<UbuntuClipboard> clipboard, UbuntuScreen *screen,
                           UbuntuInput *input, MirConnection *connection)
    : QObject(nullptr)
    , QPlatformWindow(w)
    , mId(makeId())
    , mClipboard(clipboard)
    , mSurface(new UbuntuSurface{this, screen, input, connection})
{
    DLOG("[ubuntumirclient QPA] UbuntuWindow(window=%p, screen=%p, input=%p, surf=%p)", w, screen, input, mSurface.get());
}

UbuntuWindow::~UbuntuWindow()
{
    DLOG("[ubuntumirclient QPA] ~UbuntuWindow(window=%p)", this);
}

void UbuntuWindow::handleSurfaceResized(int width, int height)
{
    QMutexLocker lock(&mMutex);
    DLOG("[ubuntumirclient QPA] handleSurfaceResize(window=%p, width=%d, height=%d)", window(), width, height);

    // The current buffer size hasn't actually changed. so just render on it and swap
    // buffers in the hope that the next buffer will match the surface size advertised
    // in this event.
    // But since this event is processed by a thread different from the one that swaps
    // buffers, you can never know if this information is already outdated as there's
    // no synchronicity whatsoever between the processing of resize events and the
    // consumption of buffers.
    if (!mSurface->hasBufferSize(width, height)) {
        mSurface->expectBufferSizeChange();
        QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(0, 0), geometry().size()));

        //NOTE: Don't flush with lock held or deadlocks will happen
        lock.unlock();
        QWindowSystemInterface::flushWindowSystemEvents();
    }
}

void UbuntuWindow::handleSurfaceFocusChange(bool focused)
{
    DLOG("[ubuntumirclient QPA] handleSurfaceFocusChange(window=%p, focused=%s)", window(), focused ? "true" : "false");
    if (!focused) {
        // Mir will send a pair of events when a new surface is focused, one for the surface
        // that was unfocused and one for the surface what just gained focus. There is no
        // equivalent Qt API to "unfocus" a single window only handleWindowActivated(NULL, ...)
        // which has different semantics (all windows lose focus) which is problematic for popups.
        // Hence unfocused events are ignored.
        return;
    }

    // System clipboard contents might have changed while this window was unfocused and without
    // this process getting notified about it because it might have been suspended (due to
    // application lifecycle policies), thus unable to listen to any changes notified through
    // D-Bus.
    // Therefore let's ensure we are up to date with the system clipboard now that we are getting
    // focused again.
    mClipboard->requestDBusClipboardContents();
    QWindowSystemInterface::handleWindowActivated(window(), Qt::ActiveWindowFocusReason);
}

void UbuntuWindow::setWindowState(Qt::WindowState state)
{
    QMutexLocker lock(&mMutex);
    DLOG("[ubuntumirclient QPA] setWindowState(window=%p, %s)", this, qtWindowStateToStr(state));
    mSurface->setState(state);
}

void UbuntuWindow::setGeometry(const QRect& rect)
{
    QMutexLocker lock(&mMutex);
    DLOG("[ubuntumirclient QPA] setGeometry (window=%p, x=%d, y=%d, width=%d, height=%d)",
           window(), rect.x(), rect.y(), rect.width(), rect.height());

    //NOTE: topLeft are just used as hints to the window manager during surface creation
    // they cannot be changed by the client after surface is shown
    auto newSize = rect.size();
    auto newGeometry = geometry();
    newGeometry.setSize(newSize);
    QPlatformWindow::setGeometry(newGeometry);

    mSurface->resize(newSize);
}

void UbuntuWindow::setVisible(bool visible)
{
    QMutexLocker lock(&mMutex);
    DLOG("[ubuntumirclient QPA] setVisible (window=%p, visible=%s)", window(), visible ? "true" : "false");

    mSurface->setVisible(visible);
    const QRect& exposeRect = visible ? QRect(QPoint(0, 0), geometry().size()) : QRect();
    QWindowSystemInterface::handleExposeEvent(window(), exposeRect);

    //NOTE: Don't flush with lock held or deadlocks will happen
    lock.unlock();
    QWindowSystemInterface::flushWindowSystemEvents();
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

void UbuntuWindow::onSwapBuffers(int newBufferWidth, int newBufferHeight)
{
    QMutexLocker lock(&mMutex);

    const bool sizeKnown = newBufferWidth > 0 && newBufferHeight > 0;
#if !defined(QT_NO_DEBUG)
    static int frameNumber = 0;
    ++frameNumber;
#endif
    if (sizeKnown && !mSurface->hasBufferSize(newBufferWidth, newBufferHeight)) {
        DLOG("[ubuntumirclient QPA] updateBufferSize(window=%p) [%d] - buffer size changed from (%d,%d) to (%d,%d)",
               window(), frameNumber, mSurface->bufferSize().width(), mSurface->bufferSize().height(), newBufferWidth, newBufferHeight);
        mSurface->updateBufferSize(newBufferWidth, newBufferHeight);

        QRect newGeometry = geometry();
        newGeometry.setSize(mSurface->bufferSize());

        QPlatformWindow::setGeometry(newGeometry);
        QWindowSystemInterface::handleGeometryChange(window(), newGeometry);
    } else if (mSurface->isWaitingForBufferSizeChange()) {
        DLOG("[ubuntumirclient QPA] onSwapBuffers(window=%p) [%d] - buffer size (%d,%d). Redrawing to catch up a resized buffer.",
               window(), frameNumber, mSurface->bufferSize().width(), mSurface->bufferSize().height());
        QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(0, 0), geometry().size()));
    } else {
        DLOG("[ubuntumirclient QPA] onSwapBuffers(window=%p) [%d] - buffer size (%d,%d)",
               window(), frameNumber, mSurface->bufferSize().width(), mSurface->bufferSize().height());
    }
}
