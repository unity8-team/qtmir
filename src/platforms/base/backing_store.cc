// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "backing_store.h"
#include "logging.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>

QUbuntuBaseBackingStore::QUbuntuBaseBackingStore(QWindow* window)
    : QPlatformBackingStore(window)
    , context_(new QOpenGLContext) {
  context_->setFormat(window->requestedFormat());
  context_->setScreen(window->screen());
  context_->create();
  DLOG("QUbuntuBaseBackingStore::QUbuntuBaseBackingStore (this=%p, window=%p)", this, window);
}

QUbuntuBaseBackingStore::~QUbuntuBaseBackingStore() {
  DLOG("QUbuntuBaseBackingStore::~QUbuntuBaseBackingStore");
  delete context_;
}

void QUbuntuBaseBackingStore::flush(QWindow* window, const QRegion& region, const QPoint& offset) {
  Q_UNUSED(region);
  Q_UNUSED(offset);
  DLOG("QUbuntuBaseBackingStore::flush (this=%p, window=%p)", this, window);
  context_->swapBuffers(window);
}

void QUbuntuBaseBackingStore::beginPaint(const QRegion& region) {
  Q_UNUSED(region);
  DLOG("QUbuntuBaseBackingStore::beginPaint (this=%p)", this);
  window()->setSurfaceType(QSurface::OpenGLSurface);
  context_->makeCurrent(window());
  device_ = new QOpenGLPaintDevice(window()->size());
}

void QUbuntuBaseBackingStore::endPaint() {
  DLOG("QUbuntuBaseBackingStore::endPaint (this=%p)", this);
  delete device_;
}

void QUbuntuBaseBackingStore::resize(const QSize& size, const QRegion& staticContents) {
  Q_UNUSED(size);
  Q_UNUSED(staticContents);
  DLOG("QUbuntuBaseBackingStore::resize (this=%p)", this);
}
