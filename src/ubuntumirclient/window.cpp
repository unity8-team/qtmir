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
#include "utils.h"
#include "logging.h"

#include <mir_toolkit/mir_client_library.h>

// Qt
#include <qpa/qwindowsysteminterface.h>
#include <QMutexLocker>
#include <QSize>
#include <QtMath>

// Platform API
#include <ubuntu/application/instance.h>

#include <EGL/egl.h>


/*
 * Note: all geometry is in device-independent pixels, except that contained in variables with the
 * suffix "Px" - whose units are (physical) pixels
 */
Q_LOGGING_CATEGORY(ubuntumirclientBufferSwap, "ubuntumirclient.bufferSwap", QtWarningMsg)

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

Spec makeSurfaceSpec(QWindow *window, UbuntuInput *input, MirConnection *connection)
{
    const auto geom = window->geometry();
    const int dpr = int(window->devicePixelRatio());
    const int widthPx = geom.width() > 0 ? geom.width() * dpr : 1;
    const int heightPx = geom.height() > 0 ? geom.height() * dpr : 1;
    const auto pixelFormat = defaultPixelFormatFor(connection);

    if (U_ON_SCREEN_KEYBOARD_ROLE == roleFor(window)) {
        qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating input method surface with size=(%dx%d)px", window, widthPx, heightPx);
        return Spec{mir_connection_create_spec_for_input_method(connection, widthPx, heightPx, pixelFormat)};
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
            auto posPx = geom.topLeft() * dpr;
            posPx -= parent->geometry().topLeft() * dpr;
            MirRectangle location{posPx.x(), posPx.y(), 0, 0};
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating menu surface with size=(%dx%d)px", window, widthPx, heightPx);
            return Spec{mir_connection_create_spec_for_menu(
                        connection, widthPx, heightPx, pixelFormat, parent->mirSurface(),
                        &location, mir_edge_attachment_any)};
        } else {
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - cannot create a menu without a parent!", window);
        }
    } else if (type == Qt::Dialog) {
        auto parent = transientParentFor(window);
        if (parent) {
            // Modal dialog
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating modal dialog with size=(%dx%d)px", window, widthPx, heightPx);
            return Spec{mir_connection_create_spec_for_modal_dialog(connection, widthPx, heightPx, pixelFormat, parent->mirSurface())};
        } else {
            // TODO: do Qt parentless dialogs have the same semantics as mir?
            qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating parentless dialog with size=(%dx%d)px", window, widthPx, heightPx);
            return Spec{mir_connection_create_spec_for_dialog(connection, widthPx, heightPx, pixelFormat)};
        }
    }
    qCDebug(ubuntumirclient, "makeSurfaceSpec(window=%p) - creating normal surface(type=0x%x, with size=(%dx%d)px", window, type, widthPx, heightPx);
    return Spec{mir_connection_create_spec_for_normal_surface(connection, widthPx, heightPx, pixelFormat)};
}

void setSizingConstraints(MirSurfaceSpec *spec, const QSize &minSizePx, const QSize &maxSizePx, const QSize &incrementPx)
{
    mir_surface_spec_set_min_width(spec, minSizePx.width());
    mir_surface_spec_set_min_height(spec, minSizePx.height());
    if (maxSizePx.width() >= minSizePx.width()) {
        mir_surface_spec_set_max_width(spec, maxSizePx.width());
    }
    if (maxSizePx.height() >= minSizePx.height()) {
        mir_surface_spec_set_max_height(spec, maxSizePx.height());
    }
    if (incrementPx.width() > 0) {
        mir_surface_spec_set_width_increment(spec, incrementPx.width());
    }
    if (incrementPx.height() > 0) {
        mir_surface_spec_set_height_increment(spec, incrementPx.height());
    }
}

