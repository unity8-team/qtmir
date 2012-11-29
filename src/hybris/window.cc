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
    , geometry_(window()->geometry()) {
  uint surfaceRole = w->property("UbuntuSurfaceRole").toUInt();
#if !defined(QT_NO_DEBUG)
  ASSERT(surfaceRole <= ON_SCREEN_KEYBOARD_ACTOR_ROLE);
  const char* const roleString[] = {
    "Main", "Tool", "Dialog", "Dash", "Launcher", "Indicator", "Menubar", "OSK"
  };
  LOG("ubuntu surface role: '%s'", roleString[surfaceRole]);
#endif
  ubuntu_application_ui_create_surface(
      &surface_, "QHybrisWindow", geometry_.width(), geometry_.height(),
      static_cast<SurfaceRole>(surfaceRole), eventCallback, this);
  ASSERT(surface_ != NULL);
  createSurface(ubuntu_application_ui_surface_to_native_window_type(surface_));
  setWindowState(window()->windowState());

  if (geometry_.x() != 0 || geometry_.y() != 0)
    ubuntu_application_ui_move_surface_to(surface_, geometry_.x(), geometry_.y());

  DLOG("QHybrisWindow::QHybrisWindow (this=%p, w=%p, screen=%p, input=%p)", this, w, screen, input);
}

QHybrisWindow::~QHybrisWindow() {
  DLOG("QHybrisWindow::~QHybrisWindow");
  ubuntu_application_ui_destroy_surface(surface_);
}

Qt::WindowState QHybrisWindow::setWindowState(Qt::WindowState state) {
  if (state == state_)
    return state;
  switch (state) {
    case Qt::WindowNoState: {
      DLOG("QHybrisWindow::setWindowState (this=%p, state='NoState')", this);
      moveResize(geometry_);
      state_ = Qt::WindowNoState;
      return Qt::WindowNoState;
    }
    case Qt::WindowFullScreen: {
      DLOG("QHybrisWindow::setWindowState (this=%p, state='FullScreen')", this);
      QRect screenGeometry(screen()->availableGeometry());
      moveResize(screenGeometry);
      state_ = Qt::WindowFullScreen;
      return Qt::WindowFullScreen;
    }
    case Qt::WindowActive:
    case Qt::WindowMinimized:
    case Qt::WindowMaximized:
    default: {
      DLOG("QHybrisWindow::setWindowState (this=%p, state='Active|Minimized|Maximized')", this);
      return state_;
    }
  }
}

void QHybrisWindow::setGeometry(const QRect& rect) {
  DLOG("QHybrisWindow::setGeometry (this=%p)", this);
  geometry_ = rect;
  if (state_ != Qt::WindowFullScreen)
    moveResize(rect);
}

void QHybrisWindow::moveResize(const QRect& rect) {
  DLOG("QHybrisWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)", this, rect.x(), rect.y(),
       rect.width(), rect.height());
  ubuntu_application_ui_move_surface_to(surface_, rect.x(), rect.y());
  ubuntu_application_ui_resize_surface_to(surface_, rect.width(), rect.height());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}
