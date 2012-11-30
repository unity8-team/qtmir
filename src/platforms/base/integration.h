// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISBASEINTEGRATION_H
#define QHYBRISBASEINTEGRATION_H

#include <qpa/qplatformintegration.h>

class QAbstractEventDispatcher;

class QHybrisBaseIntegration : public QPlatformIntegration {
 public:
  QHybrisBaseIntegration();
  ~QHybrisBaseIntegration();

  // QPlatformIntegration methods.
  bool hasCapability(QPlatformIntegration::Capability cap) const;
  QAbstractEventDispatcher* guiThreadEventDispatcher() const { return eventDispatcher_; }
  QPlatformNativeInterface* nativeInterface() const { return nativeInterface_; }
  QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context);
  QPlatformFontDatabase* fontDatabase() const { return fontDb_; }

 private:
  QAbstractEventDispatcher* eventDispatcher_;
  QPlatformNativeInterface* nativeInterface_;
  QPlatformFontDatabase* fontDb_;
};

#endif  // QHYBRISBASEINTEGRATION_H
