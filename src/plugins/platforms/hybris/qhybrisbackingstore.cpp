// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisbackingstore.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>

QT_BEGIN_NAMESPACE

QHybrisBackingStore::QHybrisBackingStore(QWindow* window)
    : QPlatformBackingStore(window)
    , m_context(new QOpenGLContext) {
  m_context->setFormat(window->requestedFormat());
  m_context->setScreen(window->screen());
  m_context->create();
}

QHybrisBackingStore::~QHybrisBackingStore() {
  delete m_context;
}

QPaintDevice *QHybrisBackingStore::paintDevice() {
  return m_device;
}

void QHybrisBackingStore::flush(QWindow* window, const QRegion& region, const QPoint& offset) {
  Q_UNUSED(region);
  Q_UNUSED(offset);

#ifdef QHYBRIS_DEBUG
  qWarning("QEglBackingStore::flush %p", window);
#endif
  m_context->swapBuffers(window);
}

void QHybrisBackingStore::beginPaint(const QRegion& ) {
  window()->setSurfaceType(QSurface::OpenGLSurface);
  m_context->makeCurrent(window());
  m_device = new QOpenGLPaintDevice(window()->size());
}

void QHybrisBackingStore::endPaint() {
  delete m_device;
}

void QHybrisBackingStore::resize(const QSize& size, const QRegion& staticContents) {
  Q_UNUSED(size);
  Q_UNUSED(staticContents);
}

QT_END_NAMESPACE
