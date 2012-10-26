// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "backing_store.h"
#include "logging.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>

QHybrisBaseBackingStore::QHybrisBaseBackingStore(QWindow* window)
    : QPlatformBackingStore(window)
    , context_(new QOpenGLContext) {
  context_->setFormat(window->requestedFormat());
  context_->setScreen(window->screen());
  context_->create();
  DLOG("QHybrisBaseBackingStore::QHybrisBaseBackingStore (this=%p, window=%p)", this, window);
}

QHybrisBaseBackingStore::~QHybrisBaseBackingStore() {
  DLOG("QHybrisBaseBackingStore::~QHybrisBaseBackingStore");
  delete context_;
}

void QHybrisBaseBackingStore::flush(QWindow* window, const QRegion& region, const QPoint& offset) {
  Q_UNUSED(region);
  Q_UNUSED(offset);
  DLOG("QHybrisBaseBackingStore::flush (this=%p, window=%p)", this, window);
  context_->swapBuffers(window);
}

void QHybrisBaseBackingStore::beginPaint(const QRegion& region) {
  Q_UNUSED(region);
  DLOG("QHybrisBaseBackingStore::beginPaint (this=%p)", this);
  window()->setSurfaceType(QSurface::OpenGLSurface);
  context_->makeCurrent(window());
  device_ = new QOpenGLPaintDevice(window()->size());
}

void QHybrisBaseBackingStore::endPaint() {
  DLOG("QHybrisBaseBackingStore::endPaint (this=%p)", this);
  delete device_;
}

void QHybrisBaseBackingStore::resize(const QSize& size, const QRegion& staticContents) {
  Q_UNUSED(size);
  Q_UNUSED(staticContents);
  DLOG("QHybrisBaseBackingStore::resize (this=%p)", this);
}
