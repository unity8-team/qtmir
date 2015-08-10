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
#include <qpa/qwindowsysteminterface.h>
#include <QMutex>
#include <QMutexLocker>
#include <QSize>
#include <QtMath>

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

class Surface
{
public:
    Surface(MirSurface *surface, UbuntuScreen *screen)
    :  mMirSurface{surface},
       mEglDisplay{screen->eglDisplay()},
       mEglConfig{screen->eglConfig()},
       mEglSurface{EGL_NO_SURFACE}
    {
    }

    ~Surface()
    {
        if (mEglSurface != EGL_NO_SURFACE)
            eglDestroySurface(mEglDisplay, mEglSurface);
        if (mMirSurface)
            mir_surface_release_sync(mMirSurface);
    }

    EGLSurface eglSurface()
    {
        if (mEglSurface == EGL_NO_SURFACE)
        {
            mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, nativeWindowFor(mMirSurface), nullptr);
        }
        return mEglSurface;
    }

    MirSurface *mirSurface() const { return mMirSurface; }

private:
    MirSurface * const mMirSurface;
    const EGLDisplay mEglDisplay;
    const EGLConfig mEglConfig;
    EGLSurface mEglSurface;
};

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

WId generateWindowID()
{
    static int id = 1;
    return ++id;
}

MirPixelFormat defaultPixelFormat(MirConnection *connection)
{
    MirPixelFormat format;
    unsigned int nformats;
    mir_connection_get_available_surface_formats(connection, &format, 1, &nformats);
    return format;
}
} // namespace

struct UbuntuWindowPrivate
{
    UbuntuWindowPrivate(UbuntuWindow *w, UbuntuScreen *screen, UbuntuInput *input, Qt::WindowState state,
        MirConnection *connection, const QSharedPointer<UbuntuClipboard>& clipboard)
        : mWindow{w}, mScreen{screen}, mInput{input}, mConnection{connection}, mPixelFormat{defaultPixelFormat(connection)},
          mClipboard{clipboard}, mId{generateWindowID()}, mState{state}
    {}

    UAUiWindowRole role();
    UbuntuWindow *transientParent();
    MirSurface *parentSurface();

    void createWindow();
    std::unique_ptr<MirSurfaceSpec, MirSpecDeleter> createSpec(const QRect& rect);
    void createSurface(QRect& geom, const char *name);

    void resize(const QRect& rect);
    void setSurfaceState(Qt::WindowState state);

    EGLSurface eglSurface()
    {
        // Sometimes an egl surface is requested without making the window visible first
        createWindow();
        return mSurface->eglSurface();
    }
    MirSurface *mirSurface() const { return mSurface->mirSurface(); }

    UbuntuWindow * const mWindow;
    UbuntuScreen * const mScreen;
    UbuntuInput * const mInput;
    MirConnection * const mConnection;
    const MirPixelFormat mPixelFormat;
    const QSharedPointer<UbuntuClipboard> mClipboard;
    const WId mId;
    Qt::WindowState mState;
    std::unique_ptr<Surface> mSurface;
    QSize mBufferSize;
    QMutex mMutex;
};

namespace
{
void eventCallback(MirSurface* surface, const MirEvent *event, void* context)
{
    (void) surface;
    DASSERT(context != NULL);
    UbuntuWindowPrivate *priv = static_cast<UbuntuWindowPrivate*>(context);
    priv->mInput->postEvent(priv->mWindow, event);
}

void surfaceCreateCallback(MirSurface* surface, void* context)
{
    DASSERT(context != NULL);
    UbuntuWindowPrivate *priv = static_cast<UbuntuWindowPrivate*>(context);
    priv->mSurface = std::move(std::unique_ptr<Surface>(new Surface(surface, priv->mScreen)));

    mir_surface_set_event_handler(surface, eventCallback, context);
}

// FIXME - in order to work around https://bugs.launchpad.net/mir/+bug/1346633
// we need to guess the panel height (3GU + 2DP)
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
    qreal densityPixelRatio = static_cast<qreal>(gridUnit) / defaultGridUnit;
    return gridUnit * 3 + qFloor(densityPixelRatio) * 2;
}
}

