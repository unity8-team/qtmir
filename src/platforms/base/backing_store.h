// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUBASEBACKINGSTORE_H
#define QUBUNTUBASEBACKINGSTORE_H

#include <qpa/qplatformbackingstore.h>

class QOpenGLContext;
class QOpenGLPaintDevice;

class QUbuntuBaseBackingStore : public QPlatformBackingStore {
 public:
  QUbuntuBaseBackingStore(QWindow* window);
  ~QUbuntuBaseBackingStore();

  // QPlatformBackingStore methods.
  void beginPaint(const QRegion&);
  void endPaint();
  void flush(QWindow* window, const QRegion& region, const QPoint& offset);
  void resize(const QSize& size, const QRegion& staticContents);
  QPaintDevice* paintDevice() { return reinterpret_cast<QPaintDevice*>(device_); }

 private:
  QOpenGLContext* context_;
  QOpenGLPaintDevice* device_;
};

#endif  // QUBUNTUBASEBACKINGSTORE_H
