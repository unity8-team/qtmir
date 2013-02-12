// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// FIXME(loicm) The fullscreen API from Ubuntu Platform isn't good enough as we can't leave
//     fullscreen. The current Ubuntu Platform fullscreen call allows the application manager to
//     know the fullscreen state of an application, it's still the application responsibility to set
//     the right surface geometry.

#include "window.h"
#include "screen.h"
#include "input.h"
#include "base/logging.h"
#include <qpa/qwindowsysteminterface.h>
#include <ubuntu/application/ui/ubuntu_application_ui.h>

static void eventCallback(void* context, const Event* event) {
  DLOG("eventCallback (context=%p, event=%p)", context, event);
  DASSERT(context != NULL);
  QUbuntuWindow* window = static_cast<QUbuntuWindow*>(context);
  window->input_->postEvent(window->window(), event);
}

QUbuntuWindow::QUbuntuWindow(
    QWindow* w, QUbuntuScreen* screen, QUbuntuInput* input, bool systemSession)
    : QUbuntuBaseWindow(w, screen)
    , input_(input)
    , state_(window()->windowState())
    , systemSession_(systemSession) {
  if (!systemSession) {
    // Non-system sessions can't resize the window geometry.
    geometry_ = screen->availableGeometry();
  } else {
    // Use client geometry if set explicitly, use available screen geometry otherwise.
    geometry_ = window()->geometry() != screen->geometry() ?
        window()->geometry() : screen->availableGeometry();
  }
  createWindow();
  DLOG("QUbuntuWindow::QUbuntuWindow (this=%p, w=%p, screen=%p, input=%p)", this, w, screen, input);
}

QUbuntuWindow::~QUbuntuWindow() {
  DLOG("QUbuntuWindow::~QUbuntuWindow");
  ubuntu_application_ui_destroy_surface(surface_);
}

void QUbuntuWindow::createWindow() {
  DLOG("QUbuntuWindow::createWindow (this=%p)", this);

  // Get surface role and flags.
  QVariant roleVariant = window()->property("role");
  int role = roleVariant.isValid() ? roleVariant.toUInt() : 1;  // 1 is the default role for apps.
  QVariant opaqueVariant = window()->property("opaque");
  uint flags = opaqueVariant.isValid() ?
      opaqueVariant.toUInt() ? static_cast<uint>(IS_OPAQUE_FLAG) : 0 : 0;
#if !defined(QT_NO_DEBUG)
  ASSERT(role <= ON_SCREEN_KEYBOARD_ACTOR_ROLE);
  const char* const roleString[] = {
    "Dash", "Default", "Indicator", "Notifications", "Greeter", "Launcher", "OSK", "ShutdownDialog"
  };
  LOG("role: '%s'", roleString[role]);
  LOG("flags: '%s'", (flags & static_cast<uint>(IS_OPAQUE_FLAG)) ? "Opaque" : "NotOpaque");
#endif

  // Get surface geometry.
  QRect geometry;
  if (state_ == Qt::WindowFullScreen) {
    geometry = screen()->geometry();
  } else if (state_ == Qt::WindowMaximized) {
    geometry = screen()->availableGeometry();
  } else {
    geometry = geometry_;
  }

  // Create surface.
  DLOG("creating surface at (%d, %d) with size (%d, %d)", geometry.x(), geometry.y(),
       geometry.width(), geometry.height());
  ubuntu_application_ui_create_surface(
      &surface_, "QUbuntuWindow", geometry.width(), geometry.height(),
      static_cast<SurfaceRole>(role), flags, eventCallback, this);
  if (geometry.x() != 0 || geometry.y() != 0)
    ubuntu_application_ui_move_surface_to(surface_, geometry.x(), geometry.y());
  ASSERT(surface_ != NULL);
  createSurface(ubuntu_application_ui_surface_to_native_window_type(surface_));
  if (state_ == Qt::WindowFullScreen) {
    ubuntu_application_ui_request_fullscreen_for_surface(surface_);
  }

  // Tell Qt about the geometry.
  QWindowSystemInterface::handleGeometryChange(window(), geometry);
  QPlatformWindow::setGeometry(geometry);
}

void QUbuntuWindow::moveResize(const QRect& rect) {
  DLOG("QUbuntuWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)", this, rect.x(), rect.y(),
       rect.width(), rect.height());
  ubuntu_application_ui_move_surface_to(surface_, rect.x(), rect.y());
  ubuntu_application_ui_resize_surface_to(surface_, rect.width(), rect.height());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}

void QUbuntuWindow::setWindowState(Qt::WindowState state) {
  DLOG("QUbuntuWindow::setWindowState (this=%p, state=%d)", this, state);
  if (state == state_)
    return;

  switch (state) {
    case Qt::WindowNoState: {
      DLOG("setting window state: 'NoState'");
      moveResize(geometry_);
      state_ = Qt::WindowNoState;
      break;
    }
    case Qt::WindowFullScreen: {
      DLOG("setting window state: 'FullScreen'");
      ubuntu_application_ui_request_fullscreen_for_surface(surface_);
      moveResize(screen()->geometry());
      state_ = Qt::WindowFullScreen;
      break;
    }
    case Qt::WindowMaximized: {
      DLOG("setting window state: 'Maximized'");
      moveResize(screen()->availableGeometry());
      state_ = Qt::WindowMaximized;
      break;
    }
    case Qt::WindowActive:
    case Qt::WindowMinimized:
    default: {
      DLOG("setting window state: 'Active|Minimized'");
      break;
    }
  }
}

void QUbuntuWindow::setGeometry(const QRect& rect) {
  DLOG("QUbuntuWindow::setGeometry (this=%p)", this);
  if (systemSession_) {
    // Non-system sessions can't resize the window geometry.
    geometry_ = rect;
    if (state_ != Qt::WindowFullScreen && state_ != Qt::WindowMaximized)
      moveResize(rect);
  }
}

void QUbuntuWindow::setVisible(bool visible) {
  DLOG("QUbuntuWindow::setVisible (this=%p, visible=%s)", this, visible ? "true" : "false");
  if (visible) {
    ubuntu_application_ui_show_surface(surface_);
    QWindowSystemInterface::handleExposeEvent(window(), QRect());
  } else {
    ubuntu_application_ui_hide_surface(surface_);
  }
}
