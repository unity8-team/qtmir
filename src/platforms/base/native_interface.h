// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUBASENATIVEINTERFACE_H
#define QUBUNTUBASENATIVEINTERFACE_H

#include <qpa/qplatformnativeinterface.h>

class QUbuntuBaseNativeInterface : public QPlatformNativeInterface {
 public:
  enum ResourceType { EglDisplay, EglContext };

  QUbuntuBaseNativeInterface();
  ~QUbuntuBaseNativeInterface();

  // QPlatformNativeInterface methods.
  void* nativeResourceForContext(const QByteArray& resourceString, QOpenGLContext* context);
  void* nativeResourceForWindow(const QByteArray& resourceString, QWindow* window);

  // New methods.
  const QByteArray& genericEventFilterType() const { return genericEventFilterType_; }

 private:
  const QByteArray genericEventFilterType_;
};

#endif  // QUBUNTUNATIVEINTERFACE_H
