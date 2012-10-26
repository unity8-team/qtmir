// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISLEGACYINTEGRATION_H
#define QHYBRISLEGACYINTEGRATION_H

#include "base/integration.h"
#include "screen.h"

class QHybrisLegacyInput;

class QHybrisLegacyIntegration : public QObject, public QHybrisBaseIntegration {
  Q_OBJECT

 public:
  QHybrisLegacyIntegration();
  ~QHybrisLegacyIntegration();

  // QPlatformIntegration methods.
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);

  // New methods.
  // FIXME(loicm) Only one window can be created for now, remove that function when adding support
  //     for multiple windows.
  QPlatformWindow* platformWindow() const { return window_; }

 private slots:
  void initInput();

 private:
  QPlatformWindow* window_;
  QPlatformScreen* screen_;
  QHybrisLegacyInput* input_;
};

#endif  // QHYBRISLEGACYINTEGRATION_H
