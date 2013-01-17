// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUINTEGRATION_H
#define QUBUNTUINTEGRATION_H

#include "base/integration.h"
#include "screen.h"

class QUbuntuInput;

class QUbuntuIntegration : public QUbuntuBaseIntegration {
 public:
  QUbuntuIntegration();
  ~QUbuntuIntegration();

  // QPlatformIntegration methods.
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);
  QPlatformInputContext* inputContext() const { return inputContext_; }

 private:
  int argc_;
  char** argv_;
  QPlatformScreen* screen_;
  QUbuntuInput* input_;
  QPlatformInputContext* inputContext_;
};

#endif  // QUBUNTUINTEGRATION_H
