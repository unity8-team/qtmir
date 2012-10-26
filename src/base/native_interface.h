// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISBASENATIVEINTERFACE_H
#define QHYBRISBASENATIVEINTERFACE_H

#include <qpa/qplatformnativeinterface.h>

class QHybrisBaseNativeInterface : public QPlatformNativeInterface {
 public:
  enum ResourceType { EglDisplay, EglContext };

  QHybrisBaseNativeInterface();
  ~QHybrisBaseNativeInterface();

  // QPlatformNativeInterface methods.
  void* nativeResourceForContext(const QByteArray& resourceString, QOpenGLContext* context);
  void* nativeResourceForWindow(const QByteArray& resourceString, QWindow* window);

  // New methods.
  const QByteArray& genericEventFilterType() const { return genericEventFilterType_; }

 private:
  const QByteArray genericEventFilterType_;
};

#endif  // QHYBRISNATIVEINTERFACE_H
