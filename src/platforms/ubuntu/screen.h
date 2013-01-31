// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUSCREEN_H
#define QUBUNTUSCREEN_H

#include "base/screen.h"

class QUbuntuScreen : public QUbuntuBaseScreen {
 public:
  QUbuntuScreen();
  ~QUbuntuScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }
  QRect availableGeometry() const { return availableGeometry_; }

 private:
  QRect geometry_;
  QRect availableGeometry_;
};

#endif  // QUBUNTUSCREEN_H
