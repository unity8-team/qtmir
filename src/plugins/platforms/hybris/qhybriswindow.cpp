// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybriswindow.h"
#include "qhybrisscreen.h"
#include "qhybrislogging.h"
#include <qpa/qwindowsysteminterface.h>
#include <surface_flinger/surface_flinger_compatibility_layer.h>

QHybrisWindow::QHybrisWindow(QWindow* w, QHybrisScreen* screen)
    : QPlatformWindow(w)
    , screen_(screen)
    , geometry_(window()->geometry()) {
  DASSERT(screen != NULL);
  static int id = 0;
  layer_ = (INT_MAX / 2) + id;
  winId_ = ++id;

  // Create the surface.
  // FIXME(loicm) SF compat set_size() function doesn't seem to work as expected, surfaces are
  //     created fullscreen and never resized for now.
  QRect screenGeometry(screen->availableGeometry());
  SfSurfaceCreationParameters parameters = {
    screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), screenGeometry.height(), -1,
    layer_, 1.0f, false, "QHybrisWindow"
  };
  // SfSurfaceCreationParameters parameters = {
  //   geometry_.x(), geometry_.y(), geometry_.width(), geometry_.height(), -1, layer_, 1.0f, false,
  //   "QHybrisWindow"
  // };
  ASSERT((sfSurface_ = sf_surface_create(screen->sfClient(), &parameters)) != NULL);
  ASSERT((eglSurface_ = eglCreateWindowSurface(
      screen->eglDisplay(), screen->eglConfig(), sf_surface_get_egl_native_window(sfSurface_),
      NULL)) != EGL_NO_SURFACE);

  setWindowState(window()->windowState());

  DLOG("QHybrisWindow::QHybrisWindow (this=%p)", this);
}

QHybrisWindow::~QHybrisWindow() {
  DLOG("QHybrisWindow::~QHybrisWindow");
  eglDestroySurface(screen_->eglDisplay(), eglSurface_);
  // FIXME(loicm) Invalid because the struct is forward declarated, we need a way to clean the
  //     handle correctly.
  // delete sfSurface_;
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

void QHybrisWindow::setOpacity(qreal level) {
  DLOG("QHybrisWindow::setOpacity (this=%p, level=%.2f)", this, level);
  sf_client_begin_transaction(screen_->sfClient());
  sf_surface_set_alpha(sfSurface_, level);
  sf_client_end_transaction(screen_->sfClient());
}

void QHybrisWindow::raise() {
  DLOG("QHybrisWindow::raise (this=%p)", this);
  layer_ = qMax(0, qMin(layer_ + 1, INT_MAX));
  sf_client_begin_transaction(screen_->sfClient());
  sf_surface_set_layer(sfSurface_, layer_);
  sf_client_end_transaction(screen_->sfClient());
}

void QHybrisWindow::lower() {
  DLOG("QHybrisWindow::lower (this=%p)", this);
  layer_ = qMax(0, qMin(layer_ - 1, INT_MAX));
  sf_client_begin_transaction(screen_->sfClient());
  sf_surface_set_alpha(sfSurface_, layer_);
  sf_client_end_transaction(screen_->sfClient());
}

void QHybrisWindow::moveResize(const QRect& rect) {
  DLOG("QHybrisWindow::moveResize (this=%p, x=%d, y=%d, w=%d, h=%d)", this, rect.x(), rect.y(),
       rect.width(), rect.height());
  // FIXME(loicm) SF compat set_size() function doesn't seem to work as expected, surfaces are
  //     created fullscreen and never moved nor resized for now.
  // sf_client_begin_transaction(screen_->sfClient());
  // sf_surface_move_to(sfSurface_, rect.x(), rect.y());
  // sf_surface_set_size(sfSurface_, rect.width(), rect.height());
  // sf_client_end_transaction(screen_->sfClient());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}
