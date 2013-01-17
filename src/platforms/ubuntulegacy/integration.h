// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTULEGACYINTEGRATION_H
#define QUBUNTULEGACYINTEGRATION_H

#include "base/integration.h"
#include "screen.h"

class QUbuntuLegacyInput;

class QUbuntuLegacyIntegration : public QObject, public QUbuntuBaseIntegration {
  Q_OBJECT

 public:
  QUbuntuLegacyIntegration();
  ~QUbuntuLegacyIntegration();

  // QPlatformIntegration methods.
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);
  QPlatformInputContext* inputContext() const { return inputContext_; }

  // New methods.
  // FIXME(loicm) Only one window can be created for now, remove that function when adding support
  //     for multiple windows.
  QPlatformWindow* platformWindow() const { return window_; }

 private slots:
  void initInput();

 private:
  QPlatformWindow* window_;
  QPlatformScreen* screen_;
  QUbuntuLegacyInput* input_;
  QPlatformInputContext* inputContext_;
};

#endif  // QUBUNTULEGACYINTEGRATION_H
