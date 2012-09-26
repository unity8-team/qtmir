// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISBACKINGSTORE_H
#define QHYBRISBACKINGSTORE_H

#include <qpa/qplatformbackingstore.h>

QT_BEGIN_NAMESPACE

class QOpenGLContext;
class QOpenGLPaintDevice;

class QHybrisBackingStore : public QPlatformBackingStore {
 public:
  QHybrisBackingStore(QWindow* window);
  ~QHybrisBackingStore();

  QPaintDevice* paintDevice();
  void beginPaint(const QRegion&);
  void endPaint();
  void flush(QWindow* window, const QRegion& region, const QPoint& offset);
  void resize(const QSize& size, const QRegion& staticContents);

 private:
  QOpenGLContext* m_context;
  QOpenGLPaintDevice* m_device;
};

QT_END_NAMESPACE

#endif  // QHYBRISBACKINGSTORE_H