UAUiWindowRole UbuntuWindowPrivate::role()
{
    QWindow *qWindow = mWindow->window();
    QVariant roleVariant = qWindow->property("role");

    if (!roleVariant.isValid())
        return U_MAIN_ROLE;

    uint role = roleVariant.toUInt();
    if (role < U_MAIN_ROLE || role > U_SHUTDOWN_DIALOG_ROLE)
        return U_MAIN_ROLE;

    return static_cast<UAUiWindowRole>(role);
}

UbuntuWindow *UbuntuWindowPrivate::transientParent()
{
    QWindow *qWindow = mWindow->window();
    QWindow *parent = qWindow->transientParent();
    return parent ? dynamic_cast<UbuntuWindow *>(parent->handle()) : nullptr;
}

MirSurface *UbuntuWindowPrivate::parentSurface()
{
    UbuntuWindow *parent = transientParent();

    // Sometimes children become visible before their parents - create the
    // parent window at this time if that's the case.
    if (parent->d->mSurface == nullptr) {
        DLOG("[ubuntumirclient QPA] (window=%p) creating parent window before it is visible!", mWindow);
        parent->d->createWindow();
    }

    return parent->d->mSurface->mirSurface();
}

std::unique_ptr<MirSurfaceSpec, MirSpecDeleter> UbuntuWindowPrivate::createSpec(const QRect& rect)
{
   using up = std::unique_ptr<MirSurfaceSpec, MirSpecDeleter>;

   const QWindow *qWindow = mWindow->window();
   const UAUiWindowRole windowRole = role();
   const int width = rect.width();
   const int height = rect.height();

   if (windowRole == U_ON_SCREEN_KEYBOARD_ROLE)
       return up{mir_connection_create_spec_for_input_method(mConnection, width, height, mPixelFormat)};

   Qt::WindowType type = qWindow->type();
   if (type == Qt::Popup) {
       auto parent = transientParent();
       if (parent) {
           auto pos = mWindow->geometry().topLeft();
           pos -= parent->geometry().topLeft();
           MirRectangle location{pos.x(), pos.y(), 0, 0};
           DLOG("[ubuntumirclient QPA] createSpec(window=%p) - creating menu surface", mWindow);
           return up{mir_connection_create_spec_for_menu(
               mConnection, width, height, mPixelFormat, parentSurface(),
               &location, mir_edge_attachment_any)};
       }
   } else if (type == Qt::Dialog) {
       auto parent = transientParent();
       if (parent) {
           // Modal dialog
           DLOG("[ubuntumirclient QPA] createSpec(window=%p) - creating modal dialog", mWindow);
           return up{mir_connection_create_spec_for_modal_dialog(
               mConnection, width, height, mPixelFormat, parentSurface())};
       } else {
           // TODO: do Qt parentless dialogs have the same semantics as mir?
           DLOG("[ubuntumirclient QPA] createSpec(window=%p) - creating parentless dialog", mWindow);
           return up{mir_connection_create_spec_for_dialog(mConnection, width, height, mPixelFormat)};
       }
   }
   DLOG("[ubuntumirclient QPA] createSpec(window=%p) - creating normal surface(type=0x%x)", mWindow, type);
   return up{mir_connection_create_spec_for_normal_surface(mConnection, width, height, mPixelFormat)};
}

void UbuntuWindowPrivate::createSurface(QRect& geom, const char *name)
{
    auto spec = createSpec(geom);
    mir_surface_spec_set_name(spec.get(), name);

    if (mState == Qt::WindowFullScreen) {
        //FIXME: How to handle multiple screens? For now just use the first display id
        auto displayConfig = mir_connection_create_display_config(mConnection);
        if (displayConfig->num_outputs > 0) {
            auto outputId = displayConfig->outputs[0].output_id;
            mir_surface_spec_set_fullscreen_on_output(spec.get(), outputId);
        }
    }
    mir_wait_for(mir_surface_create(spec.get(), surfaceCreateCallback, this));

    // Window manager can give us a final size different from what we asked for
    // so let's check what we ended up getting
    MirSurfaceParameters parameters;
    mir_surface_get_parameters(mSurface->mirSurface(), &parameters);

    geom.setWidth(parameters.width);
    geom.setHeight(parameters.height);
}

