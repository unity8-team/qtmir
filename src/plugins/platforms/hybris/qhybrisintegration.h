// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISINTEGRATION_H
#define QHYBRISINTEGRATION_H

#include "qhybrisscreen.h"
#include <qpa/qplatformintegration.h>

class QAbstractEventDispatcher;
class QHybrisInput;

class QHybrisIntegration : public QObject, public QPlatformIntegration {
  Q_OBJECT

 public:
  QHybrisIntegration();
  ~QHybrisIntegration();

  bool hasCapability(QPlatformIntegration::Capability cap) const;
  QAbstractEventDispatcher* guiThreadEventDispatcher() const { return eventDispatcher_; }
  QPlatformNativeInterface* nativeInterface() const { return nativeInterface_; }
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);
  QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context);
  QPlatformFontDatabase* fontDatabase() const { return fontDb_; }

  // FIXME(loicm) Only one window can be created for now, remove that function when adding support
  //     for multiple windows.
  QPlatformWindow* platformWindow() const { return window_; }

 private slots:
  void initInput();

 private:
  QAbstractEventDispatcher* eventDispatcher_;
  QPlatformNativeInterface* nativeInterface_;
  QPlatformWindow* window_;
  QPlatformFontDatabase* fontDb_;
  QPlatformScreen* screen_;
  QPlatformOpenGLContext* context_;
  QHybrisInput* input_;
};

#endif  // QHYBRISINTEGRATION_H
