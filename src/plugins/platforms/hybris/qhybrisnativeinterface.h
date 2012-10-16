// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISNATIVEINTERFACE_H
#define QHYBRISNATIVEINTERFACE_H

#include <qpa/qplatformnativeinterface.h>

class QHybrisNativeInterface : public QPlatformNativeInterface {
 public:
  enum ResourceType { SfClient, EglDisplay, EglContext };

  QHybrisNativeInterface();
  ~QHybrisNativeInterface();
  void* nativeResourceForContext(const QByteArray& resourceString, QOpenGLContext* context);
  void* nativeResourceForWindow(const QByteArray& resourceString, QWindow* window);
  inline const QByteArray& genericEventFilterType() const { return genericEventFilterType_; }

 private:
  const QByteArray genericEventFilterType_;
};

#endif  // QHYBRISNATIVEINTERFACE_H
