// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISINTEGRATION_H
#define QHYBRISINTEGRATION_H

#include "qhybrisscreen.h"
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformscreen.h>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QHybrisIntegration : public QPlatformIntegration {
 public:
  QHybrisIntegration();
  ~QHybrisIntegration();

  bool hasCapability(QPlatformIntegration::Capability cap) const;
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context) const;
  QPlatformFontDatabase* fontDatabase() const;
  QAbstractEventDispatcher* guiThreadEventDispatcher() const;
  QVariant styleHint(QPlatformIntegration::StyleHint hint) const;

private:
  QPlatformFontDatabase* mFontDb;
  QPlatformScreen* mScreen;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif  // QHYBRISINTEGRATION_H
