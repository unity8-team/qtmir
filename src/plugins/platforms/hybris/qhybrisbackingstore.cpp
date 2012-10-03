// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisbackingstore.h"
#include "qhybrislogging.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>

QHybrisBackingStore::QHybrisBackingStore(QWindow* window)
    : QPlatformBackingStore(window)
    , m_context(new QOpenGLContext) {
  m_context->setFormat(window->requestedFormat());
  m_context->setScreen(window->screen());
  m_context->create();
  DLOG("created QHybrisBackingStore (this=%p, window=%p)", this, window);
}

QHybrisBackingStore::~QHybrisBackingStore() {
  delete m_context;
  DLOG("destroyed QHybrisBackingStore");
}

QPaintDevice *QHybrisBackingStore::paintDevice() {
  return m_device;
}

void QHybrisBackingStore::flush(QWindow* window, const QRegion& region, const QPoint& offset) {
  Q_UNUSED(region);
  Q_UNUSED(offset);
  DLOG("QHybrisBackingStore::flush (this=%p, window=%p)", this, window);
  m_context->swapBuffers(window);
}

void QHybrisBackingStore::beginPaint(const QRegion&) {
  DLOG("QHybrisBackingStore::beginPaint (this=%p)", this);
  window()->setSurfaceType(QSurface::OpenGLSurface);
  m_context->makeCurrent(window());
  m_device = new QOpenGLPaintDevice(window()->size());
}

void QHybrisBackingStore::endPaint() {
  DLOG("QHybrisBackingStore::endPaint (this=%p)", this);
  delete m_device;
}

void QHybrisBackingStore::resize(const QSize& size, const QRegion& staticContents) {
  Q_UNUSED(size);
  Q_UNUSED(staticContents);
  DLOG("QHybrisBackingStore::resize (this=%p)", this);
}
