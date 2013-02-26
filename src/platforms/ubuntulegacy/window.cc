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

#include "window.h"
#include "screen.h"
#include "base/logging.h"
#include <qpa/qwindowsysteminterface.h>
#include <surface_flinger/surface_flinger_compatibility_layer.h>

QUbuntuLegacyWindow::QUbuntuLegacyWindow(QWindow* w, QUbuntuLegacyScreen* screen)
    : QUbuntuBaseWindow(w, screen)
    , geometry_(window()->geometry())
    , layer_((INT_MAX / 2) + winId()) {
  // FIXME(loicm) SF compat set_size() function doesn't seem to work as expected, surfaces are
  //     created fullscreen and never resized for now.
  QRect screenGeometry(screen->availableGeometry());
  SfSurfaceCreationParameters parameters = {
    screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), screenGeometry.height(), -1,
    layer_, 1.0f, false, "QUbuntuLegacyWindow"
  };
  // SfSurfaceCreationParameters parameters = {
  //   geometry_.x(), geometry_.y(), geometry_.width(), geometry_.height(), -1, layer_, 1.0f, false,
  //   "QUbuntuLegacyWindow"
  // };
  ASSERT((sfSurface_ = sf_surface_create(screen->sfClient(), &parameters)) != NULL);
  createSurface(sf_surface_get_egl_native_window(sfSurface_));
  setWindowState(window()->windowState());
  DLOG("QUbuntuLegacyWindow::QUbuntuLegacyWindow (this=%p, w=%p, screen=%p)", this, w, screen);
}

QUbuntuLegacyWindow::~QUbuntuLegacyWindow() {
  DLOG("QUbuntuLegacyWindow::~QUbuntuLegacyWindow");
  // FIXME(loicm) Invalid because the struct is forward declarated, we need a way to clean the
  //     handle correctly.
  // delete sfSurface_;
}

void QUbuntuLegacyWindow::setWindowState(Qt::WindowState state) {
  if (state == state_)
    return;

  switch (state) {
    case Qt::WindowNoState: {
      DLOG("QUbuntuLegacyWindow::setWindowState (this=%p, state='NoState')", this);
      moveResize(geometry_);
      state_ = Qt::WindowNoState;
      break;
    }
    case Qt::WindowFullScreen: {
      DLOG("QUbuntuLegacyWindow::setWindowState (this=%p, state='FullScreen')", this);
      QRect screenGeometry(screen()->availableGeometry());
      moveResize(screenGeometry);
      state_ = Qt::WindowFullScreen;
      break;
    }
    case Qt::WindowActive:
    case Qt::WindowMinimized:
    case Qt::WindowMaximized:
    default: {
      DLOG("QUbuntuLegacyWindow::setWindowState (this=%p, state='Active|Minimized|Maximized')", this);
      break;
    }
  }
}

void QUbuntuLegacyWindow::setGeometry(const QRect& rect) {
  DLOG("QUbuntuLegacyWindow::setGeometry (this=%p)", this);
  geometry_ = rect;
  if (state_ != Qt::WindowFullScreen)
    moveResize(rect);
}

void QUbuntuLegacyWindow::setOpacity(qreal level) {
  DLOG("QUbuntuLegacyWindow::setOpacity (this=%p, level=%.2f)", this, level);
  sf_client_begin_transaction(screen_->sfClient());
  sf_surface_set_alpha(sfSurface_, level);
  sf_client_end_transaction(screen_->sfClient());
}

void QUbuntuLegacyWindow::raise() {
  DLOG("QUbuntuLegacyWindow::raise (this=%p)", this);
  layer_ = qMax(0, qMin(layer_ + 1, INT_MAX));
  sf_client_begin_transaction(screen_->sfClient());
  sf_surface_set_layer(sfSurface_, layer_);
  sf_client_end_transaction(screen_->sfClient());
}

void QUbuntuLegacyWindow::lower() {
  DLOG("QUbuntuLegacyWindow::lower (this=%p)", this);
  layer_ = qMax(0, qMin(layer_ - 1, INT_MAX));
  sf_client_begin_transaction(screen_->sfClient());
  sf_surface_set_alpha(sfSurface_, layer_);
  sf_client_end_transaction(screen_->sfClient());
}

void QUbuntuLegacyWindow::moveResize(const QRect& rect) {
  DLOG("QUbuntuLegacyWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)", this, rect.x(),
       rect.y(), rect.width(), rect.height());
  // FIXME(loicm) SF compat set_size() function doesn't seem to work as expected, surfaces are
  //     created fullscreen and never moved nor resized for now.
  // sf_client_begin_transaction(screen_->sfClient());
  // sf_surface_move_to(sfSurface_, rect.x(), rect.y());
  // sf_surface_set_size(sfSurface_, rect.width(), rect.height());
  // sf_client_end_transaction(screen_->sfClient());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}