MirSurface *createMirSurface(QWindow *window, UbuntuScreen *screen, UbuntuInput *input, MirConnection *connection)
{
    auto spec = makeSurfaceSpec(window, input, connection);
    const auto title = window->title().toUtf8();
    mir_surface_spec_set_name(spec.get(), title.constData());

    setSizingConstraints(spec.get(), window->minimumSize(), window->maximumSize(), window->sizeIncrement());

    if (window->windowState() == Qt::WindowFullScreen) {
        mir_surface_spec_set_fullscreen_on_output(spec.get(), screen->mirOutputId());
    }

    auto surface = mir_surface_create_sync(spec.get());
    Q_ASSERT(mir_surface_is_valid(surface));
    return surface;
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

/*
 * UbuntuSurface - wraps a MirSurface
 * All units are in pixels only (no device pixels).
 */
class UbuntuSurface
{
public:
    UbuntuSurface(UbuntuWindow *platformWindow, UbuntuScreen *screen, UbuntuInput *input, MirConnection *connection)
        : mWindow(platformWindow->window())
        , mPlatformWindow(platformWindow)
        , mInput(input)
        , mConnection(connection)
        , mMirSurface(createMirSurface(mWindow, screen, input, connection))
        , mEglDisplay(screen->eglDisplay())
        , mEglSurface(eglCreateWindowSurface(mEglDisplay, screen->eglConfig(), nativeWindowFor(mMirSurface), nullptr))
        , mVisible(false)
        , mNeedsRepaint(false)
        , mParented(mWindow->transientParent() || mWindow->parent())
        , mWindowState(mWindow->windowState())
    {
        mir_surface_set_event_handler(mMirSurface, surfaceEventCallback, this);

        // Window manager can give us a final size different from what we asked for
        // so let's check what we ended up getting
        MirSurfaceParameters parameters;
        mir_surface_get_parameters(mMirSurface, &parameters);

        // Assume that the buffer size matches the surface size at creation time
        mBufferSizePx.rwidth() = parameters.width;
        mBufferSizePx.rheight() = parameters.height;

        qCDebug(ubuntumirclient, "created surface with size=(%dx%d)px, title='%s', role=%d",
                parameters.width, parameters.height, qPrintable(mWindow->title()), roleFor(mWindow));
        mPlatformWindow->updateWindowSize(parameters.width, parameters.height);
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

    void resize(const QSize &newSizePx);
    void setState(Qt::WindowState newState);
    void setVisible(bool state);
    void updateTitle(const QString &title);
    void setSizingConstraints(const QSize &minSizePx, const QSize &maxSizePx, const QSize &incrementPx);

    void onSwapBuffersDone();
    void handleSurfaceResized(int widthPx, int heightPx);
    int needsRepaint() const;

    EGLSurface eglSurface() const { return mEglSurface; }
    MirSurface *mirSurface() const { return mMirSurface; }

private:
    static void surfaceEventCallback(MirSurface *surface, const MirEvent *event, void *context);
    void postEvent(const MirEvent *event);
    void updateSurface();

    QWindow * const mWindow;
    UbuntuWindow * const mPlatformWindow;
    UbuntuInput * const mInput;
    MirConnection * const mConnection;

    MirSurface * const mMirSurface;
    const EGLDisplay mEglDisplay;
    const EGLSurface mEglSurface;

    bool mVisible;
    bool mNeedsRepaint;
    bool mParented;
    Qt::WindowState mWindowState;
    QSize mBufferSizePx;

    QMutex mTargetSizeMutex;
    QSize mTargetSizePx;
};

void UbuntuSurface::resize(const QSize &sizePx)
{
    qCDebug(ubuntumirclient,"resize(window=%p) to (%dx%d)px", mWindow, sizePx.width(), sizePx.height());

    if (mWindowState == Qt::WindowFullScreen || mWindowState == Qt::WindowMaximized) {
        qCDebug(ubuntumirclient, "resize(window=%p) - not resizing, window is maximized or fullscreen", mWindow);
        return;
    }

    if (sizePx.isEmpty()) {
        qCDebug(ubuntumirclient, "resize(window=%p) - not resizing, size is empty", mWindow);
        return;
    }

    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_width(spec.get(), sizePx.width());
    mir_surface_spec_set_height(spec.get(), sizePx.height());
    mir_surface_apply_spec(mMirSurface, spec.get());
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

    if (mVisible)
        updateSurface();

    // TODO: Use the new mir_surface_state_hidden state instead of mir_surface_state_minimized.
    //       Will have to change qtmir and unity8 for that.
    const auto newState = visible ? qtWindowStateToMirSurfaceState(mWindowState) : mir_surface_state_minimized;
    mir_wait_for(mir_surface_set_state(mMirSurface, newState));
}

void UbuntuSurface::updateTitle(const QString &newTitle)
{
    const auto title = newTitle.toUtf8();
    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_name(spec.get(), title.constData());
    mir_surface_apply_spec(mMirSurface, spec.get());
}

void UbuntuSurface::setSizingConstraints(const QSize &minSizePx, const QSize &maxSizePx, const QSize &incrementPx)
{
    Spec spec{mir_connection_create_spec_for_changes(mConnection)};
    ::setSizingConstraints(spec.get(), minSizePx, maxSizePx, incrementPx);
    mir_surface_apply_spec(mMirSurface, spec.get());
}

void UbuntuSurface::handleSurfaceResized(int widthPx, int heightPx)
{
    QMutexLocker lock(&mTargetSizeMutex);

    // mir's resize event is mainly a signal that we need to redraw our content. We use the
    // width/height as identifiers to figure out if this is the latest surface resize event
    // that has posted, discarding any old ones. This avoids issuing too many redraw events.
    // see TODO in postEvent as the ideal way we should handle this.
    // The actual buffer size may or may have not changed at this point, so let the rendering
    // thread drive the window geometry updates.
    mNeedsRepaint = mTargetSizePx.width() == widthPx && mTargetSizePx.height() == heightPx;
}

int UbuntuSurface::needsRepaint() const
{
    if (mNeedsRepaint) {
        if (mTargetSizePx != mBufferSizePx) {
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

void UbuntuSurface::onSwapBuffersDone()
{
    static int sFrameNumber = 0;
    ++sFrameNumber;

    EGLint eglSurfaceWidthPx = -1;
    EGLint eglSurfaceHeightPx = -1;
    eglQuerySurface(mEglDisplay, mEglSurface, EGL_WIDTH, &eglSurfaceWidthPx);
    eglQuerySurface(mEglDisplay, mEglSurface, EGL_HEIGHT, &eglSurfaceHeightPx);

    const bool validSize = eglSurfaceWidthPx > 0 && eglSurfaceHeightPx > 0;

    if (validSize && (mBufferSizePx.width() != eglSurfaceWidthPx || mBufferSizePx.height() != eglSurfaceHeightPx)) {

        qCDebug(ubuntumirclientBufferSwap, "onSwapBuffersDone(window=%p) [%d] - buffer size changed (%dx%d)px => (%dx%d)px",
                mWindow, sFrameNumber, mBufferSizePx.width(), mBufferSizePx.height(), eglSurfaceWidthPx, eglSurfaceHeightPx);

        mBufferSizePx.rwidth() = eglSurfaceWidthPx;
        mBufferSizePx.rheight() = eglSurfaceHeightPx;

        mPlatformWindow->updateWindowSize(eglSurfaceWidthPx, eglSurfaceHeightPx);
    } else {
        qCDebug(ubuntumirclientBufferSwap, "onSwapBuffersDone(window=%p) [%d] - buffer size=(%dx%d)px",
                mWindow, sFrameNumber, mBufferSizePx.width(), mBufferSizePx.height());
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
        const auto widthPx = mir_resize_event_get_width(resizeEvent);
        const auto heightPx = mir_resize_event_get_height(resizeEvent);
        qCDebug(ubuntumirclient, "resizeEvent(window=%p, size=(%dx%d)px", mWindow, widthPx, heightPx);

        QMutexLocker lock(&mTargetSizeMutex);
        mTargetSizePx.rwidth() = widthPx;
        mTargetSizePx.rheight() = heightPx;
    }

    mInput->postEvent(mPlatformWindow, event);
}

void UbuntuSurface::updateSurface()
{
    qCDebug(ubuntumirclient, "updateSurface(window=%p)", mWindow);

    if (!mParented && mWindow->type() == Qt::Dialog) {
        // The dialog may have been parented after creation time
        // so morph it into a modal dialog
        auto parent = transientParentFor(mWindow);
        if (parent) {
            qCDebug(ubuntumirclient, "updateSurface(window=%p) dialog now parented", mWindow);
            mParented = true;
            Spec spec{mir_connection_create_spec_for_changes(mConnection)};
            mir_surface_spec_set_parent(spec.get(), parent->mirSurface());
            mir_surface_apply_spec(mMirSurface, spec.get());
        }
    }
}

UbuntuWindow::UbuntuWindow(QWindow *w, const QSharedPointer<UbuntuClipboard> &clipboard, UbuntuScreen *screen,
                           UbuntuInput *input, MirConnection *connection)
    : QObject(nullptr)
    , QPlatformWindow(w)
    , mId(makeId())
    , mClipboard(clipboard)
    , mSurface(new UbuntuSurface{this, screen, input, connection})
{
    qCDebug(ubuntumirclient, "UbuntuWindow(window=%p, screen=%p, input=%p, surf=%p) with title '%s', role: '%d'",
            w, screen, input, mSurface.get(), qPrintable(window()->title()), roleFor(window()));

    enablePanelHeightHack(w->windowState() != Qt::WindowFullScreen);
}

UbuntuWindow::~UbuntuWindow()
{
    qCDebug(ubuntumirclient, "~UbuntuWindow(window=%p)", this);
}

void UbuntuWindow::updateWindowSize(int widthPx, int heightPx) // after when Mir has resized the surface
{
    const float dpr = devicePixelRatio();
    auto geom = geometry();
    geom.setWidth(divideAndRoundUp(widthPx, dpr));
    geom.setHeight(divideAndRoundUp(heightPx, dpr));

    QPlatformWindow::setGeometry(geom);
    QWindowSystemInterface::handleGeometryChange(window(), geom);

    qCDebug(ubuntumirclient) << "Surface geometry updated:" << geom;
}

void UbuntuWindow::handleSurfaceResized(int widthPx, int heightPx)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "handleSurfaceResize(window=%p, size=(%dx%d)px", window(), widthPx, heightPx);

    mSurface->handleSurfaceResized(widthPx, heightPx);

    // This resize event could have occurred just after the last buffer swap for this window.
    // This means the client may still be holding a buffer with the older size. The first redraw call
    // will then render at the old size. After swapping the client now will get a new buffer with the
    // updated size but it still needs re-rendering so another redraw may be needed.
    // A mir API to drop the currently held buffer would help here, so that we wouldn't have to redraw twice
    auto const numRepaints = mSurface->needsRepaint();
    qCDebug(ubuntumirclient, "handleSurfaceResize(window=%p) redraw %d times", window(), numRepaints);
    for (int i = 0; i < numRepaints; i++) {
        qCDebug(ubuntumirclient, "handleSurfaceResize(window=%p) repainting size=(%dx%d)dp", window(), geometry().size().width(), geometry().size().height());
        QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), geometry().size()));
    }
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

void UbuntuWindow::setWindowState(Qt::WindowState state)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setWindowState(window=%p, %s)", this, qtWindowStateToStr(state));
    mSurface->setState(state);

    enablePanelHeightHack(state != Qt::WindowFullScreen);
}