void UbuntuWindowPrivate::createWindow()
{
    // Already created
    if (mSurface != nullptr)
        return;

    DLOG("[ubuntumirclient QPA] createWindow(window=%p)", mWindow);

    QWindow *qWindow = mWindow->window();

    const QByteArray title = qWindow->title().isNull() ? "Window 1" : qWindow->title().toUtf8(); // legacy title
    const int panelHeight = ::panelHeight();

    DLOG("[ubuntumirclient QPA] panelHeight: '%d'", panelHeight);
    DLOG("[ubuntumirclient QPA] role: '%d'", role());
    DLOG("[ubuntumirclient QPA] title: '%s'", title.constData());

    // Get surface geometry.
    QRect geom;
    if (mState == Qt::WindowFullScreen) {
        DLOG("[ubuntumirclient QPA] fullscreen geometry chosen\n");
        geom = mScreen->geometry();
    } else if (mState == Qt::WindowMaximized) {
        DLOG("[ubuntumirclient QPA] maximized geometry chosen\n");
        geom = mScreen->availableGeometry();
        /*
         * FIXME: Autopilot relies on being able to convert coordinates relative of the window
         * into absolute screen coordinates. Mir does not allow this, see bug lp:1346633
         * Until there's a correct way to perform this transformation agreed, this horrible hack
         * guesses the transformation heuristically.
         *
         * Assumption: this method only used on phone devices!
         */
        geom.setY(panelHeight);
    } else {
        DLOG("[ubuntumirclient QPA] regular geometry chosen\n");
        geom = mWindow->geometry();
    }

    DLOG("[ubuntumirclient QPA] creating surface at (%d, %d) with size (%d, %d) with title '%s'\n",
            geom.x(), geom.y(), geom.width(), geom.height(), title.data());

    createSurface(geom, title.data());

    DLOG("[ubuntumirclient QPA] created surface has size (%d, %d)", geom.width(), geom.height());

    // Assume that the buffer size matches the surface size at creation time
    mBufferSize = geom.size();

    // Tell Qt about the geometry.
    QWindowSystemInterface::handleGeometryChange(qWindow, geom);
    mWindow->QPlatformWindow::setGeometry(geom);
}

void UbuntuWindowPrivate::resize(const QRect& rect)
{
    DLOG("[ubuntumirclient QPA] resize(window=%p, width=%d, height=%d)", mWindow, rect.width(), rect.height());

    if (mSurface == nullptr)
        return;

    using up = std::unique_ptr<MirSurfaceSpec, MirSpecDeleter>;
    auto spec = up{mir_connection_create_spec_for_changes(mConnection)};
    mir_surface_spec_set_width(spec.get(), rect.width());
    mir_surface_spec_set_height(spec.get(), rect.height());
    mir_surface_apply_spec(mirSurface(), spec.get());
}

void UbuntuWindowPrivate::setSurfaceState(Qt::WindowState new_state)
{
    mir_wait_for(mir_surface_set_state(mSurface->mirSurface(), qtWindowStateToMirSurfaceState(new_state)));
}

UbuntuWindow::UbuntuWindow(QWindow *w, QSharedPointer<UbuntuClipboard> clipboard, UbuntuScreen *screen,
                           UbuntuInput *input, MirConnection *connection)
    : QObject(nullptr), QPlatformWindow(w), d(new UbuntuWindowPrivate{
        this, screen, input, w->windowState(), connection, clipboard})
{
    DLOG("[ubuntumirclient QPA] UbuntuWindow(window=%p, w=%p, screen=%p, input=%p, priv=%p)", this, w, screen, input, d.get());
    QPlatformWindow::setGeometry(window()->geometry() != screen->geometry() ?
        window()->geometry() : screen->availableGeometry());
}

UbuntuWindow::~UbuntuWindow()
{
    DLOG("[ubuntumirclient QPA] ~UbuntuWindow(window=%p)", this);
}

