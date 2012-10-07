// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisbackingstore.h"
#include "qhybrislogging.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>

QHybrisBackingStore::QHybrisBackingStore(QWindow* window)
    : QPlatformBackingStore(window)
    , context_(new QOpenGLContext) {
  context_->setFormat(window->requestedFormat());
  context_->setScreen(window->screen());
  context_->create();
  DLOG("QHybrisBackingStore::QHybrisBackingStore (this=%p, window=%p)", this, window);
}

QHybrisBackingStore::~QHybrisBackingStore() {
  DLOG("QHybrisBackingStore::~QHybrisBackingStore");
  delete context_;
}

void QHybrisBackingStore::flush(QWindow* window, const QRegion& region, const QPoint& offset) {
  Q_UNUSED(region);
  Q_UNUSED(offset);
  DLOG("QHybrisBackingStore::flush (this=%p, window=%p)", this, window);
  context_->swapBuffers(window);
}

void QHybrisBackingStore::beginPaint(const QRegion& region) {
  Q_UNUSED(region);
  DLOG("QHybrisBackingStore::beginPaint (this=%p)", this);
  window()->setSurfaceType(QSurface::OpenGLSurface);
  context_->makeCurrent(window());
  device_ = new QOpenGLPaintDevice(window()->size());
}

void QHybrisBackingStore::endPaint() {
  DLOG("QHybrisBackingStore::endPaint (this=%p)", this);
  delete device_;
}

void QHybrisBackingStore::resize(const QSize& size, const QRegion& staticContents) {
  Q_UNUSED(size);
  Q_UNUSED(staticContents);
  DLOG("QHybrisBackingStore::resize (this=%p)", this);
}
