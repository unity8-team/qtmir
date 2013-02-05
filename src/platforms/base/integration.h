// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUBASEINTEGRATION_H
#define QUBUNTUBASEINTEGRATION_H

#include <qpa/qplatformintegration.h>

class QAbstractEventDispatcher;

class QUbuntuBaseIntegration : public QPlatformIntegration {
 public:
  QUbuntuBaseIntegration();
  ~QUbuntuBaseIntegration();

  // QPlatformIntegration methods.
  bool hasCapability(QPlatformIntegration::Capability cap) const;
  QAbstractEventDispatcher* guiThreadEventDispatcher() const { return eventDispatcher_; }
  QPlatformNativeInterface* nativeInterface() const { return nativeInterface_; }
  QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context);
  QPlatformFontDatabase* fontDatabase() const { return fontDb_; }
  QPlatformClipboard* clipboard() const { return clipboard_; }
  QStringList themeNames() const;
  QPlatformTheme* createPlatformTheme(const QString& name) const;

 private:
  QAbstractEventDispatcher* eventDispatcher_;
  QPlatformNativeInterface* nativeInterface_;
  QPlatformFontDatabase* fontDb_;
  QPlatformClipboard* clipboard_;
};

#endif  // QUBUNTUBASEINTEGRATION_H
