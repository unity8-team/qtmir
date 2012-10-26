// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISINTEGRATION_H
#define QHYBRISINTEGRATION_H

#include "base/integration.h"
#include "screen.h"

class QHybrisInput;

class QHybrisIntegration : public QHybrisBaseIntegration {
 public:
  QHybrisIntegration();
  ~QHybrisIntegration();

  // QPlatformIntegration methods.
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);

 private:
  QPlatformScreen* screen_;
  QHybrisInput* input_;
};

#endif  // QHYBRISINTEGRATION_H