/*
    FIXME: Mir does not let clients know the position of their windows in the virtual
    desktop space. So we have this ugly hack that assumes a phone situation where the
    window is always on the top-left corner, right below the indicators panel if not
    in fullscreen.
 */
void UbuntuWindow::enablePanelHeightHack(bool enable)
{
    QRect newGeometry = geometry();
    if (enable) {
        newGeometry.setY(panelHeight());
    } else {
        newGeometry.setY(0);
    }

    if (newGeometry != geometry()) {
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
    auto newGeometry = geometry();
    newGeometry.setSize(newSize);

    mSurface->resize(newSize * devicePixelRatio());
    // Note: don't call handleGeometryChange here, wait to see what Mir replies with.
}

void UbuntuWindow::setVisible(bool visible)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setVisible (window=%p, visible=%s)", window(), visible ? "true" : "false");

    mSurface->setVisible(visible);
    const QRect& exposeRect = visible ? QRect(QPoint(), geometry().size()) : QRect();

    lock.unlock();
    QWindowSystemInterface::handleExposeEvent(window(), exposeRect);
    QWindowSystemInterface::flushWindowSystemEvents();
}

void UbuntuWindow::setWindowTitle(const QString &title)
{
    QMutexLocker lock(&mMutex);
    qCDebug(ubuntumirclient, "setWindowTitle(window=%p) title=%s)", window(), title.toUtf8().constData());
    mSurface->updateTitle(title);
}

void UbuntuWindow::propagateSizeHints()
{
    QMutexLocker lock(&mMutex);
    const auto win = window();
    const float dpr = devicePixelRatio();
    qCDebug(ubuntumirclient, "propagateSizeHints(window=%p) min(%dx%d)dp; max(%dx%d)dp; increment(%dx%d)dp",
            win, win->minimumSize().width(), win->minimumSize().height(),
            win->maximumSize().width(), win->maximumSize().height(),
            win->sizeIncrement().width(), win->sizeIncrement().height());
    mSurface->setSizingConstraints(win->minimumSize() * dpr, win->maximumSize() * dpr, win->sizeIncrement() * dpr);
}

qreal UbuntuWindow::devicePixelRatio() const
{
    return screen() ? screen()->devicePixelRatio() : 1.0; // not impossible a Window has no attached Screen
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