void UbuntuWindow::handleSurfaceResize(int width, int height)
{
    QMutexLocker(&d->mMutex);
    DLOG("[ubuntumirclient QPA] handleSurfaceResize(window=%p, width=%d, height=%d)", this, width, height);

    // The current buffer size hasn't actually changed. so just render on it and swap
    // buffers in the hope that the next buffer will match the surface size advertised
    // in this event.
    // But since this event is processed by a thread different from the one that swaps
    // buffers, you can never know if this information is already outdated as there's
    // no synchronicity whatsoever between the processing of resize events and the
    // consumption of buffers.
    if (d->mBufferSize.width() != width || d->mBufferSize.height() != height) {
        QWindowSystemInterface::handleExposeEvent(window(), geometry());
        QWindowSystemInterface::flushWindowSystemEvents();
    }
}

void UbuntuWindow::handleSurfaceFocusChange(bool focused)
{
    DLOG("[ubuntumirclient QPA] handleSurfaceFocusChange(window=%p, focused=%s)", this, focused ? "true" : "false");
    QWindow *activatedWindow = focused ? window() : nullptr;

    // System clipboard contents might have changed while this window was unfocused and without
    // this process getting notified about it because it might have been suspended (due to
    // application lifecycle policies), thus unable to listen to any changes notified through
    // D-Bus.
    // Therefore let's ensure we are up to date with the system clipboard now that we are getting
    // focused again.
    if (focused) {
        d->mClipboard->requestDBusClipboardContents();
    }

    QWindowSystemInterface::handleWindowActivated(activatedWindow, Qt::ActiveWindowFocusReason);
}

void UbuntuWindow::setWindowState(Qt::WindowState state)
{
    QMutexLocker(&d->mMutex);
    DLOG("UbuntuWindow::setWindowState(window=%p, %s)", this, qtWindowStateToStr(state));

    if (state == d->mState)
        return;

    d->setSurfaceState(state);
    d->mState = state;
}

void UbuntuWindow::setGeometry(const QRect& rect)
{
    DLOG("[ubuntumirclient QPA] setGeometry (window=%p, x=%d, y=%d, width=%d, height=%d)", this, rect.x(), rect.y(), rect.width(), rect.height());

    bool resize;
    {
        QMutexLocker(&d->mMutex);
        resize = d->mState != Qt::WindowFullScreen && d->mState != Qt::WindowMaximized;
        QPlatformWindow::setGeometry(rect);
    }

    if (resize) {
        d->resize(rect);
    }
}

void UbuntuWindow::setVisible(bool visible)
{
    QMutexLocker(&d->mMutex);
    DLOG("[ubuntumirclient QPA] setVisible (window=%p, visible=%s)", this, visible ? "true" : "false");

    if (visible) {
        d->createWindow();
        d->setSurfaceState(d->mState);

        QWindowSystemInterface::handleExposeEvent(window(), QRect());
        QWindowSystemInterface::flushWindowSystemEvents();
    } else {
        // TODO: Use the new mir_surface_state_hidden state instead of mir_surface_state_minimized.
        //       Will have to change qtmir and unity8 for that.
        d->setSurfaceState(Qt::WindowMinimized);
    }
}

void* UbuntuWindow::eglSurface() const
{
    return d->eglSurface();
}

WId UbuntuWindow::winId() const
{
    return d->mId;
}

void UbuntuWindow::onBuffersSwapped_threadSafe(int newBufferWidth, int newBufferHeight)
{
    QMutexLocker(&d->mMutex);

    bool sizeKnown = newBufferWidth > 0 && newBufferHeight > 0;

    if (sizeKnown && (d->mBufferSize.width() != newBufferWidth ||
                d->mBufferSize.height() != newBufferHeight)) {

        DLOG("UbuntuWindow::onBuffersSwapped_threadSafe - buffer size changed from (%d,%d) to (%d,%d)",
                d->mBufferSize.width(), d->mBufferSize.height(), newBufferWidth, newBufferHeight);

        d->mBufferSize.rwidth() = newBufferWidth;
        d->mBufferSize.rheight() = newBufferHeight;

        QRect newGeometry;

        newGeometry = geometry();
        newGeometry.setWidth(d->mBufferSize.width());
        newGeometry.setHeight(d->mBufferSize.height());

        QPlatformWindow::setGeometry(newGeometry);
        QWindowSystemInterface::handleGeometryChange(window(), newGeometry, QRect());
    }
}
