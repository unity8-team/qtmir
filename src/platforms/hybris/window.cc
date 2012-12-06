// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "window.h"
#include "screen.h"
#include "input.h"
#include "base/logging.h"
#include <qpa/qwindowsysteminterface.h>
#include <ubuntu/application/ui/ubuntu_application_ui.h>

// FIXME(loicm) There is no way to get the strut from a system session. Values are hard-coded for
//     the phone right now.
static const struct {int left; int right; int top; int bottom; } kStrut = { 0, 0, 54, 0 };

static void eventCallback(void* context, const Event* event) {
  DLOG("eventCallback (context=%p, event=%p)", context, event);
  DASSERT(context != NULL);
  QHybrisWindow* window = static_cast<QHybrisWindow*>(context);
  window->input_->postEvent(window->window(), event);
}

QHybrisWindow::QHybrisWindow(QWindow* w, QHybrisScreen* screen, QHybrisInput* input)
    : QHybrisBaseWindow(w, screen)
    , input_(input)
    , geometry_(maximizedGeometry()) {
  uint surfaceRole = w->property("ubuntuSurfaceRole").toUInt();
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
  setState(window()->windowState());
  DLOG("QHybrisWindow::QHybrisWindow (this=%p, w=%p, screen=%p, input=%p)", this, w, screen, input);
}

QHybrisWindow::~QHybrisWindow() {
  DLOG("QHybrisWindow::~QHybrisWindow");
  ubuntu_application_ui_destroy_surface(surface_);
}

Qt::WindowState QHybrisWindow::setState(Qt::WindowState state) {
  switch (state) {
    case Qt::WindowNoState: {
      DLOG("QHybrisWindow::setState (this=%p, state='NoState')", this);
      moveResize(geometry_);
      state_ = Qt::WindowNoState;
      return Qt::WindowNoState;
    }
    case Qt::WindowFullScreen: {
      DLOG("QHybrisWindow::setState (this=%p, state='FullScreen')", this);
      QRect screenGeometry(screen()->availableGeometry());
      moveResize(screenGeometry);
      state_ = Qt::WindowFullScreen;
      return Qt::WindowFullScreen;
    }
    case Qt::WindowMaximized: {
      DLOG("QHybrisWindow::setState (this=%p, state='Maximized')", this);
      moveResize(maximizedGeometry());
      state_ = Qt::WindowMaximized;
      return Qt::WindowMaximized;
    }
    case Qt::WindowActive:
    case Qt::WindowMinimized:
    default: {
      DLOG("QHybrisWindow::setState (this=%p, state='Active|Minimized')", this);
      DLOG("unsupported state");
      return state_;
    }
  }
}

Qt::WindowState QHybrisWindow::setWindowState(Qt::WindowState state) {
  DLOG("QHybrisWindow::setWindowState (this=%p, state=%d)", this, state);
  if (state == state_)
    return state;
  return setState(state);
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
    QWindowSystemInterface::handleSynchronousExposeEvent(window(), geometry_);
  } else {
    ubuntu_application_ui_hide_surface(surface_);
  }
}

void QHybrisWindow::moveResize(const QRect& rect) {
  DLOG("QHybrisWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)", this, rect.x(), rect.y(),
       rect.width(), rect.height());
  ubuntu_application_ui_move_surface_to(surface_, rect.x(), rect.y());
  ubuntu_application_ui_resize_surface_to(surface_, rect.width(), rect.height());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}

QRect QHybrisWindow::maximizedGeometry() {
  DLOG("QHybrisWindow::maximizedGeometry (this=%p)", this);
  const QRect kScreenGeometry(screen()->availableGeometry());
  return QRect(kScreenGeometry.x() + kStrut.left, kScreenGeometry.y() + kStrut.top,
               kScreenGeometry.width() - kStrut.left - kStrut.right,
               kScreenGeometry.height() - kStrut.top - kStrut.bottom);
}
