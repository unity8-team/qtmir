// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
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
#include <ubuntu/application/ui/window.h>

static void eventCallback(void* context, const Event* event) {
  DLOG("eventCallback (context=%p, event=%p)", context, event);
  DASSERT(context != NULL);
  QUbuntuWindow* window = static_cast<QUbuntuWindow*>(context);
  window->input_->postEvent(window->window(), event);
}

QUbuntuWindow::QUbuntuWindow(
    QWindow* w, QUbuntuScreen* screen, QUbuntuInput* input,
    bool systemSession, UApplicationInstance* instance, bool isShell)
    : QUbuntuBaseWindow(w, screen)
    , input_(input)
    , state_(window()->windowState())
    , systemSession_(systemSession)
    , uainstance_(instance)
    , screen_(screen)
    , isShell_(isShell) {
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
  destroyEGLSurface();
  ua_ui_window_destroy(window_);
}

void QUbuntuWindow::createWindow() {
  DLOG("QUbuntuWindow::createWindow (this=%p)", this);

  // Get surface role and flags.
  QVariant roleVariant = window()->property("role");
  int role = roleVariant.isValid() ? roleVariant.toUInt() : 1;  // 1 is the default role for apps.
  QVariant opaqueVariant = window()->property("opaque");
  uint flags = opaqueVariant.isValid() ?
      opaqueVariant.toUInt() ? static_cast<uint>(IS_OPAQUE_FLAG) : 0 : 0;
  if (!systemSession_) {
    // FIXME(loicm) Opaque flag is forced for now for non-system sessions (applications) for
    //     performance reasons.
    flags |= static_cast<uint>(IS_OPAQUE_FLAG);
  }

  const QByteArray title = (!window()->title().isNull()) ? window()->title().toUtf8() : "Window 1"; // legacy title

#if !defined(QT_NO_DEBUG)
  //ASSERT(role <= ON_SCREEN_KEYBOARD_ACTOR_ROLE);
  const char* const roleString[] = {
    "Dash", "Default", "Indicator", "Notifications", "Greeter", "Launcher", "OSK", "ShutdownDialog"
  };
  LOG("role: '%s'", roleString[role]);
  LOG("flags: '%s'", (flags & static_cast<uint>(1)) ? "Opaque" : "NotOpaque");
  LOG("title: '%s'", title.constData());
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

  fprintf(stderr, "creating surface at (%d, %d) with size (%d, %d) with title '%s'", geometry.x(), geometry.y(),
          geometry.width(), geometry.height(), title.data());

  // Setup platform window creation properties
  wprops_ = ua_ui_window_properties_new_for_normal_window();
  ua_ui_window_properties_set_titlen(wprops_, title.data(), title.size());
  ua_ui_window_properties_set_role(wprops_, static_cast<UAUiWindowRole>(role));
  ua_ui_window_properties_set_input_cb_and_ctx(wprops_, &eventCallback, this);

  // Create platform window
  window_ = ua_ui_window_new_for_application_with_properties(uainstance_, wprops_);

  if (geometry.width() != 0 || geometry.height() != 0)
      ua_ui_window_resize(window_, geometry.width(), geometry.height());

  if (geometry.x() != 0 || geometry.y() != 0)
      ua_ui_window_move(window_, geometry.x(), geometry.y());

  ASSERT(window_ != NULL);
  createEGLSurface(ua_ui_window_get_native_type(window_));
  if (state_ == Qt::WindowFullScreen) {
    ua_ui_window_request_fullscreen(window_);
  }

  // Tell Qt about the geometry.
  QWindowSystemInterface::handleGeometryChange(window(), geometry);
  QPlatformWindow::setGeometry(geometry);
}

void QUbuntuWindow::moveResize(const QRect& rect) {
  fprintf(stderr, "\nQUbuntuWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)\n", this, rect.x(), rect.y(),
       rect.width(), rect.height());
  ua_ui_window_move(window_, rect.x(), rect.y());
  ua_ui_window_resize(window_, rect.width(), rect.height());
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
      ua_ui_window_request_fullscreen(window_);
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
  fprintf(stderr, "QUbuntuWindow::setGeometry (this=%p)", this);
  if (systemSession_) {
    // Non-system sessions can't resize the window geometry.
    geometry_ = rect;
    if (state_ != Qt::WindowFullScreen && state_ != Qt::WindowMaximized)
      moveResize(rect);
  }
}

void QUbuntuWindow::setVisible(bool visible) {
  DLOG("QUbuntuWindow::setVisible (this=%p, visible=%s)", this, visible ? "true" : "false");
  if (isShell_ == false)
      screen_->toggleSensors(visible);

  if (visible) {
    ua_ui_window_show(window_);
    QWindowSystemInterface::handleExposeEvent(window(), QRect());
  } else {
    ua_ui_window_hide(window_);
  }
}
