// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "window.h"
#include "screen.h"
#include "input.h"
#include "base/logging.h"
#include <qpa/qwindowsysteminterface.h>
#include <ubuntu/application/ui/ubuntu_application_ui.h>

static void eventCallback(void* context, const Event* event) {
  DLOG("eventCallback (context=%p, event=%p)", context, event);
  DASSERT(context != NULL);
  QHybrisWindow* window = static_cast<QHybrisWindow*>(context);
  window->input_->postEvent(window->window(), event);
}

QHybrisWindow::QHybrisWindow(QWindow* w, QHybrisScreen* screen, QHybrisInput* input)
    : QHybrisBaseWindow(w, screen)
    , input_(input)
    , state_(window()->windowState()) {
  // Get surface role.
  uint surfaceRole = window()->property("ubuntuSurfaceRole").toUInt();
  // Use client geometry if set explicitly, use available screen geometry otherwise.
  geometry_ = window()->geometry() != screen->geometry() ?
      window()->geometry() : screen->availableGeometry();
  createWindow();
  DLOG("QHybrisWindow::QHybrisWindow (this=%p, w=%p, screen=%p, input=%p)", this, w, screen, input);
}

QHybrisWindow::~QHybrisWindow() {
  DLOG("QHybrisWindow::~QHybrisWindow");
  ubuntu_application_ui_destroy_surface(surface_);
}

void QHybrisWindow::createWindow() {
  DLOG("QHybrisWindow::createWindow (this=%p)", this);

  // Get surface role.
  uint surfaceRole = window()->property("ubuntuSurfaceRole").toUInt();
#if !defined(QT_NO_DEBUG)
  ASSERT(surfaceRole <= ON_SCREEN_KEYBOARD_ACTOR_ROLE);
  const char* const roleString[] = {
    "Main", "Tool", "Dialog", "Dash", "Launcher", "Indicator", "Menubar", "OSK"
  };
  LOG("ubuntu surface role: '%s'", roleString[surfaceRole]);
#endif

  // Get surface geometry.
  QRect geometry;
  if (state_ == Qt::WindowFullScreen)
    geometry = screen()->geometry();
  else if (state_ == Qt::WindowMaximized)
    geometry = screen()->availableGeometry();
  else
    geometry = geometry_;

  // Create surface.
  DLOG("creating surface at (%d, %d) with size (%d, %d)", geometry.x(), geometry.y(),
       geometry.width(), geometry.height());
  ubuntu_application_ui_create_surface(
      &surface_, "QHybrisWindow", geometry.width(), geometry.height(),
      static_cast<SurfaceRole>(surfaceRole), eventCallback, this);
  if (geometry.x() != 0 || geometry.y() != 0)
    ubuntu_application_ui_move_surface_to(surface_, geometry.x(), geometry.y());
  ASSERT(surface_ != NULL);
  createSurface(ubuntu_application_ui_surface_to_native_window_type(surface_));

  // Tell Qt about the geometry.
  QWindowSystemInterface::handleGeometryChange(window(), geometry);
  QPlatformWindow::setGeometry(geometry);
}

void QHybrisWindow::moveResize(const QRect& rect) {
  DLOG("QHybrisWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)", this, rect.x(), rect.y(),
       rect.width(), rect.height());
  ubuntu_application_ui_move_surface_to(surface_, rect.x(), rect.y());
  ubuntu_application_ui_resize_surface_to(surface_, rect.width(), rect.height());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}

Qt::WindowState QHybrisWindow::setWindowState(Qt::WindowState state) {
  DLOG("QHybrisWindow::setWindowState (this=%p, state=%d)", this, state);
  if (state == state_)
    return state;
  switch (state) {
    case Qt::WindowNoState: {
      DLOG("QHybrisWindow::setState (this=%p, state='NoState')", this);
      moveResize(geometry_);
      state_ = Qt::WindowNoState;
      return Qt::WindowNoState;
    }
    case Qt::WindowFullScreen: {
      DLOG("QHybrisWindow::setState (this=%p, state='FullScreen')", this);
      moveResize(screen()->geometry());
      state_ = Qt::WindowFullScreen;
      return Qt::WindowFullScreen;
    }
    case Qt::WindowMaximized: {
      DLOG("QHybrisWindow::setState (this=%p, state='Maximized')", this);
      moveResize(screen()->availableGeometry());
      state_ = Qt::WindowMaximized;
      return Qt::WindowMaximized;
    }
    case Qt::WindowActive:
    case Qt::WindowMinimized:
    default: {
      DLOG("QHybrisWindow::setState (this=%p, state='Active|Minimized')", this);
      return state_;
    }
  }
}

void QHybrisWindow::setGeometry(const QRect& rect) {
  DLOG("QHybrisWindow::setGeometry (this=%p)", this);
  geometry_ = rect;
  if (state_ != Qt::WindowFullScreen && state_ != Qt::WindowMaximized)
    moveResize(rect);
}

void QHybrisWindow::setVisible(bool visible) {
  DLOG("QHybrisWindow::setVisible (this=%p, visible=%s)", this, visible ? "true" : "false");
  if (visible) {
    ubuntu_application_ui_show_surface(surface_);
    QWindowSystemInterface::handleSynchronousExposeEvent(window(), QRect());
  } else {
    ubuntu_application_ui_hide_surface(surface_);
  }
}
